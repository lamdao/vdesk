#ifndef __MENU_H
#define __MENU_H
//-----------------------------------------------------------------------------
#include "Event.h"
#include "Window.h"
#include "Canvas.h"
#include "Action.h"
//-----------------------------------------------------------------------------
#define ID_SEPARATOR	-1
//-----------------------------------------------------------------------------
#define MenuSeparator()	MenuItem()
//-----------------------------------------------------------------------------
class PopupMenu;
//-----------------------------------------------------------------------------
class MenuItem {
private:
	int id;
	string name;
	bool enabled;
	bool selected;

	int x, y;

	PopupMenu *parent;
public:
	MenuItem():
		id( ID_SEPARATOR ), name(""), enabled(true), selected(false) {}
	MenuItem( int n, char *s, bool e=true ):
		id( n ), name( s ), enabled(e), selected(false) {}

	bool Contains( int x, int y );

	void SetName( const char *n ) { name = string(n); }
	void SetParent( PopupMenu *p ) { parent = p; }
	void SetPosition( int x, int y ) { this->x = x; this->y = y; }
	bool IsSeparator() { return id==ID_SEPARATOR; }

	void SetEnabled( bool e ) { enabled = e; }
	bool IsEnabled() { return enabled; }

	void SetSelected( bool s );
	bool IsSelected() { return selected; }

	int X() const { return x; }
	int Y() const { return y; }
	int Length() const { return name.length(); }
	char *Name() const { return (char *)name.c_str(); }
	int Id() const { return id; }
};
//-----------------------------------------------------------------------------
class PopupMenu: public WinControl, public EventControl {
protected:
	Canvas *canvas;
	vector<MenuItem*> items;
	ActionControl *action;
	MenuItem *current;
	string title;

	int defwidth;
public:
	PopupMenu( MenuItem *item, int count );
	virtual ~PopupMenu();

	void ShowAt( int x, int y );
	void Hide();

	virtual void ProcessEvent( XEvent *e );

	virtual void Appear( XEvent *e );
	virtual void MouseLeave( XMotionEvent *e );
	virtual void MouseMove( XMotionEvent *e );
	virtual void MouseClick( XButtonEvent *e );

	void Draw();
	void DrawItem( MenuItem *mi );

	void SetTitle( const char *t );
	void SetActionControl( ActionControl *c ) { action = c; }
	int ItemHeight() const { return Font->height; }
};
//-----------------------------------------------------------------------------
#endif
