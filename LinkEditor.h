#ifndef __LINKEDITOR_H
#define __LINKEDITOR_H
//----------------------------------------------------------------------------
#include "Dialog.h"
#include "Editor.h"
#include "Link.h"
//----------------------------------------------------------------------------
class LinkEditor: public Dialog {
private:
	Editor *caption;
	Editor *command;
	Editor *icon;

	Button *save;
	Button *cancel;
public:
	LinkEditor();
	~LinkEditor();

	virtual void Appear( XEvent *e );
	virtual void Perform( int id );

	string CreateNew( int x, int y );
	void Edit( Link* l );
protected:
	virtual void Draw();
};
//----------------------------------------------------------------------------
#endif
