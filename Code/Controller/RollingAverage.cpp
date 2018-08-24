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
	UpdateRollingAverage(&average, 1000, 4);
	UpdateRollingAverage(&average, 1005, 4);
	UpdateRollingAverage(&average, 1010, 4);
	UpdateRollingAverage(&average, 1015, 4);
	UpdateRollingAverage(&average, 1020, 4);
	UpdateRollingAverage(&average, 1025, 4);
	UpdateRollingAverage(&average, 1030, 4);
	UpdateRollingAverage(&average, 1035, 4);
	UpdateRollingAverage(&average, 1040, 4);
	UpdateRollingAverage(&average, 1045, 4);
	UpdateRollingAverage(&average, 1050, 4);

	// The specific value here is not critical. It can change with different 
	// smoothing methods, but it is tested to ensure that changes are made
	// consciously, not accidentally.
	// With a 4/1 ratio, the value here will be 1030.
	// With 1/1, 1045.
	// With 1/2, 1047
	return CompareUnsigned(average, 1030, "Rising");
}

///////////////////////////////////////////////////////////////////////////////
// Validate the rolling-average function with a slowly falling series.
///////////////////////////////////////////////////////////////////////////////
bool TestAverageFalling()
{
	int average = 1000;

	UpdateRollingAverage(&average, 1000, 4);
	UpdateRollingAverage(&average, 995, 4);
	UpdateRollingAverage(&average, 990, 4);
	UpdateRollingAverage(&average, 985, 4);
	UpdateRollingAverage(&average, 980, 4);
	UpdateRollingAverage(&average, 975, 4);
	UpdateRollingAverage(&average, 970, 4);
	UpdateRollingAverage(&average, 965, 4);
	UpdateRollingAverage(&average, 960, 4);
	UpdateRollingAverage(&average, 955, 4);
	UpdateRollingAverage(&average, 950, 4);

	// With 4/1 smoothing, the result will be 966.
	// With 1/1, 954.
	// With 1/2, 952.
	return CompareUnsigned(average, 966, "Falling");
}

///////////////////////////////////////////////////////////////////////////////
// Validate the rolling-average function with a noisy series.
///////////////////////////////////////////////////////////////////////////////
bool TestAverageSmoothing()
{
	int average = 1000;
	UpdateRollingAverage(&average, 1000, 4);
	UpdateRollingAverage(&average, 1005, 4);
	UpdateRollingAverage(&average, 1010, 4);
	UpdateRollingAverage(&average, 1005, 4);
	UpdateRollingAverage(&average, 1000, 4);
	UpdateRollingAverage(&average, 1005, 4);
	UpdateRollingAverage(&average, 1010, 4);
	UpdateRollingAverage(&average, 1005, 4);
	UpdateRollingAverage(&average, 1000, 4);
	UpdateRollingAverage(&average, 1005, 4);
	
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