#include "libtimer.h"

Timer *createTimer(){
	Timer* timer = (Timer*)malloc(sizeof(Timer));
	if(!timer) return NULL;
	
	memset(timer, 0, sizeof(Timer));
	
	SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	timer->start_time = rtick.tick;
	
	return timer;
}

void startTimer(Timer *timer){
	SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	timer->start_time = rtick.tick;
	timer->current_time = 0;
	timer->running = true;
}

void pauseTimer(Timer *timer){
	SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	if(timer->running)
		timer->pause_start = rtick.tick;
	else
	{
		timer->pause_end = rtick.tick;
		timer->pause_time = timer->pause_end - timer->pause_start;
		timer->start_time += timer->pause_time;
	}
	
	timer->running = !timer->running;
}

void updateTimer(Timer *timer){	
	if(!timer->running) return;
	
	SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	timer->current_time = rtick.tick;
	
	timer->elapsed = timer->current_time - timer->start_time;
}

void stopTimer(Timer *timer){
	timer->running = false;
}

void resetTimer(Timer *timer){	
	//this is how to reset a timer
	timer->elapsed = 0;
	timer->running = false;
	timer->current_time = 0;
	timer->pause_end = 0;
	timer->pause_start = 0;
	timer->pause_time = 0;
	timer->start_time = 0;
}

void freeTimer(Timer *timer){
	free(timer);
}