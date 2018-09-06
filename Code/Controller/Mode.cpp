#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "stdafx.h"
#include <string.h>
#include <stdio.h>

#include "Mode.h"
#include "Globals.h"
#include "Utilities.h"
#include "Screen.h"
#include "SelfTest.h"
#include "ExhaustCamState.h"
#include "CrankState.h"
#include "Configuration.h"

extern Screen *ErrorScreen;

int testMode;

///////////////////////////////////////////////////////////////////////////////
// Initialize an instance of Mode
///////////////////////////////////////////////////////////////////////////////
void Mode::Initialize()
{
	ErrorCount = 0;
	InitializationErrorCount = 0;
	this->BeginCalibrating();
	ErrorScreen->Right = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// To be invoked once per iteration of the main loop
///////////////////////////////////////////////////////////////////////////////
void Mode::Update()
{
	switch (this->currentMode)
	{
	case Mode::Calibrating:
	
		if (Crank.Rpm < MINIMUM_EXAVCS_RPM)
		{
			this->BeginCalibrating();
			return;
		}

		if (this->IsCalibrated())
		{
			this->BeginWarming();
			strncpy(LastErrorMessage, ErrorMessage, DisplayWidth);
			ErrorMessage[0] = 0;
		}
		break;

	case Mode::Warming:

		if (OilTemperature > 65.5) // 65.5C = 150F, 71C = 160F
		{
			this->BeginRunning();
		}
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// To be invoked by any code when a failure is detected.
///////////////////////////////////////////////////////////////////////////////
void Mode::Fail(const char * message)
{
#ifdef ARDUINO
	Serial.println(message);
#endif

	// Expect failures during calibration - just try to re-calibrate
	// Note that the calls to BeginCalibrating need to happen below this 
	// conditional because they will reset the mode to Calibrating.
	if (this->currentMode == Mode::Calibrating)
	{
		InitializationErrorCount++;
		BeginCalibrating();
		return;
	}
	else
	{
		ErrorCount++;
		BeginCalibrating();
	}
		
	if (ErrorCount > 100)
	{
		return;
	}

	Screen *lastError = ErrorScreen;

	do
	{
		if (lastError->Right == null)
		{
			unsigned *index = new unsigned();
			*index = ErrorCount;
			lastError->Right = new SingleValueScreen(message, index);
			lastError->Right->Left = lastError;
			lastError->Right->Up = ErrorScreen->Up;
			lastError->Right->Down = ErrorScreen->Down;
			break;
		}
		else
		{
			lastError = lastError->Right;
		}
	} while (lastError != null);

	// TODO: Make the failure evident.
	//
	// Disable boost
	// Set analog output to 0 for TGV input?
	// Set a new PLX value to nonzero?
	//
	// this->Initialize(); ?
}

///////////////////////////////////////////////////////////////////////////////
// Begin calibrating cam and crank timing
///////////////////////////////////////////////////////////////////////////////
void Mode::BeginCalibrating()
{
	ClearScreen();
	this->currentMode = Mode::Calibrating;

	// Exhaust cams have two pulses per revolution.
	LeftExhaustCam.CalibrationCountdown = CalibrationCountdown * 2;
	RightExhaustCam.CalibrationCountdown = CalibrationCountdown * 2;
	Crank.CalibrationCountdown = CalibrationCountdown;
}

///////////////////////////////////////////////////////////////////////////////
// Transition to the warm-up mode
///////////////////////////////////////////////////////////////////////////////
void Mode::BeginWarming()
{
	ClearScreen(); 
	this->currentMode = Mode::Warming;
}

///////////////////////////////////////////////////////////////////////////////
// Transition to the running mode
///////////////////////////////////////////////////////////////////////////////
void Mode::BeginRunning()
{
	ClearScreen();
	this->currentMode = Mode::Running;
}

///////////////////////////////////////////////////////////////////////////////
// Indicates whether the cam and crank states are calibrated
///////////////////////////////////////////////////////////////////////////////
int Mode::IsCalibrated()
{
	return
		LeftExhaustCam.CalibrationCountdown == 0 &&
		RightExhaustCam.CalibrationCountdown == 0 &&
		Crank.CalibrationCountdown == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Clean the LCD screen
///////////////////////////////////////////////////////////////////////////////
void Mode::ClearScreen()
{
	if (!testMode)
	{
		::ClearScreen();
	}
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################

///////////////////////////////////////////////////////////////////////////////
// Ensure that the right stuff happened to after a call to BeginCalibrating
///////////////////////////////////////////////////////////////////////////////
bool ValidateRecalibrate()
{
	if (!CompareUnsigned(LeftExhaustCam.CalibrationCountdown, Mode::CalibrationCountdown * 2, "Right.SC"))
	{
		return false;
	}

	if (!CompareUnsigned(RightExhaustCam.CalibrationCountdown, Mode::CalibrationCountdown * 2, "Right.SC"))
	{
		return false;
	}

	if (!CompareUnsigned(Crank.CalibrationCountdown, Mode::CalibrationCountdown, "Crank.SC"))
	{
		return false;
	}

	if (!CompareUnsigned(mode.GetMode(), Mode::Calibrating, "Mode.1"))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Validate the one-time initialization
///////////////////////////////////////////////////////////////////////////////
bool TestInitializeMode()
{
	mode.Initialize();

	if (!CompareUnsigned(ErrorCount, 0, "ErrCnt.1"))
	{
		return false;
	}

	return ValidateRecalibrate();
}

///////////////////////////////////////////////////////////////////////////////
// Validate a transition to the warm-up state
///////////////////////////////////////////////////////////////////////////////
bool TestTransToWarming()
{
	TestInitializeMode();

	Crank.Rpm = MINIMUM_EXAVCS_RPM + 100;

	LeftExhaustCam.CalibrationCountdown = 0;
	RightExhaustCam.CalibrationCountdown = 0;
	Crank.CalibrationCountdown = 0;
	
	mode.Update();

	if (!CompareUnsigned(mode.GetMode(), Mode::Warming, "Mode.2"))
	{
		return false;
	}

	OilTemperature = 71;

	mode.Update();

	if (!CompareUnsigned(mode.GetMode(), Mode::Warming, "Mode.3"))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Validate a transition to the running state
///////////////////////////////////////////////////////////////////////////////
bool TestTransToRunning()
{
	TestTransToWarming();

	OilTemperature = 72;

	mode.Update();

	if (!CompareUnsigned(mode.GetMode(), Mode::Running, "Mode.4"))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Validate a failure during calibration
///////////////////////////////////////////////////////////////////////////////
bool TestFailCalibration()
{
	TestInitializeMode();

	LeftExhaustCam.CalibrationCountdown = 20;
	RightExhaustCam.CalibrationCountdown = 20;
	Crank.CalibrationCountdown = 20;

	mode.Fail("Testing");

	if (!CompareUnsigned(ErrorCount, 0, "Err.2"))
	{
		return false;
	}

	if (!CompareUnsigned(InitializationErrorCount, 1, "InitErr.2"))
	{
		return false;
	}

	return ValidateRecalibrate();
}

///////////////////////////////////////////////////////////////////////////////
// Validate a failure during the warming state
///////////////////////////////////////////////////////////////////////////////
bool TestFailWarming()
{
	TestTransToWarming();

	mode.Fail("Testing");

	if (!CompareUnsigned(ErrorCount, 1, "Err.3"))
	{
		return false;
	}

	if (!CompareUnsigned(InitializationErrorCount, 0, "InitErr.3"))
	{
		return false;
	}

	return ValidateRecalibrate();

}

///////////////////////////////////////////////////////////////////////////////
// Validate a failure during the running state
///////////////////////////////////////////////////////////////////////////////
bool TestFailRunning()
{
	TestTransToRunning();

	mode.Fail("Testing");

	if (!CompareUnsigned(ErrorCount, 1, "Err.4"))
	{
		return false;
	}

	if (!CompareUnsigned(InitializationErrorCount, 0, "InitErr.3"))
	{
		return false;
	}

	// TODO: Validate the appended error screen

	return ValidateRecalibrate();
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the Mode code.
///////////////////////////////////////////////////////////////////////////////
void SelfTestMode()
{

#if UNUSED

	int previous = 0;
	unsigned changes = 0;
	unsigned start = millis();
	while (millis() < start + 100)
	{
		int current = digitalRead(3);
		if (current != previous)
		{
			changes++;
		}
		previous = current;
	}

	if (changes > 0)
	{
		ShowProgress("Mode Test", changes);
		return;
	}
#endif

	testMode = 1;
	InvokeTest(InitializeMode);
	InvokeTest(TransToWarming);
	InvokeTest(TransToRunning);
	InvokeTest(FailCalibration);
	InvokeTest(FailWarming);
	InvokeTest(FailRunning);
	testMode = 0;
}