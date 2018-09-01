#pragma once

#include "stdafx.h"
#include <string.h>
#include "Globals.h"
#include "Utilities.h"
#include "Mode.h"

///////////////////////////////////////////////////////////////////////////////
// Base class from which real screen types are derived.
///////////////////////////////////////////////////////////////////////////////
class Screen
{
public:
	Screen()
	{
		this->Up = NULL;
		this->Down = NULL;
		this->Left = NULL;
		this->Right = NULL;
	}

	Screen* Up;
	Screen* Down;
	Screen* Left;
	Screen* Right;

	virtual void Update() {};
};

///////////////////////////////////////////////////////////////////////////////
// A screen that displays one line of text and one unsigned value
// Value can be 10 digits - suitable for raw timer/counter values.
///////////////////////////////////////////////////////////////////////////////
class SingleValueScreen : public Screen
{
private:
	const char *_topLine;
	unsigned *_value;

public:
	SingleValueScreen(const char *topLine, unsigned *value)
	{
		_topLine = topLine;
		_value = value;
	}

	void Update()
	{
		strncpy(DisplayLine1, _topLine, DisplayWidth);
		if (_value != null)
		{
			PrintLong(DisplayLine2, *_value);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// A screen that displays one line of text and one floating-point value
// Value can be 10 digits - suitable for raw timer/counter values.
///////////////////////////////////////////////////////////////////////////////
class SingleValueScreenF : public Screen
{
private:
	const char *_topLine;
	float *_value;

public:
	SingleValueScreenF(const char *topLine, float *value)
	{
		_topLine = topLine;
		_value = value;
	}

	void Update()
	{
		strncpy(DisplayLine1, _topLine, DisplayWidth);
		if (_value != null)
		{
			PrintFloat(DisplayLine2, *_value);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Screen that displays one line of text and two unsigned values
// Values can be 5 digits at most, e.g. RPM displays
///////////////////////////////////////////////////////////////////////////////
class TwoValueScreen : public Screen
{
private:
	char *_topLine;
	unsigned *_value1;
	unsigned *_value2;
public:
	TwoValueScreen(char* topLine, unsigned *value1, unsigned *value2)
	{
		_topLine = topLine;
		_value1 = value1;
		_value2 = value2;
	}

	void Update()
	{
		strncpy(DisplayLine1, _topLine, DisplayWidth);
		PrintShort(DisplayLine2, *_value1);

		if (_value2 != NULL)
		{
			PrintShort(&(DisplayLine2[8]), *_value2);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Screen that displays one line of text and two unsigned values
// Values can be 5 digits at most, e.g. RPM displays
///////////////////////////////////////////////////////////////////////////////
class TwoValueScreenF : public Screen
{
private:
	char *_topLine;
	float *_value1;
	float *_value2;
public:
	TwoValueScreenF(char* topLine, float *value1, float *value2)
	{
		_topLine = topLine;
		_value1 = value1;
		_value2 = value2;
	}

	void Update()
	{
		strncpy(DisplayLine1, _topLine, DisplayWidth);

		PrintSigned(DisplayLine2, (int)*_value1);

		if (_value2 != NULL)
		{
			PrintSigned(&(DisplayLine2[8]), (int)*_value2);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Screen that displays one line of text and two signed values.
///////////////////////////////////////////////////////////////////////////////
class TwoSignedValueScreen : public Screen
{
private:
	char *_topLine;
	int *_value1;
	int *_value2;
public:
	TwoSignedValueScreen(char* topLine, int *value1, int *value2)
	{
		_topLine = topLine;
		_value1 = value1;
		_value2 = value2;
	}

	void Update()
	{
		strncpy(DisplayLine1, _topLine, DisplayWidth);
		PrintSigned(DisplayLine2, *_value1);

		if (_value2 != NULL)
		{
			PrintSigned(&(DisplayLine2[8]), *_value2);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Screen that displays one line of text and three unsigned values.
// Values can be 5 digits at most - suitable for RPM values.
///////////////////////////////////////////////////////////////////////////////
class ThreeValueScreen : public Screen
{
private:
	char *_topLine;
	unsigned *_value1;
	unsigned *_value2;
	unsigned *_value3;

public:
	ThreeValueScreen(char* topLine, unsigned *value1, unsigned *value2, unsigned *value3)
	{
		_topLine = topLine;
		_value1 = value1;
		_value2 = value2;
		_value3 = value3;
	}

	void Update()
	{
		strncpy(DisplayLine1, _topLine, DisplayWidth);
		PrintShort(DisplayLine2, *_value1);
		PrintShort(&(DisplayLine2[5]), *_value2);
		PrintShort(&(DisplayLine2[10]), *_value3);
	}
};

///////////////////////////////////////////////////////////////////////////////
// One line of text, two FIXED unsigned values, one floating point value.
// TODO: put a decimal point in the floating-point value.
///////////////////////////////////////////////////////////////////////////////
class ThreeValueScreenUUF : public Screen
{
private:
	char *_topLine;
	unsigned _value1;
	unsigned _value2;
	float *_value3;

public:
	ThreeValueScreenUUF(char* topLine, unsigned value1, unsigned value2, float *value3)
	{
		_topLine = topLine;
		_value1 = value1;
		_value2 = value2;
		_value3 = value3;
	}

	void Update()
	{
		strncpy(DisplayLine1, _topLine, DisplayWidth);
		PrintShort(DisplayLine2, _value1);
		PrintShort(&(DisplayLine2[5]), _value2);
		PrintFloat(&(DisplayLine2[10]), *_value3);
	}
};

///////////////////////////////////////////////////////////////////////////////
// Screen that displays one value on each line. Values can be 10 digits.
///////////////////////////////////////////////////////////////////////////////
class TwoLongValueScreen : public Screen
{
private:
	unsigned *_value1;
	unsigned *_value2;
public:
	TwoLongValueScreen(unsigned *value1, unsigned *value2)
	{
		_value1 = value1;
		_value2 = value2;
	}

	void Update()
	{
		PrintLong(DisplayLine1, *_value1);
		PrintLong(DisplayLine2, *_value2);
	}
};

///////////////////////////////////////////////////////////////////////////////
// Displays two values on each line of the LCD screen
///////////////////////////////////////////////////////////////////////////////
class FourValueScreen : public Screen
{
private:
	unsigned *_value1;
	unsigned *_value2;
	unsigned *_value3;
	unsigned *_value4;
public:
	FourValueScreen(unsigned *value1, unsigned *value2, unsigned *value3, unsigned *value4)
	{
		_value1 = value1;
		_value2 = value2;
		_value3 = value3;
		_value4 = value4;
	}

	void Update()
	{
		PrintShort(DisplayLine1, *_value1);
		PrintShort(&(DisplayLine1[8]), *_value2);
		PrintShort(DisplayLine2, *_value3);
		PrintShort(&(DisplayLine2[8]), *_value4);
	}
};

///////////////////////////////////////////////////////////////////////////////
// This screen displays different data depending on the current mode.
///////////////////////////////////////////////////////////////////////////////
class MainScreen : public Screen
{
private:
	Mode *_mode;

	Screen *_syncScreen;
	Screen *_warmingScreen;
	Screen *_runningScreen;

public:
	MainScreen(Mode *mode, Screen *syncScreen, Screen *warmingScreen, Screen *runningScreen)
	{
		_mode = mode;
		_syncScreen = syncScreen;
		_warmingScreen = warmingScreen;
		_runningScreen = runningScreen;
	}

	void Update()
	{
		if (ErrorMessage[0] != 0)
		{
			strncpy(DisplayLine1, ErrorMessage, DisplayWidth);
			strncpy(DisplayLine2, LastErrorMessage, DisplayWidth);
			PrintShort((char*)((int)DisplayLine2 + 10), ErrorCount);
			return;
		}

		switch (_mode->GetMode())
		{
		case Mode::Calibrating:
			this->_syncScreen->Update();
			return;

		case Mode::Warming:
			this->_warmingScreen->Update();
			return;

		case Mode::Running:
			this->_runningScreen->Update();
			return;
		}
	}
};
