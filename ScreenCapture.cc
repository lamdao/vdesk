#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "Text.h"
#include "ScreenCapture.h"
//----------------------------------------------------------------------------
#ifdef __linux__
#include <pthread.h>
#define thread_t						pthread_t
#define thread_mutex_t					pthread_mutex_t
#define thr_create(sb,ss,ff,av,at,id)	pthread_create(id,at,ff,av)
#define thr_init_lock(l)				pthread_mutex_init(l, 0)
#define thr_lock(l)						pthread_mutex_lock(l)
#define thr_unlock(l)					pthread_mutex_unlock(l)
#define thr_destroy_lock(l)				pthread_mutex_destroy(l)
#define thr_kill(t)						pthread_cancel(t)
#define thr_join(t,r)					pthread_join(t,r)
#define thr_detach(t)					pthread_detach(t)
#else
#include <thread.h>
#endif
//----------------------------------------------------------------------------
ImlibImage *ScreenCapture::shot = NULL;
//----------------------------------------------------------------------------
ScreenCapture::ScreenCapture(): Dialog()
{
	width = 300;
	height = 340;
	x = (ScreenWidth - width) / 2;
	y = (ScreenHeight - height) / 2;
	save_folder = HomeFolder;

	CreateHandler();
	SetEventControl( this );
	SetTitle( "Screen capture" );
	canvas = new Canvas( this->handler );

	sample = new WinControl( this->handler, 0, 1 );
	sample->CreateHandler( 22, Font->ascent + 26, 256, 192 );
	sample->SetCursor( ArrowCursor );
	sample->Show();

	int fw = Text::Width( "File:" );
	controls.push_back(name = new Editor( this->handler ));
	name->SetParentControl( this );
	name->SetWidth( 300 - 44 - fw - 10 );
	name->MoveTo( 22 + fw + 10, sample->Y() + sample->Height() + 12 );
	name->Show();

	save = new Button( this->handler, 0, "Save", 80 );
	save->SetActionControl( this );
	save->SetParentControl( this );
	controls.push_back( save );
	int by = height - save->Height() - 12;
	cancel = new Button( this->handler, 1, "Cancel" );
	cancel->SetActionControl( this );
	cancel->SetParentControl( this );
	controls.push_back( cancel );
	save->MoveTo( (width - 160 - 20) / 2, by );
	cancel->MoveTo( save->X() + 80 + 20, by );
	cancel->Show();
	save->Show();
}
//----------------------------------------------------------------------------
ScreenCapture::~ScreenCapture()
{
	delete sample;
	delete name;
	delete save;
	delete cancel;
}
//----------------------------------------------------------------------------
void ScreenCapture::Draw()
{
	Dialog::Draw();

	int fw = Text::Width( "File:" ),
		y = name->Y() + Font->ascent + 4;
	canvas->DrawText( 22, y, "File:", 5, CForeground );
	canvas->DrawLine( 0, y + 14, width, y + 14, CForeground );
}
//----------------------------------------------------------------------------
void ScreenCapture::Appear( XEvent *e )
{
	Dialog::Appear( e );
	name->SetFocus();
	current = 0;
}
//----------------------------------------------------------------------------
void ScreenCapture::Perform( int id )
{
	if( id==0 ) {
		string s = name->GetText();
		const char *p = s.c_str();
		while( *p==' ' ) p++;
		if( !*p ) {
			XBell( display, 25 );
			return;
		}

		struct stat b;
		char *q = (char *)&p[strlen(p) - 1];
		while( q != p && *q == ' ' ) q--;
		*(q + 1) = 0;

		if( !stat( p, &b ) ) { // File exists
			if( S_ISDIR(b.st_mode) ) {
				XBell( display, 25 );
				return;
			}
			if( !Message::Confirm("File exists! Do you want to overwrite it?") )
				return;
		}
	}
	Dialog::Perform( id );
}
//----------------------------------------------------------------------------
void *ScreenCapture::Save(void *v)
{
	Imlib_save_image( ScreenData, shot,	(char *)v, NULL );
	Imlib_kill_image( ScreenData, shot );
	shot = NULL;
	return 0;
}
//----------------------------------------------------------------------------
void ScreenCapture::OnTime()
{
	while( shot != NULL );
	shot = Imlib_create_image_from_drawable( ScreenData, Root, 0,
				0, 0, ScreenWidth, ScreenHeight );
	Imlib_apply_image( ScreenData, shot, sample->Handler() );

	string s = save_folder + "screenshot.png";
	name->SetText( s.c_str() );

	Show();
	if( !Accepted() ) {
		Imlib_kill_image( ScreenData, shot );
		shot = NULL;
	}
	else {
		thread_t t;
		char *d = (char *)name->GetText().c_str();
		thr_create(0, 0, Save, d, 0, &t);

		d = strdup( d );
		save_folder = dirname( d );
		save_folder += "/";
		free( d );
	}
}
//----------------------------------------------------------------------------
void ScreenCapture::Activate( int x, int y )
{
	if( x < 0 ) x = 0;
	if( x + width >= ScreenWidth )
		x = ScreenWidth - width - 1;
	if( y < 0 ) y = 0;
	if( y + height >= ScreenHeight )
		y = ScreenHeight - height - 1;
	MoveTo( x, y );

	Timer::Add( this, 0 );
}
//----------------------------------------------------------------------------
