// Timers.cpp
// 
// Implements three timers, and interactions between them.
//
// Cam timers (left and right) are started three times per camshaft revolution. 
// There are two short periods, and one long period, per revolution. 
// A running average is used to distinguish short periods from long periods.
//
// Crank timer is started once per crankshaft revolution, upon interrupt from sensor on a timing-belt pulley.
// Elapsed time between crank sensor signal and camshaft long-pulse signal is used to calculate cam phase angle.
//
// Arduino Due timer pin assignment reference:
// https://github.com/ivanseidel/DueTimer/issues/11
//
// See also:
// C:\Users\nate\AppData\Local\arduino15\packages\arduino\hardware\sam\1.6.7\variants\arduino_due_x\variant.cpp
//
// 7 captures per revolution of the camshaft
// 3.5 per revolution of the crankshaft
// At 10k RPM, 35k captures per minute, 583 captures per second.
// Arduino Due capture timers are reportedly good to approx 1 million per second.

#include "Arduino.h"
#include "Globals.h"
#include "Mode.h"
#include "CaptureTimers.h"
#include "CamTiming.h"
#include "CrankTiming.h"

///////////////////////////////////////////////////////////////////////////////
// Get the value of the crank timer
///////////////////////////////////////////////////////////////////////////////
unsigned GetCrankTime()
{
	TC_ReadCV(TC0, 0);
}

///////////////////////////////////////////////////////////////////////////////
// Process a pulse for the given timer and cam
///////////////////////////////////////////////////////////////////////////////
void CamTimerCallback(Tc *timer, int channel, unsigned *output, CamTiming *camTiming)
{
	// Why did this interrupt happen? 
	// (Checking this value also clears the interrupt flag.)
	const uint32_t status = TC_GetStatus(timer, channel);

	// Was there an overflow?
	const bool overflowed = status & TC_SR_COVFS;

	// Was there a loading overrun? (indicates loading RA or
	// RB before the capture ISR reads the previous value)
	const bool loadoverrun = status & TC_SR_LOVRS;

	// Which signal are we capturing?
	const bool inputcaptureA = status & TC_SR_LDRAS;
	const bool inputcaptureB = status & TC_SR_LDRBS;

	if (overflowed)
	{
		mode.Fail(camTiming->Left ? "Left Overflow" : "Right Overflow");
	}

	if (loadoverrun)
	{
		mode.Fail(camTiming->Left ? "Left Overrun" : "Right Overrun");
	}

	unsigned camInterval;

	if (inputcaptureA) {
		camInterval = timer->TC_CHANNEL[channel].TC_RA;
		unsigned crankInterval = GetCrankTime();

		camTiming->BeginPulse(camInterval, crankInterval);
	}

	if (inputcaptureB)
	{
		camInterval = timer->TC_CHANNEL[channel].TC_RB;

		camTiming->EndPulse(camInterval);
	}
	
	// Restart the timer.
	// Not required - external trigger is enabled for rising and falling edges.
	// TC_Start(timer, channel);
}

///////////////////////////////////////////////////////////////////////////////
// Left cam timer callback, Timer instance 7, Timer/counter 2, Channel 1
// pin D3, fourth pin on 1602 top-right header 
///////////////////////////////////////////////////////////////////////////////
void TC7_Handler()
{
	CamTimerCallback(TC2, 0, &DebugLeft, &LeftCam);
}

///////////////////////////////////////////////////////////////////////////////
// Right cam timer callback, Timer instance 8, Timer/counter 2, channel 2
// pin D11, third pin on 1602 top-right header
///////////////////////////////////////////////////////////////////////////////
void TC8_Handler()
{
	CamTimerCallback(TC2, 1, &DebugRight, &RightCam);
}

///////////////////////////////////////////////////////////////////////////////
// Crank timer callback, Timer instance 0, Timer/counter 0, channel 0
// pin D2, fifth pin on 1602 top-right header
///////////////////////////////////////////////////////////////////////////////
void TC0_Handler()
{
	Tc *timer = TC0;
	int channel = 0;

	// Why did this interrupt happen? 
	// (Checking this value also clears the interrupt flag.)
	const uint32_t status = TC_GetStatus(timer, channel);

	if (status & TC_SR_COVFS)
	{
		mode.Fail("Crank overflow");
	}

	if (status & TC_SR_LOVRS)
	{
		mode.Fail("Crank overrun");
	}

	unsigned period = GetCrankTime();

	if (status & TC_SR_LDRAS)
	{
		Crank.BeginPulse(timer->TC_CHANNEL[channel].TC_RA);
	}

	if (status & TC_SR_LDRBS)
	{
		Crank.EndPulse(timer->TC_CHANNEL[channel].TC_RB);
	}

	// TC_Start(TC0, 0);
}

///////////////////////////////////////////////////////////////////////////////
// Configure the timer for the given pin
///////////////////////////////////////////////////////////////////////////////
void ConfigurePeripheral(int pin)
{
	// Configure pin for peripheral, not I/O
	const PinDescription *config = &g_APinDescription[pin];
	PIO_Configure(
		config->pPort,
		config->ulPinType,
		config->ulPin,
		config->ulPinConfiguration
	);
}

