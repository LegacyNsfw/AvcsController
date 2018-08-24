#if ARDUINO
#include <Arduino.h>
#include "LiquidCrystal.h"
extern LiquidCrystal lcd;
#endif

#include "stdafx.h"
#include "Globals.h"
#include "SelfTest.h"
#include "PlxProcessor.h"
#include "ExhaustCamState.h"
#include "CrankState.h"
#include "Utilities.h"
#include "Configuration.h"
#include "Feedback.h"
#include "Mode.h"

extern Mode mode;

const unsigned PlxFluidTempAddress = 2;
//const unsigned PlxFluidPressureAddress = 3;
const unsigned PlxRpmAddress = 6;
const unsigned PlxTimingAddress = 11;
const unsigned PlxDutyAddress = 20;

const unsigned PlxCrankRpmInstance = 0;
const unsigned PlxLeftRpmInstance = 1;
const unsigned PlxRightRpmInstance = 2;

// These are 2 higher than you'd think, so they can be sent as timing numbers
const unsigned PlxLeftDutyInstance = 3;
const unsigned PlxRightDutyInstance = 4;

unsigned SensorCount;

extern float DebugSolenoidDuty;

///////////////////////////////////////////////////////////////////////////////
// Initialize an instance of PlxProcessor (for test use)
///////////////////////////////////////////////////////////////////////////////
PlxProcessor::PlxProcessor()
{
	state = 0;
	input = NULL;
	output = NULL;
	sensorValue = 0;
	::OilTemperature = 0;
	::PlxPacketCount = 0;
	::PlxByteCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Configurat serial ports
///////////////////////////////////////////////////////////////////////////////
#ifdef ARDUINO
void PlxProcessor::Initialize(USARTClass *inputPort, USARTClass *outputPort)
{
	input = inputPort;
	output = outputPort;
	input->begin(19200, UARTClass::UARTModes::Mode_8N1);
	output->begin(19200, UARTClass::UARTModes::Mode_8N1);
	OilTemperature = 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// To be invoked once per iteration of the main loop
///////////////////////////////////////////////////////////////////////////////
void PlxProcessor::Update()
{
#if ARDUINO
	while(input->available())
	{
		byte b = input->read();
		ByteReceived(b);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Process a single incoming byte
///////////////////////////////////////////////////////////////////////////////
void PlxProcessor::ByteReceived(byte b)
{
	::PlxByteCount++;
	if (::PlxByteCount > 1000)
	{
		::PlxByteCount = 0;
	}

	if ((b & 0x80) != 0)
	{
		SensorCount = 0;
		state = 1;
		return;
	}

	if ((b & 0x40) != 0)
	{
		state = 0;
		FillOutputBuffer();
		SendOutput();
		ReceivedSensorCount = SensorCount;
		return;
	}

	switch (state)
	{
	case 1:
		sensorAddress = b << 6;
		state++;
		break;

	case 2:
		sensorAddress |= b;
		state++;
		break;

	case 3:
		sensorInstance = b;
		state++;
		break;

	case 4:
		sensorValue = b << 6;
		state++;
		break;

	case 5: 
		sensorValue |= b;
		SensorCount++;
		
		if ((sensorAddress == PlxFluidTempAddress) && (sensorInstance == 0))
		{
			// To convert from C to F: (temperature / .555) + 32
			// But other code expects C now.
			OilTemperature = sensorValue;

			PlxPacketCount++;
			if (PlxPacketCount >= 1000)
			{
				PlxPacketCount = 0;
			}

		}

		if ((sensorAddress == PlxRpmAddress) && (sensorInstance == PlxCrankRpmInstance))
		{
			ReceivedCrankRpm = sensorValue;
		}

		if ((sensorAddress == PlxRpmAddress) && (sensorInstance == PlxLeftRpmInstance))
		{
			ReceivedLeftRpm = sensorValue;
		}

		if ((sensorAddress == PlxRpmAddress) && (sensorInstance == PlxRightRpmInstance))
		{
			ReceivedRightRpm = sensorValue;
		}

		if ((sensorAddress != PlxRpmAddress) &&
			(sensorAddress != PlxFluidTempAddress) &&
			(sensorAddress != 4))
		{
			ReceivedFluidPressureAddress = sensorAddress;
			ReceivedFluidPressure = sensorValue;
		}

		state = 1;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Fill the output buffer
//
// Do not add more than 5 parameters! Output will be unreliable.
// Not sure if the limiting factor is Arduino or PLX sensor/protocol - if the
// latter, then downstream sensors in the PLX chain could make things worse.
///////////////////////////////////////////////////////////////////////////////
byte* PlxProcessor::FillOutputBuffer()
{	
	outputIndex = 0;

	AddByte(0x80);
//	AddSensorBytes(PlxFluidTempAddress, 0, OilTemperature);

	//AddSensorBytes(PlxRpmAddress, PlxCrankRpmInstance, (int) ((float)Crank.Rpm / 19.55f));
	//AddSensorBytes(PlxRpmAddress, PlxLeftRpmInstance, LeftExhaustCam.Rpm);
	//AddSensorBytes(PlxRpmAddress, PlxRightRpmInstance, RightExhaustCam.Rpm);
	
#ifndef SIMULATOR
//	AddSensorBytes(PlxRpmAddress, PlxCrankRpmInstance, (int)((float)Crank.Rpm / 19.55f));
//	AddSensorBytes(PlxRpmAddress, PlxLeftRpmInstance, (int)((float)LeftExhaustCam.Rpm / 19.55f));
//	AddSensorBytes(PlxRpmAddress, PlxRightRpmInstance, (int)((float)RightExhaustCam.Rpm / 19.55f));

	//AddSensorBytes(PlxTimingAddress, 2, LeftFeedback.Output + 64);
//	AddSensorBytes(PlxTimingAddress, 2, RightFeedback.Output + 64);
	//AddSensorBytes(PlxTimingAddress, 2, CamTargetAngle + 64);

//	AddSensorBytes(PlxDutyAddress, PlxLeftDutyInstance, RightFeedback.Output * 10.23);

	// The +64 corresponds to how the value is scaled by the PLX protocol.

	//if (Crank.Rpm < MAXIMUM_BASELINE_RPM)
	if (mode.GetMode() == Mode::Synchronizing)
	{
		AddSensorBytes(PlxTimingAddress, 0, LeftExhaustCam.Baseline + 64);
		AddSensorBytes(PlxTimingAddress, 1, RightExhaustCam.Baseline + 64);
	}
	else
	{
		AddSensorBytes(PlxTimingAddress, 0, LeftExhaustCam.Angle + 64);
		AddSensorBytes(PlxTimingAddress, 1, RightExhaustCam.Angle + 64);
	}
	
//	AddSensorBytes(PlxDutyAddress, PlxLeftDutyInstance, LeftFeedback.Output * 10.23);
//	AddSensorBytes(PlxDutyAddress, PlxRightDutyInstance, RightFeedback.Output * 10.23);

	float baseDuty = 0; //  45.0f;
	AddSensorBytes(PlxTimingAddress, PlxLeftDutyInstance, LeftFeedback.Output + 64 + baseDuty);
	AddSensorBytes(PlxTimingAddress, PlxRightDutyInstance, RightFeedback.Output + 64 + baseDuty);
//	AddSensorBytes(PlxTimingAddress, PlxRightDutyInstance, DebugSolenoidDuty + 64);
#endif // !SIMULATOR

	AddByte(0x40);

	// For test use
	return outputBuffer;
}

///////////////////////////////////////////////////////////////////////////////
// Add the output bytes for a single PLX sensor
///////////////////////////////////////////////////////////////////////////////
void PlxProcessor::AddSensorBytes(byte address, byte instance, int value)
{
	AddByte(address >> 6); // FluidTemp address MSB
	AddByte(address & 0x3F); // FluidTemp address LSB
	AddByte(instance); // FluidTemp instance
	AddByte((byte)(value >> 6));
	AddByte((byte)(value & 0x3F));
}

///////////////////////////////////////////////////////////////////////////////
// Add a single byte to the output buffer
///////////////////////////////////////////////////////////////////////////////
void PlxProcessor::AddByte(byte value)
{
	outputBuffer[outputIndex] = value;
	outputIndex++;
}

///////////////////////////////////////////////////////////////////////////////
// Send the output
///////////////////////////////////////////////////////////////////////////////
void PlxProcessor::SendOutput()
{
	if (output == null)
	{
		return;
	}

#if ARDUINO
	size_t written = output->write(outputBuffer, outputIndex);	
#else 
	size_t written = 0;
#endif
}

// ############################################################################
// ############################################################################
//
// Test cases
//
// ############################################################################
// ############################################################################


///////////////////////////////////////////////////////////////////////////////
// Simulate receiving a complete PLX packet
// Then simulate sending the data to the next device in the PLX chain
///////////////////////////////////////////////////////////////////////////////
bool TestSendReceive()
{
	int step = 1;

	unsigned expectedTemp = 72;

	// No data added
	// unsigned expectedBufferSize = 7;

	// RPM data added
	unsigned expectedBufferSize = 22;

	// RPM and cam angle data added
	//unsigned expectedBufferSize = 32;

	// These will be included in the output packet
	Crank.Rpm = 1000;
	LeftExhaustCam.Rpm = 1001;
	RightExhaustCam.Rpm = 1002;

	byte packet[] =
	{
		0x80, // Start of packet
		0x00, // SM - AFR Address MSB 
		0x00, // SM - AFR Address LSB 
		0x00, // SM - AFR Instance 
		0x00, // SM - AFR Data MSB 
		0x04, // SM - AFR Data LSB
		0x00, // SM - Temp Address MSB 
		0x02, // SM - Temp Address LSB 
		0x00, // SM - Temp Instance 
		expectedTemp >> 6, // SM - Temp Data MSB 
		expectedTemp & 0x3F, // SM - Temp Data LSB 
		0x40, // End of packet
	};

	PlxProcessor test;

	for (int i = 0; i < sizeof(packet); i++)
	{
		test.ByteReceived(packet[i]);
	}

	if (!CompareUnsigned(OilTemperature, expectedTemp, "OilTemp 1"))
	{
		return false;
	}

	if (!CompareUnsigned((unsigned)test.outputIndex, expectedBufferSize, "BufferSize 1"))
	{
		return false;
	}

	// Feed the output buffer into the processor to confirm that it can be parsed.
	int bufferSize = test.outputIndex;
	test.outputIndex = 0;
	
	for (int i = 0; i < bufferSize; i++)
	{
		test.ByteReceived(test.outputBuffer[i]);
	}

	if (!CompareUnsigned(OilTemperature, expectedTemp, "OilTemp 2"))
	{
		return false;
	}

/*	if (!CompareUnsigned((unsigned)test.outputIndex, expectedBufferSize, "BufferSize 2"))
	{
		return false;
	}

	if (!CompareUnsigned(::receivedCrankRpm, Crank.Rpm, "CRPM"))
	{
		return false;
	}


	if (!CompareUnsigned(::receivedLeftRpm, LeftCam.Rpm, "LRPM"))
	{
		return false;
	}

	if (!CompareUnsigned(::receivedRightRpm, RightCam.Rpm, "RRPM"))
	{
		return false;
	}
*/

	// Don't interfere with warmup after the self-test
	OilTemperature = 0;

	// Reset the values we munged
	Crank.Rpm = 0;
	LeftExhaustCam.Rpm = 0;
	RightExhaustCam.Rpm = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Self-test the PLX code
///////////////////////////////////////////////////////////////////////////////
void SelfTestPlxProcessor()
{
//	InvokeTest(SendReceive);
}