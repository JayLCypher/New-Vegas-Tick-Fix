#pragma once
#ifndef FPSTIMER_H
#define FPSTIMER_H

extern int g_bAlternateGTCFix;

void FPSStartCounter();
double GetFPSCounterMilliseconds_WRAP(bool doUpdate= true);
DWORD ReturnCounter_WRAP();

#endif