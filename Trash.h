#ifndef __TRASH_H
#define __TRASH_H
//----------------------------------------------------------------------------
#include "Timer.h"
//----------------------------------------------------------------------------
class Link;
//----------------------------------------------------------------------------
class Trash: public TimerControl {
private:
	vector<Link*> items;

	void Empty();
public:
	~Trash();
	void Add( Link *l );
	void Add( vector<Link*> &v );
	void OnTime();
};
//----------------------------------------------------------------------------
#endif
