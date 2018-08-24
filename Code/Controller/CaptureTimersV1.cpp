#include "Arduino.h"
#include "Timers2.h"
#include "Globals.h"

void CamCaptureHandler(Tc *timer, int channel, int *output)
{
	// Why did this interrupt happen? 
	// (Checking this value also clears the interrupt flag.)
	const uint32_t status = TC_GetStatus(timer, channel);

	// Was there an overflow?
	const bool overflowed = status & TC_SR_COVFS;

	// Was there a loading overrun? (indicates loading RA or RB before the capture event)
	const bool loadoverrun = status & TC_SR_LOVRS;

	// Which signal are we capturing?
	const bool inputcaptureA = status & TC_SR_LDRAS;
	const bool inputcaptureB = status & TC_SR_LDRBS;

	// This value is shown on the LCD for debugging.
	unsigned x = *output;

	if (inputcaptureA) {
		const uint32_t ra = timer->TC_CHANNEL[channel].TC_RA;
		DebugLong1 = ra;
		x |= 1;
	}

	if (inputcaptureB) {
		const uint32_t rb = timer->TC_CHANNEL[channel].TC_RB;
		DebugLong2 = rb;
		x |= 2;
	}

	if (overflowed) x |= 4;
	if (loadoverrun) x |= 8;
	*output = x;

	// Restart the timer.
	// 
	// This will really happen in the crank signal handler, 
	// but restarting here makes testing simpler.
	TC_Start(timer, channel);
}

// Timer instance 1 = Timer Counter 0, Channel 1
void TC1_Handler()
{
	CamCaptureHandler(TC0, 1, (int*)&DebugLeft);
}

// Timer instance 2 = Timer counter 0, channel 2
void TC2_Handler()
{
	CamCaptureHandler(TC0, 2, (int*)&DebugRight);
}

void CaptureTimers::Initialize()
{
	// Set pins to input mode.
	pinMode(A4, INPUT);
	pinMode(A6, INPUT);

	// Enable pull-up resistors
	digitalWrite(A4, HIGH);
	digitalWrite(A6, HIGH);

	// Enable writes to the timer mode register
	// See 36.7.20 / page 908
	REG_TC0_WPMR = 0x54494D00;
	
	// Enable writes to the IO control register.
	// See 32.7.42 for the origin of the magic number
	REG_PIOA_WPMR = 0x50494F00;

	// Enables clock configuration.
	pmc_set_writeprotect(false);

	// Enable the timer peripheral
	//
	// Note that these TCs range from 0-9 because
	// they are instance/interrupt numbers, not 
	// timer/counter module numbers.
	pmc_enable_periph_clk(ID_TC1);
	pmc_enable_periph_clk(ID_TC2);

	// Probably only needed for wave mode.
	TC_SetRC(TC0, 1, 0xFFFFFFFF);
	TC_SetRC(TC0, 2, 0xFFFFFFFF);

	const PinDescription *configA4 = &g_APinDescription[A4];
	PIO_Configure(
		configA4->pPort,
		configA4->ulPinType,
		configA4->ulPin,
		configA4->ulPinConfiguration
	);

	const PinDescription *configA6 = &g_APinDescription[A6];
	PIO_Configure(
		configA6->pPort,
		configA6->ulPinType,
		configA6->ulPin,
		configA6->ulPinConfiguration
	);
	
	// Map the IO controller to the desired pin
	// Analog pin 4 is port A, P6 (TIOB1, TC0, ch1, TC instance 1)
	// Analog pin 6 is port A, P3 (TIOB2, TC0, ch2, TC instance 2)
	// https://www.arduino.cc/en/Hacking/PinMappingSAM3X
	// https://github.com/ivanseidel/DueTimer/issues/11
	//
	// Datasheet Table 9-2 says PA3 = TIOB1/AD1, PA6 = TIOB2/AD3.
	// 
	// This disables PIO, enables the timer periperal.
	// REG_PIOA_PDR |= PIO_PDR_P6 | PIO_PDR_P3;

	// Select the right A/B connections. 
	// Bits set to 0 = peripheral A
	// Bits set to 1 = peripheral B
	// See 31.7.24 and table 37-4 in section 37.5.1.
	// REG_PIOA_ABSR |= PIO_ABSR_P6 | PIO_ABSR_P3;

	// Disable output on PIOA P3/P6;
	// REG_PIOA_ODR |= PIO_PDR_P6 | PIO_PDR_P3;

	// See 36.6.4, Clock Control
	// Here we set TC_CMR.
	//
	// Load RA (on falling) followed by RB (on rising) after each trigger.
	// The crank sensor will trigger both of the timers, then the cam sensors will
	// capture the timer values. Cam advance is proportional to the elapsed time.
	//
	// CLOCK1 = 84mhz / 2 = 42mhz.
	// This is the best resolution, and it overflows in 51 seconds, which is a reasonable timeout.
	//
	// WAVE bit is cleared, to set the timer to capture mode.
	TC_Configure(TC0, 1, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_LDRA_FALLING | TC_CMR_LDRB_RISING);
	TC_Configure(TC0, 2, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_LDRA_FALLING | TC_CMR_LDRB_RISING);

	// Interrupt on counter overflow and load overrun for diagnostics.
	// Interrupt on RA (falling), and RB (rising) for reasons above.
	const uint32_t flags = TC_IER_COVFS | TC_IER_LOVRS | TC_IER_LDRAS | TC_IER_LDRBS;
	TC0->TC_CHANNEL[1].TC_IER = flags;
	TC0->TC_CHANNEL[2].TC_IDR = ~flags;

	TC0->TC_CHANNEL[1].TC_IER = flags;
	TC0->TC_CHANNEL[2].TC_IDR = ~flags;

	// Enable the interrupt for each timer instance.
	// Note that the TCs here are instance numbers (0-9).
	NVIC_DisableIRQ(TC1_IRQn);
	NVIC_ClearPendingIRQ(TC1_IRQn);
	NVIC_SetPriority(TC1_IRQn, 0);
	NVIC_EnableIRQ(TC1_IRQn); // TC1_Handler

	NVIC_DisableIRQ(TC2_IRQn);
	NVIC_ClearPendingIRQ(TC2_IRQn);
	NVIC_SetPriority(TC2_IRQn, 0);
	NVIC_EnableIRQ(TC2_IRQn); // TC2_Handler

	// Not sure if actually useful.
	// These TC numbers are timer/counter module numbers again.
	// TC_GetStatus(TC0, 1);
	// TC_GetStatus(TC0, 2);

	// Prevent re-configuration by setting the same write-protect
	// bits that were turned off at the top of this function.
	REG_TC0_WPMR = 0x54494D01;
	REG_PIOA_WPMR = 0x50494F01;
	pmc_set_writeprotect(true);
}

void CaptureTimers::Begin()
{
	// Start the timers.
	TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
	TC0->TC_CHANNEL[2].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;

	// This might be equivalent to the above?
	// TC_Start(TC0, 1);
	// TC_Start(TC0, 2);
}

