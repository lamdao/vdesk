#ifndef __IMAGE_H
#define __IMAGE_H
//-----------------------------------------------------------------------------
#include "Window.h"
//-----------------------------------------------------------------------------
class Image: public WinControl {
private:
	bool broken;
	bool Glowing;
	bool FreeAlpha;
	bool StateChanged;

	uchar *rgb;
	uchar *alpha[2];
	ImlibImage *source, *bg;

public:
	Image( char *f, Window p = Root );
	virtual ~Image();

	void Draw();
	void SetGlowing( bool g );
	void SetSource( char *f );
	void SetBroken( bool b );

private:
	int Load( char *f );
	void FreeBuffer();
};
//-----------------------------------------------------------------------------
#endif
