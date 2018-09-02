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

	// Oscillates slightly (when feedback is calculated in the main loop)
//	ProportionalGain = 10.0f;
//	IntegralGain = 1.0f;
//	DerivativeGain = 0.0001f;

	// Oscillates rarely, but is slow to find target
//	ProportionalGain = 0.75f;
//	IntegralGain = 0.40f;
//	DerivativeGain = 0.0001f;

	// This works fairly well, but is prone to big overshoots at times, and it takes a while to recover from them.
//	ProportionalGain = 0.5f;
//	IntegralGain = 0.50f;
//	DerivativeGain = 0.0001f;

	// Also overshoots with sudden 5% target changes
	// But worked well without sudden target changes
//	ProportionalGain = 1.0f; 
//	IntegralGain = 0.30f; 
//	DerivativeGain = 0.001f;

	// Proportional gain, with 0.3f integral:
	// 1.2f = oscillation
	// 1.0f = overshoots on abrupt changes (e.g. 5% target at cruise RPM), but works well if no abrupt changes (e.g. 0% target at cruise RPM)
	// 0.25 = 
	// Integral gain of 0.3f moves DC approx 1% per second.
	//
	// Derivative? Dunno yet.

	// P=.25  D=.50 = overshoots on sudden chanages
	// P=.10  D=.30 = overshoots on sudden changes

	// Worked well when feedback was updated with every interation of the main loop
	ProportionalGain = 1.0f; 
	IntegralGain = 0.5f; 
	DerivativeGain = 0.001f;

	ProportionalGain = 1.0f;
	IntegralGain = 7.5f;
	DerivativeGain = 0.001f;

	// Try more P, less I
	// Try more D

	// Reset state variables
	ProportionalTerm = 0;
	IntegralTerm = 0;
	DerivativeTerm = 0;

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
	
	ProportionalTerm = error * ProportionalGain;
	IntegralTerm += (error * time) * IntegralGain;
	DerivativeTerm = DerivativeGain * (errorChange / time);

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

	UpdateRollingAverage(&(Average[bucket]), Output, 5);

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
