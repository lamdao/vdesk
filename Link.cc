#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Link.h"
#include "BusyCursor.h"
#include "Dialog.h"
#include "LinkEditor.h"
//-----------------------------------------------------------------------------
enum {
	LINK_EXECUTE = 0,
	LINK_COMMAND = 2,
	LINK_ICON    = 3,
	LINK_EDIT    = 4,
	LINK_RENAME  = 5,
	LINK_DELETE  = 6,
};
//-----------------------------------------------------------------------------
static MenuItem items[] = {
	MenuItem( LINK_EXECUTE, "Execute" ),
	MenuSeparator(),
	MenuItem( LINK_RENAME, "Rename", false ),
	MenuItem( LINK_DELETE, "Delete" ),
	MenuSeparator(),
	MenuItem( LINK_EDIT, "Properties" ),
};
//-----------------------------------------------------------------------------
PopupMenu *Link::menu = NULL;
LinkEditor *Link::editor = NULL;
//-----------------------------------------------------------------------------
Link::Link( char *file, Table *t ): EventControl()
{
	x = t->QueryAsInt( "X" );
	y = t->QueryAsInt( "Y" );
	Icon = new Image( t->QueryAsStr( "Icon" ) );
	Text = new Label( t->QueryAsStr( "Caption" ) );
	Command = t->Query( "Command" );
	FileName = string( file );
	IconName = string( t->QueryAsStr( "Icon" ) );
	Caption = string( t->QueryAsStr( "Caption" ) );
	Dragging = false;
	Dragged = false;

	width = max( Icon->Width(), Text->Width() );
	height = Icon->Height() + Text->Height() + ICON_TEXT_SPACE;

	Text->SetEventControl( this );
	Icon->SetEventControl( this );
	SetPosition( x, y );
	Check();
}
//-----------------------------------------------------------------------------
Link::~Link()
{
	delete Icon;
	delete Text;
}
//-----------------------------------------------------------------------------
void Link::CreateMenu()
{
	if( !menu ) {
		menu = new PopupMenu( items, SIZE_OF_ARRAY(items) );
		editor = new LinkEditor();
	}
}
//-----------------------------------------------------------------------------
void Link::DeleteMenu()
{
	if( menu ) {
		delete editor;
		delete menu;
		menu = NULL;
	}
}
//----------------------------------------------------------------------------
string Link::ShowEditor( int x, int y )
{
	return editor->CreateNew( x, y );
}
//----------------------------------------------------------------------------
void Link::ShowPopupMenu( int x, int y )
{
	if( menu ) {
		char *p = strdup( Caption.c_str() );
		for( int i=strlen(p)-1; i>=0; i-- )
			if( p[i] == ';' ) p[i] = ' ';
		menu->SetTitle( p );
		menu->SetActionControl( this );
		menu->ShowAt( x, y );
		free( p );
	}
}
//-----------------------------------------------------------------------------
void Link::Perform( int id )
{
	switch( id ) {
		case LINK_EXECUTE:
			Execute();
			break;
		case LINK_EDIT:
			Edit();
			break;
		case LINK_RENAME:
			Rename();
			break;
		case LINK_DELETE:
			Delete();
			break;
	}
}
//-----------------------------------------------------------------------------
void Link::Edit()
{
	editor->Edit( this );
	Check();
}
//-----------------------------------------------------------------------------
void Link::Rename()
{
	// TODO:
	// Show editor box at Text
}
//-----------------------------------------------------------------------------
void Link::Delete()
{
	char s[256];
	sprintf( s, "Are you sure you want to delete [ %s ] ?", Text->Caption() );
	if( Message::Confirm( s ) ) {
		unlink( FileName.c_str() );
		parent->Delete( this );
	}
}
//-----------------------------------------------------------------------------
void Link::GetProperties( string &file, string &caption, string &cmd, string &icon )
{
	file = FileName;
	caption = Caption;
	cmd = Command;
	icon = IconName;
}
//-----------------------------------------------------------------------------
void Link::SetProperties( string file, string caption, string cmd, string icon )
{
	bool r = false;

	Command = cmd;
	FileName = file;
	if( Caption != caption ) {
		Caption = caption;
		Text->SetCaption( (char *)caption.c_str() );
		r = true;
	}
	if( IconName != icon ) {
		IconName = icon;
		Icon->SetSource( (char *)icon.c_str() );
		r = true;
	}
	if( r ) Update();
}
//-----------------------------------------------------------------------------
bool Link::Match( Window w )
{
	return w==Icon->Handler() || w==Text->Handler();
}
//-----------------------------------------------------------------------------
void Link::Save()
{
	Database *db;
	Table    *tb;

	db = new Database( FileName );
	if( (tb = db->Query( "Icon" )) ) {
		tb->Set( "X", x );
		tb->Set( "Y", y );
		db->Save();
	}
	delete db;
}
//-----------------------------------------------------------------------------
void Link::Show()
{
	Update();
	Icon->Show();
	Text->Show();
}
//-----------------------------------------------------------------------------
void Link::Hide()
{
	Icon->Hide();
	Text->Hide();
}
//-----------------------------------------------------------------------------
void Link::Update()
{
	Text->UpdateBackground();
	Text->Update();
	Icon->Draw();
}
//-----------------------------------------------------------------------------
void Link::SetPosition( int x, int y )
{
	if( x < 0 ) x = 0;
	if( y < 0 ) y = 0;
	if( x + width >= ScreenWidth )
		x = ScreenWidth - width;
	if( y + height >= ScreenHeight )
		y = ScreenHeight - height;

	int sx = x + (Icon->Width() - Text->Width())/2;
	Text->MoveTo( sx, y + Icon->Height() + ICON_TEXT_SPACE );
	Icon->MoveTo( x, y );
}
//-----------------------------------------------------------------------------
void Link::Check()
{
	executable = false;
	if( Command.length() > 0 ) {
		struct stat b;
		char *p = (char *)Command.c_str();
		while( *p && *p==' ' ) p++;
		if( !*p ) goto __set_status;
		p = strtok( strdup(p), " " );
		if( strrchr( p, '/' ) && stat( p, &b ) >= 0 && S_ISREG( b.st_mode ) ) {
			if( (b.st_mode & S_IXOTH) ||
				(b.st_uid == MyUID && (b.st_mode & S_IXUSR)) ||
				(b.st_gid == MyGID && (b.st_mode & S_IXGRP)) )
				executable = true;
		}
		else
		for( int i=0; i<SystemPath.size(); i++ ) {
			string cmd = string(SystemPath[i]) + "/" + p;
			if( stat( cmd.c_str(), &b ) >= 0 ) {
				if( !S_ISREG( b.st_mode ) ) continue;
				if( (b.st_mode & S_IXOTH) ||
					(b.st_uid == MyUID && (b.st_mode & S_IXUSR)) ||
					(b.st_gid == MyGID && (b.st_mode & S_IXGRP)) ) {
					executable = true;
					break;
				}
			}
		}
		free( p );
	}
__set_status:
	Icon->SetBroken( !executable );
}
//-----------------------------------------------------------------------------
void Link::Appear( XEvent *e )
{
	Text->Update();
}
//-----------------------------------------------------------------------------
void Link::MousePress( XButtonEvent *e )
{
	if( e->button == Button1 ) {
		sp.x = e->x_root;
		sp.y = e->y_root;
		Dragging = true;
		Text->Raise();
		Icon->Raise();
	}
}
//-----------------------------------------------------------------------------
void Link::MouseRelease( XButtonEvent *e )
{
	if( e->button == Button1 ) {
		static long time = 0;

		Dragging = false;
		if( (SingleClick && !Dragged) || (e->time-time<=200) ) {
			Execute();
			time = 0;
		}
		else time = e->time;

		if( !Locked && Dragged ) {
			Update();
			Save();
		}
		Dragged = false;
		Icon->Lower();
		Text->Lower();
	}
	else
	if( e->button == Button3 ) {
		ShowPopupMenu( e->x_root, e->y_root );
	}
}
//-----------------------------------------------------------------------------
void Link::MouseEnter( XMotionEvent *e )
{
	if( Transparency )
		Icon->SetGlowing( true );
}
//-----------------------------------------------------------------------------
void Link::MouseLeave( XMotionEvent *e )
{
	if( Transparency )
		Icon->SetGlowing( false );
}
//-----------------------------------------------------------------------------
void Link::MouseMove( XMotionEvent *e )
{
	if( !Dragging ) return;
	Dragged = true;
	if( Locked ) return;

	x -= sp.x - e->x_root;
	y -= sp.y - e->y_root;
	SetPosition( x, y );
	sp.x = e->x_root;
	sp.y = e->y_root;
}
//----------------------------------------------------------------------------
void Link::Execute()
{
	if( !executable ) {
		Message::Error( 0, "File not found or don't have permision!" );
		return;
	}

	ActiveWindow = Icon->Handler();
	WaitCursor->Activate();

	pid_t pid = fork();
	switch( pid ) {
		case -1:
			WaitCursor->Deactivate();
			cerr << "Fork in Vdesk::Link::Execute(X) Failed.\n";
			_exit(1);
			break;
		case 0:
			setsid();
			if( (pid=fork()) < 0 ) {
				cerr << "Fork in Vdesk::Link::Execute(X) Failed.\n";
				_exit(1);
			}
			if( pid == 0 ) {
				setsid();
				execlp("/bin/sh", "/bin/sh", "-c", Command.c_str(), NULL);

				cerr << "Vdesk::Link::Execute( \""
					 <<	Command.c_str() << "\" ) Failed.\n";
				exit(0);
			}
			_exit(0);
			break;
	}
	waitpid(pid, NULL, 0);
}
//-----------------------------------------------------------------------------
