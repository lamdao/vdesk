#ifndef __DEVICE_H
#define __DEVICE_H
//-----------------------------------------------------------------------------
#include "Link.h"
#include "Menu.h"
//-----------------------------------------------------------------------------
class Device: public Link {
private:
	bool mounted;
	string device;
	string folder;
	string mount;
	string umount;
	string images[2];

	static PopupMenu *menu;
public:
	Device( char *f, Table *t );
	virtual ~Device();

	virtual void Perform( int id );
	virtual void ShowPopupMenu( int x, int y );
	virtual void Execute();

	void SetMounted( bool m );
	bool IsMounted() const { return mounted; }
	char *Entry() const { return (char *)device.c_str(); }

	static void CreateMenu();
	static void DeleteMenu();
private:
	void UpdateStatus();
	void UpdateMenuItem();
	bool Execute( const char *cmd );
};
//-----------------------------------------------------------------------------
#endif
