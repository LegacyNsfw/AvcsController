#ifdef ARDUINO
#include <Arduino.h>
#include <LiquidCrystal.h>
#endif
#include "stdafx.h"

#include <String.h>
#include "Utilities.h"
#include "SelfTest.h"

#define EXTERN
#include "Globals.h"
#include "ExhaustCamState.h"
#include "CrankState.h"
#include "Mode.h"
#include "Feedback.h"


void PrintUnsigned(char* buffer, unsigned value, int magnitude);

///////////////////////////////////////////////////////////////////////////////
// Print a value using a whole line of the LCD screen
///////////////////////////////////////////////////////////////////////////////
void PrintLong(char* buffer, unsigned value)
{
	PrintUnsigned(buffer, value, 1000 * 1000 * 1000);
}

///////////////////////////////////////////////////////////////////////////////
// Print a value using only 5 characters of the LCD screen
///////////////////////////////////////////////////////////////////////////////
void PrintShort(char* buffer, unsigned value)
{
	PrintUnsigned(buffer, value, 10 * 1000);
}

///////////////////////////////////////////////////////////////////////////////
// Common code for printing unsigned values
///////////////////////////////////////////////////////////////////////////////
void PrintUnsigned(char* buffer, unsigned value, int magnitude)
{
	int i = 0;
	int leadingSpace = true;

	for (int div = magnitude; div > 0; div /= 10)
	{
		int x = value / div;
		int mod = value % div;

		if (!leadingSpace || (x != 0) || (div == 1)) 
		{
			leadingSpace = false;
			buffer[i] = x + '0';
		}
		else
		{
			buffer[i] = ' ';
		}

		i++;
		value = mod;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Print a signed value
///////////////////////////////////////////////////////////////////////////////
void PrintSigned(char* buffer, int value)
{
	int i = 0;
	int leadingSpace = true;

	for (int div = 100000; div > 0; div /= 10)
	{
		int x = value / div;
		int mod = value % div;

		if (!leadingSpace || (x != 0) || (i == 5))
		{
			leadingSpace = false;
			buffer[i] = '0' + ((x > 0) ? x : -x);
		}
		else
		{
			if (value < 0)
			{
				buffer[i] = '-';
			}
			else
			{
				buffer[i] = ' ';
			}
		}

		i++;
		value = mod;
	}

	buffer[i] = 0;
}

void PrintFloat(char* buffer, float value)
{
	sprintf(buffer, "%+3.2f", value);
}

#ifdef ARDUINO

extern LiquidCrystal lcd;

///////////////////////////////////////////////////////////////////////////////
// Clear the LCD screen and backing buffers
///////////////////////////////////////////////////////////////////////////////
const char* empty = "                ";

void ClearScreen()
{
	lcd.setCursor(0, 0);
	lcd.print(empty);
	lcd.setCursor(0, 1);
	lcd.print(empty);

	strncpy(DisplayLine1, empty, DisplayWidth);
	strncpy(DisplayLine2, empty, DisplayWidth);
}

void ClearScreenBuffer()
{
	strncpy(DisplayLine1, empty, DisplayWidth);
	strncpy(DisplayLine2, empty, DisplayWidth);
}

#else

///////////////////////////////////////////////////////////////////////////////
// Clear the screen, but not really, because this is only used for unit tests.
///////////////////////////////////////////////////////////////////////////////
void ClearScreen()
{
	printf("#####\r\n");
}

#endif // ARDUINO

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################

int stepDelay = 25;

///////////////////////////////////////////////////////////////////////////////
// Clear a text buffer.
///////////////////////////////////////////////////////////////////////////////
void ClearBuffer(char *buffer)
{
	for (int i = 0; i < 16; i++)
	{
		buffer[i] = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Test printing "long" (10 character) numbers
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
	unsigned number;
	char* string;
} UnsignedTestValue;

bool TestPrintLong()
{
	ClearBuffer(DisplayLine1);

	UnsignedTestValue values[] = {
		{ 0,           "         0" },
		{ 1,           "         1" },
		{ 10,          "        10" },
		{ 100,         "       100" },
		{ 1000,        "      1000" },
		{ 100000,      "    100000" },
		{ 10000000,    "  10000000" },
		{ 1000000000,  "1000000000" },
		{ 4294967295,  "4294967295" },
		{ 2,           "" }
	};

	for (int i = 0; values[i].number != 2; i++)
	{
		PrintLong(DisplayLine1, values[i].number);
		SETCURSOR(0, 0);
		PRINT(DisplayLine1);
		
		if (!CompareStrings(values[i].string, DisplayLine1))
		{
			strcpy(FailureMessage, values[i].string);
			return 0;
		}

		DELAY(stepDelay);
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Test printing short (5 character) numbers
///////////////////////////////////////////////////////////////////////////////
bool TestPrintShort()
{
	ClearBuffer(DisplayLine1);

	UnsignedTestValue values[] = {
		{ 0,           "    0" },
		{ 1,           "    1" },
		{ 12,          "   12" },
		{ 123,         "  123" },
		{ 1234,        " 1234" },
		{ 12345,       "12345" },
		{ 2,           "" }
	};

	for (int i = 0; values[i].number != 2; i++)
	{
		PrintShort(DisplayLine1, values[i].number);
		SETCURSOR(0, 0);
		PRINT(DisplayLine1);

		if (!CompareStrings(values[i].string, DisplayLine1))
		{
			strcpy(FailureMessage, values[i].string);
			return 0;
		}

		DELAY(stepDelay);
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Test printing short signed numbers
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int number;
	char* string;
} SignedTestValue;

bool TestPrintSigned()
{
	ClearBuffer(DisplayLine1);

	SignedTestValue values[] = {
		{ 0,            "     0" },
		{ 1,            "     1" },
		{ -1,           "-----1" },
		{ 10,           "    10" },
		{ -10,          "----10" },
		{ 1234,         "  1234" },
		{ -1234,        "--1234" },
		{ 2,           "" } 
	};

	for (int i = 0; values[i].number != 2; i++)
	{
		PrintSigned(DisplayLine1, values[i].number);
		SETCURSOR(0, 0);
		PRINT(DisplayLine1);

		if (!CompareStrings(values[i].string, DisplayLine1))
		{
			strcpy(FailureMessage, values[i].string);
			return 0;
		}

		DELAY(stepDelay);
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the utilities code
///////////////////////////////////////////////////////////////////////////////
void SelfTestUtilities()
{
	InvokeTest(PrintLong);
	InvokeTest(PrintShort);
	InvokeTest(PrintSigned);
}
