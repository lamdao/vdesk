#include "Color.h"
//----------------------------------------------------------------------------
Color::Color( const char *name )
{
	Set( name );
}
//----------------------------------------------------------------------------
Color::~Color()
{
	Free();
}
//----------------------------------------------------------------------------
void Color::Free()
{
	if( ready ) {
		XftColorFree( Resource::display, Resource::ScreenVisual,
			Resource::ScreenColormap, &data );
		ready = false;
	}
}
//----------------------------------------------------------------------------
void Color::Copy( const Color &c, bool reverse )
{
	ready = c.ready;
	if( ready ) {
		if( !reverse ) {
			XRenderColor xrdc;
			xrdc.red = c.data.color.red;
			xrdc.blue = c.data.color.blue;
			xrdc.green = c.data.color.green;
			XftColorAllocValue( Resource::display, Resource::ScreenVisual,
					Resource::ScreenColormap, &xrdc, &data );
		}
		else {
			char s[32];
			sprintf( s, "#%02X%02X%02X",
						(uchar)(255 - (c.data.color.red >> 8)),
						(uchar)(255 - (c.data.color.blue >> 8)),
						(uchar)(255 - (c.data.color.green >> 8)) );
			XftColorAllocName( Resource::display, Resource::ScreenVisual,
					Resource::ScreenColormap, s, &data );
		}
	}
}
//----------------------------------------------------------------------------
void Color::Set( const char *name )
{
	if( !name ) return;

	Free();
	XftColorAllocName( Resource::display, Resource::ScreenVisual,
			Resource::ScreenColormap, (char *)name, &data );
	ready = true;
}
//----------------------------------------------------------------------------
void Color::Set( int v, const XftColor *c )
{
	XRenderColor xrdc;

	Free();
	xrdc.red = (ushort)v - c->color.red;
	xrdc.blue = (ushort)v - c->color.blue;
	xrdc.green = (ushort)v - c->color.green;
	XftColorAllocValue( Resource::display, Resource::ScreenVisual,
			Resource::ScreenColormap, &xrdc, &data );
	ready = true;
}
//----------------------------------------------------------------------------
void Color::Load( Table *table, char *name, char *defname )
{
	Set( table->QueryAsStr( name, defname ) );
}
//----------------------------------------------------------------------------
string Color::ConvertToString()
{
	char s[32];
	sprintf( s, "#%02X%02X%02X", 
			(uchar)(data.color.red >> 8),
			(uchar)(data.color.green >> 8),
			(uchar)(data.color.blue >> 8) );
	return string( s );
}
//----------------------------------------------------------------------------
