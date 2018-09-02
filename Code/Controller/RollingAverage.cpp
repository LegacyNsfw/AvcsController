///////////////////////////////////////////////////////////////////////////////
// Code to handle the "three minus one" timing pattern of intake cams.
// This has only been tested with Simulator_IntakeCams.ino, not with a car.
///////////////////////////////////////////////////////////////////////////////

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "stdafx.h"
#include "RollingAverage.h"
#include "SelfTest.h"


float GetRollingAverageWeight(int rpm)
{
	if (rpm > 2000)
	{
		return 1.0f;
	}

	return 0.1f;
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################

///////////////////////////////////////////////////////////////////////////////
// Validate the rolling-average function with a slowly rising series.
///////////////////////////////////////////////////////////////////////////////
bool TestAverageRising()
{
	int average = 1000;
	UpdateRollingAverage(&average, 1000, 0.25);
	UpdateRollingAverage(&average, 1005, 0.25);
	UpdateRollingAverage(&average, 1010, 0.25);
	UpdateRollingAverage(&average, 1015, 0.25);
	UpdateRollingAverage(&average, 1020, 0.25);
	UpdateRollingAverage(&average, 1025, 0.25);
	UpdateRollingAverage(&average, 1030, 0.25);
	UpdateRollingAverage(&average, 1035, 0.25);
	UpdateRollingAverage(&average, 1040, 0.25);
	UpdateRollingAverage(&average, 1045, 0.25);
	UpdateRollingAverage(&average, 1050, 0.25);

	// The specific value here is not critical. It can change with different 
	// smoothing methods, but it is tested to ensure that changes are made
	// consciously, not accidentally.
	// With a 4/1 ratio, the value here will be 1030.
	// With 1/1, 1045.
	// With 1/2, 1047
	return CompareUnsigned(average, 1033, "Rising");
}

///////////////////////////////////////////////////////////////////////////////
// Validate the rolling-average function with a slowly falling series.
///////////////////////////////////////////////////////////////////////////////
bool TestAverageFalling()
{
	int average = 1000;

	UpdateRollingAverage(&average, 1000, 0.25);
	UpdateRollingAverage(&average, 995, 0.25);
	UpdateRollingAverage(&average, 990, 0.25);
	UpdateRollingAverage(&average, 985, 0.25);
	UpdateRollingAverage(&average, 980, 0.25);
	UpdateRollingAverage(&average, 975, 0.25);
	UpdateRollingAverage(&average, 970, 0.25);
	UpdateRollingAverage(&average, 965, 0.25);
	UpdateRollingAverage(&average, 960, 0.25);
	UpdateRollingAverage(&average, 955, 0.25);
	UpdateRollingAverage(&average, 950, 0.25);

	// With 4/1 smoothing, the result will be 966.
	// With 1/1, 954.
	// With 1/2, 952.
	return CompareUnsigned(average, 962, "Falling");
}

///////////////////////////////////////////////////////////////////////////////
// Validate the rolling-average function with a noisy series.
///////////////////////////////////////////////////////////////////////////////
bool TestAverageSmoothing()
{
	int average = 1000;
	UpdateRollingAverage(&average, 1000, 0.25);
	UpdateRollingAverage(&average, 1005, 0.25);
	UpdateRollingAverage(&average, 1010, 0.25);
	UpdateRollingAverage(&average, 1005, 0.25);
	UpdateRollingAverage(&average, 1000, 0.25);
	UpdateRollingAverage(&average, 1005, 0.25);
	UpdateRollingAverage(&average, 1010, 0.25);
	UpdateRollingAverage(&average, 1005, 0.25);
	UpdateRollingAverage(&average, 1000, 0.25);
	UpdateRollingAverage(&average, 1005, 0.25);
	
	// With 4/1 smoothing, the result will be 1001.
	// With 1/1, 1003.
	// With 1/2, 1002.
	return CompareUnsigned(average, 1001, "Smoothing");
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the cam timing code.
///////////////////////////////////////////////////////////////////////////////
void SelfTestRollingAverage()
{
	InvokeTest(AverageRising);
	InvokeTest(AverageFalling);
	InvokeTest(AverageSmoothing);
}