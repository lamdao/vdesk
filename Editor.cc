#include "Editor.h"
#include "Text.h"
#include <X11/keysym.h>
//----------------------------------------------------------------------------
#define ID_EDITORMASK \
	KeyPressMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask| \
	FocusChangeMask|ExposureMask
//----------------------------------------------------------------------------
Editor::Editor( Window parent, string t ):
								WinControl( parent, ID_EDITORMASK ),
								EventControl(), text(t)
{
	mouse_down = 0;
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
	left_margin = 4;
	right_margin = width - 4;
	canvas = new Canvas( this->handler );
	enabled = true;
	caret_visible = false;
	sw = Text::Width( "a a" ) - 2 * Text::Width( "a" );
	cp = sl = text.length();
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
	canvas->DrawText( left_margin, Font->ascent + 2, s, strlen(s),
						enabled ? CForeground : CDisabledText );
	if( sl != cp ) {
		int x, n, w;
		if( cp < sl ) {
			s += cp - sp;
			x = GetCaretPosition();
			w = Text::Width( s, n = sl - cp );
		}
		else
		if( sl > sp ) {
			n = cp - sl;
			x = left_margin + Text::Width( s, sl - sp ); 
			w = GetCaretPosition() - x;
			s += sl - sp;
		}
		else {
			n = cp - sp;
			x = left_margin;
			w = GetCaretPosition() - x;
		}
		if( w > right_margin ) w = right_margin;
		canvas->DrawRectangle( x, 2, w, height - 4, CSelectedBar );
		canvas->DrawText( x, Font->ascent + 2, s, n, CSelectedText );
	}
	canvas->DrawRectangle( right_margin, 0, 4, height, CBackground );
	if( caret_visible ) ShowCaret();
}
//----------------------------------------------------------------------------
int Editor::GetCaretPosition()
{
	int x;
	char *s = (char *)text.c_str();
	x = cp==sp ? left_margin : left_margin + Text::Width(&s[sp], cp - sp) + 2;
	if( cp > 0 && s[cp-1] == ' ' ) x += sw;
	return x;
}
//----------------------------------------------------------------------------
void Editor::ShowCaret()
{
	int x = GetCaretPosition();
	backup = canvas->GetImage( x, 3, 1, height-6 );
	canvas->DrawLine( x, 3, x, height-4, CForeground );
	if( sl != cp ) {
		x += cp < sl ? 1 : -1;
		canvas->DrawLine( x, 3, x, height-4, CBackground );
	}
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
int PrevPosition( const string &text, int p )
{
	unsigned char *s = (unsigned char *)text.c_str();
	unsigned char *q = s + p - 1;
	
	while( q > s && *q > 127 ) {
		if( (*q >= 0xC3 && *q <= 0xC6) || *q == 0xE1 )
			break;
		q--;
	}
	return q - s;
}
//----------------------------------------------------------------------------
int NextPosition( const string& text, int p )
{
	unsigned char *s = (unsigned char *)text.c_str();
	unsigned char *q = s + p + 1;

	while( *q > 127 ) {
		if( (*q >= 0xC3 && *q <= 0xC6) || *q == 0xE1 )
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
void Editor::Cut()
{
	Copy();
	Delete();
	Draw();
}
//----------------------------------------------------------------------------
void Editor::Copy()
{
	if( sl == cp ) return;

	string s(cp < sl ? text.substr( cp, sl - cp ) : text.substr( sl, cp - sl ));
	XStoreBuffer( display, s.c_str(), s.length(), 0 );
}
//----------------------------------------------------------------------------
void Editor::Paste()
{
	int n;
	char *p = XFetchBuffer( display, &n, 0 );
	if( n > 0 ) {
		if( sl != cp )
			Delete();

		text.insert( cp, p, n );

		cp += n;
		sl = cp;
		while( GetCaretPosition() > right_margin )
			sp = NextPosition( text, sp );
		Draw();
	}
	XFree( p );
}
//----------------------------------------------------------------------------
void Editor::Delete()
{
	int n = cp - sl;
	if( n > 0 )
		cp = sl;
	else {
		sl = cp;
		n = -n;
	}
	text.erase( cp , n );
	if( sp >= cp )
		while( sp > 0 ) {
			sp = PrevPosition( text, sp );
			if( GetCaretPosition() >= width / 2 ) break;
		}
}
//----------------------------------------------------------------------------
void Editor::KeyDown( XKeyEvent *e )
{
	KeySym k;
	char s[64];
	int n, shift;

	if( !enabled ) return;

	n = XLookupString( e, s, sizeof(s)-1, &k, 0 );
	shift = (e->state & ShiftMask) != 0;
	switch( k ) {
		case XK_Home:
			cp = sp = 0;
			if( !shift ) sl = cp;
			Draw();
			break;
		case XK_End:
			cp = text.length();
			if( !shift ) sl = cp;
			while( GetCaretPosition() > right_margin )
				sp = NextPosition( text, sp );
			Draw();
			break;
		case XK_Left:
			if( cp > 0 ) {
				int psl = sl != cp;
				HideCaret();
				cp = PrevPosition( text, cp );
				if( !shift ) sl = cp;
				if( cp >= sp ) {
					if( sl != cp || psl )
						Draw();
					else
						ShowCaret();
				}
				else {
					sp = cp;
					Draw();
				}
			}
			break;
		case XK_Right:
			if( cp < text.length() ) {
				int ssp = sp, psl = sl != cp;

				HideCaret();
				cp = NextPosition( text, cp );
				if( !shift ) sl = cp;
				while( GetCaretPosition() > right_margin )
					sp = NextPosition( text, sp );
				if( sp != ssp || sl != cp || psl )
					Draw();
				else
					ShowCaret();
			}
			break;
		case XK_BackSpace:
			if( sl != cp ) {
				Delete();
				Draw();
			}
			else
			if( cp > 0 ) {
				n = cp;
				cp = PrevPosition( text, cp );
				n -= cp;
				text.erase( cp, n );
				if( sp >= cp )
					while( sp > 0 ) {
						sp = PrevPosition( text, sp );
						if( GetCaretPosition() >= width / 2 ) break;
					}
				sl = cp;
				Draw();
			}
			break;
		case XK_Delete:
			if( sl != cp ) {
				Delete();
				Draw();
			}
			else
			if( cp < text.length() ) {
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
		case XK_Return:
			if( pctrl ) pctrl->AcceptProcess();
			break;
		default:
			if( (e->state & ControlMask) ) {
				switch( k ) {
					case XK_x:
						Cut();
						break;
					case XK_c:
						Copy();
						break;
					case XK_v:
						Paste();
						break;
				}
			}
			else
			if( k < 256 && n > 0 ) {
				if( sl != cp )
					Delete();
				s[n] = 0;
				text.insert( cp, s );
				cp += n;
				sl = cp;
				while( GetCaretPosition() > right_margin )
					sp = NextPosition( text, sp );
				Draw();
			}
			break;
	}
}
//----------------------------------------------------------------------------
void Editor::MouseToCaretPosition( int p )
{
	int x = GetCaretPosition();
	if( x <= p ) {
		while( cp < text.length() && GetCaretPosition() < p )
			cp = NextPosition( text, cp );
	}
	else {
		while( cp > 0 && GetCaretPosition() > p )
			cp = PrevPosition( text, cp );
	}
}
//----------------------------------------------------------------------------
void Editor::MousePress( XButtonEvent *e )
{
	if( !enabled ) return;

	if( mouse_down ) {
		mouse_down |= (1 << e->button);
		return;
	}
	mouse_down |= (1 << e->button);

	if( caret_visible )
		HideCaret();

	if( text.length() ) {
		int psl = sl != cp;
		MouseToCaretPosition( e->x );
		sl = cp;
		if( psl ) Draw();
	}
	if( caret_visible )
		ShowCaret();
	else {
		SetFocus();
		if( pctrl ) pctrl->SetActiveControl( this );
	}
}
//----------------------------------------------------------------------------
void Editor::MouseMove( XMotionEvent *e )
{
	if( (mouse_down & (1 << Button1)) ) {
		if( text.length() ) {
			int scp = cp;
			MouseToCaretPosition( e->x );
			if( scp != cp ) Draw();
		}
	}
}
//----------------------------------------------------------------------------
void Editor::MouseRelease( XButtonEvent *e )
{
	mouse_down &= ~(1 << e->button );
	switch( e->button ) {
		case Button1:
			Copy();
			break;
		case Button2:
			if( !mouse_down ) Paste();
			break;
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
