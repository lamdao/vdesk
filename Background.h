#ifndef __BACKGROUND_H
#define __BACKGROUND_H
//----------------------------------------------------------------------------
#include "Action.h"
#include "Timer.h"
//----------------------------------------------------------------------------
class Background: public Resource, public TimerControl {
private:
	vector<char*> data;
	ImlibImage *SpareRoot;
	int delay, current, mode;
	bool refreshable;
	char *source;

	ActionControl *controls;
	int command;

public:
	Background( char *s, char *m=NULL, int d=0 );
	~Background();

	void Init();
	void SetDelay( int d );
	void SetSource( char *s );
	void AddRefreshListener( ActionControl *a, int cmd );
	void Refresh();

	ImlibImage *Crop( int x, int y, int w, int h );

	bool Refreshable() { return refreshable; }
	bool IsOneShot() { return false; }
	void OnTime();

private:
	void ScanSource( char *s );
	void ChangeImage();
};
//----------------------------------------------------------------------------
#endif
