// Timers.cpp
// 
// Implements three timers, and interactions between them.
//
// Cam timers (left and right) are started three times per camshaft revolution. 
// There are two short periods, and one long period, per revolution. 
// A running average is used to distinguish short periods from long periods.
//
// Crank timer is started once per camshaft revolution, upon interrupt from sensor on a timing-belt pulley.
// Elapsed time between crank sensor signal and camshaft long-pulse signal is used to calculate cam phase angle.

#include "TrivialTimer.h"
#include "InterruptHandlers.h"
#include "Globals.h"
#include "Mode.h"
#include "ExhaustCamState.h"
#include "CrankState.h"

//#define UseCaptureTimers

// pin D3, fourth pin on 1602 top-right header, yellow wire, driver side
int LeftCamPin = 3;

// pin D11, third pin on 1602 top-right header, blue wire, passenger side
int RightCamPin = 11;

// pin D2, fifth pin on 1602 top-right header
int CrankPin = 2;

// This pin will be high while the de-noising pin-read code is active.
// This can be used with a scope to determine if the pin read process
// is covering a large enough span of time to filter out noise.
int DiagnosticTimingPin = 22;
int DiagnosticOutputPin = 23;

#ifdef UseCaptureTimers
#include "CaptureTimer.h"

CaptureTimer LeftCamTimer = Timer0;
CaptureTimer RightCamTimer = Timer1;
CaptureTimer CrankTimer = Timer2;

const unsigned TicksPerSecond = 42 * 1000 * 1000;
#else
TrivialTimer LeftCamTimer;
TrivialTimer RightCamTimer;
TrivialTimer CrankTimer;

const unsigned TicksPerSecond = 1000 * 1000;
#endif

const unsigned TicksPerMinute = TicksPerSecond * 60;
const int timeout = TicksPerSecond;

enum PinState
{
	None,
	Low,
	High
};

PinState LeftCamPinState = PinState::None;
PinState RightCamPinState = PinState::None;
PinState CrankPinState = PinState::None;

int PinOfInterest = RightCamPin;

PinState GetPinState(int pin)
{
	if (pin == PinOfInterest)
	{
		digitalWrite(DiagnosticTimingPin, HIGH);
	}

	int total = 0;
	int samples = 50;
	int tolerance = samples / 4;
	for (int i = 0; i < samples; i++)
	{
		if (digitalRead(pin) == HIGH)
		{
			total++;
		}

//		delayMicroseconds(1);
	}

	if (pin == PinOfInterest)
	{
		digitalWrite(DiagnosticTimingPin, LOW);
	}

	if (total > (samples - tolerance))
	{
		if (pin == PinOfInterest)
		{
			digitalWrite(DiagnosticOutputPin, HIGH);
		}

		return PinState::High;
	}

	if (total < (tolerance))
	{
		if (pin == PinOfInterest)
		{
			digitalWrite(DiagnosticOutputPin, LOW);
		}

		return PinState::Low;
	}

	return PinState::None;
}

void StartLeftCamTimer()
{
	LeftCamTimer.start();
}

void StartRightCamTimer()
{
	RightCamTimer.start();
}
	
void StartCrankTimer()
{
	CrankTimer.start();
}

void LeftCamTimeout(unsigned status)
{
	LeftExhaustCam.Timeout++;
	StartLeftCamTimer();
	mode.Fail("Left Cam Timeout");

}

void RightCamTimeout(unsigned status)
{
	RightExhaustCam.Timeout++;
	StartRightCamTimer();
	mode.Fail("Rght Cam Timeout");
}

void CrankTimeout(unsigned status)
{
	Crank.Timeout++;
	StartCrankTimer();
	mode.Fail("Crank Timeout");
}

void LeftCamSignalChange()
{
	unsigned camInterval = LeftCamTimer.getElapsed();
	unsigned crankInterval = CrankTimer.getElapsed();

	PinState pinState = GetPinState(LeftCamPin);

	// Ignore noise
	if ((pinState == LeftCamPinState) || (pinState == PinState::None))
	{
		return;
	}

	LeftCamPinState = pinState;

	//if (digitalRead(LeftCamPin) == LOW))
	if (LeftCamPinState == PinState::Low)
	{
		DebugLeft = camInterval;
		LeftExhaustCam.BeginPulse(camInterval, crankInterval);
		StartLeftCamTimer();
	}
	else if (LeftCamPinState == PinState::High)
	{
		LeftExhaustCam.EndPulse(camInterval);
	}

}

void RightCamSignalChange()
{
	unsigned camInterval = RightCamTimer.getElapsed();
	unsigned crankInterval = CrankTimer.getElapsed();

	PinState pinState = GetPinState(RightCamPin);

	// Ignore noise
	if ((pinState == RightCamPinState) || (pinState == PinState::None))
	{
		return;
	}

	RightCamPinState = pinState;

	if (RightCamPinState == PinState::Low)
	{
		DebugRight = camInterval;
		RightExhaustCam.BeginPulse(camInterval, crankInterval);
		StartRightCamTimer();
	}
	else if (RightCamPinState == PinState::High)
	{
		RightExhaustCam.EndPulse(camInterval);
	}
}

void CrankSignalChange()
{
	unsigned interval = CrankTimer.getElapsed();
	
	PinState pinState = GetPinState(CrankPin);

	// Ignore noise
	if ((pinState == CrankPinState) || (pinState == PinState::None))
	{
		return;
	}

	CrankPinState = pinState;

	if (CrankPinState == PinState::Low)
	{
		DebugCrank = interval;
		Crank.BeginPulse(interval);
		StartCrankTimer();
		LeftExhaustCam.StartCycle();
		RightExhaustCam.StartCycle();
	}
	else if (CrankPinState == PinState::High)
	{
		Crank.EndPulse(interval);
	}
}

void InterruptHandlers::Initialize()
{
	pinMode(DiagnosticOutputPin, OUTPUT);
	pinMode(DiagnosticTimingPin, OUTPUT);

#ifdef UseCaptureTimers
	LeftCamTimer.configure(timeout);
	RightCamTimer.configure(timeout);
	CrankTimer.configure(timeout);

	LeftCamTimer.attachInterrupt(LeftCamTimeout);
	RightCamTimer.attachInterrupt(RightCamTimeout);
	CrankTimer.attachInterrupt(CrankTimeout);
#else
	attachInterrupt(digitalPinToInterrupt(LeftCamPin), LeftCamSignalChange, CHANGE);
	attachInterrupt(digitalPinToInterrupt(RightCamPin), RightCamSignalChange, CHANGE);
	attachInterrupt(digitalPinToInterrupt(CrankPin), CrankSignalChange, CHANGE);
#endif
	StartLeftCamTimer();
	StartRightCamTimer();
	StartCrankTimer();
}


