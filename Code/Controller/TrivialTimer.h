// Timer based on calls to micros()

class TrivialTimer
{
protected:
	unsigned startTime;

public:

	TrivialTimer();
	void configure(unsigned timeout);
	void attachInterrupt(void (*isr)(unsigned));
	void detachInterrupt(void);
	void start();
	void stop(void);	
	unsigned getElapsed(void) const;
};
