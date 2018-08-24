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
	SyncErrorCount = 0;
	this->BeginSynchronizing();
	ErrorScreen->Right = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// To be invoked once per iteration of the main loop
///////////////////////////////////////////////////////////////////////////////
void Mode::Update()
{
	switch (this->currentMode)
	{
	case Mode::Synchronizing:
	
		if (Crank.Rpm < MINIMUM_EXAVCS_RPM)
		{
			this->BeginSynchronizing();
			return;
		}

		if (this->IsSynchronized())
		{
			this->BeginWarming();
			strncpy(LastErrorMessage, ErrorMessage, DisplayWidth);
			ErrorMessage[0] = 0;
		}
		break;

	case Mode::Warming:

		if (OilTemperature > 71) // 71C = 160F
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

	// Expect failures during sync - just try to re-sync
	// Note that the calls to BeginSynchronizing need to happen below this 
	// conditional because they will reset the mode to Synchronizing.
	if (this->currentMode == Mode::Synchronizing)
	{
		SyncErrorCount++;
		BeginSynchronizing();
		return;
	}
	else
	{
		ErrorCount++;
		BeginSynchronizing();
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
// Begin synchronizing cam and crank sensors and state
///////////////////////////////////////////////////////////////////////////////
void Mode::BeginSynchronizing()
{
	ClearScreen();
	this->currentMode = Mode::Synchronizing;

	// Exhaust cams have two pulses per revolution.
	LeftExhaustCam.SyncCountdown = SyncCountdown * 2;
	RightExhaustCam.SyncCountdown = SyncCountdown * 2;
	Crank.SyncCountdown = SyncCountdown;
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
// Indicates whether the cam and crank states are synchronized
///////////////////////////////////////////////////////////////////////////////
int Mode::IsSynchronized()
{
	return
		LeftExhaustCam.SyncCountdown == 0 &&
		RightExhaustCam.SyncCountdown == 0 &&
		Crank.SyncCountdown == 0;
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
// Ensure that the right stuff happened to after a call to BeginSynchronizing
///////////////////////////////////////////////////////////////////////////////
bool ValidateResync()
{
	if (!CompareUnsigned(LeftExhaustCam.SyncCountdown, Mode::SyncCountdown * 2, "Right.SC"))
	{
		return false;
	}

	if (!CompareUnsigned(RightExhaustCam.SyncCountdown, Mode::SyncCountdown * 2, "Right.SC"))
	{
		return false;
	}

	if (!CompareUnsigned(Crank.SyncCountdown, Mode::SyncCountdown, "Crank.SC"))
	{
		return false;
	}

	if (!CompareUnsigned(mode.GetMode(), Mode::Synchronizing, "Mode.1"))
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

	return ValidateResync();
}

///////////////////////////////////////////////////////////////////////////////
// Validate a transition to the warm-up state
///////////////////////////////////////////////////////////////////////////////
bool TestTransToWarming()
{
	TestInitializeMode();

	Crank.Rpm = MINIMUM_EXAVCS_RPM + 100;

	LeftExhaustCam.SyncCountdown = 0;
	RightExhaustCam.SyncCountdown = 0;
	Crank.SyncCountdown = 0;
	
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
// Validate a failure during synchronization
///////////////////////////////////////////////////////////////////////////////
bool TestFailSync()
{
	TestInitializeMode();

	LeftExhaustCam.SyncCountdown = 20;
	RightExhaustCam.SyncCountdown = 20;
	Crank.SyncCountdown = 20;

	mode.Fail("Testing");

	if (!CompareUnsigned(ErrorCount, 0, "Err.2"))
	{
		return false;
	}

	if (!CompareUnsigned(SyncErrorCount, 1, "SyncErr.2"))
	{
		return false;
	}

	return ValidateResync();
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

	if (!CompareUnsigned(SyncErrorCount, 0, "SyncErr.3"))
	{
		return false;
	}

	return ValidateResync();

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

	if (!CompareUnsigned(SyncErrorCount, 0, "SyncErr.3"))
	{
		return false;
	}

	// TODO: Validate the appended error screen

	return ValidateResync();
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
	InvokeTest(FailSync);
	InvokeTest(FailWarming);
	InvokeTest(FailRunning);
	testMode = 0;
}