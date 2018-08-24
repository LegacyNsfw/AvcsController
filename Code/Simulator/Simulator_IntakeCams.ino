//#include <Arduino.h>

void setup() {
	pinMode(13, OUTPUT);
	pinMode(30, OUTPUT);
	pinMode(32, OUTPUT);
	pinMode(34, OUTPUT);
	Serial.begin(115200, UARTClass::UARTModes::Mode_8N1);
}

void spinWait(unsigned microseconds)
{
	unsigned start = micros();
	unsigned end = micros() + microseconds;
	while (micros() < end)
	{
		// nothing
	}
}

void PulseCams(unsigned microseconds)
{
	digitalWrite(30, LOW);
	digitalWrite(32, LOW);
	spinWait(microseconds);
	digitalWrite(30, HIGH);
	digitalWrite(32, HIGH);
}

unsigned pulse1;
unsigned pulse2;
unsigned pulse3;
unsigned lastPrint;
unsigned lastShift;
unsigned speed = 0;

// Using a 42mhz timer clock 
// 1ms = 41,790 ticks
// 25ms = 1,133,789 ticks
// 100ms = 4,241,795
// 1000ms = 1,979,157
void loop() {
	unsigned multiplier = 0;
	switch (speed)
	{
	case 0:
		// 600 RPM
		multiplier = 1000;
		break;

	case 1:
		// 2400 RPM
		multiplier = 250;
		break;

	case 2:
		// 4800 RPM 
		multiplier = 125;
		break;

	case 3:
		// 7200 RPM
		multiplier = 83;
		break;

	case 4:
		// 9000 RPM (in theory... actually only about 8000)
		multiplier = 75;
		break;

	case 5:
		// 9000 RPM (tuned by hand)
		multiplier = 66;
		break;

	default:
		multiplier = 2000;
		break;
	}

	digitalWrite(13, HIGH);
	
	unsigned startTime = micros();
	
	spinWait(24 * multiplier);
	pulse1 = micros() - startTime;
	startTime = micros();
	PulseCams(multiplier);
	

	spinWait(24 * multiplier);
	pulse2 = micros() - startTime;
	startTime = micros();
	PulseCams(multiplier);


	digitalWrite(13, LOW);
	spinWait(24 * multiplier);
	
	digitalWrite(34, HIGH);
	spinWait(1 * multiplier);
	digitalWrite(34, LOW);
	spinWait(24 * multiplier);

	pulse3 = micros() - startTime;
	PulseCams(multiplier);

	if (millis() > lastPrint + 1000)
	{
		lastPrint = millis();
		Serial.print(pulse1);
		Serial.print(", ");
		Serial.print(pulse2);
		Serial.print(", ");
		Serial.print(pulse3);
		Serial.println(" ");
	}

	/*
	if (millis() > lastShift + 10000)
	{
		lastShift = millis();

		mode++;
		if (mode > 1)
		{
			mode = 0;
		}
	}*/
}
