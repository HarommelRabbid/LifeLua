#ifndef __TIMER__
#define __TIMER__

#include <malloc.h>
#include <vitasdk.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	uint32_t elapsed;
	
	bool running;
	
	uint32_t current_time;
	uint32_t start_time;
	uint32_t pause_time;
	uint32_t pause_start;
	uint32_t pause_end;
} Timer;

Timer *createTimer();

void startTimer(Timer *timer);

void pauseTimer(Timer *timer);

void updateTimer(Timer *timer);

void stopTimer(Timer *timer);

void resetTimer(Timer *timer);

void freeTimer(Timer *timer);

#endif