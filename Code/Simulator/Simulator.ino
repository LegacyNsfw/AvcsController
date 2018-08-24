// COM7
#include <DFR_Key.h>
#include <LiquidCrystal.h>

enum Mode
{
	Sweep = 0,
	Idle,
	Test1,
	Test2,
	Test3,
	Test4,
	Cruise,
	High,
};

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
DFR_Key keys(2);
float rpm;

int mode = Idle;
float camAngle = 0;

unsigned lastPlxSend;
//unsigned endOfLastIteration;
unsigned startCycle = 0;

void setup() {

	rpm = 1000.0f;
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	lcd.print("1234567890213456");
	lcd.setCursor(0, 1);
	lcd.print("1234567890213456");
	delay(100);
	lcd.clear();

	pinMode(13, OUTPUT);
	pinMode(22, OUTPUT);
	pinMode(30, OUTPUT); // crank
	pinMode(32, OUTPUT); // left cam
	pinMode(34, OUTPUT); // right cam
	Serial.begin(115200, UARTClass::UARTModes::Mode_8N1);

	Serial3.begin(19200, UARTClass::UARTModes::Mode_8N1);
}

void spinWaitUntil(unsigned microseconds)
{
	while (microseconds > micros())
	{
		// nothing
	}
}

void ReadKeys()
{

	// Read keys
	int key = keys.getKey();
	switch (key)
	{
	case UP_KEY:
		if (mode > Sweep)
			mode--;
		break;

	case DOWN_KEY:
		if (mode < High)
			mode++;
		break;

	case LEFT_KEY:
		if (camAngle > 1.0f)
		{
			camAngle -= 5.0f;
		}
		else
		{
			camAngle = 0;
		}
		break;

	case RIGHT_KEY:
		if (camAngle < 30.0f)
		{
			camAngle += 5.0f;
		}
		else
		{
			camAngle = 30.0;
		}
		break;
	}
}

void SendPlxPacket()
{
	unsigned temp = 72;

	byte packet[] =
	{
		0x80, // Start of packet
		0x00, // SM - Temp Address MSB 
		0x02, // SM - Temp Address LSB 
		0x00, // SM - Temp Instance 
		temp >> 6, // SM - Temp Data MSB 
		temp & 0x3F, // SM - Temp Data LSB 
		0x40, // End of packet
	};

	Serial3.write(packet, 7);
}

/////////////////////////////////////////////
// Important: The simulator is only accurate
// up to about 2000 RPM with zero cam retard,
// or 1500 RPM with 15 degrees retard.
/////////////////////////////////////////////
void UpdateRpm()
{
	int x = millis() % 18000;

	switch (mode)
	{
	case Sweep:

		if (x < 9000)
		{
			rpm = x + 1000;
		}
		else
		{
			rpm = (18000 - x) + 1000;
		}
		break;

	case Idle:
		rpm = 1000;
		break;

	case Test1:
		rpm = 1175;
		break;

	case Test2:
		rpm = 1225;
		break;

	case Test3:
		rpm = 1500;
		break;

	case Test4:
		rpm = 2000;
		break;

	case Cruise:
		rpm = 2500;
		break;

	case High:
		rpm = 3000;
		break;
	}
}

// Using a 42mhz timer clock 
// 1ms = 41,790 ticks
// 25ms = 1,133,789 ticks
// 100ms = 4,241,795
// 1000ms = 1,979,157
void loop() {
	if (startCycle > 0)
	{
		spinWaitUntil(startCycle);

	}
	//unsigned deadTime = micros() - endOfLastIteration;
	digitalWrite(13, HIGH);

	// Calculate pulse timing.
	float startTime = micros();
	digitalWrite(30, HIGH);
	
	// 60,000 micros = 180 degrees of cam, 360 of crank, at 1000 RPM
	// 15k = 45 degrees
	// 5k = 15 degrees
	float ticksPerSecond = (1000.f * 1000.0f);
	float rotationsPerSecond = (rpm / 60.f); // 33.3 at 2000 RPM
	float tickPerCrankRotation =  ticksPerSecond / rotationsPerSecond;  // 30,000 at 2000

	// For crank math, use 360 here. But cams rotate half as fast...
	float ticksPerDegree = tickPerCrankRotation / 180.0f; // 83 at 2000

	float camRetard = camAngle * ticksPerDegree;

	// 45 degrees
	float leftCamStart1 = startTime + ((45 + camAngle) * ticksPerDegree);

	spinWaitUntil(leftCamStart1);
	digitalWrite(32, LOW);

	// 60 degrees
	float leftCamEnd1 = startTime + ((50 + camAngle) * ticksPerDegree);

	spinWaitUntil(leftCamEnd1);
	digitalWrite(32, HIGH);

	// 90 degrees
	float crankLow = startTime + ((90 + camAngle) * ticksPerDegree);

	digitalWrite(13, LOW);
	spinWaitUntil(crankLow);
	digitalWrite(30, LOW);

	ReadKeys();

	// 135 degrees
	float rightCamStart1 = startTime + ((135 + camAngle) * ticksPerDegree);

	spinWaitUntil(rightCamStart1);
	digitalWrite(34, LOW);

	// 150 degrees
	float rightCamEnd1 = startTime + ((140 + camAngle) * ticksPerDegree);

	spinWaitUntil(rightCamEnd1);
	digitalWrite(34, HIGH);

	// 225 degrees
	float leftCamStart2 = leftCamStart1 + (180 * ticksPerDegree);

	spinWaitUntil(leftCamStart2);
	digitalWrite(32, LOW);

	// 240 degrees
	float leftCamEnd2 = leftCamStart1 + (185 * ticksPerDegree);

	spinWaitUntil(leftCamEnd2);
	digitalWrite(32, HIGH);

	// 315 degrees
	float rightCamStart2 = rightCamStart1 + (180 * ticksPerDegree);

	spinWaitUntil(rightCamStart2);
	digitalWrite(34, LOW);

	// 330 degrees - reduced to 320
	float rightCamEnd2 = rightCamStart1 + (185 * ticksPerDegree);

	spinWaitUntil(rightCamEnd2);
	digitalWrite(34, HIGH);
	

	// 360 degreees
	//float endOfCycle = startTime + (360 * ticksPerDegree);
	startCycle = startTime + (360 * ticksPerDegree);

	if (millis() > lastPlxSend + 1000)
	{
		lastPlxSend = millis();
		SendPlxPacket();
	}

	// Update RPM depending on mode.
	UpdateRpm();

	// Update display.
	lcd.setCursor(0, 0);
	lcd.print(rpm);

	lcd.setCursor(0, 1);
	lcd.print(camAngle);
	lcd.print("  ");

	int deadTime = startCycle - micros();
	//deadTime /= 1000;
	lcd.print(deadTime);
	lcd.print("  ");

	//spinWaitUntil(endOfCycle - deadTime);
	//endOfLastIteration = micros();
}
