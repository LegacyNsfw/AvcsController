#ifdef ARDUINO
#include <Arduino.h>
#include <LiquidCrystal.h>
extern LiquidCrystal lcd;
#endif

#include "stdafx.h"
#include <string.h>
#include "Globals.h"
#include "SelfTest.h"
#include "Utilities.h"
#include "Mode.h"
#include "ExhaustCamState.h"
#include "PlxProcessor.h"
#include "Feedback.h"
#include "PeriodicJobs.h"
#include "RollingAverage.h"
#include "CurveTable.h"

void SelfTestGetGainModifier();

int anyFailed;
const int assertDelay = 50;

#define RunSuite(x) CLEAR(); DELAY(25); SETCURSOR(0, 0); PRINT(#x); SelfTest##x();

///////////////////////////////////////////////////////////////////////////////
// Run all test suites
///////////////////////////////////////////////////////////////////////////////
void SelfTest()
{
#if ARDUINO
	lcd.clear();
#endif

	SETCURSOR(0, 0);
	PRINT("Self test...");
	DELAY(100);

	RunSuite(SelfTest);
	//RunSuite(Breadcrumbs);
	RunSuite(Utilities);
	RunSuite(Mode);
	RunSuite(RollingAverage);
	//RunSuite(IntakeCamTiming);
	RunSuite(ExhaustCamTiming);
	RunSuite(PlxProcessor);
	RunSuite(Feedback);
	RunSuite(PeriodicJobs);
	RunSuite(CurveTable);
	RunSuite(GetGainModifier);

#if ARDUINO
	lcd.clear();

	Serial.write("All tests passed.\r\n");

	SETCURSOR(0, 0);
	PRINT("Self test done. ");
	SETCURSOR(0, 1);
	PRINT("All tests passed");
	DELAY(100);
#else
	if (anyFailed)
	{
		printf("\r\n\r\n  There were failures!");
	}
	else
	{
		printf("\r\n\r\n  All tests passed.");
	}
#endif
}

#ifndef ARDUINO
const char Banner[] = "######################################################\r\n";
const char Separator[] = "\r\n\r\n";

///////////////////////////////////////////////////////////////////////////////
// Print the top of an error banner
///////////////////////////////////////////////////////////////////////////////
void BeginErrorBanner()
{
	printf(Separator);
	printf(Banner);
}

