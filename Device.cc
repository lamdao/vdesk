#include <errno.h>
#include "Timer.h"
#include "Device.h"
#include "Menu.h"
#include "Dialog.h"
//-----------------------------------------------------------------------------
#define DEVICE_ON		((char *)images[1].c_str())
#define DEVICE_OFF		((char *)images[0].c_str())
//-----------------------------------------------------------------------------
enum {
	DEVICE_BROWSE	= 0,
	DEVICE_MOUNT	= 2,
	DEVICE_UMOUNT	= 3,
	DEVICE_BROWSER	= 5,
	DEVICE_ICON		= 6,
	DEVICE_RENAME	= 7,
	DEVICE_DELETE	= 8,
	DEVICE_EDIT     = 9,
};
//-----------------------------------------------------------------------------
static MenuItem items[] = {
	MenuItem( DEVICE_BROWSE, "Browse", false ),
	MenuSeparator(),
	MenuItem( DEVICE_MOUNT, "Mount" ),
	MenuItem( DEVICE_UMOUNT, "Unmount", false ),
	MenuSeparator(),
	MenuItem( DEVICE_RENAME, "Rename", false ),
	MenuItem( DEVICE_DELETE, "Delete" ),
	MenuSeparator(),
	MenuItem( DEVICE_EDIT, "Properties", false ),
};
//-----------------------------------------------------------------------------
class DeviceMonitor: public TimerControl {
private:
	vector<Device*> devices;
public:
	DeviceMonitor()
	{
		Timer::Add( this, 15 );
	}
	bool IsOneShot()
	{
		return false;
	}
	void OnTime()
	{
		if( !devices.size() ) return;

		char buf[1024], *p;
		FILE *fp = fopen( "/proc/mounts", "r" );

		if( !fp ) return;

		vector<string> entry;
		while( fgets(buf, sizeof(buf), fp) )
			entry.push_back( string(strtok(buf, " ")) );
		fclose( fp );

		for( int i=0; i<devices.size(); i++ ) {
			int n;
			Device *d = devices[i];
			if( d->IsMounted() ) {
				for( n=0; n<entry.size(); n++ )
					if( entry[n] == d->Entry() ) break;
				if( n>=entry.size() )
					d->SetMounted( false );
			}
			else {
				for( n=0; n<entry.size(); n++ )
					if( entry[n] == d->Entry() ) break;
				if( n<entry.size() )
					d->SetMounted( true );
			}
		}
	}

	void Add( Device *d )
	{
		devices.push_back( d );
	}

	void Remove( Device *d )
	{
		for( int i=0; i<devices.size(); i++ )
			if( d==devices[i] ) {
				devices.erase( devices.begin() + i );
				return;
			}
	}
};
//-----------------------------------------------------------------------------
DeviceMonitor monitor;
//-----------------------------------------------------------------------------
// Device
//-----------------------------------------------------------------------------
PopupMenu *Device::menu = NULL;
//-----------------------------------------------------------------------------
Device::Device( char *f, Table *t ): Link( f, t )
{
	device = t->Query( "Device" );
	if( device=="" ) throw "Need to specify device!";

	folder = t->Query( "Folder" );
	if( folder=="" ) throw "Need to specify mount point!";

	mount =  t->Query( "Mount" );
	if( mount=="" ) mount = string("mount");
	mount = mount + " " + device + " " + folder + " 2>&1; echo -e \"\\n$?\"";

	umount = t->Query( "Umount" );
	if( umount=="" ) umount = string( "umount" );
	umount = umount + " " + folder + " 2>&1; echo -e \"\\n$?\"";

	images[0] = t->Query( "Icon" );
	images[1] = t->Query( "ActiveIcon" );

	UpdateStatus();
	monitor.Add( this );
}
//-----------------------------------------------------------------------------
Device::~Device()
{
	monitor.Remove( this );
}
//-----------------------------------------------------------------------------
void Device::CreateMenu()
{
	if( !menu ) {
		menu = new PopupMenu( items, SIZE_OF_ARRAY(items) );
	}
}
//-----------------------------------------------------------------------------
void Device::DeleteMenu()
{
	if( menu ) {
		delete menu;
		menu = NULL;
	}
}
//-----------------------------------------------------------------------------
void Device::UpdateStatus()
{
	FILE *fp = popen( "mount", "r" );
	if( !fp ) return;

	char buf[1024], *p = (char *)device.c_str();

	mounted = false;
	while( fgets(buf, sizeof(buf), fp) ) {
		if( strstr(buf, p)==buf ) {
			SetMounted( true );
			break;
		}
	}
	pclose( fp );
}
//-----------------------------------------------------------------------------
void Device::SetMounted( bool m )
{
	mounted = m;
	Icon->SetSource( m ? DEVICE_ON : DEVICE_OFF );
	Icon->Draw();
}
//-----------------------------------------------------------------------------
void Device::UpdateMenuItem()
{
	items[ DEVICE_MOUNT ].SetEnabled( !mounted );
	items[ DEVICE_UMOUNT ].SetEnabled( mounted );
	items[ DEVICE_BROWSE ].SetEnabled( mounted );
}
//-----------------------------------------------------------------------------
void Device::Perform( int id )
{
	switch( id ) {
		case DEVICE_BROWSE:
			if( !mounted )
				Perform( DEVICE_MOUNT );
			else
			if( Command!="" )
				Link::Execute();
			break;
		case DEVICE_MOUNT:
			if( Execute( mount.c_str() ) ) {
				SetMounted( true );
				Perform( DEVICE_BROWSE );
			}
			break;
		case DEVICE_UMOUNT:
			if( Execute( umount.c_str() ) )
				SetMounted( false );
			break;
		case DEVICE_RENAME:
			Rename();
			break;
		case DEVICE_DELETE:
			Delete();
			break;
	}
}
//-----------------------------------------------------------------------------
void Device::ShowPopupMenu( int x, int y )
{
	if( menu ) {
		UpdateMenuItem();
		menu->SetTitle( Text->Caption() );
		menu->SetActionControl( this );
		menu->ShowAt( x, y );
	}
}
//-----------------------------------------------------------------------------
bool Device::Execute( const char *cmd )
{
	FILE *fp = popen( cmd, "r" );
	if( !fp ) {
		Message::Error( errno, strerror(errno) );
		return false;
	}

	int err = 0;
	string errmsg = "";
	char s[256];

	while( fgets( s, sizeof(s), fp ) ) {
		if( *s=='\n' ) break;
		errmsg += s;
	}
	fgets( s, sizeof(s), fp );
	pclose( fp );

	err = atoi( s );
	if( err ) {
		Message::Error( err, (char *)errmsg.c_str() );
		return false;
	}
	if( errmsg.length() )
		Message::Warning( (char *)errmsg.c_str() );
	return true;
}
//-----------------------------------------------------------------------------
void Device::Execute()
{
	Perform( DEVICE_BROWSE );
}
//-----------------------------------------------------------------------------
