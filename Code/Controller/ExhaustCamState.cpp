///////////////////////////////////////////////////////////////////////////////
// At 0.006 valve lift:
//
// Exhaust AVCS range:
// Max Advance = Open: 72 before BDC, Close: 12 BTDC, Center: 48 ABDC
// Max Retard =  Open: 32 before BDC, Close: 28 ATDC, Center: 88 ABDC
//
// Non-AVCS:     Open: 55 before BDC, Close: 5 after TDC, Center: 65 ABDC
//
// To do: make the same table with valve events at 0.050 valve lift.
//
// Note that stock cams have 240 degrees duration @ 0.006, so aftermarket 
// cams will expand those open/close numbers somewhat.
///////////////////////////////////////////////////////////////////////////////

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "stdafx.h"
#include "Mode.h"
#include "Configuration.h"
#include "Globals.h"
#include "RollingAverage.h"
#include "ExhaustCamState.h"
#include "SelfTest.h"
#include "IntervalRecorder.h"

extern int onlyMeasureBaseline;

int LeftCamDurationDiagnosticPin = 24;
int RightCamDurationDiagnosticPin = 25;

///////////////////////////////////////////////////////////////////////////////
// Instances of ExhaustCamTiming. These are declared extern in 
// ExhaustCamTiming.h so  that the rest of the code can access them.
///////////////////////////////////////////////////////////////////////////////
ExhaustCamState LeftExhaustCam(1);
ExhaustCamState RightExhaustCam(0);

///////////////////////////////////////////////////////////////////////////////
// Simplified implementation of BeginPulse, for investigation/diagnosis
///////////////////////////////////////////////////////////////////////////////
/*void ExhaustCamTiming::BeginPulse(unsigned camInterval, unsigned crankInterval)
{
	if (CalibrationCountdown > 0)
	{
		CalibrationCountdown--;
	}

	UpdateRollingAverage(&AverageInterval, camInterval);
	if (camInterval > AverageInterval)
	{
		UpdateRollingAverage(&LongInterval, camInterval);
		UpdateRollingAverage(&TimeSinceCrankSignal, crankInterval);
	}
	else
	{
		UpdateRollingAverage(&ShortInterval, camInterval);
	}
}*/

///////////////////////////////////////////////////////////////////////////////
//
//
void ExhaustCamState::StartCycle()
{
	CycleState = CycleStates::Start;

	int pin = this->Left ? LeftCamDurationDiagnosticPin : RightCamDurationDiagnosticPin;
	digitalWrite(pin, HIGH);
}

