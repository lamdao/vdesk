#ifndef __LINK_H
#define __LINK_H
//-----------------------------------------------------------------------------
#include "Database.h"
#include "Container.h"
#include "Image.h"
#include "Label.h"
#include "Menu.h"
//-----------------------------------------------------------------------------
#define	LINK_SPACE			20
#define ICON_TEXT_SPACE		4
//-----------------------------------------------------------------------------
class LinkEditor;
//-----------------------------------------------------------------------------
class Link: public EventControl, public ActionControl {
private:
	static PopupMenu *menu;
	static LinkEditor *editor;

	Container *parent;
	Point sp;
protected:
	int x, y;
	int width, height;

	Image *Icon;
	Label *Text;
	string Command;
	string FileName;
	string IconName;
	string Caption;
	bool Dragging;
	bool Dragged;
	bool executable;
public:
	Link( char *file, Table *t );
	virtual ~Link();

	void Show();
	void Hide();
	void Save();
	void Update();
	void SetPosition( int x, int y );

	virtual void ShowPopupMenu( int x, int y );
	virtual void Execute();

	virtual void Appear( XEvent *e );
	virtual void MouseEnter( XMotionEvent *e );
	virtual void MouseLeave( XMotionEvent *e );
	virtual void MouseMove( XMotionEvent *e );
	virtual void MousePress( XButtonEvent *e );
	virtual void MouseRelease( XButtonEvent *e );

	virtual void Perform( int id );
	virtual void Edit();
	virtual void Rename();
	virtual void Delete();

	int X() const { return x; }
	int Y() const { return y; }
	int Width() const { return width; }
	int Height() const { return height; }
	void SetParent( Container *p ) { parent = p; }
	void GetProperties( string &file, string &caption, string &cmd, string &icon );
	void SetProperties( string file, string caption, string cmd, string icon );

	bool Match( Window w );

	static void CreateMenu();
	static void DeleteMenu();
	static string ShowEditor( int x, int y );
private:
	void Check();
};
//-----------------------------------------------------------------------------
#endif
