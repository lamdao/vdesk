#include "Preferences.h"
#include <sys/stat.h>
#include <sys/types.h>
//-----------------------------------------------------------------------------
#define XCLEARLYU "-mutt-clearlyu-medium-r-normal--17-120-100-100-p-124-iso10646-1" 
//-----------------------------------------------------------------------------
Table *Preferences::Config = NULL;
//-----------------------------------------------------------------------------
bool Preferences::Locked = false;
bool Preferences::FontBold = false;
bool Preferences::SingleClick = false;
//-----------------------------------------------------------------------------
int Preferences::DarkRatio = 40;
int Preferences::DarkLevel = 100;
bool Preferences::AdaptiveText = false;
bool Preferences::HighContrast = false;
char *Preferences::HighContrastValue = NULL;
//-----------------------------------------------------------------------------
bool Preferences::Shadow = false;
int Preferences::ShadowX = 0;
int Preferences::ShadowY = 0;
//-----------------------------------------------------------------------------
int Preferences::Transparency = 0;
//-----------------------------------------------------------------------------
string Preferences::HomeFolder;
string Preferences::VdeskFolder;
//-----------------------------------------------------------------------------
vector<string> Preferences::IconFolders;
//-----------------------------------------------------------------------------
Preferences::Preferences()
{
	if( Config ) return;

	HomeFolder = string(getenv( "HOME" )) + "/";
	VdeskFolder = HomeFolder + ".vdesk/";
	IconFolders.push_back( VdeskFolder + "icons/" );
	IconFolders.push_back( VdeskFolder );
	Load();
}
//-----------------------------------------------------------------------------
Preferences::~Preferences()
{
}
//-----------------------------------------------------------------------------
void Preferences::Load()
{
	char *p;
	Database *db;
	string dbFile = VdeskFolder + "config";
	db = new Database( dbFile );
	if( !(Config = db->Query( "Config", true )) ) {
		cerr << "* Error: Can't find config file, or missing 'Config' table "
			 << "in the config file.\n"
			 << "Vdesk try to create a default settings...";
		mkdir( VdeskFolder.c_str(), 0700 );
		mkdir( IconFolders[0].c_str(), 0700 );
		SetDefaultConfig();
		db->Add( Config );
		db->Save();
		Config = db->Query( "Config", true );
		cerr << "done.\n";
	}
	delete db;

	FontBold = Config->QueryAsBool( "Bold" );
	Locked = Config->QueryAsBool( "Locked" );
	Shadow = Config->QueryAsBool( "Shadow" );
	ShadowX = Config->QueryAsInt( "ShadowX", 1 );
	ShadowY = Config->QueryAsInt( "ShadowY", 1 );
	SingleClick = Config->QueryAsBool( "SingleClick" );
	Transparency = Config->QueryAsInt( "Transparency" );
	HighContrast = Config->QueryAsBool( "HighContrast" );
	HighContrastValue = Config->QueryAsStr( "HighContrast" );
	AdaptiveText = HighContrast ? false : Config->QueryAsBool( "AdaptiveText" );
	DarkLevel = Config->QueryAsInt( "DarkLevel", 100 );
	DarkRatio = Config->QueryAsInt( "DarkRatio", 40 );

	string ip( Config->QueryAsStr( "IconPath" ) );
	if( ip != "" ) {
		p = strtok( (char *)ip.c_str(), ":" );
		while( p ) {
			string sp(p);
			if( p[0] == '~' && p[1] == '/' )
				sp.replace( 0, 2, HomeFolder );
			if( sp[sp.size() - 1] != '/' )
				sp += "/";
			IconFolders.push_back( sp );
			p = strtok(NULL, ":");
		}
	}
}
//-----------------------------------------------------------------------------
void Preferences::Save()
{
	if( !Config )
		Config = new Table( "Config" );
	
	Config->Set( "Bold", FontBold );
	Config->Set( "Locked", Locked );
	Config->Set( "Shadow", Shadow );
	Config->Set( "ShadowX", ShadowX );
	Config->Set( "ShadowY", ShadowY );
	Config->Set( "SingleClick", SingleClick );
	Config->Set( "Transparency", Transparency );
	Config->Set( "AdaptiveText", AdaptiveText );
	Config->Set( "DarkLevel", DarkLevel );
	Config->Set( "DarkRatio", DarkRatio );

	if( HighContrast && HighContrastValue && HighContrastValue[0] == '#' )
		Config->Set( "HighContrast", string(HighContrastValue) );
	else
		Config->Set( "HighContrast", HighContrast );
}
//-----------------------------------------------------------------------------
void Preferences::SetDefaultConfig()
{
	FontBold = false;
	Locked = false;
	Shadow = true;
	ShadowX = 1;
	ShadowY = 2;
	SingleClick = false;
	Transparency = 80;
	HighContrast = false;
	AdaptiveText = true;
	DarkLevel = 100;
	DarkRatio = 40;
	Save();

	Config->Set( "FontName", string("clearlyu") );
	Config->Set( "FontSize", 16 );
	Config->Set( "FontColor", string("white") );
	Config->Set( "ShadowColor", string("black") );
	Config->Set( "XFontName", string(XCLEARLYU) );

	SetDefaultControlColor();
}
//-----------------------------------------------------------------------------
void Preferences::SetDefaultControlColor()
{
	Config->Set( "Control.Foreground", string("black") );
	Config->Set( "Control.Background", string("white") );
	Config->Set( "Control.BarText", string("white") );
	Config->Set( "Control.BarColor", string("SteelBlue") );
	Config->Set( "Control.SelectedText", string("white") );
	Config->Set( "Control.SelectedBar", string("black") );
	Config->Set( "Control.DisabledText", string("gray70") );
}
//-----------------------------------------------------------------------------
