#ifndef __COLOR_H
#define __COLOR_H
//----------------------------------------------------------------------------
#include "Resource.h"
//----------------------------------------------------------------------------
class Color {
private:
	bool ready;
	XftColor data;
public:
	Color(): ready(false) {}
	Color( const char *s );
	~Color();

	void Set( const char *s );
	void Set( int v, const XftColor *c );
	void Load( Table *table, char *name, char *defname=NULL );
	void SetAlpha( int a ) { data.color.alpha = a; }
	void Free();

	string ConvertToString();

	operator unsigned long () const { return data.pixel; }
	operator const XftColor * () const { return &data; }
	operator XftColor * () { return &data; }
};
//----------------------------------------------------------------------------
#endif
