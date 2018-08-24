// This is just here to keep the compiler happy when building the unit test project.
// Visual Studio's implementation of precompiled headers resets the preprocessor state
// which means that if you use this...
//
// #ifdef ARDUINO 
// #include <Arduino.h>
// #else 
// #include "stdafx.h"
// #endif
//
// ...then you get an error because it when it reaches the #endif it has forgotten 
// all about #ifdef above it. So we just always include "stdafx.h" instead. When
// building the Arduino project, it pulls in this file. When building the unit-test
// project, it pulls in the real (precompiled) header file.