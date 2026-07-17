#include <stdio.h>
#include <math.h>
#include <windows.h>

static DWORD gtime;

void startclock (void) 
{
  gtime = GetTickCount();
  return;
}

float stopclock (void)
{
  float period;
  
  period = (float)(GetTickCount() - gtime) / 1000.0f;
  return period;
}

