#include "Text.h"
//-----------------------------------------------------------------------------
Text::Text( char *text ): Resource()
{
	data = text;
	length = strlen( text );
	width = Text::Width( data, length );
}
//-----------------------------------------------------------------------------
int Text::Width( const char *s, int l )
{
	if( l==0 ) l = strlen( s );
#ifdef USE_MB_TEXT
	XRectangle r;
	XmbTextExtents( FontSet, s, l, 0, &r );
#else
	XGlyphInfo r;
	XftTextExtentsUtf8( display, Font, (XftChar8*)s, l, &r );
#endif
	return r.width;
}
//-----------------------------------------------------------------------------
