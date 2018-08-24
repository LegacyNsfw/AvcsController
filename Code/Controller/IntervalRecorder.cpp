#if ARDUINO
#include <Arduino.h>
#endif

#include "stdafx.h"
#include "Globals.h"
#include "SelfTest.h"
#include "Utilities.h"
#include "IntervalRecorder.h"

#if !ARDUINO
long micros()
{
	return 0;
}
#endif

class IntervalRecorder : public IIntervalRecorder
{
private:
	long intervals[Intervals::IntervalCount];
	int line;

public:
	IntervalRecorder()
	{
		for (int i = Intervals::CrankHigh; i < Intervals::IntervalCount; i++)
		{
			intervals[i] = 0;
		}

		line = 0;
	}

	void Initialize()
	{
		// Probably don't really need this method.
	}

	long LogInterval(int id)
	{
		if (id == Intervals::CrankHigh)
		{
			long now = micros();
			long result = now - intervals[Intervals::CrankHigh];
			intervals[Intervals::CrankHigh] = now;
			return result;
		}

		long elapsed = micros() - intervals[Intervals::CrankHigh];
		intervals[id] = elapsed;
		return elapsed;
	}
	
	void WriteToSerial()
	{
#if ARDUINO
		if (!Serial.availableForWrite())
		{
			return;
		}

		line++;

		// The 'low' transitions are excluded for now because they are not consistent.
		// Need to figure out why.
		switch (line)
		{
		case 1: WriteLine("CrankHigh", Intervals::CrankHigh); break;
		case 2: WriteLine("LeftExhaustCamHigh1", Intervals::LeftExhaustCamHigh1); break;
		// case 3: WriteLine("RightExhaustCamLow2", Intervals::RightExhaustCamLow2); break;
		case 3: WriteLine("RightExhaustCamHigh1", Intervals::RightExhaustCamHigh1); break;
		// case 5: WriteLine("LeftExhaustCamLow1", Intervals::LeftExhaustCamLow1); break;
		// case 6: WriteLine("RightExhaustCamLow1", Intervals::RightExhaustCamLow1); break;
		case 4: WriteLine("LeftExhaustCamHigh2", Intervals::LeftExhaustCamHigh2); break;
		case 5: WriteLine("CrankLow", Intervals::CrankLow); break;
		// case 9: WriteLine("LeftExhaustCamLow2", Intervals::LeftExhaustCamLow2); break;
		case 6: WriteLine("RightExhaustCamHigh2", Intervals::RightExhaustCamHigh2); break;
		case 7: Serial.write("\r\n\r\n"); line = 0; break;
		}
#endif
	}

	void WriteLine(const char* name, int id)
	{
		const int lineLength = 100;
		char logData[lineLength];
		snprintf(logData, lineLength, "%-25s - %12ld\r\n", name, intervals[id]);
#if ARDUINO
		Serial.write(logData);
#endif
	}
};

static IntervalRecorder *instance = 0;

IIntervalRecorder * IIntervalRecorder::GetInstance()
{
	if (instance == NULL)
	{
		instance = new IntervalRecorder();
	}

	return instance;
}
