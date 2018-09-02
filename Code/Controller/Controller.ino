// Controller.ino
//
// Entry point for Dual AVCS controller.
//
// Requires:
//   Arduino Due
//   Sainsmart 1602 LCD/Keypad shield
//
// Recommended:
//   Arduino Due or Mega prototyping shield
//
// Arduino Pin Configuration:
// 
// Digital pins 4-9 are for LCD
// Digital/PWM pin 10 is LCD backlight
//
// Pin D3 = TIOA7 = left cam 
// Pin D11 = TIOA8 = right cam 
// Pin D2 = TIOA0 = crank
//
// Cam timers are triggered on rising and falling edges of cam signal
// Crank time is read 'manually' when interrupt is processed - would
// rather do a hardware capture but haven't got that figured out yet.
// 
// Analog input 0 is used for keypad keys
// Analog input 1 is used for crank sensor analog (which is only useful for diagnostics)
// Analog 8 = MAP sensor
// Analog 9 = input knob 1
// Analog 10 = input knob 2
// Analog 10-15 available for whatever else.
// 
// Digital pin 36 = PWM out for left cam
// Digital pin 38 = PWM out for right cam
//
// TX3 / RX3 = serial from PLX oil temperature, to PLX oil pressure
//
// Core AVCS functionality:
// When the signal from the crank trigger goes high, start timers.
// When the callbacks for the timers detect the sensor pulse, note the time since crank trigger.
// Elapsed time corresponds to cam phase angle.
// Use PID feedback to set AVCS solenoid PWM, to drive cam angle toward desired angle.

#include "SelfTest.h"
#include "Mode.h"
#include "ScreenNavigator.h"
#include "LiquidCrystal.h"
#include "DFR_Key.h"
#include "RollingAverage.h"
#include "ExhaustCamState.h"
#include "CrankState.h"
#include "InterruptHandlers.h"
#include "Globals.h"
#include "Screen.h"
#include "Utilities.h"
#include "PlxProcessor.h"
#include "Feedback.h"
#include "PeriodicJobs.h"
#include "IntervalRecorder.h"
#include "Terminal.h"
#include "Configuration.h"
#include "CurveTable.h"

//#include <..\Pwm_Lib\pwm_lib.h>
#include "pwm_lib\pwm_lib.h"
//#include "pwm_lib.h"

ScreenNavigator navigator;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 
DFR_Key keys;
Mode mode;
PlxProcessor plx;
InterruptHandlers interruptHandlers;
unsigned iterationCounter;
IPeriodicJobs *jobs = IPeriodicJobs::GetInstance();
IIntervalRecorder *intervalRecorder = IIntervalRecorder::GetInstance();
ITerminal *terminal = ITerminal::GetInstance();
CurveTable *table = CurveTable::CreateExhaustCamTable();

// Do not change these at run-time!
//
// By default, the controller will discover the baseline cam
// angle at run-time, by measuring it shortly after startup,
// while the the solenoids are disabled. This kinda works, but
// you really need to hold a constant RPM for a few seconds to
// get a good baseline. (Idle RPM jumps around too much to be
// useful for this measurement.) Sometimes it'll be off by a
// couple degrees or so.
//
// Set onlyMeasureBaseline to force the controller to measure
// the cam baseline angle continuously, never attempting to
// control the cam angle. This is useful to determine the 
// static baseline angles to use.
//
int onlyMeasureBaseline = 0;
//
// Set the useStaticBaseline flag to skip the measurement step
// and use hard-coded baseline values instead of discovered
// baseline values. The values are in ExhaustCamState.cpp.
//
int useStaticBaseline = 1;

// Exhaust cam solenoid drivers
using namespace arduino_due::pwm_lib;
pwm<pwm_pin::PWML1_PC4> RightSolenoid; // pin 36, blue, passenger side
pwm<pwm_pin::PWML2_PC6> LeftSolenoid; // pin 38, yellow, driver side

// 300hz = 3.33ms
// = 3330.0 microseconds
// Period is defined in hundredths of a microsecond
#define PWM_PERIOD 333 * 1000

