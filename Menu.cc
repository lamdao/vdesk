#include "Menu.h"
#include "Text.h"
#include "Desktop.h"
//-----------------------------------------------------------------------------
#define ID_POPUPMASK \
	ButtonPressMask|ButtonReleaseMask|PointerMotionMask|LeaveWindowMask| \
	ExposureMask
//-----------------------------------------------------------------------------
PopupMenu::PopupMenu( MenuItem *items, int count ):
				WinControl( Root, ID_POPUPMASK ), EventControl()
{
	width = 0;
	height = Font->height + 8;
	for( int i=0; i<count; i++ ) {
		items[i].SetParent( this );
		items[i].SetPosition( 8, height );
		this->items.push_back( &items[i] );
		if( items[i].IsSeparator() )
			height += 8;
		else {
			int w = Text::Width( items[i].Name(), items[i].Length() );
			height += Font->height;
			if( width<w ) width = w;
		}
	}
	width += 32;
	height += 4;
	defwidth = width;
	CreateHandler();
	SetEventControl( this );
	canvas = new Canvas( this->handler );
	current = NULL;
	action = NULL;
}
//-----------------------------------------------------------------------------
PopupMenu::~PopupMenu()
{
	delete canvas;
}
//-----------------------------------------------------------------------------
void PopupMenu::SetTitle( const char *t )
{
	int w = Text::Width( t ) + 18;
	if( w>width ) Resize( w, height );
	if( w<defwidth && width>defwidth ) Resize( defwidth, height );
	title = string( t );
}
//-----------------------------------------------------------------------------
void PopupMenu::ShowAt( int x, int y )
{
	if( x+width>=ScreenWidth )
		x -= width;
	if( y+height>=ScreenHeight )
		y -= height;

	XMoveWindow( display, handler, this->x=x, this->y=y );
	XMapRaised( display, handler );
}
//-----------------------------------------------------------------------------
void PopupMenu::Hide()
{
	EventControl::Release();
	WinControl::Hide();
}
//-----------------------------------------------------------------------------
void PopupMenu::Draw()
{
	canvas->DrawRectangle( 0, 0, width, height, CBackground );
	canvas->DrawRectangle( 0, 0, width, Font->height+4, CBarColor );
	canvas->DrawText( 9, Font->ascent + 4,
						(char *)title.c_str(), title.length(), CForeground );
	canvas->DrawText( 8, Font->ascent + 3,
						(char *)title.c_str(), title.length(), CBarText );
	for( int i=0; i<items.size(); i++ )
		DrawItem( items[i] );
}
//-----------------------------------------------------------------------------
void PopupMenu::DrawItem( MenuItem *mi )
{
	if( mi->IsSeparator() ) {
		canvas->DrawLine( 0, mi->Y()+4, width, mi->Y()+4, CForeground );
		return;
	}

	Color *fc, *bc;
	if( mi->IsSelected() ) {
		bc = &CSelectedBar;
		fc = mi->IsEnabled() ? &CSelectedText : &CDisabledText;
	}
	else {
		bc = &CBackground;
		fc = mi->IsEnabled() ? &CForeground : &CDisabledText;
	}

	canvas->DrawRectangle( 4, mi->Y(), width-8, Font->height, *bc );
	canvas->DrawText( mi->X(), mi->Y()+Font->ascent,
						mi->Name(), mi->Length(), *fc );
}
//-----------------------------------------------------------------------------
void PopupMenu::ProcessEvent( XEvent *e )
{
	if( e->xany.window!=this->handler ) {
		if( this->IsVisibled() && e->type==ButtonPress ) {
			Hide();
			return;
		}
		Desktop::ProcessEvent( e );
		return;
	}

	EventControl::ProcessEvent( e );
}
//-----------------------------------------------------------------------------
void PopupMenu::Appear( XEvent *e )
{
	EventControl::Grab( handler, this );
	Draw();
}
//-----------------------------------------------------------------------------
void PopupMenu::MouseMove( XMotionEvent *e )
{
	int x = e->x,
		y = e->y;

	if( current && current->Contains( x, y ) ) return;

	for( int i=0; i<items.size(); i++ )
		if( items[i]->Contains( x, y ) ) {
			if( current )
				current->SetSelected( false );
			if( items[i]->IsSeparator() )
				current = NULL;
			else
				(current = items[i])->SetSelected( true );
			return;
		}
}
//-----------------------------------------------------------------------------
void PopupMenu::MouseLeave( XMotionEvent *e )
{
	if( current ) {
		current->SetSelected( false );
		current = NULL;
	}
}
//-----------------------------------------------------------------------------
void PopupMenu::MouseClick( XButtonEvent *e )
{
	Hide();
	if( action && current && current->IsEnabled() )
		action->Perform( current->Id() );
}
//-----------------------------------------------------------------------------
// MenuItem
//-----------------------------------------------------------------------------
void MenuItem::SetSelected( bool s )
{
	selected = s;
	if( parent ) parent->DrawItem( this );
}
//-----------------------------------------------------------------------------
bool MenuItem::Contains( int x, int y )
{
	if( x<this->x || x>this->x+parent->Width() )
		return false;

	if( IsSeparator() )
		return y>=this->y && y<=this->y+8;

	return y>=this->y && y<=this->y+parent->ItemHeight();
}
//-----------------------------------------------------------------------------
