#ifndef __TEXT_H
#define __TEXT_H
//-----------------------------------------------------------------------------
#include "Resource.h"
//-----------------------------------------------------------------------------
class Label;
//-----------------------------------------------------------------------------
class Text: public Resource {
	friend class Label;
private:
	int x, y;
	int width;
	int length;
	char *data;
public:
	Text( char *text );

	static int Width( const char *s, int l=0 );
};
//-----------------------------------------------------------------------------
#endif
