#include "Dialog.h"
#include "Text.h"
#include "Desktop.h"
#include "Shadow.h"
//----------------------------------------------------------------------------
#define ID_DIALOGMASK	ExposureMask
//----------------------------------------------------------------------------
// Dialog
//----------------------------------------------------------------------------
Dialog::Dialog(): WinControl( Root, ID_DIALOGMASK ), EventControl()
{
	SetCursor( ArrowCursor );
	current = -1;
}
//-----------------------------------------------------------------------------
void Dialog::Show()
{
	Shadow::Instance()->Show( this );
	XMapRaised( display, this->handler );
	SetVisibled( true );
	while( IsVisibled() ) {
		XEvent e;
		XNextEvent( display, &e );
		ProcessEvent( &e );
	}
	Shadow::Instance()->Hide();
}
//-----------------------------------------------------------------------------
void Dialog::ProcessEvent( XEvent *e )
{
	if( e->xany.window!=this->handler ) {
		Desktop::ProcessEvent( e );
		return;
	}

	EventControl::ProcessEvent( e );
}
//----------------------------------------------------------------------------
void Dialog::Appear( XEvent *e )
{
	EventControl::Grab( handler, this );
	Draw();
}
//----------------------------------------------------------------------------
void Dialog::Draw()
{
	canvas->DrawRectangle( 0, 0, width, height, CBackground );
	canvas->DrawRectangle( 0, 0, width, Font->height+4, CBarColor );
	canvas->DrawText( 9, Font->ascent + 4,
						(char *)title.c_str(), title.length(), CForeground );
	canvas->DrawText( 8, Font->ascent + 3,
						(char *)title.c_str(), title.length(), CBarText );
}
//----------------------------------------------------------------------------
void Dialog::Perform( int id )
{
	EventControl::Release();
	result = id;
	Hide();
}
//----------------------------------------------------------------------------
void Dialog::NextControl()
{
	if( controls.size()>0 ) {
		current++;
		if( current >= controls.size() )
			current = 0;
		controls[current]->SetFocus();
	}
}
//----------------------------------------------------------------------------
void Dialog::PrevControl()
{
	if( controls.size()>0 ) {
		current--;
		if( current < 0 )
			current = controls.size() - 1;
		controls[current]->SetFocus();
	}
}
//----------------------------------------------------------------------------
void Dialog::SetActiveControl( WinControl *c )
{
	for( int i=0; i<controls.size(); i++ )
		if( controls[i] == c ) {
			current = i;
			return;
		}
}
//----------------------------------------------------------------------------
// Message
//----------------------------------------------------------------------------
Message::Message( string text, bool info ): Dialog()
{
	width = 0;
	height = 0;
	char *p = strtok( (char *)text.c_str(), "\n" );
	while( p ) {
		int w = Text::Width( p );
		if( width<w ) width = w;
		height += Font->height;
		messages.push_back( string(p) );
		p = strtok( 0, "\n" );
	}
	width += 64;
	height += 2 * Font->height + 56;
	x = (ScreenWidth - width) / 2;
	y = (ScreenHeight - height) / 2;

	CreateHandler();
	SetEventControl( this );
	canvas = new Canvas( this->handler );

	ok = new Button( this->handler, 0, "OK", 80 );
	ok->SetActionControl( this );

	int by = height - ok->Height() - 10;
	if( info ) {
		ok->MoveTo( (width - 80) / 2, by );
		cancel = NULL;
	}
	else {
		cancel = new Button( this->handler, 1, "Cancel" );
		cancel->SetActionControl( this );
		ok->MoveTo( (width - 160 - 20) / 2, by );
		cancel->MoveTo( ok->X() + 80 + 20, by );
		cancel->Show();
	}
	ok->Show();
}
//----------------------------------------------------------------------------
Message::~Message()
{
	delete ok;
	if( cancel ) delete cancel;
}
//----------------------------------------------------------------------------
void Message::Draw()
{
	Dialog::Draw();

	int y = Font->ascent + 16 + Font->height;
	for( int i=0; i<messages.size(); i++ ) {
		canvas->DrawText( 16, y, 
					(char *)messages[i].c_str(),
					messages[i].length(), CForeground );
		y += Font->height;
	}
	y = y - Font->height + 16;
	canvas->DrawLine( 0, y, width, y, CForeground );
}
//----------------------------------------------------------------------------
void Message::Info( char *t )
{
	Message m( t );
	m.SetTitle( "Information" );
	m.Show();
}
//----------------------------------------------------------------------------
void Message::Warning( char *t )
{
	Message m( t );
	m.SetTitle( "Warning" );
	m.Show();
}
//----------------------------------------------------------------------------
void Message::Error( int n, char *t )
{
	char s[32];
	if( n )
		sprintf( s, "Error [#%d]", n );
	else
		strcpy( s, "Error" );
	Message m( t );
	m.SetTitle( string(s) );
	m.Show();
}
//----------------------------------------------------------------------------
bool Message::Confirm( char *t )
{
	Message m( t, false );
	m.SetTitle( "Confirmation" );
	m.Show();
	return m.Accepted();
}
//----------------------------------------------------------------------------
