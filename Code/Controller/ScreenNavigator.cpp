#include "ScreenNavigator.h"
#include "MenuBuilder.h"

///////////////////////////////////////////////////////////////////////////////
// Combine multiple screens into a row that can be navigated with left/right buttons.
///////////////////////////////////////////////////////////////////////////////
Screen* ScreenNavigator::BuildRow(Screen **screens)
{
	int index = 0;
	Screen *previous = NULL;
	while (screens[index] != (NULL))
	{
		Screen *current = screens[index];
		if (previous != NULL)
		{
			screens[index]->Left = previous;
			previous->Right = current;
		}

		previous = current;
		index++;
	}

	return screens[0];
}

///////////////////////////////////////////////////////////////////////////////
// Combines multiple rows, so the user can navigate between them using up/down buttons.
///////////////////////////////////////////////////////////////////////////////
Screen* ScreenNavigator::CombineRows(Screen **rows)
{
	int index = 0;
	Screen *previous = NULL;
	while (rows[index] != NULL)
	{
		Screen *current = rows[index];
		if (previous != NULL)
		{
			// Up buttons should all point at the previous screen.
			//rows[index]->Up = previous;
			for (Screen* s = rows[index]; s != null; s = s->Right)
			{
				s->Up = previous;
			}
			
			// Previous row's Down buttons should all point at the current screen.
			//previous->Down = current;
			for (Screen *s = previous; s != null; s = s->Right)
			{
				s->Down = current;
			}
		}

		previous = current;
		index++;
	}

	return rows[0];
}

///////////////////////////////////////////////////////////////////////////////
// Creates menu of screens and sets the current screen.
///////////////////////////////////////////////////////////////////////////////
void ScreenNavigator::Initialize(Mode *mode)
{
	this->currentScreen = MenuBuilder::BuildMenu();
}

///////////////////////////////////////////////////////////////////////////////
// Gets the current screen.
///////////////////////////////////////////////////////////////////////////////
Screen* ScreenNavigator::GetCurrentScreen()
{
	return this->currentScreen;
}

///////////////////////////////////////////////////////////////////////////////
// Navigate to another screen, based on a key press.
// Returns true if the current screen has changed, false if not.
// Caller can use return value to clear screen when changing screens.
///////////////////////////////////////////////////////////////////////////////
bool ScreenNavigator::Update(const int key)
{
	if (key == SAMPLE_WAIT)
	{
		return false;
	}

	if (key == NO_KEY)
	{
		this->keyProcessed = false;
		return false;
	}

	if (this->keyProcessed == true)
	{
		return false;
	}

	switch (key)
	{
	case UP_KEY:
		this->Navigate(this->currentScreen->Up);
		break;

	case DOWN_KEY:
		this->Navigate(this->currentScreen->Down);
		break;

	case LEFT_KEY:
		this->Navigate(this->currentScreen->Left);
		break;

	case RIGHT_KEY:
		this->Navigate(this->currentScreen->Right);
		break;

	default:
		return false;
	}

	this->keyProcessed = true;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Sets the current screen to the given screen (unless the given screen is null)
///////////////////////////////////////////////////////////////////////////////
void ScreenNavigator::Navigate(Screen* next)
{
	if (next != null)
	{
		this->currentScreen = next;
	}
}