#pragma once

///////////////////////////////////////////////////////////////////////////////
// Utility for smoothing various timing-related values
// Seemed like a good idea, but may introduce too much lag.
//
// Might be worth revisiting with two timing marks on the cam pulley, as that
// would give twice as much crank and crank-to-cam data to work with.
//
// A weight of one means no smoothing at all - the new value replaces the old.
// A weight of 0.5 means the average will be the midpoint of the new and old.
// A weight of 0.1 means the "average" will be moved 10% closer to the new value.
///////////////////////////////////////////////////////////////////////////////

template<typename T> void UpdateRollingAverage(T *average, T newValue, float weight)
{
	if (weight == 1.0f)
	{
		*average = newValue;
		return;
	}
	
	*average = (T) (weight * newValue) + ((1 - weight) * *average);
}

float GetRollingAverageWeight(int rpm);

///////////////////////////////////////////////////////////////////////////////
// Self-test the rolling-average code
///////////////////////////////////////////////////////////////////////////////
void SelfTestRollingAverage();