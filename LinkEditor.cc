 #include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "LinkEditor.h"
//----------------------------------------------------------------------------
LinkEditor::LinkEditor(): Dialog()
{
	struct {
		char *text;
		Editor **editor;
	} p[] = {
		{ "Caption:", &caption },
		{ "Command:", &command },
		{ "Icon:", &icon },
	};

	width = 300;
	height = 3 * Font->height + 8 + 3 * ( Font->height + 20 ) + 16;
	x = (ScreenWidth - width) / 2;
	y = (ScreenHeight - height) / 2;

	CreateHandler();
	SetEventControl( this );
	SetTitle( "Link editor" );
	canvas = new Canvas( this->handler );

	for( int i=0; i<SIZE_OF_ARRAY(p); i++ ) {
		int y = Font->height + 16 + i * ( Font->height + 20 );
		int w = Text::Width( p[i].text );
		controls.push_back(*p[i].editor = new Editor( this->handler ));
		(*p[i].editor)->SetParentControl( this );
		(*p[i].editor)->SetWidth( 300 - 48 - w );
		(*p[i].editor)->MoveTo( w + 32, y );
		(*p[i].editor)->Show();
	}

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
LinkEditor::~LinkEditor()
{
	delete caption;
	delete command;
	delete icon;
	delete save;
	delete cancel;
}
//----------------------------------------------------------------------------
void LinkEditor::Draw()
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
void LinkEditor::Appear( XEvent *e )
{
	Dialog::Appear( e );
	caption->SetFocus();
	current = 0;
	current = 0;
}
//----------------------------------------------------------------------------
void LinkEditor::Perform( int id )
{
	if( id==0 ) {
		const char *p = caption->GetText().c_str();
		while( *p==' ' ) p++;
		if( !*p ) {
			XBell( display, 25 );
			return;
		}
	}
	Dialog::Perform( id );
}
//----------------------------------------------------------------------------
string LinkEditor::CreateNew( int x, int y )
{
	int xx = x, yy = y;
	if( x < 0 ) x = 0;
	if( x + width >= ScreenWidth )
		x = ScreenWidth - width - 1;
	if( y < 0 ) y = 0;
	if( y + height >= ScreenHeight )
		y = ScreenHeight - height - 1;
	MoveTo( x, y );
	caption->SetText( "" );
	command->SetText( "" );
	icon->SetText( "default.png" );
	Show();
	if( Accepted() ) {
		int n = 1;
		string fn = VdeskFolder + caption->GetText() + ".lnk";
		while( true ) {
			struct stat b;
			char num[16];
			if( stat( fn.c_str(), &b ) < 0 ) break;
			sprintf(num, "%d", n++);
			fn = VdeskFolder + caption->GetText() + num + ".lnk";
		}
		Table *t = new Table( "Icon" );
		t->Set( "Caption", caption->GetText() );
		t->Set( "Command", command->GetText() );
		t->Set( "Icon", icon->GetText() );
		t->Set( "X", xx );
		t->Set( "Y", yy );
		Database::SaveToFile( fn, t );
		delete t;
		return fn;
	}
	return "";
}
//----------------------------------------------------------------------------
void LinkEditor::Edit( Link *l )
{
	string cmd, txt, ico, file;

	x = l->X(), y = l->Y();
	if( x < 0 ) x = 0;
	if( x + width >= ScreenWidth )
		x = ScreenWidth - width - 1;
	if( y < 0 ) y = 0;
	if( y + height >= ScreenHeight )
		y = ScreenHeight - height - 1;
	l->GetProperties( file, txt, cmd, ico );
	caption->SetText( txt );
	command->SetText( cmd );
	icon->SetText( ico );
	MoveTo( x, y );
	Show();
	if( Accepted() ) {
		l->SetProperties( file, caption->GetText(), command->GetText(), icon->GetText() );

		Table *t = new Table( "Icon" );
		t->Set( "Caption", caption->GetText() );
		t->Set( "Command", command->GetText() );
		t->Set( "Icon", icon->GetText() );
		t->Set( "X", l->X() );
		t->Set( "Y", l->Y() );
		Database::SaveToFile( file, t );
		delete t;
	}
}
//----------------------------------------------------------------------------
