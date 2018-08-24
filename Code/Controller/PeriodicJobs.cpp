// Invokes jobs periodically
#ifdef ARDUINO
#include <Arduino.h>
#include <LiquidCrystal.h>

extern LiquidCrystal lcd;
#endif

#include "stdafx.h"
#include "PeriodicJobs.h"
#include "Globals.h"
#include "Utilities.h"
#include "SelfTest.h"
#include "Terminal.h"

static unsigned iterationCounter;

static unsigned testJob0Count;
static unsigned testJob25Count;
static unsigned testJob50Count;
static unsigned testJob75Count;

class PeriodicJobs : public IPeriodicJobs
{
public:
	typedef struct Job
	{
		unsigned offset;
		void(*callback)(void);
		Job *nextJob;
	} Job;

	Job *firstJob;
	Job *currentJob;

	unsigned lastTick;

	unsigned (*getMilliseconds)(void);
	
	PeriodicJobs(unsigned(*timerWrapper)(void))
	{
		getMilliseconds = timerWrapper;
		firstJob = NULL;
		currentJob = NULL;
		lastTick = 0;
	}

#ifdef ARDUINO
	static void Job0()
	{
		IterationsPerSecond = iterationCounter * 10;
		iterationCounter = 0;

		// Updating a line on the LCD takes roughly 5 milliseconds of background processing
		lcd.setCursor(0, 0);
		lcd.print(DisplayLine1);
	}

	static void Job25()
	{
		digitalWrite(13, HIGH);  // turn the LED on (HIGH is the voltage level)

		ITerminal::GetInstance()->Update();
	}

	static void Job50()
	{
		// Updating a line on the LCD takes roughly 5 milliseconds of background processing
		lcd.setCursor(0, 1);
		lcd.print(DisplayLine2);
	}

	static void Job75()
	{
		digitalWrite(13, LOW);   // turn the LED off by making the voltage LOW

		ITerminal::GetInstance()->Update();
	}

	void Initialize()
	{
		AddJob(0, Job0);
		AddJob(25, Job25);
		AddJob(50, Job50);
		AddJob(75, Job75);
	}
#else 
	void Initialize() { }
#endif

	static void TestJob0()  { testJob0Count++; }
	static void TestJob25() { testJob25Count++; }
	static void TestJob50() { testJob50Count++; }
	static void TestJob75() { testJob75Count++; }

	void TestInitialize()
	{
		AddJob(0, TestJob0);
		AddJob(25, TestJob25);
		AddJob(50, TestJob50);
		AddJob(75, TestJob75);
	}

	void AddJob(unsigned offset, void(*callback)(void))
	{
		Job *newJob = new Job();
		newJob->offset = offset;
		newJob->callback = callback;
		newJob->nextJob = NULL;

		if (firstJob == NULL)
		{
			firstJob = newJob;
			return;
		}

		Job* job = firstJob;
		while (job->nextJob != null)
		{
			job = job->nextJob;
		}

		job->nextJob = newJob;
	}

	virtual void Update()
	{
		iterationCounter++;

		if (currentJob == NULL)
		{
			currentJob = firstJob;
		}

		if (currentJob == NULL)
		{
			return;
		}

		unsigned milliseconds = getMilliseconds();
		if ((milliseconds % 100 == 0) && (milliseconds > lastTick))
		{
			lastTick = milliseconds;
		}

		// There's an assumption here that Update is called at least once per millisecond.
		// If that assumption is violated, jobs will be skipped.
		if (milliseconds % 100 == currentJob->offset)
		{
			currentJob->callback();
			currentJob = currentJob->nextJob;
		}
	}

	static unsigned GetClock()
	{
#ifdef ARDUINO
		return millis();
#else
		return 0;
#endif
	}
};

///////////////////////////////////////////////////////////////////////////////
// Factory method
///////////////////////////////////////////////////////////////////////////////
IPeriodicJobs* IPeriodicJobs::GetInstance()
{
	return new PeriodicJobs(PeriodicJobs::GetClock);
}

///////////////////////////////////////////////////////////////////////////////
// Test tools
///////////////////////////////////////////////////////////////////////////////

unsigned testClock;

unsigned GetTestClock()
{
	return testClock;
}

IPeriodicJobs* GetPeriodicJobsForTesting()
{
	testJob0Count = 0;
	testJob25Count = 0;
	testJob50Count = 0;
	testJob75Count = 0;

	return new PeriodicJobs(GetTestClock);
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################

bool TestInitializeJobs()
{
	PeriodicJobs *instance = (PeriodicJobs*)GetPeriodicJobsForTesting();
	instance->TestInitialize();
	return true;
}

bool TestExecuteJobs()
{
	testClock = 100;
	PeriodicJobs *instance = (PeriodicJobs*)GetPeriodicJobsForTesting();
	instance->TestInitialize();
	instance->Update();
	instance->Update();
	testClock = 101;
	instance->Update();
	testClock = 25;
	instance->Update();
	instance->Update();
	testClock = 50;
	instance->Update();
	instance->Update();
	testClock = 75;
	instance->Update();

	if (!CompareUnsigned(testJob0Count, 1, "Job0"))
	{
		return false;
	}

	if (!CompareUnsigned(testJob25Count, 1, "Job25"))
	{
		return false;
	}

	if (!CompareUnsigned(testJob50Count, 1, "Job50"))
	{
		return false;
	}

	if (!CompareUnsigned(testJob75Count, 1, "Job75"))
	{
		return false;
	}

	return true;
}

bool TestExecuteStress()
{
	PeriodicJobs *instance = (PeriodicJobs*)GetPeriodicJobsForTesting();
	instance->TestInitialize();

	for (int clock = 99; clock < 399; clock++)
	{
		for (int iterations = 0; iterations < 4; iterations++)
		{
			testClock = clock;
			instance->Update();
		}
	}

	if (!CompareUnsigned(testJob0Count, 3, "Job0"))
	{
		return false;
	}

	if (!CompareUnsigned(testJob25Count, 3, "Job25"))
	{
		return false;
	}

	if (!CompareUnsigned(testJob50Count, 3, "Job50"))
	{
		return false;
	}

	if (!CompareUnsigned(testJob75Count, 3, "Job75"))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the PeriodicJobs code.
///////////////////////////////////////////////////////////////////////////////
void SelfTestPeriodicJobs()
{
	InvokeTest(InitializeJobs);
	InvokeTest(ExecuteJobs);
	InvokeTest(ExecuteStress);
}