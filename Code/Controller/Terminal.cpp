#if ARDUINO
#include <Arduino.h>
#endif

#include "stdafx.h"
#include "Globals.h"
#include "SelfTest.h"
#include "Utilities.h"
#include "Mode.h"
#include "IntervalRecorder.h"
#include "Terminal.h"
#include "ExhaustCamState.h"
#include "CrankState.h"
#include "Feedback.h"

extern Mode mode;

class Terminal;

enum TerminalMode
{
	LogCsv = 0,
	ShowIntervals,
	ShowMenu,
	SetParameter,
};

enum Parameter
{
	None = 0,
	ProportionalGain,
	IntegralGain,
	DerivativeGain,
};

class Terminal;
typedef void (Terminal::*LogMethodPtr)();

class TerminalMenuItem
{
private:
	const char *_name;
	char _key;
	TerminalMode _mode;
	LogMethodPtr _logMethod;
	Parameter _parameter;
public:

	TerminalMenuItem(
		const char *name, 
		char key, 
		TerminalMode mode, 
		LogMethodPtr logMethod, 
		Parameter parameter)
	{
		_name = name;
		_key = key;
		_mode = mode;
		_logMethod = logMethod;
		_parameter = parameter;
	}

	const char* GetName() { return _name; }
	char GetKey() { return _key; }
	TerminalMode GetMode() { return _mode; }
	LogMethodPtr GetLogMethod() { return _logMethod; }
	Parameter GetParameter() { return _parameter; }
};

class Terminal : public ITerminal
{
private:
	TerminalMenuItem **menuItems;
	TerminalMode terminalMode;
	LogMethodPtr logMethod;
	Parameter parameter;
	int logSkipCount;

	static const int MaxLogLineLength = 1000;
	char logData[MaxLogLineLength];

public:

	virtual void Initialize()
	{
		terminalMode = TerminalMode::ShowMenu;
		logMethod = NULL;
		logSkipCount = 0;

		// This is a little bit hacky...
		//ITerminal::GetInstance();
		//PrintMenu();
	}

	// This is used to show the menu when Update is invoked for the first time.
	long showedMenu = 0;

	virtual void Update()
	{
		if (showedMenu == 0)
		{
			PrintMenu();
			showedMenu = 1;
		}

		ProcessInput();
		SendOutput();
	}

	void ProcessInput()
	{
		if (!Serial.available())
		{
			return;
		}

		int input = Serial.read();
		if (input == -1)
		{
			return;
		}

		int handled = 0;
		for (int i = 0; menuItems[i] != null; i++)
		{
			TerminalMenuItem *item = menuItems[i];
			char key = item->GetKey();
			if (input == key ||
				(isupper(key) && input == tolower(key)))
			{
				handled = 1;
				terminalMode = item->GetMode();
				switch (terminalMode)
				{
				case TerminalMode::LogCsv:
					logMethod = item->GetLogMethod();
					logSkipCount = 0;
					break;

				case TerminalMode::ShowIntervals:
					logMethod = NULL;
					break;

				case TerminalMode::SetParameter:
					logMethod = &Terminal::WriteLogLeft;
					logSkipCount = 0;
					break;

				default:
					handled = 0;
				}
			}
		}

		if (handled == 0)
		{
			PrintMenu();
		}
	}

	void SendOutput()
	{
		switch (terminalMode)
		{
		case TerminalMode::ShowMenu:
			break;

		case TerminalMode::LogCsv:
			WriteLog();
			break;

		case TerminalMode::ShowIntervals:
			IIntervalRecorder::GetInstance()->WriteToSerial();
			break;

		case TerminalMode::SetParameter:
			break;
		}
	}

	void WriteLog()
	{
		if (logMethod != NULL)
		{
			/*if (!Serial.availableForWrite())
			{
				logSkipCount++;
				return;
			}*/

			if (!(millis() % 20) == 0)
			{
				return;
			}

			(this->*logMethod)();

			Serial.write(logData);

			logSkipCount = 0;
		}
	}

	void PrintMenu()
	{
		Serial.print("\r\n");
		Serial.print("vvvvvvvvvv\r\n");

		for (int i = 0; menuItems[i] != null; i++)
		{
			TerminalMenuItem *item = menuItems[i];
			char line[100];
			snprintf(line, 100, "%c - %s\r\n", item->GetKey(), item->GetName());
			Serial.print(line);
		}

		Serial.print("^^^^^^^^^^");
		Serial.print("\r\n");
	}

	void WriteLogDefault()
	{
		snprintf(
			logData,
			MaxLogLineLength,
			"Default,%d,%d,%d,%d,%2.2f,%2.4f,%2.2f,%2.4f\r\n",
			millis(),
			mode.GetMode(),
			ErrorCount,
			OilTemperature,
			LeftExhaustCam.Angle,
			LeftFeedback.Output,
			RightExhaustCam.Angle,
			RightFeedback.Output);
	}

