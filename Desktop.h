#ifndef __DESKTOP_H
#define __DESKTOP_H
//----------------------------------------------------------------------------
#include "Container.h"
#include "Resource.h"
#include "Action.h"
//----------------------------------------------------------------------------
class Timer;
class Trash;
class ScreenCapture;
//----------------------------------------------------------------------------
class Desktop: public Resource, public Container, public ActionControl {
private:
	vector<Link*> items;
	Timer *timer;
	Trash *trash;
	ScreenCapture *capturer;

	bool shutdown;
public:
	Desktop();
	~Desktop();

	void Load();
	void Save();
	void Arrange();
	void Refresh();
	void Reload();
	void SetLock( bool l );

	void Add( Link *l );
	void Delete( Link *l );

	void Process();
	void Perform( int id );

	static void ProcessEvent( XEvent *e );
private:
	void CreateMenu();
	void DeleteMenu();
};
//----------------------------------------------------------------------------
#endif
