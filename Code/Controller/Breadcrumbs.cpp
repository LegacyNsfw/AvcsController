#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "stdafx.h"
#include <stdio.h>
#include "Mode.h"
#include "Breadcrumbs.h"
#include "SelfTest.h"

#ifndef ARDUINO
static unsigned mockTime;
unsigned micros()
{
	return ++mockTime;
}
#endif

const int MaxBreadcrumbs = 50;
const int MaxBreadcrumbLength = 20;
const int MaxBreadcrumbLineLength = MaxBreadcrumbs * MaxBreadcrumbLength;
char breadcrumbLogData[MaxBreadcrumbLineLength];

IBreadcrumbs *singleton;

class Breadcrumbs : public IBreadcrumbs
{
public:

	class Event
	{
	public:
		unsigned time;
		char* id;
		unsigned value;
	};

	Event array[MaxBreadcrumbs] = {};
	unsigned eventIndex;
	int readOnly;
	int enabled;
	
	Breadcrumbs()
	{
		readOnly = 0;
		enabled = 0;
		Initialize();
	}

	void Initialize()
	{
		enabled = 0;

		// "Discourage" ISRs from making changes (not deterministic, but hopefully good enough)
		readOnly++;

		for (int index = 0; index < MaxBreadcrumbs; index++)
		{
			array[index].id = 0;
			array[index].value = 0;
		}

		eventIndex = 0;

		readOnly--;
	}

	void AddEvent(char* id, unsigned value)
	{
		if (!enabled || readOnly || IsFull())
		{
			return;
		}

		array[eventIndex].time = micros();
		array[eventIndex].id = id;
		array[eventIndex].value = value;

		eventIndex++;
	}

	void Enable()
	{
		enabled = 1;
	}

	int IsEnabled()
	{
		return enabled;
	}

	int IsFull()
	{
		return (eventIndex > (MaxBreadcrumbs - 5));
	}

	void WriteAndReset()
	{
		readOnly++;

		int logIndex = 0;
		breadcrumbLogData[logIndex] = 0;

		for (int index = 0; index < eventIndex && index < (MaxBreadcrumbs - 5); index++)
		{
			int length = sprintf(
				&(breadcrumbLogData[logIndex]),
				"%d,%s,%d\r\n",
				array[index].time,
				array[index].id,
				array[index].value);

			if (length >= MaxBreadcrumbLength)
			{
				mode.Fail(array[index].id);

				// Not decrementing readOnly.
				// Deliberately leaving this in a read-only state.
				return;
			}

			logIndex += length;
		}

		Initialize();

		readOnly--;

#ifdef ARDUINO
		Serial.println(breadcrumbLogData);
#endif
	}
};

IBreadcrumbs* IBreadcrumbs::GetInstance()
{
	if (singleton == NULL)
	{
		singleton = new Breadcrumbs();
	}

	return singleton;
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################


///////////////////////////////////////////////////////////////////////////////
// Test printing "long" (10 character) numbers
///////////////////////////////////////////////////////////////////////////////

bool TestDisabled()
{
	Breadcrumbs test;
	test.Initialize();
	test.AddEvent("test", 123);

	if (!CompareUnsigned(test.eventIndex, 0, "eventIndex"))
	{
		return false;
	}

	return true;
}

bool TestAddNone()
{
	Breadcrumbs test;
	test.Initialize();

	if (!CompareUnsigned(test.eventIndex, 0, "eventIndex"))
	{
		return false;
	}

	if (!CompareStrings("", breadcrumbLogData))
	{
		return false;
	}

	return true;
}

bool TestAddSingle()
{
	Breadcrumbs test;
	test.Initialize();
	test.Enable();

	test.AddEvent("test", 123);

	if (!CompareUnsigned(test.eventIndex, 1, "eventIndex"))
	{
		return false;
	}

	test.WriteAndReset();

	if (!CompareUnsigned(test.eventIndex, 0, "eventIndex"))
	{
		return false;
	}

	if (!CompareStrings("test=123, ", breadcrumbLogData))
	{
		return false;
	}

	return true;
}

bool TestAddMultiple()
{
	Breadcrumbs test;
	test.Initialize();
	test.Enable();
	test.AddEvent("one", 123);
	test.AddEvent("two", 456);

	if (!CompareUnsigned(test.eventIndex, 2, "eventIndex"))
	{
		return false;
	}

	test.WriteAndReset();

	if (!CompareUnsigned(test.eventIndex, 0, "eventIndex"))
	{
		return false;
	}

	if (!CompareStrings("one=123, two=456, ", breadcrumbLogData))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the breadcrumb code
///////////////////////////////////////////////////////////////////////////////
void SelfTestBreadcrumbs()
{
	InvokeTest(Disabled);
	InvokeTest(AddNone);
	InvokeTest(AddSingle);
	InvokeTest(AddMultiple);
}
