#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "Background.h"
//----------------------------------------------------------------------------
Background::Background( char *s, char *m, int d ): TimerControl(), Resource(),
						save(NULL), show(NULL), SpareRoot(NULL),
						delay(0), mode(0), refreshable(false),
						source(NULL), srctime(0), images(""),
						controls(NULL), command(0)
{
	Init();
	ScanSource( source = strdup( s ) );
	SetDelay( d );
	ChangeImage();
}
//----------------------------------------------------------------------------
Background::~Background()
{
	if( SpareRoot ) Imlib_kill_image( ScreenData, SpareRoot );
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
	if( !show ) {
		show = &data[0];
		save = &data[1];
	}
	srand( time(0) );
}
//----------------------------------------------------------------------------
void Background::FreeData()
{
	if( source ) free( source );
	for( int n=0; n<2; n++ ) {
		for( int i=0; i<data[n].size(); i++ )
			delete data[n][i];
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
	if( d>=0 && d<=60 ) {
		delay = d;
		if( show->size() + save->size() <= 1 ) return;
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
	if( !s || !*s ) return;

	struct stat b;
	string path = s[0]=='~' ? HomeFolder + &s[1] : s;

	if( stat( path.c_str(), &b ) < 0 ) {
		cerr << "[vdesk] Background's source not found." << endl;
		return;
	}

	if( srctime == b.st_mtime ) return;

	srctime = b.st_mtime;
	if( !S_ISDIR( b.st_mode ) )
		show->push_back( strdup( path.c_str() ) );
	else {
		struct dirent **files;
		int p, n = scandir( path.c_str(), &files, 0, 0 );
		path += "/";
		for( int i=0; i<n; i++, free( files[i] ) ) {
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
}
//----------------------------------------------------------------------------
void Background::ChangeImage()
{
	ImlibImage *si = NULL;

	if( !show->size() && save->size() )
		SwapData();

	if( show->size() > 0 ) {
		while( true ) {
			char *f;
			int c = (int)((float)(show->size()-1) * rand() / (RAND_MAX+1.0));
			si = Imlib_load_image( ScreenData, f = (*show)[c] );
		#if 0
			cerr << "[" << c << "/" << show->size() << "]" << f << endl;
		#endif
			show->erase( show->begin() + c );
			if( si ) {
				save->push_back( f );
				break;
			}
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

	Imlib_render( ScreenData, si, ScreenWidth, ScreenHeight );
	Pixmap p = Imlib_copy_image( ScreenData, si );
	SpareRoot = Imlib_create_image_from_drawable( ScreenData, p, 0,
					0, 0, ScreenWidth, ScreenHeight );
	Imlib_apply_image( ScreenData, SpareRoot, Root );
	Imlib_free_pixmap( ScreenData, p );
	Imlib_kill_image( ScreenData, si );

	ScanSource( source );
}
//----------------------------------------------------------------------------
