#ifndef __BUTTON_H
#define __BUTTON_H
//----------------------------------------------------------------------------
#include "Event.h"
#include "Action.h"
#include "Window.h"
#include "Canvas.h"
//----------------------------------------------------------------------------
class Button: public WinControl, public EventControl {
private:
	int id;
	string caption;
	Canvas *canvas;
	ActionControl *action;

	bool enabled;
public:
	Button( Window parent, int id, const char *text, int w=0, int h=0 );
	virtual ~Button();

	virtual void Appear( XEvent *e );
	virtual void MouseClick( XButtonEvent *e );
	virtual void KeyDown( XKeyEvent *e );
	virtual void HasFocus( XFocusInEvent *e );
	virtual void LostFocus( XFocusOutEvent *e );

	void SetEnabled( bool e ) { enabled = e; Draw(); }
	void SetActionControl( ActionControl *a ) { action = a; }
private:
	void Draw();
};
//----------------------------------------------------------------------------
#endif
