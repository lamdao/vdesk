#include "Event.h"
#include "Window.h"
//-----------------------------------------------------------------------------
EventControl *EventControl::active = NULL;
vector<EventControl *> EventControl::active_list;
//-----------------------------------------------------------------------------
EventControl *EventControl::Find( Window w )
{
	XPointer p = NULL;

	XFindContext( display, w, VdeskCID, &p );
	return (EventControl *)p;
}
//----------------------------------------------------------------------------
void EventControl::Grab( Window w, EventControl *a )
{
	if( active == a ) return;
//	XGrabKeyboard(display, w, True,	GrabModeAsync, GrabModeAsync, CurrentTime);
	int x = XGrabPointer(display, w, True, ButtonPressMask|ButtonReleaseMask,
			GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	// TODO:
	// MUST check x here
	active_list.push_back( active );
	active = a;
}
//----------------------------------------------------------------------------
void EventControl::Release()
{
	XUngrabPointer(display, CurrentTime);
	XUngrabKeyboard(display, CurrentTime);
	active = active_list[ active_list.size() - 1 ];
	active_list.erase( active_list.begin() + active_list.size() - 1 );
}
//----------------------------------------------------------------------------
void EventControl::ProcessEvent( XEvent *e )
{
	switch( e->type ) {
		case Expose:
			Appear( e );
			break;
		case EnterNotify:
			MouseEnter( (XMotionEvent *)e );
			break;
		case MotionNotify:
			MouseMove( (XMotionEvent *)e );
			break;
		case LeaveNotify:
			MouseLeave( (XMotionEvent *)e );
			break;
		case ButtonPress:
			btime = e->xbutton.time;
			MousePress( (XButtonEvent *)e );
			break;
		case ButtonRelease:
			if( e->xbutton.button==Button1 && (e->xbutton.time-btime<=200) ) {
				MouseClick( (XButtonEvent *)e );
				btime = 0;
			}
			MouseRelease( (XButtonEvent *)e );
			break;
		case KeyPress:
			ktime = e->xkey.time;
			KeyDown( (XKeyEvent *)e );
			break;
		case KeyRelease:
			if( e->xkey.time-ktime<=200 ) {
				KeyClick( (XKeyEvent *)e );
				ktime = 0;
			}
			KeyUp( (XKeyEvent *)e );
			break;
		case FocusIn:
			HasFocus( (XFocusInEvent *)e );
			break;
		case FocusOut:
			LostFocus( (XFocusOutEvent *)e );
			break;
		case VisibilityNotify:
			Visibility( ((XVisibilityEvent *)e)->state );
			break;
	}
}
//----------------------------------------------------------------------------
