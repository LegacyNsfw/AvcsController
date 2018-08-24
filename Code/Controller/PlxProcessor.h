#pragma once

#if !ARDUINO
#define USARTClass char
#endif

///////////////////////////////////////////////////////////////////////////////
// Parses incoming data from a PLX oil temperature sensor.
// Also injects data from this app into the PLX stream.
///////////////////////////////////////////////////////////////////////////////

#ifndef ARDUINO
#define byte char
#endif

class PlxProcessor
{
private:
	// Output buffer size right now is just 32 bytes, this gives some extra room
	static const int outputByteCount = 100;

	USARTClass *input;
	USARTClass *output;

	int state;
	int sensorAddress;
	int sensorInstance;
	int sensorValue;

public:
	PlxProcessor();

	void Initialize(USARTClass *inputPort, USARTClass *outputPort);

	void Update();

	// These are public to ease testing.
	void ByteReceived(byte b);
	byte* FillOutputBuffer();
	int outputIndex;
	byte outputBuffer[outputByteCount];

private:
	void AddByte(byte value);
	void AddSensorBytes(byte address, byte instance, int value);
	void SendOutput();
};

///////////////////////////////////////////////////////////////////////////////
// Self-tests for the PLX code.
///////////////////////////////////////////////////////////////////////////////
void SelfTestPlxProcessor();
