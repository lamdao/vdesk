#ifndef __BACKGROUND_H
#define __BACKGROUND_H
//----------------------------------------------------------------------------
#include "Action.h"
#include "Timer.h"
//----------------------------------------------------------------------------
class Background: public Resource, public TimerControl {
	class Cleaner: public TimerControl {
	private:
		ImlibImage **root;
	public:
		Cleaner( ImlibImage **r ): root( r ) {}
		void OnTime() {
			Imlib_kill_image( ScreenData, *root );
			*root = NULL;
		}
	};
	class Pattern {
	private:
		Pixmap pixmap;
	public:
		Pattern(): pixmap(None) {}
		virtual void Render() = 0;
	};

	class SolidPattern: public Pattern {
	public:
		SolidPattern();
		void Render();
	};

	class ModulaPattern: public Pattern {
	public:
		ModulaPattern();
		void Render();
	};

	class GradientPattern: public Pattern {
	public:
		GradientPattern();
		void Render();
	};
private:
	vector<char*> data[2];
	vector<char*> *save, *show;
	Pixmap RootPixmap;
	ImlibImage *SpareRoot;
	int delay, mode;
	bool refreshable;
	char *source, *current;
	time_t srctime;
	string images;

	Cleaner cleaner;
	ActionControl *controls;
	int command;

public:
	Background( char *s, char *m=NULL, int d=0 );
	~Background();

	void SetDelay( int d );
	void SetSource( char *s );
	void AddRefreshListener( ActionControl *a, int cmd );
	void Reload();
	void Refresh();

	ImlibImage *Crop( int x, int y, int w, int h );

	bool Refreshable() { return refreshable; }
	bool IsOneShot() { return false; }
	void OnTime();

private:
	void Init();
	void FreeData();
	void SwapData();
	void ScanSource( char *s );
	void SetRootAtoms( Pixmap p );
	void RenderBackground( ImlibImage *si, bool apply = true );
	void ChangeImage();
};
//----------------------------------------------------------------------------
#endif
