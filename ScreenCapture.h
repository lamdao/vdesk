#ifndef __SCREENCAPTURE_H
#define __SCREENCAPTURE_H
//----------------------------------------------------------------------------
#include "Dialog.h"
#include "Editor.h"
#include "Timer.h"
//----------------------------------------------------------------------------
class ScreenCapture: public Dialog, public TimerControl {
private:
	WinControl *sample;
	Editor *name;

	Button *save;
	Button *cancel;

	string save_folder;

	static ImlibImage *shot;

public:
	ScreenCapture();
	~ScreenCapture();

	virtual void Appear( XEvent *e );
	virtual void Perform( int id );

	void Activate( int x, int y );

	virtual void OnTime();
protected:
	virtual void Draw();

private:
	static void *Save(void *);
};
//----------------------------------------------------------------------------
#endif
