#ifndef __PREFERENCES_H
#define __PREFERENCES_H
//------------------------------------------------------
#include  "Database.h"
//------------------------------------------------------
class Preferences {
protected:
	static Table *Config;

	static bool Locked;
	static bool FontBold;
	static bool SingleClick;
	static bool AdaptiveText;
	static bool HighContrast;
	static char *HighContrastValue;
	static bool TextShadow;
	static int TextShadowX;
	static int TextShadowY;
	static int Transparency;
	static int DarkLevel;
	static int DarkRatio;

	static string HomeFolder;
	static string VdeskFolder;
	static vector<string> IconFolders;

	void SetDefaultControlColor();
	virtual void SetDefaultConfig();
public:
	Preferences();
	~Preferences();

	virtual void Load();
	virtual void Save();
};
//------------------------------------------------------
#endif
