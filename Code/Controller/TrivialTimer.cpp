// TrivialTimer.cpp - Timer based on calls to micros()

#include <Arduino.h>
#include "TrivialTimer.h"

TrivialTimer::TrivialTimer()
{
}

void TrivialTimer::attachInterrupt(void (*isr)(unsigned))
{	
}

void TrivialTimer::detachInterrupt(void)
{
}

void TrivialTimer::start()
{
	startTime = micros();
}

void TrivialTimer::stop(void)
{
}

void TrivialTimer::configure(unsigned timeout)
{
}

unsigned TrivialTimer::getElapsed(void) const 
{
	unsigned currentTime = micros();
	if (currentTime > startTime)
	{
		return currentTime - startTime;
	}
	else
	{
		unsigned a = 0xFFFFFFFF - startTime;
		unsigned b = currentTime;
		return a + b;
	}
}