	void WriteLogVerbose()
	{
		snprintf(
			logData,
			MaxLogLineLength,
			"Vervose,%d,%d,%d,%d,L1,%d,%d,L2,%d,%d,R1,%d,%d,R2,%d,%d,C,%d,%d,%d\r\n",
			millis(),
			mode.GetMode(),
			ErrorCount,
			InitializationErrorCount,

			// L1
			LeftExhaustCam.Rpm,
			LeftExhaustCam.AverageInterval,

			// L2 
			LeftExhaustCam.PulseDuration,
			LeftExhaustCam.CalibrationCountdown,

			// R1
			RightExhaustCam.Rpm,
			RightExhaustCam.AverageInterval,

			// R2
			RightExhaustCam.PulseDuration,
			RightExhaustCam.CalibrationCountdown,

			// C
			Crank.Rpm,
			Crank.AverageInterval,
			Crank.PulseDuration);
	}

	void WriteLogBaseline()
	{
		snprintf(
			logData,
			MaxLogLineLength,
			"LB,%04d,%2.2f,%2.2f,RB,%04d,%2.2f,%2.2f\r\n",
			LeftExhaustCam.Rpm,
			LeftExhaustCam.Baseline,
			LeftExhaustCam.Angle,
			RightExhaustCam.Rpm,
			RightExhaustCam.Baseline,
			RightExhaustCam.Angle);
	}

	void WriteLogLeft()
	{
		snprintf(
			logData,
			MaxLogLineLength,
			"Left,%d,%d,%04d,%2.2f,%2.2f,%2.4f\r\n",
			mode.GetMode(),
			ErrorCount,
			LeftExhaustCam.Rpm,
			LeftExhaustCam.Baseline,
			LeftExhaustCam.Angle,
			LeftFeedback.Output);
	}

	void WriteLogRight()
	{
		snprintf(
			logData,
			MaxLogLineLength,
			"Right,%d,%d,%04d,%2.2f,%2.2f,%2.4f\r\n",
			mode.GetMode(),
			ErrorCount,
			RightExhaustCam.Rpm,
			RightExhaustCam.Baseline,
			RightExhaustCam.Angle,
			RightFeedback.Output);
	}

	void WriteLogCrank()
	{
		snprintf(
			logData,
			MaxLogLineLength,
			"Crank,%d,%d,%04d,%d,%d,%d,%d\r\n",
			millis(),
			mode.GetMode(),
			Crank.Rpm,
			Crank.AverageInterval,
			Crank.PulseDuration,
			Crank.AnalogValue,
			0); // Crank.AnalogHigh);
	}

	Terminal()
	{
		menuItems = new TerminalMenuItem*[12]
		{
			new TerminalMenuItem("Show Menu", 'M', TerminalMode::ShowMenu, NULL, Parameter::None),
			new TerminalMenuItem("Show Sequence", 'S', TerminalMode::ShowIntervals, NULL, Parameter::None),
			new TerminalMenuItem("Default Log", '1', TerminalMode::LogCsv, &Terminal::WriteLogDefault, Parameter::None),
			new TerminalMenuItem("Verbose Log", '2', TerminalMode::LogCsv, &Terminal::WriteLogVerbose, Parameter::None),
			new TerminalMenuItem("Baseline Log", 'B', TerminalMode::LogCsv, &Terminal::WriteLogBaseline, Parameter::None),
			new TerminalMenuItem("Left Log", 'L', TerminalMode::LogCsv, &Terminal::WriteLogLeft, Parameter::None),
			new TerminalMenuItem("Right Log", 'R', TerminalMode::LogCsv, &Terminal::WriteLogRight, Parameter::None),
			new TerminalMenuItem("Crank Log", 'C', TerminalMode::LogCsv, &Terminal::WriteLogCrank, Parameter::None),
//			new TerminalMenuItem("Show Parameters", 'P', TerminalMode::ShowParameters, NULL, Parameter::ProportionalGain),
//			new TerminalMenuItem("Adjust Proportional Gain", 'P', TerminalMode::SetParameter, NULL, Parameter::ProportionalGain),
//			new TerminalMenuItem("Adjust Integral Gain", 'I', TerminalMode::SetParameter, NULL, Parameter::IntegralGain),
//			new TerminalMenuItem("Adjust Derivative Gain", 'D', TerminalMode::SetParameter, NULL, Parameter::DerivativeGain),
			NULL
		};
	}
};

static Terminal* instance;

ITerminal* ITerminal::GetInstance()
{
	if (instance == null)
	{
		instance = new Terminal();
	}

	return instance;
}