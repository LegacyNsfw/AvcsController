#pragma once

class CrankState
{
public:
	unsigned SyncCountdown;
	unsigned Rpm;
	unsigned AverageInterval;
	unsigned PulseDuration;
	unsigned PinState;
	unsigned PulseState;
	unsigned Timeout;
	unsigned AnalogValue;

	CrankState()
	{
		SyncCountdown = 0;
		Rpm = 0;
		PulseDuration = 0;
		PinState = 0;
		PulseState = 0;
		Timeout = 0;
	}

	void BeginPulse(unsigned interval);
	void EndPulse(unsigned interval);

	void Process()
	{
		// Clean up if wraparound happened due to a race condition
		if (SyncCountdown > 10000)
		{
			SyncCountdown = 0;
		}
	}
};

extern CrankState Crank;
