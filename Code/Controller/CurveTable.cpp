#include "stdafx.h"

#include <stdio.h>
#include "SelfTest.h"
#include "CurveTable.h"
#include "Configuration.h"

CurveTable *pFilterWeightTable = CurveTable::CreateRpmFilterTable();

CurveTable * CurveTable::CreateExhaustCamTable()
{
	// Intake advance, for comparison   0.0     15.0      30.0    30.0     15.0      10.0
	static float input[] = { 800.0f, 1000.0f, 1250.0f, 1500.0f, 3200.0f, 5600.0f,  8000.0f };
	static float output[] = {  1.0f,    5.0f,    1.0f,    1.0f,    1.0f,   15.0f,    20.0f };

	// With 15 degrees in cruise, (and 30 degrees intake advance), the engine ran rough.
	// With BC 272s, that would be 11 degrees of overlap @ 0.050. So, no wonder.

	return new CurveTable(
		7,
		input,
		output);
}

CurveTable * CurveTable::CreateRpmFilterTable()
{
	// RPM needs to be filtered when low because it jumps around a lot at idle.
	// The 0.7 value isn't final, it's just the second thing I tried and it worked well.
	// At 0.1 (the first thing I tried) I could hear the idle getting lumpy from cam 
	// retard before seeing the retard show up in the output, so that's way too low.
	static float input[] = {  1100.0f,  2000.0f };
	static float output[] = {    0.7f,     1.0f };

	return new CurveTable(
		2,
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
