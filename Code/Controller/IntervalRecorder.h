enum Intervals
{
	// In the array, the CrankHigh slot will contain an absolute time. All others
	// will contain the elapsed time since the CrankHigh event.
	CrankHigh = 0,
	CrankLow,

	LeftExhaustCamHigh1,
	LeftExhaustCamLow1,
	LeftExhaustCamHigh2,
	LeftExhaustCamLow2,

	RightExhaustCamHigh1,
	RightExhaustCamLow1,
	RightExhaustCamHigh2,
	RightExhaustCamLow2,

	IntervalCount,
};

class IIntervalRecorder
{
protected:
	IIntervalRecorder() {}

public:
	static IIntervalRecorder* GetInstance();

	virtual void Initialize() = 0;

	virtual void WriteToSerial() = 0;

	virtual long LogInterval(int id) = 0;
};
