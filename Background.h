#ifndef __BACKGROUND_H
#define __BACKGROUND_H
//----------------------------------------------------------------------------
#include "Action.h"
#include "Timer.h"
//----------------------------------------------------------------------------
class Background: public Resource, public TimerControl {
private:
	vector<char*> data[2];
	vector<char*> *save, *show;
	ImlibImage *SpareRoot;
	int delay, mode;
	bool refreshable;
	char *source;
	time_t srctime;
	string images;

	ActionControl *controls;
	int command;

public:
	Background( char *s, char *m=NULL, int d=0 );
	~Background();

	void SetDelay( int d );
	void SetSource( char *s );
	void AddRefreshListener( ActionControl *a, int cmd );
	void Reload();
	void Refresh();

	ImlibImage *Crop( int x, int y, int w, int h );

	bool Refreshable() { return refreshable; }
	bool IsOneShot() { return false; }
	void OnTime();

private:
	void Init();
	void FreeData();
	void SwapData();
	void ScanSource( char *s );
	void ChangeImage();
};
//----------------------------------------------------------------------------
#endif
