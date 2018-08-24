// CaptureTimers.h
#pragma once

// My car idled at 750 before I put in bigger cams, and it idles at
// 1000 RPM now.
#define IDLE_RPM 1000

// Solenoids will be disabled below this RPM. The factory dual-AVCS
// tunes also keep them off at idle, probably because the idle RPM 
// fluctuates so much that it's not possible to get accurate timing 
// information.
#define MINIMUM_EXAVCS_RPM (IDLE_RPM + 500)

// Opinions can differ about what the right threshold is for this.
// Note that the stock system looks at water temp, not oil temp,
// and oil temp lags behind water temp during warmp-up, so my chosen
// value is probably pretty conservative. Also, 71 C == 160 F.
#define MINIMUM_TEMPERATURE_C 71

// For first iteration of this, the "crank" signal was taken from a
// cam pulley with one timing mark on it. I later realized that two
// marks would have been better. I may or may not change that in my
// car... I suspect it will work well enough. But two would probably
// allow for higher PID gains. 
// 
// If a second mark is added, consider giving it a different width,
// and extending the code to distinguish between the two of them.
#define DEGREES_PER_CRANK_PULSE 360.0f