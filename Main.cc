#include "Desktop.h"
//----------------------------------------------------------------------------
void init()
{
	/*
	if( fork()>0 ) exit(0);
	for( int i=0; i<256; i++ ) close(i);
	setsid();
	/ *
	open("/dev/console", O_RDONLY);
	open("/dev/console", O_WRONLY);
	open("/dev/console", O_WRONLY);
	*/
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
	init();

	Desktop d;
	d.Load();
	d.Arrange();
	d.Process();

	return 0;
}
//----------------------------------------------------------------------------
