#include "Editor.h"
#include "Text.h"
#include <X11/keysym.h>
//----------------------------------------------------------------------------
#define ID_EDITORMASK \
	KeyPressMask|ButtonPressMask|FocusChangeMask|ExposureMask
//----------------------------------------------------------------------------
Editor::Editor( Window parent, string t ):
								WinControl( parent, ID_EDITORMASK ),
								EventControl(), text(t)
{
	Create();
}
//----------------------------------------------------------------------------
Editor::~Editor()
{
	if( backup ) XDestroyImage( backup );
	delete canvas;
}
//----------------------------------------------------------------------------
void Editor::Create()
{
	backup = NULL;
	SetCursor( TextCursor );
	CreateHandler( 100, Font->height + 4 );
	SetEventControl( this );
	canvas = new Canvas( this->handler );
	enabled = true;
	caret_visible = false;
	sw = Text::Width( "a a" ) - 2 * Text::Width( "a" );
	cp = text.length();
	sp = 0;
}
//----------------------------------------------------------------------------
void Editor::SetEnabled( bool e )
{
	enabled = e;
	if( caret_visible ) {
		caret_visible = false;
		HideCaret();
	}
	Draw();
}
//----------------------------------------------------------------------------
void Editor::Draw()
{
	char *s = (char *)&text.c_str()[sp];
	canvas->DrawRectangle( 0, 0, width, height, CBackground );
	canvas->DrawText( 4, Font->ascent + 2, s, strlen(s),
						enabled ? CForeground : CDisabledText );
	canvas->DrawRectangle( width-4, 0, 4, height, CBackground );
	if( caret_visible ) ShowCaret();
}
//----------------------------------------------------------------------------
int Editor::GetCaretPosition()
{
	int x;
	char *s = (char *)text.c_str();
	x = cp==sp ? 4 : Text::Width( &s[sp], cp - sp ) + 6;
	if( cp>0 && s[cp-1]==' ' ) x += sw;
	return x;
}
//----------------------------------------------------------------------------
void Editor::ShowCaret()
{
	int x = GetCaretPosition();
	backup = canvas->GetImage( x, 3, 1, height-6 );
	canvas->DrawLine( x, 3, x, height-4, CForeground );
}
//----------------------------------------------------------------------------
void Editor::HideCaret()
{
	int x = GetCaretPosition();
	if( !backup )
		canvas->DrawLine( x, 3, x, height-4, CBackground );
	else {
		canvas->PutImage( x, 3, backup );
		XDestroyImage( backup );
		backup = NULL;
	}
}
//----------------------------------------------------------------------------
int PrevPosition( string &text, int p )
{
	unsigned char *s = (unsigned char *)text.c_str();
	unsigned char *q = s + p;
	
	if( *--q<=127 )
		return q - s;

	while( q > s && *q > 127 ) {
		if( (*q>=0xC3 && *q<=0xC6) || *q==0xE1 )
			break;
		q--;
	}
	return q - s;
}
//----------------------------------------------------------------------------
int NextPosition( string& text, int p )
{
	unsigned char *s = (unsigned char *)text.c_str();
	unsigned char *q = s + p;

	if( *++q<=127 )
		return q - s;

	while( *q > 127 ) {
		if( (*q>=0xC3 && *q<=0xC6) || *q==0xE1 )
			break;
		q++;
	}
	return q - s;
}
//----------------------------------------------------------------------------
void Editor::Appear( XEvent *e )
{
	SetVisibled( true );
	Draw();
}
//----------------------------------------------------------------------------
void Editor::KeyDown( XKeyEvent *e )
{
	KeySym k;
	char s[64];
	int n;

	if( !enabled ) return;

	n = XLookupString( e, s, sizeof(s)-1, &k, 0 );
	switch( k ) {
		case XK_Home:
			cp = sp = 0;
			Draw();
			break;
		case XK_End:
			cp = text.length();
			while( GetCaretPosition()>width-4 )
				sp = NextPosition( text, sp );
			Draw();
			break;
		case XK_Left:
			if( cp>0 ) {
				HideCaret();
				cp = PrevPosition( text, cp );
				if( cp >= sp )
					ShowCaret();
				else {
					sp = cp;
					Draw();
				}
			}
			break;
		case XK_Right:
			if( cp<text.length() ) {
				int ssp = sp;

				HideCaret();
				cp = NextPosition( text, cp );
				while( GetCaretPosition()>width-4 )
					sp = NextPosition( text, sp );
				if( sp!=ssp )
					Draw();
				else
					ShowCaret();
			}
			break;
		case XK_BackSpace:
			if( cp>0 ) {
				n = cp;
				cp = PrevPosition( text, cp );
				n -= cp;
				text.erase( cp, n );
				if( sp>=cp )
					while( sp>0 ) {
						sp = PrevPosition( text, sp );
						if( GetCaretPosition()>=width/2 ) break;
					}
				Draw();
			}
			break;
		case XK_Delete:
			if( cp<text.length() ) {
				n = cp;
				n = NextPosition( text, cp ) - n;
				text.erase( cp, n );
				Draw();
			}
			break;
		case XK_Tab:
			if( pctrl ) pctrl->NextControl();
			break;
		case XK_ISO_Left_Tab:
			if( pctrl ) pctrl->PrevControl();
			break;
		case XK_Escape:
			if( pctrl ) pctrl->CancelProcess();
			break;
		default:
			if( k<256 && n>0 ) {
				s[n] = 0;
				text.insert( cp, s );
				cp += n;
				while( GetCaretPosition()>width-4 )
					sp = NextPosition( text, sp );
				Draw();
			}
			break;
	}
}
//----------------------------------------------------------------------------
void Editor::MousePress( XButtonEvent *e )
{
	if( !enabled ) return;

	if( caret_visible )
		HideCaret();

	if( text.length() ) {
		int x = GetCaretPosition();
		if( x<=e->x ) {
			while( cp < text.length() && GetCaretPosition() < e->x )
				cp = NextPosition( text, cp );
		}
		else {
			while( cp > 0 && GetCaretPosition() > e->x )
				cp = PrevPosition( text, cp );
		}
	}
	if( caret_visible )
		ShowCaret();
	else {
		SetFocus();
		if( pctrl ) pctrl->SetActiveControl( this );
	}
}
//----------------------------------------------------------------------------
void Editor::HasFocus( XFocusInEvent *e )
{
	caret_visible = true;
	ShowCaret();
}
//----------------------------------------------------------------------------
void Editor::LostFocus( XFocusOutEvent *e )
{
	if( caret_visible ) {
		caret_visible = false;
		HideCaret();
	}
}
//----------------------------------------------------------------------------
