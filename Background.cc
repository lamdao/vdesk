#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "Background.h"
//----------------------------------------------------------------------------
#define CLEANER_TIMEOUT		10
//----------------------------------------------------------------------------
int VDESK_DELAY = 0;
//----------------------------------------------------------------------------
Background::Background( char *s, char *m, int d ): TimerControl(), Resource(),
						save(NULL), show(NULL),
						SpareRoot(NULL), RootPixmap(None),
						delay(0), mode(0), refreshable(false),
						source(NULL), srctime(0), images(""),
						controls(NULL), command(0),
						cleaner(&SpareRoot)
{
	Init();
	ScanSource( source = strdup( s ) );
	SetDelay( d );
	ChangeImage();
	Timer::Add( &cleaner, CLEANER_TIMEOUT );
}
//----------------------------------------------------------------------------
Background::~Background()
{
	if( SpareRoot ) Imlib_kill_image( ScreenData, SpareRoot );
	if( RootPixmap != None ) Imlib_free_pixmap( ScreenData, RootPixmap );
	FreeData();
}
//----------------------------------------------------------------------------
void Background::Init()
{
#ifdef __VDESK_TESTING__
	Window r;
	XSetWindowAttributes attr;

	attr.background_pixmap = ParentRelative;
	attr.backing_store = Always;
	attr.event_mask = ExposureMask;
	attr.override_redirect = True;
	r = XCreateWindow( display, Root, 0, 0, ScreenWidth, ScreenHeight, 0,
				CopyFromParent, CopyFromParent, CopyFromParent,
				CWBackPixmap|CWBackingStore|CWOverrideRedirect|CWEventMask,
				&attr );

	if( r != None ) {
		XMapWindow( display, r );
		OriginalRoot = Imlib_create_image_from_drawable(
							ScreenData, r, 0, 0, 0, ScreenWidth, ScreenHeight );
		XDestroyWindow( display, r );
	}

	if( !OriginalRoot )
		OriginalRoot = Imlib_create_image_from_drawable( ScreenData, Root, 0,
							0, 0, ScreenWidth, ScreenHeight );
#endif
	char *p = getenv( "__VDESK_DELAY__" );
	if( p ) {
		VDESK_DELAY = atoi( p );
		if( VDESK_DELAY < 0 ) VDESK_DELAY = 0;
	}
	if( !show ) {
		show = &data[0];
		save = &data[1];
	}
	srand( time(0) );
	rand();
}
//----------------------------------------------------------------------------
void Background::FreeData()
{
	if( source ) free( source );
	for( int n = 0; n < 2; n++ ) {
		for( int i = 0; i < data[n].size(); i++ )
			free( data[n][i] );
		data[n].clear();
	}
}
//----------------------------------------------------------------------------
void Background::SwapData()
{
	vector<char*> *p = show;
	show = save;
	save = p;
}
//----------------------------------------------------------------------------
void Background::SetDelay( int d )
{
	if( d >= 0 && d <= 60 ) {
		delay = d;
		if( !delay )
			Timer::Remove( this );
		else 
			Timer::Add( this, VDESK_DELAY ? VDESK_DELAY : delay * 60 );
	}
}
//----------------------------------------------------------------------------
void Background::SetSource( char *s )
{
	if( s ) {
		FreeData();
		ScanSource( source = strdup( s ) );
	}
}
//----------------------------------------------------------------------------
void Background::Refresh()
{
	if( !refreshable ) return;

	if( SpareRoot ) Imlib_kill_image( ScreenData, SpareRoot );
	SpareRoot = Imlib_create_image_from_drawable(
					ScreenData, Root, 0, 0, 0, ScreenWidth, ScreenHeight );
}
//----------------------------------------------------------------------------
void Background::RenderBackground( ImlibImage *si, bool apply )
{
	Imlib_render( ScreenData, si, ScreenWidth, ScreenHeight );
	Pixmap p = Imlib_copy_image( ScreenData, si );
	SpareRoot = Imlib_create_image_from_drawable( ScreenData, p, 0,
					0, 0, ScreenWidth, ScreenHeight );
	Imlib_kill_image( ScreenData, si );
	if( !apply )
		Imlib_free_pixmap( ScreenData, p );
	else {
		Imlib_apply_image( ScreenData, SpareRoot, Root );
		SetRootAtoms( p );
	}
}
//----------------------------------------------------------------------------
ImlibImage *Background::Crop( int x, int y, int w, int h )
{
	ImlibImage *si = NULL, *s = NULL;

	if( !SpareRoot ) {
		if( !current ) {
			cerr << "[vdesk] Eh??? No SpareRoot???" << endl;
			cerr << endl << endl << endl;
			exit(0);
		}
		RenderBackground( s = Imlib_load_image( ScreenData, current ), false );
	}

	si = Imlib_crop_and_clone_image( ScreenData, SpareRoot, x, y, w, h );
	if( s )
		Timer::Add( &cleaner, CLEANER_TIMEOUT );
	else
		cleaner.Reset();
	return si;
}
//----------------------------------------------------------------------------
void Background::ScanSource( char *s )
{
	if( !s || !*s ) return;

	int p, n;
	struct stat b;
	string path = s[0]=='~' ? HomeFolder + &s[1] : s;

	if( stat( path.c_str(), &b ) < 0 ) {
		cerr << "[vdesk] Background's source not found." << endl;
		if( show->size() + save->size() < 1 ) {
			// TODO:
			// Create a default vdesk bacground here
			// and don't remove it from timer list
			Timer::Remove( this );
		}
		return;
	}

	if( srctime == b.st_mtime ) return;

	srctime = b.st_mtime;
	if( !S_ISDIR( b.st_mode ) ) {
		p = images.find( "|" + path + "|" );
		if( p < 0 ) {
			images += "|" + path + "|";
			show->push_back( strdup( path.c_str() ) );
		}
	}
	else {
		struct dirent **files;
		n = scandir( path.c_str(), &files, 0, 0 );
		path += "/";

		for( int i = 0; i < n; free( files[i] ), i++ ) {
			string f = path + files[i]->d_name;
			if( stat( f.c_str(), &b ) < 0 ) continue;
			if( S_ISDIR( b.st_mode ) ) {
				/* TODO: Each sub-folder will be loaded as collection
				 */
				continue;
			}

			p = images.find( "|" + f + "|" );
			if( p >= 0 ) continue;

			images += "|" + f + "|";
			show->push_back( strdup( f.c_str() ) );
		}
		free( files );
	}
}
//----------------------------------------------------------------------------
void Background::AddRefreshListener( ActionControl *a, int cmd )
{
	controls = a;
	command = cmd;
}
//----------------------------------------------------------------------------
void Background::OnTime()
{
	ChangeImage();
	if( controls ) controls->Perform( command );
	if( SpareRoot ) {
		Imlib_kill_image( ScreenData, SpareRoot );
		SpareRoot = NULL;
	}
}
//----------------------------------------------------------------------------
void Background::ChangeImage()
{
	ImlibImage *si = NULL;

	ScanSource( source );

	if( SpareRoot && (show->size() + save->size() <= 1) )
		return;

	if( !show->size() && save->size() )
		SwapData();

	if( show->size() > 0 ) {
		string sf;
		while( true ) {
			int c = (int)((float)(show->size()-1) * rand() / (RAND_MAX+1.0));
			si = Imlib_load_image( ScreenData, current = (*show)[c] );
			if( VDESK_DELAY )
				cerr << "[" << c << "/" << show->size() << "]"
					 << current << endl;
			show->erase( show->begin() + c );
			if( si ) {
				save->push_back( current );
				break;
			}

			sf = string("|") + current + "|";
			c = images.find( sf.c_str() );
			if( c >= 0 ) images.erase( c, sf.length() );
			free( current );

			if( !show->size() )
				SwapData();
		}
	}

	if( SpareRoot ) {
		Imlib_kill_image( ScreenData, SpareRoot );
		SpareRoot = NULL;
	}

	if( !si ) {
	#if 0
		cerr << "No image found, set spare root => default root" << endl;
	#endif
		refreshable = true;
		Refresh();
		return;
	}

	RenderBackground( si );
}
//----------------------------------------------------------------------------
// This routine was stolen from bsetroot.cc of fluxbox ;-)
// But don't know why it makes xchat crash all the times :-))
//----------------------------------------------------------------------------
void Background::SetRootAtoms(Pixmap p)
{
	Atom atom_root, atom_eroot, type;
	unsigned char *data_root, *data_eroot;
	unsigned long length, after;
	int format;

	atom_root = XInternAtom(display, "_XROOTMAP_ID", true);
	atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", true);

	// doing this to clean up after old background
	if( atom_root != None && atom_eroot != None ) {
		XGetWindowProperty(display, Root,
				atom_root, 0L, 1L, false, AnyPropertyType,
				&type, &format, &length, &after, &data_root);

		if( type == XA_PIXMAP ) {
			XGetWindowProperty(display, Root,
					atom_eroot, 0L, 1L, False, AnyPropertyType,
					&type, &format, &length, &after, &data_eroot);

			if( data_root && data_eroot && type == XA_PIXMAP &&
				*((Pixmap *) data_root) == *((Pixmap *) data_eroot) ) {
				XKillClient(display, *((Pixmap *) data_root));
			}
		}
	}

	atom_root = XInternAtom(display, "_XROOTPMAP_ID", false);
	atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", false);

	if( atom_root == None || atom_eroot == None ) {
		cerr << "couldn't create pixmap atoms, giving up!" << endl;
		return;
	}

	// setting new background atoms
	XChangeProperty(display, Root, atom_root, XA_PIXMAP, 32, PropModeReplace,
			(unsigned char *) &p, 1);
	XChangeProperty(display, Root, atom_eroot, XA_PIXMAP, 32, PropModeReplace,
			(unsigned char *) &p, 1);

	// release old root's pixmap & update new one
	if( RootPixmap != None )
		Imlib_free_pixmap( ScreenData, RootPixmap );
	RootPixmap = p;
}
//----------------------------------------------------------------------------
