#include "stdafx.h"
#include "Globals.h"
#include "RollingAverage.h"
#include "Mode.h"
#include "CrankState.h"
#include "ExhaustCamState.h"
#include "IntervalRecorder.h"

CrankState Crank;

void CrankState::BeginPulse(unsigned elapsed)
{
	IIntervalRecorder::GetInstance()->LogInterval(Intervals::CrankHigh);

	Crank.PulseState = 1;

	if (CalibrationCountdown > 0)
	{
		CalibrationCountdown--;

		// Seed the average
		if (CalibrationCountdown > (Mode::CalibrationCountdown * 0.8f))
		{
			AverageInterval = elapsed;
		}
	}

	UpdateRollingAverage(&AverageInterval, elapsed, 1);

	unsigned rpm = (TicksPerMinute / AverageInterval) * 2; // x2 because the sensor is on a cam pulley, not the crank itself.
	UpdateRollingAverage(&Rpm, rpm, 0);
}

void CrankState::EndPulse(unsigned interval)
{
	IIntervalRecorder::GetInstance()->LogInterval(Intervals::CrankLow);

	Crank.PulseState = 0;

	if (CalibrationCountdown > (Mode::CalibrationCountdown * 0.8f))
	{
		PulseDuration = interval;
	}

	UpdateRollingAverage(&PulseDuration, interval, 1);
}