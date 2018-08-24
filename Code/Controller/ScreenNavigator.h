// ScreenNavigator.h

#pragma once

#include "arduino.h"
#include "Screen.h"
#include "DFR_Key.h"
#include "Mode.h"

///////////////////////////////////////////////////////////////////////////////
// Manages the Sainsmart 1602 LCD/keypad user interface. Allows the user to
// navigate through different Screen instances, based on keypad button presses.
///////////////////////////////////////////////////////////////////////////////
class ScreenNavigator
{
public:
	void Initialize(Mode *mode);
	Screen* GetCurrentScreen();
	bool Update(int key);

	// These are public so that MenuBuilder can use them
	static Screen* CombineRows(Screen **rows);
	static Screen* BuildRow(Screen **screens);

private:
	Screen* currentScreen;
	int keyProcessed;
	void Navigate(Screen* next);
};

