#ifndef __EDITOR_H
#define __EDITOR_H
//----------------------------------------------------------------------------
#include "Window.h"
#include "Event.h"
#include "Canvas.h"
//----------------------------------------------------------------------------
class Editor: public WinControl, public EventControl {
private:
	int mouse_down;
	int left_margin, right_margin;

	int sp, cp, sw, sl;
	bool enabled, caret_visible;
	string text;
	Canvas *canvas;
	XImage *backup;
public:
	Editor( Window parent, string t = "" );
	~Editor();

	string GetText() const { return text; }
	void SetText( string t ) { text = t; sp = cp = sl = 0; }
	void SetWidth( int w ) { Resize( w, height ); right_margin = w - 4; }
	void SetEnabled( bool e );

	virtual void Appear( XEvent *e );
	virtual void KeyDown( XKeyEvent *e );
	virtual void MousePress( XButtonEvent *e );
	virtual void MouseMove( XMotionEvent *e );
	virtual void MouseRelease( XButtonEvent *e );
	virtual void HasFocus( XFocusInEvent *e );
	virtual void LostFocus( XFocusOutEvent *e );

	virtual void Cut();
	virtual void Copy();
	virtual void Paste();
	virtual void Delete();

	void ClearSelection() { sl = cp; }

protected:
	void Create();

	int GetCaretPosition();

	void Draw();
	void ShowCaret();
	void HideCaret();
	void MouseToCaretPosition( int p );
};
//----------------------------------------------------------------------------
#endif
