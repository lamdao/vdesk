#ifndef __RESOURCE_H
#define __RESOURCE_H
//------------------------------------------------------
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>
#include <Imlib.h>
//-----------------------------------------------------------------------------
#include "Preferences.h"
//-----------------------------------------------------------------------------
#define VDESK_ID		"VDESKTOP"
//-----------------------------------------------------------------------------
#define SIZE_OF_ARRAY(a)	(sizeof(a)/sizeof(a[0]))
//-----------------------------------------------------------------------------
typedef unsigned char uchar;
//-----------------------------------------------------------------------------
class BusyCursor;
class Color;
class Background;
//-----------------------------------------------------------------------------
typedef struct Point {
	int x;
	int y;
};
//-----------------------------------------------------------------------------
class Resource: public Preferences {
private:
	friend class Color;
	static int count;

protected:
	static Display *display;
	static int screen;

	static int ScreenWidth;
	static int ScreenHeight;
	static Visual *ScreenVisual;
	static Colormap ScreenColormap;

	static Window Root;
	static Window ActiveWindow;
	static XContext VdeskCID;

	static int SysKey;
	static int SysButton;

	static Cursor ArrowCursor;
	static Cursor HandCursor;
	static Cursor HourCursor;
	static Cursor TextCursor;
	static BusyCursor *WaitCursor;

	static int FontSize;
	static string FontName;
	static string XFontName;
	static string DefaultIcon;
	static int DefaultIconSize;

	static XftFont *Font;
	static Color FColor;
	static Color BColor;
	static Color SColor;
	static Color RColor;
	static Color CForeground;
	static Color CBackground;
	static Color CBarText;
	static Color CBarColor;
	static Color CSelectedText;
	static Color CSelectedBar;
	static Color CDisabledText;

	static ImlibData *ScreenData;
	static ImlibImage *BrokenIcon;
	static Background *VdeskBg;

	static vector<char*> SystemPath;
	static int MyUID, MyGID;
public:
	Resource();
	~Resource();

	void GrabKey(int Key, int Mask);
	void GrabButton(int Button, int Key, int Mask);

protected:
#ifdef USE_MB_TEXT
	void LocaleInit();
#endif
	void LoadFont();
	void LoadFontSet();
	void LoadColor();
	void LoadCursor();

	void FreeFont();
	void FreeCursor();

	void Save();
private:
	void CreateDefaultIcon();
};
//-----------------------------------------------------------------------------
#include "Color.h"
//-----------------------------------------------------------------------------
#endif
