#ifndef __SHADOW_H
#define __SHADOW_H
//-----------------------------------------------------------------------------
#include "Window.h"
//-----------------------------------------------------------------------------
class Shadow: public Resource {
private:
	WinControl *Right;
	WinControl *Bottom;

public:
	Shadow();
	virtual ~Shadow();

	void Show( WinControl *context );
	void Hide();

	static Shadow *Instance();
};
//-----------------------------------------------------------------------------
extern Shadow *ControlShadow;
//-----------------------------------------------------------------------------
#endif
