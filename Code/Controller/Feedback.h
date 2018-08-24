#pragma once

class Feedback
{
public:
	static const int BucketCount = 20;
	float Average[BucketCount];
	float ProportionalTerm;
	float IntegralTerm;
	float DerivativeTerm;

	float ProportionalGain;
	float IntegralGain;
	float DerivativeGain;

	float PreviousError;

	float Output;

	long lastTime;

	Feedback();
	void Reset(int gainType);
	void Update(long currentTimeInMicroseconds, unsigned rpm, float actual, float target);
};

extern Feedback LeftFeedback;
extern Feedback RightFeedback;

void SelfTestFeedback();