///////////////////////////////////////////////////////////////////////////////
// Print the bottom of an error banner
///////////////////////////////////////////////////////////////////////////////
void EndErrorBanner()
{
	printf(Banner);
	printf(Separator);
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Invoke a single test case
///////////////////////////////////////////////////////////////////////////////
bool InvokeTestCase(const char *name, bool(*test)(void))
{
	FailureMessage[0] = 0;

	SETCURSOR(0, 1);
	PRINT(name);
	DELAY(5);

#if ARDUINO
	Serial.write(name);
	Serial.write("\r\n");
#else
	if (strlen(name) > 16)
	{
		BeginErrorBanner();
		printf("#### %s is %d characters too long \r\n", name, strlen(name) - 16);
		EndErrorBanner();
		anyFailed = true;
	}
#endif

	bool result = (*test)();

	if (result == true)
	{
		DELAY(25);
		return result;
	}

	anyFailed = true;

#if ARDUINO
	CLEAR();
	SETCURSOR(0, 0);
	PRINT(name);
	SETCURSOR(0, 1);
	PRINT(FailureMessage);

	pinMode(13, OUTPUT); // onboard LED

	while (1)
	{
		digitalWrite(13, HIGH); // LED on
		delay(500);
		digitalWrite(13, LOW);  // LED off
		delay(1000);
	}
#else
	BeginErrorBanner();
	printf("#### %s: %s ", name, FailureMessage);
	EndErrorBanner();
#endif

	return result;
}

///////////////////////////////////////////////////////////////////////////////
// This can be handy when debugging tests on Arduino
///////////////////////////////////////////////////////////////////////////////
void ShowProgress(char* msg, int step)
{
#if ARDUINO
	Serial.print("Note: ");
	Serial.print(msg);
	Serial.print(": ");
	Serial.println(step);
#endif

	CLEAR();
	strcpy(DisplayLine1, "                ");
	PrintShort(DisplayLine1, step);
	SETCURSOR(0, 0);
	PRINT(DisplayLine1);
	SETCURSOR(0, 1);
	PRINT(msg);
	DELAY(1000);
}

///////////////////////////////////////////////////////////////////////////////
// Compare two strings
///////////////////////////////////////////////////////////////////////////////
bool CompareStrings(char* expected, char *actual)
{
	if (strlen(expected) != strlen(actual))
	{
		sprintf(FailureMessage, "%s", "CompareStrings00");
		return false;
	}

	for (int i = 0; expected[i] != 0; i++)
	{
		if (expected[i] != actual[i])
		{
			sprintf(FailureMessage, "%s", "CompareStringsNN");
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Compare two unsigned integers
///////////////////////////////////////////////////////////////////////////////
bool CompareUnsigned(unsigned actual, unsigned expected, const char *message)
{
	if (actual == expected)
	{
		return true;
	}

	sprintf(FailureMessage, "%s=%d", message, actual);
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Set FailureMessage after a test case fails
///////////////////////////////////////////////////////////////////////////////
void TestFailed(char *message)
{
	strcpy(FailureMessage, message);
}

///////////////////////////////////////////////////////////////////////////////
// Set FailureMessage with a parameter after a test case fails
///////////////////////////////////////////////////////////////////////////////
void TestFailed(char *message1, char *message2)
{
	sprintf(FailureMessage, "%s %s", message1, message2);
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################

///////////////////////////////////////////////////////////////////////////////
// Validate CompareStrings 
///////////////////////////////////////////////////////////////////////////////
bool TestCompareStrings()
{
	bool result = CompareStrings("", "");
	if (!result)
	{
		TestFailed("Empty String");
		return false;
	}

	result = CompareStrings(" ", " ");
	if (!result)
	{
		TestFailed("Whitespace");
		return false;
	}

	result = CompareStrings("test", "test");
	if (!result)
	{
		TestFailed("Simple");
		return false;
	}

	result = CompareStrings("a", "b");
	if (result)
	{
		TestFailed("a != b");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Validate CompareUnsigned
///////////////////////////////////////////////////////////////////////////////
bool TestCompareUnsigned()
{
	bool result = CompareUnsigned(0, 0, "zero");
	if (!result)
	{
		TestFailed("zero");
		return false;
	}

	result = CompareUnsigned(1, 2, "selftest");
	if (result)
	{
		TestFailed("1 == 2");
		return false;
	}

	if (!CompareStrings("selftest=1", FailureMessage))
	{
		TestFailed("FailureMsg");
		return false;
	}

	FailureMessage[0] = 0;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Validate WithinOnePercent
///////////////////////////////////////////////////////////////////////////////
bool TestOnePercent()
{
	bool result = WithinOnePercent(100, 100, "simple");
	if (!result)
	{
		TestFailed("1% 100");
		return false;
	}

	result = WithinOnePercent(99, 100, "simple");
	if (!result)
	{
		TestFailed("1% 99");
		return false;
	}

	result = WithinOnePercent(101, 100, "simple");
	if (!result)
	{
		TestFailed("1% 101");
		return false;
	}
	
	result = WithinOnePercent(102, 100, "simple");
	if (result)
	{
		TestFailed("1% 102");
		return false;
	}
	
	result = WithinOnePercent(98, 100, "simple");
	if (result)
	{
		TestFailed("1% 98");
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Make sure we catch test cases with very long names
//
// This will print a banner every time, so it is only uncommented while
// actively tinkering with the banner-printing code.
///////////////////////////////////////////////////////////////////////////////
bool TestCaseWithVeryLongName()
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the self-test utilities.
///////////////////////////////////////////////////////////////////////////////
void SelfTestSelfTest()
{
	InvokeTest(CompareStrings);
	InvokeTest(CompareUnsigned);
	InvokeTest(OnePercent);
//	InvokeTest(CaseWithVeryLongName);
}