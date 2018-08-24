#pragma once

///////////////////////////////////////////////////////////////////////////////
// THIS IS FOR DIAGNOSTIC USE ONLY!
//
// DO NOT USE THIS CODE WITH A RUNNING ENGINE!
//
// IT IS NOT THREAD-SAFE!
//
// ONLY USE THIS TO GATHER DATA FOR OFFLINE ANALYSIS. THEN TURN IT OFF.
//
// TODO: Add code to disable AVCS solenoids if an instance of Breadcrumbs
// has been created. Because this is only for testing, not for running.
///////////////////////////////////////////////////////////////////////////////

class IBreadcrumbs
{
public:
	virtual void Initialize() = 0;
	virtual void Enable() = 0;
	virtual void AddEvent(char* id, unsigned value) = 0;
	virtual void WriteAndReset() = 0;

	virtual int IsEnabled() = 0;
	virtual int IsFull() = 0;

	static IBreadcrumbs* GetInstance();
};

///////////////////////////////////////////////////////////////////////////////
// Unit tests for breadcrumb tracking and logging
///////////////////////////////////////////////////////////////////////////////
void SelfTestBreadcrumbs();