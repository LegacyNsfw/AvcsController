#include "stdafx.h"

#include <stdio.h>
#include "SelfTest.h"
#include "CurveTable.h"
#include "Configuration.h"

CurveTable * CurveTable::CreateExhaustCamTable()
{
	// Intake advance, for comparison               30       30       15        10
	static float input[] = { MINIMUM_EXAVCS_RPM,  2000.0f, 3000.0f, 4000.0f,  8000.0f };
	static float output[] = { 0.0f,                  1.0f,    5.0f,   15.0f,    20.0f };
	// With 15 degrees in cruise, (and 30 degrees intake advance), the engine ran rough.
	// That would be 11 degrees of overlap @ 0.050. So no wonder it was rough!

	return new CurveTable(
		5,
		input,
		output);
}

///////////////////////////////////////////////////////////////////////////////
// Tests for the ExhaustCamTable instance.
///////////////////////////////////////////////////////////////////////////////
bool TestExhaustCamTable()
{
	CurveTable *testTable = CurveTable::CreateExhaustCamTable();

	WithinOnePercent(
		testTable->GetValue(1000.0f),
		0.0f,
		"Idle");

	WithinOnePercent(
		testTable->GetValue(1600.0f),
		8.5f,
		"Middle");

	WithinOnePercent(
		testTable->GetValue(2500.0f),
		17.0f,
		"Cruise");

	WithinOnePercent(
		testTable->GetValue(3500.0f),
		18.5f,
		"MidHigh");

	WithinOnePercent(
		testTable->GetValue(6000.0f),
		20.0f,
		"High");

	WithinOnePercent(
		testTable->GetValue(8500.0f),
		20.0f,
		"Redline");

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Tests for the CurveTable class.
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Self-test the cam timing code.
///////////////////////////////////////////////////////////////////////////////
void SelfTestCurveTable()
{
	InvokeTest(ExhaustCamTable);
}
