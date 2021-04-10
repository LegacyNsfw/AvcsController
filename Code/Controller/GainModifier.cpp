#if ARDUINO
#include <Arduino.h>
#endif

#include "stdafx.h"
#include "CurveTable.h"
#include "SelfTest.h"

CurveTable* gainTable = CurveTable::CreateGainTable();

// The time at which the controller entered Running mode
long startTime = 0;

// How many milliseconds to ramp up the gain from zero.
const long rampInTime = 2000;

float GetRampInFactor(long now)
{
	float rampInFactor = 0;

	if (startTime == 0)
	{
		startTime = now;
	}

	if (now < (startTime + rampInTime))
	{
		rampInFactor = ((float)(now - startTime)) / ((float)rampInTime);
	}
	else
	{
		rampInFactor = 1.0f;
	}

	return rampInFactor;
}

float GetGainModifier(float rpm, long now)
{
	if (rpm < 500 || now < 1000)
	{
		return 0;
	}

	float rampInGain = GetRampInFactor(now);
	float rpmGain = gainTable->GetValue(rpm);

	return rampInGain * rpmGain;
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################


bool TestRampInFactor()
{
	startTime = 100;
	if (GetRampInFactor(100) != 0)
	{
		TestFailed("RampIn should be zero at first.");
		return false;
	}
	
	if (!WithinOnePercent(GetRampInFactor(1100), 0.5f, "1 second"))
	{
		TestFailed("RampIn should be 50% after one second.");
		return false;
	}

	if (!GetRampInFactor(2100) == 1.0f)
	{
		TestFailed("RampIn should be 100% after two seconds.");
		return false;
	}

	if (!GetRampInFactor(5000) == 1.0f)
	{
		TestFailed("RampIn should remain 100% long after ramp-in.");
		return false;
	}

	return true;
}

bool TestGainModifier()
{
	CurveTable *table = CurveTable::CreateGainTable();

	startTime = 100;
	long now = 1100;
	float rpm = 1000.0f;

	if (!WithinOnePercent(GetGainModifier(rpm, now), 0.25f, "50, 50"))
	{
		TestFailed("Overall gain should be 25% when rampin and rpm both dictate 50%.");
		return false;
	}

	now = 5000;
	rpm = 2500.0f;

	if (!WithinOnePercent(GetGainModifier(rpm, now), 1.0f, "fully ramped in and driving"))
	{
		TestFailed("Overall gain should be 100% when rampin and rpm both dictate 100%.");
		return false;
	}

	return true;
}

void SelfTestGetGainModifier()
{
	InvokeTest(RampInFactor);
	InvokeTest(GainModifier);
}

