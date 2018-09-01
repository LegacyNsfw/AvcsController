#pragma once

///////////////////////////////////////////////////////////////////////////////
// Holds the state for a single exhaust cam and its associated pulse train
//
// TODO: Migrate ExhaustCamTiming to the interface pattern
///////////////////////////////////////////////////////////////////////////////
class ExhaustCamState
{
private:
	
	enum CycleStates
	{
		Start,
		Pulse1,
		Pulse2,
		End,
	};

public:
	// Nonzero for left cam, zero for right cam.
	unsigned Left;

	// Time between start-of-cam-pulse events.
	unsigned AverageInterval;

	// Time between start of pulse and end of pulse.
	unsigned PulseDuration;

	// Each cam instance maintains an RPM value so it can be sanity-checked against the others.
	unsigned Rpm;
	unsigned CalibrationCountdown; // May go slightly negative due to race conditions
	CycleStates CycleState;
	unsigned TimeSinceCrankSignal;
	float Baseline; 
	float Angle;
	unsigned PinState; // set by the .ino code, should match PulseState
	unsigned PulseState; // set by the interrupt handler, should match PinState
	unsigned Timeout;

	ExhaustCamState(int left)
	{
		Left = left;
		AverageInterval = 0;
		PulseDuration = 0;
		Rpm = 0;
		CalibrationCountdown = 0;
		TimeSinceCrankSignal = 0;
		Baseline = 0;
		Angle = 0;
		PinState = 0;
		PulseState = 0;
		Timeout = 0;
	}

	void StartCycle();
	void BeginPulse(unsigned camInterval, unsigned crankInterval);
	void EndPulse(unsigned camInterval);

	// Clean up if wraparound happened due to a race condition
	void Process()
	{
		if (CalibrationCountdown > 10000)
		{
			CalibrationCountdown = 0;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Self-test the cam timing code
///////////////////////////////////////////////////////////////////////////////
void SelfTestExhaustCamTiming();

///////////////////////////////////////////////////////////////////////////////
// Global instances of ExhaustCamTiming
///////////////////////////////////////////////////////////////////////////////
extern ExhaustCamState LeftExhaustCam;
extern ExhaustCamState RightExhaustCam;
