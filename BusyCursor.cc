#include <signal.h>
#include "BusyCursor.h"
//-----------------------------------------------------------------------------
BusyCursor::BusyCursor(): Resource(), TimerControl()
{
	active = 0;
}
//-----------------------------------------------------------------------------
void BusyCursor::Activate()
{
	if( !active ) {
		XGrabPointer( display, ActiveWindow, True, 0,
				GrabModeAsync, GrabModeAsync, None,
				HourCursor, CurrentTime );
	}
	Timer::Add( this, TIMEOUT );
	active++;
}
//-----------------------------------------------------------------------------
void BusyCursor::Deactivate()
{
	if( active>0 ) {
		--active;
		if( !active ) Stop();
	}
}
//-----------------------------------------------------------------------------
void BusyCursor::Update()
{
	Deactivate();
}
//-----------------------------------------------------------------------------
void BusyCursor::OnTime()
{
	if( active ) Stop();
}
//-----------------------------------------------------------------------------
void BusyCursor::Stop()
{
	active = 0;
	XUngrabPointer( display, CurrentTime );
	XSync( display, False );
	Timer::Remove( this );
}
//-----------------------------------------------------------------------------
