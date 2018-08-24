// UnitTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Mode.h"
#include "Screen.h"
#include "Utilities.h"
#include "SelfTest.h"
#include <math.h>

Mode mode;
Screen screen;
Screen *ErrorScreen = new Screen();

const unsigned TicksPerSecond = 1000 * 1000;
const unsigned TicksPerMinute = TicksPerSecond * 60;;

#define NUM_TIMERS 9

#define VARIANT_MCK			84000000
double _frequency[NUM_TIMERS] = { -1,-1,-1,-1,-1,-1,-1,-1,-1 };

unsigned bestClock(double frequency, unsigned *retRC) {
	/*
	Pick the best Clock, thanks to Ogle Basil Hall!

	Timer		Definition
	TIMER_CLOCK1	MCK /  2
	TIMER_CLOCK2	MCK /  8
	TIMER_CLOCK3	MCK / 32
	TIMER_CLOCK4	MCK /128
	*/
	const struct {
		unsigned flag;
		unsigned divisor;
	} clockConfig[] = {
		{ 0,   2 },
		{ 1,   8 },
		{ 2,  32 },
		{ 3, 128 }
	};
	float ticks;
	float error;
	int clkId = 3;
	int bestClock = 3;
	float bestError = 3.4e38;
	do
	{
		ticks = (float)VARIANT_MCK / frequency / (float)clockConfig[clkId].divisor;
		// error = abs(ticks - round(ticks));
		error = clockConfig[clkId].divisor * fabs((float)(ticks - round(ticks)));	// Error comparison needs scaling
		if (error < bestError)
		{
			bestClock = clkId;
			bestError = error;
		}
	} while (clkId-- > 0);
	ticks = (float)VARIANT_MCK / frequency / (float)clockConfig[bestClock].divisor;
	*retRC = (unsigned)round(ticks);
	return clockConfig[bestClock].flag;
}

int main()
{
	unsigned ret;
	unsigned clock = bestClock(42 * 1000 * 1000, &ret);

	SelfTest();
	printf("\r\n");
	printf("Press enter to exit.");
	fgetc(stdin);
    return 0;
}

