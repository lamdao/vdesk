#include "Canvas.h"
//-----------------------------------------------------------------------------
Canvas::Canvas( Window drawable ): Resource()
{
	draw = XftDrawCreate( display, drawable, ScreenVisual, ScreenColormap );
	gc = XCreateGC( display, drawable, 0, 0 );
	this->drawable = drawable;
}
//-----------------------------------------------------------------------------
Canvas::~Canvas()
{
	XFreeGC( display, gc );
	XftDrawDestroy( draw );
}
//-----------------------------------------------------------------------------
void Canvas::DrawText( int x, int y, char *text, int length, Color color )
{
#ifdef USE_MB_TEXT
	XSetForeground( display, gc, color );
	XmbDrawString( display, drawable, FontSet, gc, x, y, text, length );
#else
	XftDrawStringUtf8( draw, color, Font, x, y, (XftChar8*)text, length );
#endif
}
//-----------------------------------------------------------------------------
void Canvas::DrawRectangle( int x, int y, int w, int h, Color color )
{
	XSetForeground( display, gc, color );
	XFillRectangle( display, drawable, gc, x, y, w, h );
}
//-----------------------------------------------------------------------------
void Canvas::DrawLine( int x1, int y1, int x2, int y2, Color color )
{
	XSetForeground( display, gc, color );
	XDrawLine( display, drawable, gc, x1, y1, x2, y2 );
}
//-----------------------------------------------------------------------------
XImage* Canvas::GetImage( int x, int y, int w, int h )
{
	return XGetImage( display, drawable, x, y, w, h, AllPlanes, ZPixmap );
}
//-----------------------------------------------------------------------------
void Canvas::PutImage( int x, int y, XImage *im )
{
	XPutImage( display, drawable, gc, im, 0, 0, x, y, im->width, im->height );
}
//-----------------------------------------------------------------------------