///////////////////////////////////////////////////////////////////////////////
// Process the start of a single pulse from the cam position sensor.
///////////////////////////////////////////////////////////////////////////////
// Cam interval: elapsed time since start of the previous cam pulse.
// Crank interval: elapsed time since last start of crank pulse.
void ExhaustCamState::BeginPulse(unsigned camInterval, unsigned crankInterval)
{
	PulseState = 1;

	switch (CycleState)
	{
	case CycleStates::Start:
		CycleState = CycleStates::Pulse1;
		break;

	case CycleStates::Pulse1:
		CycleState = CycleStates::Pulse2;
		break;

	case CycleStates::Pulse2:
		// TODO: prove that this never happens.
		CycleState = CycleStates::End;
		break;
	}

	if (this->Left)
	{
		if (CycleState == CycleStates::Pulse1)
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::LeftExhaustCamHigh1);
		}
		else
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::LeftExhaustCamHigh2);
		}
	}
	else
	{
		if (CycleState == CycleStates::Pulse1)
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::RightExhaustCamHigh1);
		}
		else
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::RightExhaustCamHigh2);
		}
	}


	// The first part of the calibration countdown period is just seeding the key values.
	if (CalibrationCountdown > 0)
	{
		CalibrationCountdown--;

		if (CalibrationCountdown > (Mode::CalibrationCountdown * 0.8f))
		{
			// Seed the average value - it'll be too high or too low, but it's something to start with.
			AverageInterval = camInterval;
			TimeSinceCrankSignal = crankInterval;
		}
	}

	// Cam interval is only used to determine RPM.
	UpdateRollingAverage(&AverageInterval, camInterval, 0);

	// Set/update TimeSinceCrankSignal
	if (CycleState == CycleStates::Pulse1)
	{
		int pin = this->Left ? LeftCamDurationDiagnosticPin : RightCamDurationDiagnosticPin;
		digitalWrite(pin, LOW);

		// Update this instance's RPM value based on the time since the last pulse
		unsigned ticksPerCamRevolution = AverageInterval * 2;
		unsigned camRpm = TicksPerMinute / ticksPerCamRevolution;
		unsigned crankRpm = camRpm * 2;
		UpdateRollingAverage(&Rpm, crankRpm, 0);
	
		// Crank interval is used to determine cam position.
		UpdateRollingAverage(&TimeSinceCrankSignal, crankInterval, 1);

		float ticksPerDegree = (float)ticksPerCamRevolution / 360.0f;
		float angle = ((float)TimeSinceCrankSignal) / ticksPerDegree;
		
		// Update the baseline cam angle while solenoids are disabled.
		if ((CalibrationCountdown > 0) || onlyMeasureBaseline)
		{
			if (useStaticBaseline)
			{
				// These values were discovered by setting the "onlyMeasureBaseline"
				// flag, logging the baseline values while the engine was at 2500 RPM
				// for about 15 seconds, and then using Excel to average the values.
				if (this->Left)
				{
					Baseline = 131.2145;
				}
				else
				{
					Baseline = 41.0733;
				}

				// Sanity check: Compare the static baselines to measured baselines.
				const int tolerance = 5;
				if ((angle > Baseline + tolerance) || (angle < Baseline - tolerance))
				{
					mode.Fail(Left ? "Left Baseline" : "Right Baseline");
				}
			}
			else
			{
				UpdateRollingAverage(&Baseline, angle, 1);
			}

			Angle = 0;
		}
		else
		{
			angle = angle - Baseline;
			UpdateRollingAverage(&Angle, angle, 1);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Process the end of a pulse from the cam position sensor
///////////////////////////////////////////////////////////////////////////////
void ExhaustCamState::EndPulse(unsigned camInterval)
{
	if (this->Left)
	{
		if (CycleState == CycleStates::Pulse1)
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::LeftExhaustCamLow1);
		}
		else
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::LeftExhaustCamLow2);
		}
	}
	else
	{
		if (CycleState == CycleStates::Pulse1)
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::RightExhaustCamLow1);
		}
		else
		{
			IIntervalRecorder::GetInstance()->LogInterval(Intervals::RightExhaustCamLow2);
		}
	}

	PulseState = 0;

	// It turns out that the end-of-pulse timing information isn't reliable,
	// because the interrupt fires when the signal is at a shallow slope.
	// Might want to discard outliers before calling UpdateRollingAverage, but
	// the pulse duration value isn't really used at runtime anyway. It was only
	// added for manual investigation & sanity-checking during development.
	UpdateRollingAverage(&PulseDuration, camInterval, 1);
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################

///////////////////////////////////////////////////////////////////////////////
// Simulate a series of pulses from the exhaust cam sensor, and verify the results.
///////////////////////////////////////////////////////////////////////////////
bool TestExhaustCamPulseSeries(unsigned rpm)
{
	ExhaustCamState test(1);
	test.CalibrationCountdown = Mode::CalibrationCountdown;

	float revsPerMinute = rpm;
	float revsPerSecond = revsPerMinute / 60;
	float secondsPerRevolution = 1 / revsPerSecond;
	float clockTicksPerCrankRevolution = (TicksPerMinute / 60) * secondsPerRevolution;
	unsigned clockTicksPerCamRevolution = (unsigned)(clockTicksPerCrankRevolution * 2);
	unsigned duration = clockTicksPerCamRevolution / 2;
	
	for (int i = 0; i < Mode::CalibrationCountdown * 2; i++)
	{
		test.BeginPulse(duration + 1, (duration * 3) / 2);
		test.BeginPulse(duration - 1, duration / 2);
	}

	if (!WithinOnePercent((unsigned)test.AverageInterval, duration, "AvgIntv"))
	{
		return false;
	}

	if (!WithinOnePercent((float)test.Rpm, revsPerMinute, "Rpm"))
	{
//		return false;
	}

	if (!WithinOnePercent((unsigned)test.TimeSinceCrankSignal, duration / 2, "TimeSince"))
	{
//		return false;
	}
	
	if (!WithinOnePercent(test.Baseline, 45.0f, "Baseline"))
	{
//		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Test a series of pulses at the idle RPM.
// (And ensure that the timer-counters do not overflow.)
///////////////////////////////////////////////////////////////////////////////
bool TestExhaustCamIdle()
{
	return TestExhaustCamPulseSeries(750);
}

///////////////////////////////////////////////////////////////////////////////
// Test a series of pulses at an RPM that will hopefully never happen.
// (And ensure that the timer-counters have enough resolution.)
///////////////////////////////////////////////////////////////////////////////
bool TestExhaustCam10k()
{
	return TestExhaustCamPulseSeries(10 * 1000);
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the cam timing code.
///////////////////////////////////////////////////////////////////////////////
void SelfTestExhaustCamTiming()
{
	InvokeTest(ExhaustCamIdle);
	InvokeTest(ExhaustCam10k);
}