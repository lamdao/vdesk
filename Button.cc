#include <X11/keysym.h>
#include "Button.h"
#include "Text.h"
//-----------------------------------------------------------------------------
#define ID_BUTTONMASK \
	ButtonPressMask|ButtonReleaseMask|EnterWindowMask|LeaveWindowMask| \
	ExposureMask|KeyPressMask|FocusChangeMask
//----------------------------------------------------------------------------
Button::Button( Window parent, int id, const char *text, int w, int h ):
				WinControl( parent, ID_BUTTONMASK ), EventControl()
{
	this->id = id;
	this->caption = text;
	if( !w ) w = Text::Width( text ) + 16;
	if( !h ) h = Font->height + 8;

	CreateHandler( w, h );
	SetEventControl( this );

	canvas = new Canvas( handler );
	enabled = true;
}
//----------------------------------------------------------------------------
Button::~Button()
{
	SetEventControl( NULL );
	delete canvas;
}
//----------------------------------------------------------------------------
void Button::Draw()
{
	Color *bc, *fc;

	bc = &CBackground;
	fc = enabled ? &CForeground : &CDisabledText;
	int x = (width - Text::Width( caption.c_str(), caption.length() )) / 2,
		y = 4 + Font->ascent;
	canvas->DrawRectangle( 0, 0, width, height, *bc );
	canvas->DrawText( x, y, (char *)caption.c_str(), caption.length(), *fc );
	XSetWindowBorder( display, this->handler, CForeground );
}
//----------------------------------------------------------------------------
void Button::Appear( XEvent *e )
{
	SetVisibled( true );
	Draw();
}
//----------------------------------------------------------------------------
void Button::MouseClick( XButtonEvent *e )
{
	if( action && enabled )
		action->Perform( id );
}
//----------------------------------------------------------------------------
void Button::KeyDown( XKeyEvent *e )
{
	KeySym k = XKeycodeToKeysym(display, e->keycode, e->state & 0x03 ? 1 : 0);
	switch( k ) {
		case XK_Return:
		case XK_space:
			if( action && enabled ) action->Perform( id );
			break;
		case XK_Escape:
			if( pctrl ) pctrl->CancelProcess();
			break;
		case XK_Tab:
			if( pctrl ) pctrl->NextControl();
			break;
		case XK_ISO_Left_Tab:
			if( pctrl ) pctrl->PrevControl();
			break;
	}
}
//----------------------------------------------------------------------------
void Button::HasFocus( XFocusInEvent *e )
{
	XSetWindowBorder( display, this->handler, CDisabledText );
}
//----------------------------------------------------------------------------
void Button::LostFocus( XFocusOutEvent *e )
{
	XSetWindowBorder( display, this->handler, CForeground );
}
//----------------------------------------------------------------------------
