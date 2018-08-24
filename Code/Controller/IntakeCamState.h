#pragma once

///////////////////////////////////////////////////////////////////////////////
// Holds the state for a single intake cam and its associated pulse train
// 
// TODO: migrate IntakeCamTiming to interface pattern
///////////////////////////////////////////////////////////////////////////////
class IntakeCamTiming
{
private:
	enum CountdownStates
	{
		Reset,
		Countdown1,
		Initialize1,
		Initialize2,
		Countdown2,
		Run,
	};

	void RollingAverage(unsigned *average, unsigned newValue);

public:
	unsigned Left;
	unsigned AverageInterval;
	unsigned ShortInterval;
	unsigned LongInterval;
	unsigned PulseDuration;
	unsigned IntervalState; // 0 = long interval, 1 = first short interval, 2 = second short interval
	unsigned Rpm;
	unsigned SyncCountdown; // May go slightly negative due to race conditions
    CountdownStates CountdownState;
	unsigned TimeSinceCrankSignal;
	unsigned Baseline; 
	unsigned Angle;
	unsigned PinState; // set by the .ino code, should match PulseState
	unsigned PulseState; // set by the interrupt handler, should match PinState
	unsigned Timeout;

	IntakeCamTiming(int left)
	{
		CountdownState = CountdownStates::Reset;
		IntervalState = 0;
		Left = left;
		AverageInterval = 0;
		ShortInterval = 0;
		LongInterval = 0;
		PulseDuration = 0;
		IntervalState = 0;
		Rpm = 0;
		SyncCountdown = 0;
		TimeSinceCrankSignal = 0;
		Baseline = 0;
		Angle = 0;
		PinState = 0;
		PulseState = 0;
		Baseline = 0;
		Timeout = 0;
	}

	void BeginPulse(unsigned camInterval, unsigned crankInterval);
	void EndPulse(unsigned camInterval);

	// Clean up if wraparound happened due to a race condition
	void Process()
	{
		if (SyncCountdown > 10000)
		{
			SyncCountdown = 0;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Self-test the cam timing code
///////////////////////////////////////////////////////////////////////////////
void SelfTestIntakeCamTiming();

///////////////////////////////////////////////////////////////////////////////
// Global instances of CamTiming
///////////////////////////////////////////////////////////////////////////////
extern IntakeCamTiming LeftIntakeCam;
extern IntakeCamTiming RightIntakeCam;
