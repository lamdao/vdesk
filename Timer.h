#ifndef __TIMER_H
#define __TIMER_H
//----------------------------------------------------------------------------
#include <time.h>
//----------------------------------------------------------------------------
#include "Resource.h"
//----------------------------------------------------------------------------
class TimerControl {
private:
	long start;
	long delay;
public:
	void SetDelay( long d ) { delay = d; start = time(0); }
	long GetDelay( long n ) { return start + delay - n; }
	long Reset() { start = time(0); return delay; }

	virtual bool IsOneShot() { return true; }
	virtual void OnTime() = 0;
};
//----------------------------------------------------------------------------
class Timer: public Resource {
private:
	static vector<TimerControl*> items;

	int c;
public:
	Timer();
	void Update();

public:
	static void Add( TimerControl *t, long d );
	static void Remove( TimerControl *t );
};
//----------------------------------------------------------------------------
#endif
