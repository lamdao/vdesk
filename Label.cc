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
	uchar *rgb = bg->rgb_data;

	long sum = 0;
	for( int i = width * height - 1; i >= 0; i--, rgb += 3 )
		sum += (19595 * rgb[0] + 38470 * rgb[1] + 7471 * rgb[2]) >> 16;
	sum /= (width * height);
	reverse = sum < 100;

	Imlib_apply_image( ScreenData, bg, this->handler );
	Imlib_kill_image( ScreenData, bg );
}
//-----------------------------------------------------------------------------
void Label::Update() 
{
	XClearWindow( display, this->handler );

	for( int i=0; i<data.size(); i++ ) {
		Text *txt = data[i];
		if( HighContrast ) {
			for( int i = 0; i < 5; i++ ) {
				canvas->DrawText( txt->x + i - 2, txt->y,
							txt->data, txt->length, BColor );
				canvas->DrawText( txt->x, txt->y + i - 2,
							txt->data, txt->length, BColor );
			}
		}

		if( Shadow ) {
			canvas->DrawText( txt->x + ShadowX, txt->y + ShadowY,
							txt->data, txt->length, SColor );
		}

		canvas->DrawText( txt->x, txt->y, txt->data, txt->length, reverse ? CSelectedText : FColor );
	}
}
//-----------------------------------------------------------------------------
