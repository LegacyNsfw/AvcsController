#include "stdafx.h"
#include "Globals.h"
#include "RollingAverage.h"
#include "Mode.h"
#include "CurveTable.h"
#include "CrankState.h"
#include "ExhaustCamState.h"
#include "IntervalRecorder.h"

CrankState Crank;
extern CurveTable *pFilterWeightTable;

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

	// x2 because the sensor is on a cam pulley, not the crank itself.
	unsigned rpm = (TicksPerMinute / AverageInterval) * 2; 

	// RPM is filtered because it jumps around a lot at idle.
	float weight = pFilterWeightTable->GetValue(Rpm);
	UpdateRollingAverage(&Rpm, rpm, weight);
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