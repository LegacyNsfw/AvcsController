#include "stdafx.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "Globals.h"
#include "RollingAverage.h"
#include "Utilities.h"
#include "Feedback.h"
#include "SelfTest.h"

///////////////////////////////////////////////////////////////////////////////
// Values from intake AVCS logs posted by a romraider user:
// When target angle is zero, logs show duty cycle of 9.41%.
// Above 1250 RPM, duty cycle goes to 44.71%.
// At constant AVCS angle, DC was 44% on one side and 47% on the other side.
// Max DC was 58.82%.
///////////////////////////////////////////////////////////////////////////////
Feedback LeftFeedback;
Feedback RightFeedback;

///////////////////////////////////////////////////////////////////////////////
// Initialize an instance of Feedback.
///////////////////////////////////////////////////////////////////////////////
Feedback::Feedback()
{
	Reset(0);
}

///////////////////////////////////////////////////////////////////////////////
// Reset all members to sensible defaults.
///////////////////////////////////////////////////////////////////////////////
void Feedback::Reset(int gainType)
{
	lastTime = 0;

	// This works, but probably could be much improved.
	// Does not oscillate while driving, tracks well as RPM increases.
	// Overshoots target significantly when transitioning above RPM threshold (target changes abruptly from 0 degrees to 1 degree).
	// Try more P, less I
	// Try more I (7.5 was tried briefly, but not while driving, might work fine.)
	// Try more D
	ProportionalGain = 1.0f;
	IntegralGain = 2.5f; 
	DerivativeGain = 0.001f;

	// Reset state variables
	ProportionalTerm = 0;
	IntegralTerm = 0;
	DerivativeTerm = 0;

	GainModifier = 1.0f;

	PreviousError = 0;
	Output = 0;

	for (int i = 0; i < Feedback::BucketCount; i++)
	{
		Average[i] = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Update the Output value based on actual and target values
///////////////////////////////////////////////////////////////////////////////
void Feedback::Update(long currentTimeInMicroseconds, unsigned rpm, float actual, float target)
{
	// 2^63 microseconds = 292,805 years.
	// So, wraparound won't be a problem.
	float time = ((float)(currentTimeInMicroseconds - lastTime)) / ((float)TicksPerSecond);
	lastTime = currentTimeInMicroseconds;

	float error = target - actual;
//	error = -error;
	float errorChange = error - PreviousError;
	
	ProportionalTerm = error * (ProportionalGain * GainModifier);
	IntegralTerm += (error * time) * (IntegralGain * GainModifier);
	DerivativeTerm = (DerivativeGain * GainModifier) * (errorChange / time);

	// Make sure that the integral term never gets excessive.
	float integralLimit = 10;
	if (IntegralTerm > integralLimit)
	{
		IntegralTerm = integralLimit;
	}

	if (IntegralTerm < -integralLimit)
	{
		IntegralTerm = -integralLimit;
	}

	Output = ProportionalTerm + IntegralTerm + DerivativeTerm;

	unsigned bucket = rpm / 500;
	if (bucket >= Feedback::BucketCount)
	{
		bucket = Feedback::BucketCount - 1;
	}

	UpdateRollingAverage(&(Average[bucket]), Output, 0.1f);

	PreviousError = error;
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################

bool TestProportionalRet()
{
	Feedback test;

	// Cam position is 5 degrees too advanced
	test.Update(1000, 1000, 50, 55);

	// Higher output will retard cam
	if (test.ProportionalTerm <= 0)
	{
		TestFailed("Sign Error");
		return false;
	}

	return true;
}

bool TestProportionalAdv()
{
	Feedback test;

	// Cam position is 5 degrees too retarded
	test.Update(1000, 1000, 60, 55);

	// Lower output will advance cam
	if (test.ProportionalTerm >= 0)
	{
		TestFailed("Sign Error");
		return false;
	}

	return true;
}

bool TestIntegralRet()
{
	int rpm = 2500;
	long delta = (long)((1 / (rpm / 60.0f)) * 1000 * 1000);
	long elapsed = 0;

	Feedback test;

	for (int i = 0; i < 100; i++)
	{
		test.Update(elapsed, rpm, 50, 55);
		elapsed += delta;
	}

	if (test.IntegralTerm <= 0)
	{
		TestFailed("Sign Err 1");
		return false;
	}

	float oldValue = test.IntegralTerm;

	for (int i = 0; i < 100; i++)
	{
		test.Update(elapsed, rpm, 50, 55);
		elapsed += delta;
	}

	if (test.IntegralTerm <= 0)
	{
		TestFailed("Sign Err 2");
		return false;
	}

	// To retard the cam, the term should be getting larger.
	if (test.IntegralTerm <= oldValue)
	{
		TestFailed("Integral Ret");
		return false;
	}

	return true;
}

bool TestIntegralAdv()
{
	int rpm = 2500;
	long delta = (long)((1 / (rpm / 60.0f)) * 1000 * 1000);
	long elapsed = 0;

	Feedback test;

	for (int i = 0; i < 100; i++)
	{
		test.Update(elapsed, rpm, 60, 55);
		elapsed += delta;
	}

	if (test.IntegralTerm >= 0)
	{
		TestFailed("Sign Err 1");
		return false;
	}

	float oldValue = test.IntegralTerm;

	for (int i = 0; i < 100; i++)
	{
		test.Update(elapsed, rpm, 60, 55);
		elapsed += delta;
	}

	if (test.IntegralTerm >= 0)
	{
		TestFailed("Sign Err 2");
		return false;
	}

	// To advance the cam, the term should be getting smaller.
	if (test.IntegralTerm >= oldValue)
	{
		TestFailed("Integral Adv");
		return false;
	}

	return true;
}

bool TestDerivativeRet()
{
	int rpm = 2500;
	long delta = (long)(1 / (rpm / 60.0f)) * 1000 * 1000;
	long elapsed = 0;

	Feedback test;

	test.Update(0, rpm, 50, 55);
	test.Update(elapsed, rpm, 54, 55);

	// To retard the cam, proportional and integral terms should be positive.
	// But derivative should now be going negative since error has gotten smaller.
	if (test.DerivativeTerm >= 0)
	{
		TestFailed("Sign error.");
		return false;
	}

	return true;
}

bool TestDerivativeAdv()
{
	int rpm = 2500;
	long delta = (long)(1 / (rpm / 60.0f)) * 1000 * 1000;
	long elapsed = 0;

	Feedback test;

	test.Update(0, rpm, 60, 55);
	test.Update(elapsed, rpm, 59, 55);

	// To advance the cam, proportional and integral terms should be negative
	// But derivative should now be going positive since error has gotten smaller.
	if (test.DerivativeTerm <= 0)
	{
		TestFailed("Sign error.");
		return false;
	}

	return true;
}

// I wish I could remember what I was thinking when I wrote this test case.
bool TestBaseline()
{
	int rpm = 2500;
	long delta = (long)(1 / (rpm / 60.0f)) * 1000 * 1000;
	long elapsed = 0;

	Feedback test;

	for (int i = 0; i < 10; i++)
	{
		// Seed buckets 1 and 4.
		test.Update(elapsed, 750, 55, 55); // 250, 750, 1250, 1750, 2250
		test.Update(elapsed, 2250, 55, 55);//   0    1     2     3     4
	}

	for (int i = 0; i < Feedback::BucketCount; i++)
	{
		if (i == 1)
		{
			if (!WithinOnePercent(test.Average[1], 40.0f, "Bucket 1"))
			{
				return false;
			}

			continue;
		}

		if (i == 4)
		{
			if (!WithinOnePercent(test.Average[4], 40.0f, "Bucket 4"))
			{
				return false;
			}

			continue;
		}

		if (test.Average[i] != 0)
		{
			PrintShort(FailureMessage, (unsigned)i);
			FailureMessage[6] = ' ';
			PrintShort(&(FailureMessage[7]), (unsigned)test.Average[i]);
			return false;
		}
	}

	return true;
}

bool TestAccumulator()
{
	// At 2500 RPM, it should take this many seconds 
	// for the PWM output to rise from 40% to 45%
	int seconds = 5;
	int crankRpm = 2500;
	float crankRevsPerSecond = crankRpm / 60.0f;
	int iterations = (int) crankRevsPerSecond * seconds;
	int elapsed = 0;
	int delta = (long)((1 / crankRevsPerSecond) * 1000 * 1000);
	Feedback test;

	for (int iteration = 0; iteration < iterations; iteration++)
	{
		elapsed += delta;
		test.Update(elapsed, crankRpm, 45, 60);
	}

	if (test.IntegralTerm < 4.5f)
	{
		TestFailed("Need greater integral gain.");
		return false;
	}

	if (test.IntegralTerm > 5.5f)
	{
		TestFailed("Need less integral gain.");
		return false;
	}

	return true;
}

void SelfTestFeedback()
{
	/*
	InvokeTest(ProportionalRet);
	InvokeTest(ProportionalAdv);
	InvokeTest(IntegralRet);
	InvokeTest(IntegralAdv);
	InvokeTest(DerivativeRet);
	InvokeTest(DerivativeAdv);
	InvokeTest(Baseline); 
	*/
//	InvokeTest(Accumulator);
}