///////////////////////////////////////////////////////////////////////////////
// Enable the given IRQ
///////////////////////////////////////////////////////////////////////////////
void ConfigureIrq(IRQn_Type irq)
{
	NVIC_DisableIRQ(irq);
	NVIC_ClearPendingIRQ(irq);
	NVIC_SetPriority(irq, 0);
	NVIC_EnableIRQ(irq); 
}

// Left cam, TC7, pin D3 (TIOA7, PIOC,   PC28, B)
// Right cam, TC8, pin D11 (TIOA8, PIOD, PD7,  B)
// Crank - use TC0, pin D2 (TIOA0, PIOB, PB25, B)

///////////////////////////////////////////////////////////////////////////////
// Initialize the timers
///////////////////////////////////////////////////////////////////////////////
void CaptureTimers::Initialize()
{
	// Enable writes to the timer mode register
	// See 36.7.20 / page 908
	REG_TC0_WPMR = 0x54494D00;

	// Enable writes to the IO control register.
	// See 32.7.42 for the origin of the magic number
	REG_PIOA_WPMR = 0x50494F00;
	REG_PIOB_WPMR = 0x50494F00;
	REG_PIOC_WPMR = 0x50494F00;
	REG_PIOD_WPMR = 0x50494F00;

	// Enables clock configuration.
	pmc_set_writeprotect(false);

	// Enable the timer peripheral
	//
	// Note that these TCs range from 0-9 because
	// they are instance/interrupt numbers, not 
	// timer/counter module numbers.
	pmc_enable_periph_clk(ID_TC7);
	pmc_enable_periph_clk(ID_TC8);
	pmc_enable_periph_clk(ID_TC0);

	// Probably only needed for wave mode.
	TC_SetRC(TC2, 2, 0xFFFFFFFF);
	TC_SetRC(TC2, 1, 0xFFFFFFFF);
	TC_SetRC(TC0, 0, 0xFFFFFFFF);

	ConfigurePeripheral(3);
	ConfigurePeripheral(11);
	ConfigurePeripheral(2);

	// See 36.6.4, Clock Control
	// Here we set TC_CMR.
	//
	// Load RA (on falling) followed by RB (on rising) after each trigger.
	//
	// CLOCK1 = 84mhz / 2 = 42mhz.
	// This is the best resolution, and it overflows in 51 seconds, which is a reasonable timeout.
	//
	// WAVE bit is cleared, to set the timer to capture mode.
	unsigned camTimerFlags =
		TC_CMR_TCCLKS_TIMER_CLOCK1 | // 42mhz
		TC_CMR_LDRA_FALLING |
		TC_CMR_LDRB_RISING |
		TC_CMR_ABETRG | // Trigger on TIOA
		TC_CMR_ETRGEDG_RISING; // Trigger on rising edge

	unsigned crankTimerFlags =
		TC_CMR_TCCLKS_TIMER_CLOCK1 | // 42mhz
		TC_CMR_LDRA_RISING |
		TC_CMR_LDRB_FALLING |
		TC_CMR_ABETRG | // Trigger on TIOA
		TC_CMR_ETRGEDG_RISING; // Trigger on rising edge

	TC_Configure(TC2, 2, camTimerFlags);
	TC_Configure(TC2, 1, camTimerFlags);
	TC_Configure(TC0, 0, crankTimerFlags);
	
	// Interrupt on counter overflow and load overrun for diagnostics.
	// Interrupt on RA (falling), and RB (rising) for the conditions specified above.
	const uint32_t flags = TC_IER_COVFS | TC_IER_LOVRS | TC_IER_LDRAS | TC_IER_LDRBS;
	TC2->TC_CHANNEL[2].TC_IER = flags;
	TC2->TC_CHANNEL[2].TC_IDR = ~flags;

	TC2->TC_CHANNEL[1].TC_IER = flags;
	TC2->TC_CHANNEL[1].TC_IDR = ~flags;

	TC0->TC_CHANNEL[0].TC_IER = flags;
	TC0->TC_CHANNEL[0].TC_IDR = ~flags;
	
	// Enable the interrupt for each timer instance.
	// Note that the TCs here are instance numbers (0-9).
	ConfigureIrq(TC7_IRQn);
	ConfigureIrq(TC8_IRQn);
	ConfigureIrq(TC0_IRQn);

	// Prevent re-configuration by setting the same write-protect
	// bits that were turned off at the top of this function.
	REG_TC0_WPMR = 0x54494D01;
	REG_PIOA_WPMR = 0x50494F01;
	REG_PIOB_WPMR = 0x50494F01;
	REG_PIOC_WPMR = 0x50494F01;
	REG_PIOD_WPMR = 0x50494F01;
	pmc_set_writeprotect(true);

	// Not required, see TC_CMR_ETRGEDGE_ values in timer flags.
	//TC_Start(TC0, 0);
	//TC_Start(TC2, 1);
	//TC_Start(TC2, 2);
}
