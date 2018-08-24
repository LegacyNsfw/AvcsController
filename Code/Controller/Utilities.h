#pragma once

void ClearScreen();
void ClearScreenBuffer();
void Fail(char *message);
int IsSynchronized();
void PrintShort(char *buffer, unsigned value);
void PrintSigned(char *buffer, int value);
void PrintLong(char *buffer, unsigned value);
void PrintFloat(char *buffer, float value);

void WriteLogLine();

void InitializeUtilities();
void SelfTestUtilities();