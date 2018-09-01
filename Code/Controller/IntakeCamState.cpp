///////////////////////////////////////////////////////////////////////////////
// Code to handle the "three minus one" timing pattern of intake cams.
// This has only been tested with Simulator_IntakeCams.ino, not with a car.
///////////////////////////////////////////////////////////////////////////////

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "stdafx.h"
#include "Mode.h"
#include "Globals.h"
//#include "Timing.h"
#include "IntakeCamState.h"
#include "SelfTest.h"

///////////////////////////////////////////////////////////////////////////////
// Instances of IntakeCamState. These are declared extern in IntakeCamState.h so that
// the rest of the code can access them.
///////////////////////////////////////////////////////////////////////////////
IntakeCamState LeftIntakeCam(1);
IntakeCamState RightIntakeCam(0);

///////////////////////////////////////////////////////////////////////////////
// Simplified implementation of BeginPulse, for investigation/diagnosis
///////////////////////////////////////////////////////////////////////////////
/*
void IntakeCamState::BeginPulse(unsigned camInterval, unsigned crankInterval)
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
}
*/

///////////////////////////////////////////////////////////////////////////////
// Process the start of a single pulse from the cam position sensor.
///////////////////////////////////////////////////////////////////////////////
void IntakeCamState::BeginPulse(unsigned camInterval, unsigned crankInterval)
{
	PulseState = 1;

	// Spend half of the calibration countdown period just establishing the average interval.
	// For the second half of the calibration countdown period, compute the rest of the values.
	//
	// TODO: replace calibration countdown with "RPM > 500 for 5 seconds"
	if (CalibrationCountdown > 0)
	{
		CalibrationCountdown--;

		if (CalibrationCountdown > ((4 * Mode::CalibrationCountdown) / 5)) // > 72
		{
			// Seed the average value - it'll be too high or too low, but it'll at least be within 50%.
			CountdownState = CountdownStates::Reset;
			Rpm = 0;
			AverageInterval = camInterval;
			ShortInterval = 0;
			LongInterval = 0;
			PulseDuration = 0;
			IntervalState = 0;
			return;
		}
		else if (CalibrationCountdown > ((3 * Mode::CalibrationCountdown) / 5)) // > 54
		{
			// Smooth the average pulse length to a reasonable value.
			CountdownState = CountdownStates::Countdown1;
			UpdateRollingAverage(&AverageInterval, camInterval);
			return;
		}
		else if (CalibrationCountdown > ((2 * Mode::CalibrationCountdown) / 5)) // > 36
		{
			// Seed the long/short metrics
			CountdownState = CountdownStates::Initialize1;
		}
		else if (CalibrationCountdown > ((1 * Mode::CalibrationCountdown) / 5)) // > 18
		{
			// Smooth all metrics
			CountdownState = CountdownStates::Countdown2;
		}
		else // 0-18
		{
			// Establish the cam baseline timing
			CountdownState = CountdownStates::Initialize2;
		}
	}
	else
	{
		// Enable long/short pulse calibration checking
		CountdownState = CountdownStates::Run;
	}

	UpdateRollingAverage(&AverageInterval, camInterval);
	
	if (camInterval > AverageInterval)
	{
		// Process a long pulse
		if (CountdownState == CountdownStates::Initialize1)
		{
			LongInterval = camInterval;
			ShortInterval = camInterval / 2;
			TimeSinceCrankSignal = crankInterval;
		}
		else
		{
			UpdateRollingAverage(&LongInterval, camInterval);
			UpdateRollingAverage(&TimeSinceCrankSignal, crankInterval);
		}

		unsigned camRpm = TicksPerMinute / LongInterval;
		unsigned crankRpm = camRpm / 2;

		unsigned ticksPerRevolution = LongInterval * 2;
		unsigned retard = (crankInterval * 360) / ticksPerRevolution;

		if (CountdownState == CountdownStates::Initialize1)
		{
			Rpm = crankRpm;
			Baseline = retard;
		}
		else
		{
			UpdateRollingAverage(&Rpm, crankRpm);
		}

		if (CountdownState == CountdownStates::Initialize2)
		{
			// Baseline is not modified after initialization.
			UpdateRollingAverage(&Baseline, retard);
		}

		retard = retard - Baseline;
		UpdateRollingAverage(&Angle, retard);

		// Validate long/short pulse calibration
		if (CountdownState == CountdownStates::Run)
		{
			if (IntervalState == 0)
			{
				mode.Fail(Left ? "L Cal Long 0" : "R Cal Long 0");
			}
			else if (IntervalState == 1)
			{
				mode.Fail(Left ? "L Cal Long 1" : "R Cal Long 1");
			}
		}

		IntervalState = 0;
	}
	else
	{
		// Process a short pulse
		if (CountdownState == CountdownStates::Initialize1)
		{
			LongInterval = camInterval * 2;
			ShortInterval = camInterval;
		}
		else
		{
			UpdateRollingAverage(&ShortInterval, camInterval);
		}

		// Validate long/short pulse calibration
		if (IntervalState == 0)
		{
			IntervalState = 1;
		}
		else if (IntervalState == 1)
		{
			IntervalState = 2;
		}
		else if (CountdownState == CountdownStates::Run)
		{
			mode.Fail(Left ? "Left Cal Short" : "Right Cal Short");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Process the end of a pulse from the cam position sensor
///////////////////////////////////////////////////////////////////////////////
void IntakeCamState::EndPulse(unsigned camInterval)
{
	PulseState = 0;

	if (CountdownState == CountdownStates::Reset)
	{
		PulseDuration = camInterval;
	}
	else
	{
		UpdateRollingAverage(&PulseDuration, camInterval);
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
// Simulate a series of pulses from the cam sensor, and verify the results.
///////////////////////////////////////////////////////////////////////////////
bool TestIntakeCamPulseSeries(unsigned rpm)
{
	IntakeCamState test(1);
	test.CalibrationCountdown = Mode::CalibrationCountdown;

	float revsPerMinute = rpm;
	float revsPerSecond = revsPerMinute / 60;
	float secondsPerRevolution = 1 / revsPerSecond;
	float clockTicksPerCrankRevolution = (TicksPerMinute / 60) * secondsPerRevolution;
	unsigned clockTicksPerCamRevolution = (unsigned)(clockTicksPerCrankRevolution * 2);
	unsigned shortDuration = clockTicksPerCamRevolution / 4;
	shortDuration /= 2;
	
	for (int i = 0; i < Mode::CalibrationCountdown * 2; i++)
	{
		test.BeginPulse(shortDuration, (shortDuration / 2) + shortDuration);
		test.BeginPulse(shortDuration, (shortDuration / 2) + shortDuration + shortDuration);
		test.BeginPulse(shortDuration * 2, shortDuration / 2);
	}

	if (!WithinOnePercent(test.ShortInterval, shortDuration, "ShortIntv"))
	{
		return false;
	}

	if (!WithinOnePercent(test.LongInterval, shortDuration * 2, "LongIntv"))
	{
		return false;
	}

	if (!WithinOnePercent(test.Rpm, revsPerMinute, "Rpm"))
	{
		return false;
	}

	if (!WithinOnePercent(test.TimeSinceCrankSignal, shortDuration / 2, "TimeSince"))
	{
		return false;
	}
	
	if (!WithinOnePercent(test.Baseline, 45, "Baseline"))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Test a series of pulses at the idle RPM.
// (And ensure that the timer-counters do not overflow.)
///////////////////////////////////////////////////////////////////////////////
bool TestIntakeCamIdle()
{
	return TestIntakeCamPulseSeries(750);
}

///////////////////////////////////////////////////////////////////////////////
// Test a series of pulses at an RPM that will hopefully never happen.
// (And ensure that the timer-counters have enough resolution.)
///////////////////////////////////////////////////////////////////////////////
bool TestIntakeCam10k()
{
	return TestIntakeCamPulseSeries(10 * 1000);
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the cam timing code.
///////////////////////////////////////////////////////////////////////////////
void SelfTestIntakeCamState()
{
	InvokeTest(IntakeCamIdle);
	InvokeTest(IntakeCam10k);
}