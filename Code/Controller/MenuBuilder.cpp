///////////////////////////////////////////////////////////////////////////////
// Combines multiple screens into a menu
///////////////////////////////////////////////////////////////////////////////
#include "Screen.h"
#include "ScreenNavigator.h"
#include "MenuBuilder.h"
#include "Globals.h"
#include "ExhaustCamState.h"
#include "CrankState.h"
#include "Feedback.h"

///////////////////////////////////////////////////////////////////////////////
// At run time, in an error happens, this screen will have additional screens 
// added to its right, so that error details can be reviewed.
//
// Declaring it globally is bit hacky, but the Mode class needs access to this 
// screen, and other users of MenuBuilder do not, so it doesn't belong in the
// MenuBuilder interface.
///////////////////////////////////////////////////////////////////////////////
Screen *ErrorScreen = new TwoValueScreen("RunErr  InitErr", &ErrorCount, &InitializationErrorCount);

Screen* MenuBuilder::BuildMenu()
{
	Screen *calibrationScreen = new ThreeValueScreen(
		"Calibrating",
		&LeftExhaustCam.CalibrationCountdown,
		&Crank.CalibrationCountdown,
		&RightExhaustCam.CalibrationCountdown);

	Screen *warmingScreen = new SingleValueScreen(
		"Warming",
		&OilTemperature);

	Screen *rpmScreen = new ThreeValueScreen(
		"Left Crank Right",
		&LeftExhaustCam.Rpm,
		&Crank.Rpm,
		&RightExhaustCam.Rpm);

	// TODO: MainScreen should alternate between rpmScreen and camErrorScreen
	Screen *camErrorScreen = new TwoValueScreenF(
		"Cams.Error",
		&LeftCamError,
		&RightCamError);

	Screen* MainRow[] = {
		new MainScreen(&mode, calibrationScreen, warmingScreen, rpmScreen),
		new SingleValueScreen("Update Rate", &IterationsPerSecond),
		new ThreeValueScreen("Timeouts", &LeftExhaustCam.Timeout, &Crank.Timeout, &RightExhaustCam.Timeout),
		new ThreeValueScreen("DbgL DbgC DbgR", &DebugLeft, &DebugCrank, &DebugRight),
		new TwoValueScreen("Left Pin & Pulse", &LeftExhaustCam.PinState, &LeftExhaustCam.PulseState),
		new TwoValueScreen("Rght Pin & Pulse", &RightExhaustCam.PinState, &RightExhaustCam.PulseState),
		new TwoValueScreen("Crnk Pin & Pulse", &Crank.PinState, &Crank.PulseState),
		//new TwoLongValueScreen(&DebugLong1, &DebugLong2),
		//new FourValueScreen(&LeftCam.PinState, &RightCam.PinState, &Crank.SensorState, &KnobState),
		0
	};

	Screen* ErrorRow[] = {
		ErrorScreen,
		0
	};

	Screen* PlxRow[] = {
		new TwoValueScreen("PLX Data", &OilTemperature, &PlxPacketCount),
		new SingleValueScreen("PLX RX Sensors", &ReceivedSensorCount),
		new ThreeValueScreen("PLX RX RPM", &ReceivedLeftRpm, &ReceivedCrankRpm, &ReceivedRightRpm),
		new TwoValueScreen("PLX RX Pressure", &ReceivedFluidPressureAddress, &ReceivedFluidPressure),
		0
	};

	Screen* LeftCamRow[] = {
		new SingleValueScreen("Left Rpm", &LeftExhaustCam.Rpm),
		new SingleValueScreen("Left Interval", &LeftExhaustCam.AverageInterval),
		new SingleValueScreen("Left Duration", &LeftExhaustCam.PulseDuration),
		new SingleValueScreen("Left Since Crank", &LeftExhaustCam.TimeSinceCrankSignal),
		new SingleValueScreenF("Left Angle", &LeftExhaustCam.Angle),
		new SingleValueScreenF("Left Baseline", &LeftExhaustCam.Baseline),
		0
	};

	Screen* RightCamRow[] = {
		new SingleValueScreen("Right Rpm", &RightExhaustCam.Rpm),
		new SingleValueScreen("Right Interval", &RightExhaustCam.AverageInterval),
		new SingleValueScreen("Right Duration", &RightExhaustCam.PulseDuration),
		new SingleValueScreen("Right Since Cran", &RightExhaustCam.TimeSinceCrankSignal),
		new SingleValueScreenF("Right Angle", &RightExhaustCam.Angle),
		new SingleValueScreenF("Right Baseline", &RightExhaustCam.Baseline),
		0
	};

	Screen* CrankRow[] = {
		new SingleValueScreen("Crank Rpm", &Crank.Rpm),
		new SingleValueScreen("Crank Pulse", &Crank.PulseDuration),
		0
	};

	Screen* CamAngleRow[] = {
		camErrorScreen,
		new TwoValueScreenF("Cams.Actual", &LeftExhaustCam.Angle, &RightExhaustCam.Angle),
		new SingleValueScreenF("Cams.Target", &CamTargetAngle),
		NULL,
	};

	Screen** FeedbackAverageRow = new Screen*[Feedback::BucketCount + 2];
	FeedbackAverageRow[0] = new TwoValueScreenF("Actuator DC", &LeftFeedback.Output, &RightFeedback.Output);
	for (int i = 0; i < Feedback::BucketCount; i++)
	{
		FeedbackAverageRow[i + 1] = new ThreeValueScreenUUF(
			"Left Baseline",
			i * 500,
			i * 500 + 500,
			&(LeftFeedback.Average[i]));
	}
	FeedbackAverageRow[Feedback::BucketCount + 1] = NULL;

	Screen* SignalRow[] = {
		new TwoValueScreen("Sensors.Cams", &LeftExhaustCam.PinState, &RightExhaustCam.PinState),
		new TwoValueScreen("Sensors.Crank", &Crank.PinState, &Crank.AnalogValue),
		new TwoValueScreen("Sensors.Analog", &MapSensorState, &KnobState),
		NULL,
	};


	Screen* rows[] =
	{
		ScreenNavigator::BuildRow(MainRow),
		ScreenNavigator::BuildRow(ErrorRow),
		ScreenNavigator::BuildRow(PlxRow),
		ScreenNavigator::BuildRow(LeftCamRow),
		ScreenNavigator::BuildRow(RightCamRow),
		ScreenNavigator::BuildRow(CrankRow),
		ScreenNavigator::BuildRow(CamAngleRow),
		ScreenNavigator::BuildRow(FeedbackAverageRow),
		ScreenNavigator::BuildRow(SignalRow),
		NULL,
	};

	return ScreenNavigator::CombineRows(rows);
}
