#include "Background.h"
#include "Desktop.h"
#include "BusyCursor.h"
#include "ScreenCapture.h"
#include "Device.h"
#include "Timer.h"
#include "Trash.h"
#include "Menu.h"
#include "Dialog.h"
#include <dirent.h>
#include <X11/keysym.h>
//-----------------------------------------------------------------------------
enum {
	DESKTOP_LINK	= 0,
	DESKTOP_REFRESH	= 2,
	DESKTOP_RELOAD	= 3,
	DESKTOP_LOCK	= 4,
	DESKTOP_CAPTURE	= 6,
	DESKTOP_SETTING = 8,
	DESKTOP_ABOUT	= 9,
	DESKTOP_EXIT	= 11,
};
//-----------------------------------------------------------------------------
static MenuItem vdesk_items[] = {
	MenuItem( DESKTOP_LINK, "Create link" ),
	MenuSeparator(),
	MenuItem( DESKTOP_REFRESH, "Refresh" ),
	MenuItem( DESKTOP_RELOAD, "Reload" ),
	MenuItem( DESKTOP_LOCK, "Lock icon" ),
	MenuSeparator(),
	MenuItem( DESKTOP_CAPTURE, "Screen capture" ),
	MenuSeparator(),
	MenuItem( DESKTOP_SETTING, "Settings..." ),
	MenuItem( DESKTOP_ABOUT, "About" ),
	MenuSeparator(),
	MenuItem( DESKTOP_EXIT, "Exit" ),
};
//----------------------------------------------------------------------------
PopupMenu *menu = NULL;
//----------------------------------------------------------------------------
// Desktop
//----------------------------------------------------------------------------
Desktop::Desktop(): Resource()
{
	CreateMenu();
	SetLock( Locked );
	shutdown = false;
	trash = new Trash();
	timer = new Timer();
	capturer = new ScreenCapture();
	VdeskBg->AddRefreshListener( this, DESKTOP_REFRESH );
}
//----------------------------------------------------------------------------
Desktop::~Desktop()
{
	delete capturer;
	delete timer;
	delete trash;

	DeleteMenu();
	for( int i=0; i<items.size(); i++ )
		delete items[i];
}
//----------------------------------------------------------------------------
void Desktop::Load()
{
	struct dirent **Files;
	string FileName;
	Database *db;
	Table    *tb;
	int FileCount;

	FileCount = scandir( VdeskFolder.c_str(), &Files, 0, alphasort );

	for( int i=0; i<FileCount; i++ ) {
		FileName = VdeskFolder + Files[i]->d_name;
		if( FileName.rfind( ".lnk" ) != FileName.length() - 4 ) {
			free( Files[ i ] );
			continue;
		}
		db = new Database( FileName );
		if(!(tb = db->Query("Icon"))) {
			free( Files[ i ] );
			delete db;
			continue;
		}
		Link *link;
		try {
			if( !tb->Contains( "Device" ) )
				link = new Link( (char *)FileName.c_str(), tb );
			else
				link = new Device( (char *)FileName.c_str(), tb );
			link->SetParent( this );
			items.push_back( link );
		}
		catch( const char *msg ) {
			Message::Warning( (char *)msg );
			delete link;
		}
		free( Files[ i ] );
		delete db;
	}
	free( Files );
}
//----------------------------------------------------------------------------
void Desktop::Add( Link *l )
{
	l->SetParent( this );
	items.push_back( l );
}
//----------------------------------------------------------------------------
void Desktop::Delete( Link *l )
{
	for( int i=0; i<items.size(); i++ )
		if( items[i] == l ) {
			items.erase( items.begin() + i );
			trash->Add( l );
			return;
		}
}
//----------------------------------------------------------------------------
void Desktop::Save()
{
	for( int i=0; i<items.size(); i++ )
		items[i]->Save();
}
//----------------------------------------------------------------------------
void Desktop::Refresh()
{
	if( !VdeskBg->Refreshable() ) {
		for( int i=0; i<items.size(); i++ )
			items[i]->Update();
	}
	else {
		int i;
		for( i=0; i<items.size(); i++ )
			items[i]->Hide();
		VdeskBg->Refresh();
		for( i=0; i<items.size(); i++ )
			items[i]->Show();
	}
}
//----------------------------------------------------------------------------
void Desktop::Reload()
{
	trash->Add( items );
	Load();
	Arrange();
}
//----------------------------------------------------------------------------
void Desktop::SetLock( bool l )
{
	Locked = l;
	vdesk_items[ DESKTOP_LOCK ].SetName( l ? "Unlock icon" : "Lock icon" );
	Resource::Save();
}
//----------------------------------------------------------------------------
void Desktop::Arrange()
{
	int w = 0;
	int i, x, y = LINK_SPACE;

	if( !items.size() ) return;

	for( i=0; i<items.size(); i++ ) {
		Link *l = items[i];
		if( l->Width() > w ) w = l->Width();
	}

	x = ScreenWidth - w - LINK_SPACE;

	for( i=0; i<items.size(); i++ ) {
		Link *l = items[i];
		if( y + l->Height() > ScreenHeight ) {
			y = LINK_SPACE;
			x = x - w - LINK_SPACE;
		}
		if( l->X() == 0 && l->Y() == 0 ) {
			l->SetPosition(x + ((w - l->Width())/2), y);
			y += l->Height() + LINK_SPACE;
		}
		l->Show();
	}
}
//----------------------------------------------------------------------------
void Desktop::Perform( int id )
{
	switch( id ) {
		case DESKTOP_LINK:
			{
				string fn = Link::ShowEditor( menu->X(), menu->Y() );
				if( fn!="" ) {
					Database db( fn );
					Link *l = new Link((char *)fn.c_str(), db.Query( "Icon" ));
					l->Show();
					Add( l );
				}
			}
			break;
		case DESKTOP_REFRESH:
			Refresh();
			break;
		case DESKTOP_RELOAD:
			Reload();
			break;
		case DESKTOP_LOCK:
			SetLock( !Locked );
			break;
		case DESKTOP_CAPTURE:
			capturer->Activate( menu->X(), menu->Y() );
			break;
		case DESKTOP_SETTING:
			break;
		case DESKTOP_ABOUT:
			Message::Info(	"Vdesk version "VERSION"\n"
							"(C) 2003 MrChuoi <mrchuoi@yahoo.com>\n"
							"http://xvnkb.sourceforge.net/vdesk");
			break;
		case DESKTOP_EXIT:
			exit( 0 );
			shutdown = true;
			break;
	}
}
//----------------------------------------------------------------------------
void Desktop::Process()
{
	XGrabKey( display, XKeysymToKeycode(display, XK_Print), ControlMask, Root,
				True, GrabModeAsync, GrabModeAsync );
	XGrabButton( display, Button3, ControlMask, Root, True, ButtonReleaseMask,
					GrabModeAsync, GrabModeAsync, None, None );
	XSelectInput( display, Root, SubstructureNotifyMask );

	while( !shutdown ) {
		XEvent e;
		if( !XPending( display ) )
			timer->Update();
		else {
			XNextEvent( display, &e );
			EventControl *c = EventControl::Active();
			if( c )
				c->ProcessEvent( &e );
			else
			if( e.type == KeyPress /*&& e.xany.window == Root*/ ) {
				cerr << "kc=" << ((XKeyEvent *)&e)->keycode << endl;
				cerr <<	"ks=" << ((XKeyEvent *)&e)->state << endl;
				//capturer->Activate( menu->X(), menu->Y() );
			}
			else
				Desktop::ProcessEvent( &e );
		}
	}
}
//----------------------------------------------------------------------------
void Desktop::ProcessEvent( XEvent *e )
{
	EventControl *c = EventControl::Find( e->xany.window );

	if( c )
		c->ProcessEvent( e );
	else {
		if( e->type == MapNotify )
			WaitCursor->Update();
		else
		if( e->type == ButtonRelease && e->xany.window == Root )
			menu->ShowAt( e->xbutton.x_root, e->xbutton.y_root );
		else {
			// TODO:
			// Send e to e.xany.window
		}
	}
}
//----------------------------------------------------------------------------
void Desktop::CreateMenu()
{
	menu = new PopupMenu( vdesk_items, SIZE_OF_ARRAY(vdesk_items) );
	menu->SetTitle( " Vdesktop "VERSION" " );
	menu->SetActionControl( this );
	Link::CreateMenu();
	Device::CreateMenu();
}
//----------------------------------------------------------------------------
void Desktop::DeleteMenu()
{
	Device::DeleteMenu();
	Link::DeleteMenu();
	delete menu;
}
//----------------------------------------------------------------------------