float DebugSolenoidDuty;

///////////////////////////////////////////////////////////////////////////////
// The setup function runs once when you press reset or power the board.
///////////////////////////////////////////////////////////////////////////////
void setup() {
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	lcd.print("1234567890213456");
	lcd.setCursor(0, 1);
	lcd.print("1234567890213456");
	delay(100);
	lcd.clear();
	
	SelfTest();

	DisplayLine1[DisplayWidth] = 0;
	DisplayLine2[DisplayWidth] = 0;

	pinMode(A0, INPUT); // keypad
	pinMode(A1, INPUT); // crank angle sensor
	analogWrite(10, 60); // backlight
	
	pinMode(2, INPUT_PULLUP); // crank
	pinMode(3, INPUT_PULLUP); // left cam
	pinMode(11, INPUT_PULLUP); // right cam
	pinMode(13, OUTPUT); // onboard LED

	// This is the power supply for the TCRT5000 crank sensor
	// It draws 1ma, the output can source 15ma, so no worries.
	pinMode(53, OUTPUT);
	digitalWrite(53, HIGH);

	// Testing
	pinMode(22, OUTPUT);
	
	// pinMode(A8, INPUT); // MAP sensor
	// pinMode(A9, INPUT); // Knob?

	navigator.Initialize(&mode);
	plx.Initialize(&Serial3, &Serial2);	
	interruptHandlers.Initialize();
	mode.Initialize();
	jobs->Initialize();
	intervalRecorder->Initialize();
	terminal->Initialize();

	LeftCamError = -10;
	RightCamError = 10;

	LeftSolenoid.start(PWM_PERIOD, 0);
	RightSolenoid.start(PWM_PERIOD, 0);

	Serial.begin(115200);
}

///////////////////////////////////////////////////////////////////////////////
// This is invoked repeatedly by the Arduino core library.
///////////////////////////////////////////////////////////////////////////////
void loop()
{
	iterationCounter++;

	int key = keys.getKey();
	if (navigator.Update(key))
	{
		ClearScreen();
	}
	else
	{
		ClearScreenBuffer();
	}

	navigator.GetCurrentScreen()->Update();

	jobs->Update();
	mode.Update();
	plx.Update();
	terminal->Update();
	
	CamTargetAngle = table->GetValue(Crank.Rpm);

	// RPM jumps around a lot at idle, so rather than chasing noisy 
	// data I am just letting the cams rest. At least for now.
	// Might be fun to try creating overlap at idle, just to see if 
	// it starts to sound like an old-school muscle car...
	if ((mode.GetMode() == Mode::Running) && (Crank.Rpm > MINIMUM_EXAVCS_RPM) && !onlyMeasureBaseline)
	{
		LeftFeedback.Update(micros(), Crank.Rpm, LeftExhaustCam.Angle, CamTargetAngle);
		RightFeedback.Update(micros(), Crank.Rpm, RightExhaustCam.Angle, CamTargetAngle);

		float baseDuty = 44.0f;

		float ratio = (baseDuty + LeftFeedback.Output) / 100.0f;
		float duty_float = PWM_PERIOD * ratio;
		LeftSolenoid.set_duty((uint32_t)duty_float);

		ratio = (baseDuty + RightFeedback.Output) / 100.0f;
		duty_float = PWM_PERIOD * ratio;
		RightSolenoid.set_duty((uint32_t)duty_float);

		DebugSolenoidDuty = RightFeedback.Output + baseDuty;
	}
	else
	{
		LeftFeedback.Reset(0);
		RightFeedback.Reset(1);

		LeftSolenoid.set_duty(0);
		RightSolenoid.set_duty(0);
	}

	LeftExhaustCam.PinState = (unsigned)digitalRead(3);
	RightExhaustCam.PinState = (unsigned)digitalRead(11);
	Crank.PinState = (unsigned)digitalRead(2);
	Crank.AnalogValue = (unsigned)analogRead(A1);

	LeftExhaustCam.Process();
	RightExhaustCam.Process();
	Crank.Process();
}
