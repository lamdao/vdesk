#ifndef __WINDOW_H
#define __WINDOW_H
//----------------------------------------------------------------------------
#include "Resource.h"
#include "Event.h"
//----------------------------------------------------------------------------
class WinControl: public Resource {
private:
	XSetWindowAttributes attr;
	bool visible;

protected:
	int x, y;
	int width, height;
	int border;

	Window parent;
	Window handler;
	WinControl *pctrl;
public:
	WinControl( int mask );
	WinControl( Window p, int mask, int b=1 );
	virtual ~WinControl();

	void SetEventMask( int mask );
	void ClearEventMask( int mask );
	void SetCursor( Cursor c );
	void SetBorderWidth( int w );

	void CreateHandler();
	void CreateHandler( int w, int h );
	void CreateHandler( int x, int y, int w, int h );
	void Resize( int w, int h );
	void MoveTo( int x, int y );
	void Raise();
	void Lower();

	virtual void Show();
	virtual void Hide();
	virtual void ShowAt( int x, int y );
	virtual void SetEventControl( EventControl *c );
	virtual void SetFocus();

	virtual void NextControl() {}
	virtual void PrevControl() {}
	virtual void SetActiveControl( WinControl *c ) {}
	virtual void SetParentControl( WinControl *p ) { pctrl = p; }
	virtual void CancelProcess() {}
	virtual void AcceptProcess() {}

	int X() const { return x; }
	int Y() const { return y; }
	int Width() const { return width; }
	int Height() const { return height; }
	Window Handler() const { return handler; }
	Window Parent() const { return parent; }

	virtual bool IsVisibled() { return visible; }
protected:
	virtual void SetVisibled( bool v ) { visible = v; }
};
//----------------------------------------------------------------------------
#endif
