#include "Resource.h"
#include "BusyCursor.h"
#include "Penguin.h"
#include "Broken.h"
#include "Background.h"
//-----------------------------------------------------------------------------
int Resource::count = 0;
Display *Resource::display = NULL;
//-----------------------------------------------------------------------------
int Resource::screen = 0;
int Resource::ScreenWidth = 0;
int Resource::ScreenHeight = 0;
Visual *Resource::ScreenVisual;
Colormap Resource::ScreenColormap;
//-----------------------------------------------------------------------------
XContext Resource::VdeskCID;
//-----------------------------------------------------------------------------
Window Resource::Root;
Window Resource::ActiveWindow;
Cursor Resource::ArrowCursor;
Cursor Resource::HandCursor;
Cursor Resource::HourCursor;
Cursor Resource::TextCursor;
BusyCursor *Resource::WaitCursor = NULL;
//-----------------------------------------------------------------------------
int Resource::FontSize;
string Resource::FontName;
string Resource::XFontName;
string Resource::DefaultIcon;
int Resource::DefaultIconSize = 0;
//-----------------------------------------------------------------------------
XftFont *Resource::Font;
Color Resource::FColor;
Color Resource::BColor;
Color Resource::SColor;
Color Resource::RColor;
Color Resource::CForeground;
Color Resource::CBackground;
Color Resource::CBarText;
Color Resource::CBarColor;
Color Resource::CSelectedText;
Color Resource::CSelectedBar;
Color Resource::CDisabledText;
//-----------------------------------------------------------------------------
ImlibData *Resource::ScreenData = NULL;
ImlibImage *Resource::BrokenIcon = NULL;
Background *Resource::VdeskBg = NULL;
//-----------------------------------------------------------------------------
vector<char*> Resource::SystemPath;
int Resource::MyUID = -1;
int Resource::MyGID = -1;
//-----------------------------------------------------------------------------
Resource::Resource(): Preferences()
{
	if( ++count > 1 ) return;

#ifdef USE_MB_TEXT
	LocaleInit();
#endif

	display = XOpenDisplay( NULL );
	if( !display ) {
		cout << "* Error: Cannot open display." << endl;
		exit( 0 );
	}
	screen = DefaultScreen( display );
	ScreenVisual = DefaultVisual( display, screen );
	ScreenColormap = DefaultColormap( display, screen );
	ScreenWidth = WidthOfScreen( DefaultScreenOfDisplay( display ) );
	ScreenHeight = HeightOfScreen( DefaultScreenOfDisplay( display ) );
	Root = DefaultRootWindow( display );
	VdeskCID = XUniqueContext();
	ScreenData = Imlib_init( display );
	BrokenIcon = Imlib_inlined_png_to_image( ScreenData, BrokenImage,
								SIZE_OF_ARRAY(BrokenImage) );
	VdeskBg = new Background( Config->QueryAsStr( "Background.Source" ),
								Config->QueryAsStr( "Background.Mode" ),
								Config->QueryAsInt( "Background.Delay" ) );

	DefaultIconSize = Config->QueryAsInt( "DefaultIconSize", 0 );
	if( DefaultIconSize <= 0 )
		DefaultIconSize = 0;
	else
	if( DefaultIconSize < 16 )
		DefaultIconSize = 16;
	else
	if( DefaultIconSize > 128 )
		DefaultIconSize = 128;

	char *p = getenv( "PATH" );
	if( p ) {
		p = strdup( p );
		p = strtok( p, ":" );
		while( p ) {
			SystemPath.push_back( p );
			p = strtok( NULL, ":" );
		}
	}
	MyUID = getuid();
	MyGID = getgid();

	LoadFont();
	LoadColor();
	LoadCursor();
	CreateDefaultIcon();
}
//-----------------------------------------------------------------------------
Resource::~Resource()
{
	if( --count ) return;

	free( SystemPath[0] );
	Imlib_kill_image( ScreenData, BrokenIcon );
	delete VdeskBg;

	FreeCursor();
	FreeFont();

	XCloseDisplay( display );
}
//-----------------------------------------------------------------------------
void Resource::CreateDefaultIcon()
{
	int size;
	ImlibImage *p, *dc;

	if( DefaultIconSize )
		size = DefaultIconSize;
	else
	if( ScreenWidth<=800 )
		size = 32;
	else
	if( ScreenWidth<=1024 )
		size = 48;
	else
	if( ScreenWidth>=1600 )
		size = 100;
	else
		size = 64;

	DefaultIcon = IconFolders[0] + "default.png";
	dc = Imlib_load_image( ScreenData, (char *)DefaultIcon.c_str() );
	if( !dc || size != dc->width ) {
		if( dc ) Imlib_kill_image( ScreenData, dc );
		dc = Imlib_inlined_png_to_image( ScreenData,
										Penguin, SIZE_OF_ARRAY(Penguin));
		p = Imlib_clone_scaled_image( ScreenData, dc, size, size );
		Imlib_save_image( ScreenData, p, (char *)DefaultIcon.c_str(), NULL );
		Imlib_kill_image( ScreenData, p );
	}
	Imlib_kill_image( ScreenData, dc );
}
//-----------------------------------------------------------------------------
#ifdef USE_MB_TEXT
void Resource::LocaleInit()
{
	char *lang = getenv("LANG");
	lang = lang ? strdup(lang) : strdup("en_US");
	if( !strstr(lang, ".UTF-8") ) {
		char *p = strchr(lang, '.');
		if( p ) *p = 0;
		p = (char *)malloc(strlen(lang)+8);
		sprintf(p, "%s%s", lang, ".UTF-8");
		setenv("LANG", p, 1);
		free(lang);
		lang = p;
	}
	if( !setlocale(LC_ALL, "") ) {
		printf("Error: Cannot set locale %s!\n", lang);
		exit(0);
	}
	free(lang);
}
#endif
//-----------------------------------------------------------------------------
void Resource::LoadFont()
{
	FontSize = Config->QueryAsInt( "FontSize" );
	FontName = Config->Query( "FontName" );
	Font = XftFontOpen( display, screen,
				XFT_FAMILY, XftTypeString, (char *)FontName.c_str(),
				XFT_SIZE, XftTypeDouble, (double)FontSize,
				XFT_WEIGHT, XftTypeInteger,
				FontBold ? XFT_WEIGHT_BOLD : XFT_WEIGHT_MEDIUM,
				NULL );

	if( !Font ) {
		cerr << "* Error: Can't find font: " << FontName << endl;
		exit( -1 );
	}

	LoadFontSet();
}
//-----------------------------------------------------------------------------
void Resource::LoadFontSet()
{
#ifdef USE_MB_TEXT
	char *name;
	char *style;
	char fn[128], *xfn;
	char **mc, *dc;
	int n;

	XFontName = Config->Query( "XFontName" );
	xfn = (char *)XFontName.c_str();

	if( xfn && *xfn )
		strcpy( fn, xfn );
	else {
		XftPatternGetString(Font->pattern, XFT_FAMILY, 0, &name);
		XftPatternGetString(Font->pattern, XFT_STYLE, 0, &style);
		style = (char *)(!strcasecmp(style, "Bold") ? "bold" : "medium");
		sprintf( fn, "*-%s-%s-r-*", name, style );
	}
	FontSet = XCreateFontSet( display, fn, &mc, &n, &dc );
#endif
}
//-----------------------------------------------------------------------------
void Resource::LoadColor()
{
	FColor.Set( Config->QueryAsStr( "FontColor" ) );
	SColor.Set( Config->QueryAsStr( "ShadowColor", "black" ) );
	RColor.Copy( SColor, true );

	if( HighContrastValue[ 0 ] == '#' ) {
		BColor.Set( HighContrastValue );
		HighContrast = true;
	}
	else {
		BColor.Set( 65535, (XftColor *)FColor );
	}
	BColor.SetAlpha( 21845 );

	CForeground.Set( Config->QueryAsStr( "Control.Foreground", "black" ) );
	CBackground.Set( Config->QueryAsStr( "Control.Background", "white" ) );
	CBarText.Set( Config->QueryAsStr( "Control.BarText", "white" ) );
	CBarColor.Set( Config->QueryAsStr( "Control.BarColor", "SteelBlue" ) );
	CSelectedText.Set( Config->QueryAsStr( "Control.SelectedText", "white" ) );
	CSelectedBar.Set( Config->QueryAsStr( "Control.SelectedBar", "black" ) );
	CDisabledText.Set( Config->QueryAsStr( "Control.DisabledText", "gray70" ) );
}
//-----------------------------------------------------------------------------
void Resource::LoadCursor()
{
	ArrowCursor = XCreateFontCursor( display, XC_left_ptr );
	HourCursor = XCreateFontCursor( display, XC_watch );
	HandCursor = XCreateFontCursor( display, XC_hand2 );
	TextCursor = XCreateFontCursor( display, XC_xterm );
	WaitCursor = new BusyCursor();
}
//-----------------------------------------------------------------------------
void Resource::FreeFont()
{
#ifdef USE_MB_TEXT
	XFreeFontSet( display, FontSet );
#endif
	XftFontClose( display, Font );
}
//-----------------------------------------------------------------------------
void Resource::FreeCursor()
{
	delete WaitCursor;
	XFreeCursor( display, TextCursor );
	XFreeCursor( display, HandCursor );
	XFreeCursor( display, HourCursor );
	XFreeCursor( display, ArrowCursor );
}
//-----------------------------------------------------------------------------
void Resource::Save()
{
	Preferences::Save();

	Config->Set( "FontName", FontName );
	Config->Set( "FontSize", FontSize );
	Config->Set( "FontColor", FColor.ConvertToString() );
	Config->Set( "ShadowColor", SColor.ConvertToString() );

	Config->Set( "Control.Foreground", CForeground.ConvertToString() );
	Config->Set( "Control.Background", CBackground.ConvertToString() );
	Config->Set( "Control.BarText", CBarText.ConvertToString() );
	Config->Set( "Control.BarColor", CBarColor.ConvertToString() );
	Config->Set( "Control.SelectedText", CSelectedText.ConvertToString() );
	Config->Set( "Control.SelectedBar", CSelectedBar.ConvertToString() );
	Config->Set( "Control.DisabledText", CDisabledText.ConvertToString() );
	Database::SaveToFile( VdeskFolder + "config", Config );
}
//-----------------------------------------------------------------------------
