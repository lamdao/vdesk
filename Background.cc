#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "Background.h"
//----------------------------------------------------------------------------
Background::Background( char *s, char *m, int d ): TimerControl(), Resource(),
				SpareRoot(NULL), delay(0), current(-1), mode(0),
				refreshable(false), source(NULL), controls(NULL), command(0)
{
	Init();
	ScanSource( source = strdup( s ) );
	SetDelay( d );
}
//----------------------------------------------------------------------------
Background::~Background()
{
	if( SpareRoot ) Imlib_kill_image( ScreenData, SpareRoot );
	if( source ) free( source );
	for( int i=0; i<data.size(); i++ )
		delete data[i];
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
}
//----------------------------------------------------------------------------
void Background::SetDelay( int d )
{
	if( d>=0 && d<=60 ) {
		delay = d;
		if( data.size()<=1 ) return;
		if( !delay )
			Timer::Remove( this );
		else
			Timer::Add( this, delay * 60 );
	}
}
//----------------------------------------------------------------------------
void Background::SetSource( char *s )
{
	if( s ) {
		if( source ) free( source );
		for( int i=0; i<data.size(); i++ )
			delete data[i];
		data.clear();

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
ImlibImage *Background::Crop( int x, int y, int w, int h )
{
	if( !SpareRoot ) {
		cerr << "Erh???????" << endl;
		cerr << endl << endl << endl;
	}
	return Imlib_crop_and_clone_image( ScreenData, SpareRoot, x, y, w, h );
}
//----------------------------------------------------------------------------
void Background::ScanSource( char *s )
{
	if( s && *s ) {
		struct stat b;
		string path = s[0]=='~' ? HomeFolder + &s[1] : s;

		if( stat( path.c_str(), &b ) >= 0 ) {
			if( !S_ISDIR( b.st_mode ) )
				data.push_back( strdup( path.c_str() ) );
			else {
				struct dirent **files;
				int n = scandir( path.c_str(), &files, 0, 0 );
				path += "/";
				for( int i=0; i<n; i++ ) {
					if( !strcmp( files[i]->d_name, "." ) ||
						!strcmp( files[i]->d_name, ".." ) ) continue;

					string f = path + files[i]->d_name;
					data.push_back( strdup( f.c_str() ) );
					free( files[i] );
				}
			}
		}
	}
	ChangeImage();
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
}
//----------------------------------------------------------------------------
void Background::ChangeImage()
{
	ImlibImage *si = NULL;

	if( data.size() > 0 ) {
		int c = current < 0 ? 0 : current;

		current = c;
		while( true ) {
			si = Imlib_load_image( ScreenData, data[current] );
			current = (current + 1) % data.size();
			if( si || current==c ) break;
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

	Imlib_render( ScreenData, si, ScreenWidth, ScreenHeight );
	Pixmap p = Imlib_copy_image( ScreenData, si );
	SpareRoot = Imlib_create_image_from_drawable( ScreenData, p, 0,
					0, 0, ScreenWidth, ScreenHeight );
	Imlib_apply_image( ScreenData, SpareRoot, Root );
	Imlib_free_pixmap( ScreenData, p );
	Imlib_kill_image( ScreenData, si );
}
//----------------------------------------------------------------------------
