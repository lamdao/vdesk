#ifndef __EVENT_H
#define __EVENT_H
//----------------------------------------------------------------------------
#include "Resource.h"
//----------------------------------------------------------------------------
class EventControl: public Resource {
private:
	static EventControl *active;
	static vector<EventControl *> active_list;

	long btime, ktime;
public:
	EventControl(): Resource() { btime = ktime = 0; }

	virtual void ProcessEvent( XEvent *e );

	virtual void Appear( XEvent *e ) {}
	virtual void MouseEnter( XMotionEvent *e ) {}
	virtual void MouseMove( XMotionEvent *e ) {}
	virtual void MouseLeave( XMotionEvent *e ) {}
	virtual void MousePress( XButtonEvent *e ) {}
	virtual void MouseRelease( XButtonEvent *e ) {}
	virtual void MouseClick( XButtonEvent *e ) {}
	virtual void KeyDown( XKeyEvent *e ) {}
	virtual void KeyUp( XKeyEvent *e ) {}
	virtual void KeyClick( XKeyEvent *e ) {}
	virtual void HasFocus( XFocusInEvent *e ) {}
	virtual void LostFocus( XFocusOutEvent *e ) {}
	virtual void Visibility( int state ) {}

	static EventControl *Find( Window w );
	static EventControl *Active() { return active; }
	static void Grab( Window w, EventControl *a );
	static void Release();
};
//----------------------------------------------------------------------------
#endif
