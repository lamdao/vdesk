#ifndef __DIALOG_H
#define __DIALOG_H
//----------------------------------------------------------------------------
#include "Shadow.h"
#include "Window.h"
#include "Event.h"
#include "Action.h"
#include "Canvas.h"
#include "Button.h"
//----------------------------------------------------------------------------
class Dialog: public WinControl, public EventControl, public ActionControl {
protected:
	long result;
	string title;
	Canvas *canvas;
	vector<WinControl *> controls;
	int current;

	Shadow *shadow;
public:
	Dialog();
	virtual ~Dialog();

	void SetTitle( string t ) { title = t; }

	virtual void Show();
	virtual void ProcessEvent( XEvent *e );
	virtual void Appear( XEvent *e );

	virtual void Perform( int id );
	virtual bool Accepted() const { return result==0; }
	virtual bool Rejected() const { return result!=0; }
	virtual long Result() const { return result; }

	virtual void NextControl();
	virtual void PrevControl();
	virtual void SetActiveControl( WinControl *c );
	virtual void CancelProcess() { Perform(1); }
	virtual void AcceptProcess() { Perform(0); }

protected:
	virtual void Draw();
};
//----------------------------------------------------------------------------
class Message: public Dialog {
private:
	vector<string> messages;

	Button *ok;
	Button *cancel;
public:
	Message( string text, bool info=true );
	~Message();

	static void Info( char *msg );
	static void Warning( char *msg );
	static void Error( int n, char *msg );
	static bool Confirm( char *msg );
protected:
	virtual void Draw();
};
//----------------------------------------------------------------------------
#endif
