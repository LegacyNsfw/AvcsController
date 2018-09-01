#pragma once

class CrankState
{
public:
	unsigned CalibrationCountdown;
	unsigned Rpm;
	unsigned AverageInterval;
	unsigned PulseDuration;
	unsigned PinState;
	unsigned PulseState;
	unsigned Timeout;
	unsigned AnalogValue;

	CrankState()
	{
		CalibrationCountdown = 0;
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
		if (CalibrationCountdown > 10000)
		{
			CalibrationCountdown = 0;
		}
	}
};

extern CrankState Crank;
