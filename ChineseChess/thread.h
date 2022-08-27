#pragma once
#ifndef _THREAD_H__
#define _THREAD_H__

int
thread_init();

void
thread_quit();

extern HANDLE thread_event;
extern struct Thread thread;

struct Thread {
public:
	void sync_thread_end();
	int sync_thread_start();
	void sync_run();
	void sync_stop();
};

void msg_callback(int action);
void append_text();



#endif // !_THREAD_H__
