#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ScreenCapture.h"
//----------------------------------------------------------------------------
ScreenCapture::ScreenCapture(): Dialog()
{
	struct {
		char *text;
		Editor **editor;
	} p[] = {
		{ "File:", &name },
	};

	width = 300;
	height = 300;
	x = (ScreenWidth - width) / 2;
	y = (ScreenHeight - height) / 2;

	CreateHandler();
	SetEventControl( this );
	SetTitle( "Screen capture" );
	canvas = new Canvas( this->handler );

	sample = new WinControl( this->handler, 0 );
	sample->CreateHandler( 22, Font->ascent + 42, 256, 192 );
	controls.push_back(name = new Editor( this->handler ));
	name->SetParentControl( this );
	name->SetWidth( 300 - 44 );
	name->MoveTo( 22, sample->Y() + sample->Height() );
	name->Show();

	save = new Button( this->handler, 0, "Save", 80 );
	save->SetActionControl( this );
	save->SetParentControl( this );
	controls.push_back( save );
	int by = height - save->Height() - 10;
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

	char *s[] = { "Caption:", "Command:", "Icon:" };
	int y = Font->ascent + 20 + Font->height;
	for( int i=0; i<SIZE_OF_ARRAY(s); i++ ) {
		canvas->DrawText( 16, y, s[i], strlen(s[i]), CForeground );
		y += ( Font->height + 20 );
	}
	y = y - Font->height + 2;
	canvas->DrawLine( 0, y, width, y, CForeground );
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
		const char *p = name->GetText().c_str();
		while( *p==' ' ) p++;
		if( !*p ) {
			XBell( display, 25 );
			return;
		}
	}
	Dialog::Perform( id );
}
//----------------------------------------------------------------------------
void ScreenCapture::Activate( int x, int y )
{
	int xx = x, yy = y;
	if( x < 0 ) x = 0;
	if( x + width >= ScreenWidth )
		x = ScreenWidth - width - 1;
	if( y < 0 ) y = 0;
	if( y + height >= ScreenHeight )
		y = ScreenHeight - height - 1;
	MoveTo( x, y );
	name->SetText( "" );
	Show();
	if( Accepted() ) {
	}
}
//----------------------------------------------------------------------------
