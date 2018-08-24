#pragma once

///////////////////////////////////////////////////////////////////////////////
// Utility for smoothing various timing-related values
// Seemed like a good idea, but may introduce too much lag.
//
// Might be worth revisiting with two timing marks on the cam pulley, as that
// would give twice as much crank and crank-to-cam data to work with.
//
// A smoothingRate of zero means no smoothing at all.
// A smoothingRate of one means that the average will move 50% of the way toward
// the latest sample.
// A smoothingRate of ten means that the average will move 10% of the way toward
// the latest sample.
///////////////////////////////////////////////////////////////////////////////

template<typename T> void UpdateRollingAverage(T *average, T newValue, int smoothingRate)
{
	if (smoothingRate == 0)
	{
		*average = newValue;
		return;
	}

	float numeratorFactor = (float)smoothingRate;
	float denominatorFactor = (float)smoothingRate + 1;

	float temp = (float) (*average);
	temp = ((temp * numeratorFactor) + newValue) / denominatorFactor;
	*average = (T)temp;
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the rolling-average code
///////////////////////////////////////////////////////////////////////////////
void SelfTestRollingAverage();