#include "Background.h"
#include "Label.h"
//-----------------------------------------------------------------------------
#define ID_TEXTMASK \
	SubstructureRedirectMask|SubstructureNotifyMask| \
	ButtonPressMask|ButtonReleaseMask|PointerMotionMask| \
	ExposureMask|EnterWindowMask|LeaveWindowMask
//-----------------------------------------------------------------------------
Label::Label( char *text ): WinControl( ID_TEXTMASK )
{
	CreateHandler( 1, 1 );
	canvas = new Canvas( this->handler );
	caption = NULL;
	SetCaption( text );
	reverse = false;
}
//-----------------------------------------------------------------------------
Label::~Label()
{
	Free();
	delete canvas;
}
//-----------------------------------------------------------------------------
void Label::Free()
{
	free( caption );

	for( int i=data.size()-1; i>=0; i-- )
		delete data[i];
	data.clear();
}
//-----------------------------------------------------------------------------
void Label::SetCaption( char *text )
{
	int i;
	char *p;

	if( caption ) Free();

	caption = strdup( text );
	p = strtok( caption, ";" );
	for( i=0; p; i++ ) {
		data.push_back( new Text( p ) );
		p = strtok( NULL, ";" );
	}

	int y = Font->ascent;
	for( i=width=0; i<data.size(); i++ ) {
		Text *txt = data[i];
		if( width < txt->width )
			width = txt->width;
		txt->y = y + i * Font->height;
	}

	width += 4;
	for( i=0; i<data.size(); i++ )
		data[i]->x = (width - data[i]->width)/2;

	height = i * Font->height;

	Resize( width, height );
}
//-----------------------------------------------------------------------------
void Label::UpdateBackground()
{
	ImlibImage *bg = VdeskBg->Crop( x, y, width, height );

	if( AdaptiveText ) {
		register uchar *rgb = bg->rgb_data;

		long a = 0, b = 0;
		for( int i = width * height; i > 0; i--, rgb += 3 ) {
			if( ((19595 * rgb[0] + 38470 * rgb[1] + 7471 * rgb[2]) >> 16) <= DarkLevel )
				a++;
			else
				b++;
		}
		reverse = (100 - DarkRatio) * a >= DarkRatio * b;
	}

	Imlib_apply_image( ScreenData, bg, this->handler );
	Imlib_kill_image( ScreenData, bg );
}
//-----------------------------------------------------------------------------
void Label::Update() 
{
	Color *fcolor, *scolor;

	if( reverse ) {
		fcolor = &BColor;
		scolor = &RColor;
	}
	else {
		fcolor = &FColor;
		scolor = &SColor;
	}

	XClearWindow( display, this->handler );

	for( int n = 0; n < data.size(); n++ ) {
		Text *txt = data[n];
		if( HighContrast ) {
			for( int i = 0; i < 5; i++ ) {
				canvas->DrawText( txt->x + i - 2, txt->y,
							txt->data, txt->length, BColor );
				canvas->DrawText( txt->x, txt->y + i - 2,
							txt->data, txt->length, BColor );
			}
		}
		else
		if( Shadow ) {
			canvas->DrawText( txt->x + ShadowX, txt->y + ShadowY,
							txt->data, txt->length, *scolor );
		}

		canvas->DrawText( txt->x, txt->y, txt->data, txt->length, *fcolor );
	}
}
//-----------------------------------------------------------------------------
