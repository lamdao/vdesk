#ifndef __EDITOR_H
#define __EDITOR_H
//----------------------------------------------------------------------------
#include "Window.h"
#include "Event.h"
#include "Canvas.h"
//----------------------------------------------------------------------------
class Editor: public WinControl, public EventControl {
private:
	int sp, cp, sw;
	bool enabled, caret_visible;
	string text;
	Canvas *canvas;
	XImage *backup;
public:
	Editor( Window parent, string t = "" );
	~Editor();

	string GetText() const { return text; }
	void SetText( string t ) { text = t; sp = cp = 0;}
	void SetWidth( int w ) { Resize( w, height ); }
	void SetEnabled( bool e );

	virtual void Appear( XEvent *e );
	virtual void KeyDown( XKeyEvent *e );
	virtual void MousePress( XButtonEvent *e );
	virtual void HasFocus( XFocusInEvent *e );
	virtual void LostFocus( XFocusOutEvent *e );
protected:
	void Create();

	int GetCaretPosition();

	void Draw();
	void ShowCaret();
	void HideCaret();
};
//----------------------------------------------------------------------------
#endif
