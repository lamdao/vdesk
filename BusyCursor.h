#ifndef __BUSYCURSOR_H
#define __BUSYCURSOR_H
//-----------------------------------------------------------------------------
#include "Resource.h"
#include "Timer.h"
//-----------------------------------------------------------------------------
#ifndef TIMEOUT
#define TIMEOUT		30
#endif
//-----------------------------------------------------------------------------
class BusyCursor: public Resource, TimerControl {
private:
	int active;

	void Stop();
public:
	BusyCursor();
	void Activate();
	void Deactivate();
	void Update();
	void OnTime();
};
//-----------------------------------------------------------------------------
#endif
