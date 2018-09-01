#pragma once

///////////////////////////////////////////////////////////////////////////////
// Global variables
//
// Basically anything that might need to be shared between different classes
// in this project. 
//
// The Utilities.cpp file has EXTERN defined to nothing, so that including
// this file causes the variables to be defined rather than merely declared.
///////////////////////////////////////////////////////////////////////////////

#ifndef EXTERN
#define EXTERN extern
#endif

//#ifndef byte
//#define byte char
//#endif

EXTERN void* null;

const unsigned DisplayWidth = 16;

// Extra characters allow for padding values with trailing spaces
EXTERN char DisplayLine1[DisplayWidth + 1];
EXTERN char DisplayLine2[DisplayWidth + 1];

// For display usage
EXTERN char CurrentModeName[DisplayWidth];
EXTERN char ErrorMessage[DisplayWidth];
EXTERN char LastErrorMessage[DisplayWidth];

// For unit testing
EXTERN char FailureMessage[100];

// For debugging/diagnostics
EXTERN unsigned InitializationErrorCount;
EXTERN unsigned ErrorCount;
EXTERN unsigned DebugLeft;
EXTERN unsigned DebugCrank;
EXTERN unsigned DebugRight;

EXTERN unsigned DebugLong1;
EXTERN unsigned DebugLong2;

// To measure update rate
EXTERN unsigned IterationsPerSecond;

// Raw sensor state
EXTERN unsigned MapSensorState;
EXTERN unsigned KnobState;

// Centigrade (160F = 71C)
EXTERN unsigned OilTemperature;

EXTERN unsigned PlxPacketCount;
EXTERN unsigned PlxByteCount;

// Angles
EXTERN float CamTargetAngle;
EXTERN float LeftCamError;
EXTERN float RightCamError;

// Actuator Duty Cycle
EXTERN unsigned LeftSolenoidDutyCycle;
EXTERN unsigned RightSolenoidDutyCycle;

// PLX receive testing
EXTERN unsigned ReceivedCrankRpm;
EXTERN unsigned ReceivedLeftRpm;
EXTERN unsigned ReceivedRightRpm;
EXTERN unsigned ReceivedFluidPressureAddress;
EXTERN unsigned ReceivedFluidPressure;
EXTERN unsigned ReceivedSensorCount;

// Constants for turning timer ticks into RPM and degrees.
//
// This one is defined in InterruptHandlers.cpp because it depends on which
// type of timer is being used. (There is currently one type proven and another
// in development.)
extern const unsigned TicksPerSecond;
extern const unsigned TicksPerMinute;
