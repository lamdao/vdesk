#ifndef __LABEL_H
#define __LABEL_H
//-----------------------------------------------------------------------------
#include "Window.h"
#include "Canvas.h"
#include "Text.h"
//-----------------------------------------------------------------------------
class Label: public WinControl {
private:
	char *caption;
	vector<Text*> data;
	bool reverse;

	Canvas *canvas;
public:
	Label( char *text );
	virtual ~Label();

	void Update();
	void UpdateBackground();

	char *Caption() const { return caption; }
	void SetCaption( char *text );

private:
	void Free();
};
//-----------------------------------------------------------------------------
#endif
