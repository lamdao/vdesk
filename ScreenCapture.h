#ifndef __SCREENCAPTURE_H
#define __SCREENCAPTURE_H
//----------------------------------------------------------------------------
#include "Dialog.h"
#include "Editor.h"
//----------------------------------------------------------------------------
class ScreenCapture: public Dialog {
private:
	WinControl *sample;
	Editor *name;

	Button *save;
	Button *cancel;
public:
	ScreenCapture();
	~ScreenCapture();

	virtual void Appear( XEvent *e );
	virtual void Perform( int id );

	void Activate( int x, int y );
protected:
	virtual void Draw();
};
//----------------------------------------------------------------------------
#endif
