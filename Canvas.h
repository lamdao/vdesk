#ifndef __CANVAS_H
#define __CANVAS_H
//-----------------------------------------------------------------------------
#include "Resource.h"
//-----------------------------------------------------------------------------
class Canvas: public Resource {
private:
	XftDraw *draw;
	Window drawable;
	GC gc;
public:
	Canvas( Window drawable );
	~Canvas();

	void DrawText( int x, int y, char *text, int length, Color color );
	void DrawRectangle( int x, int y, int w, int h, Color color );
	void DrawLine( int x1, int y1, int x2, int y2, Color color );

	XImage *GetImage( int x, int y, int w, int h );
	void PutImage( int x, int y, XImage *im );
};
//-----------------------------------------------------------------------------
#endif
