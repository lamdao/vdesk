#include <signal.h>
#include "Desktop.h"
//----------------------------------------------------------------------------
void init()
{
//	if( fork()>0 ) exit(0);
	for( int i=3; i<256; i++ ) close(i);
	setsid();
/*
#if 0
	signal(SIGFPE, SIG_IGN);
	signal(SIGSEGV, SIG_IGN);
	signal(SIGILL, SIG_IGN);
#endif
	signal(SIGSYS, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTRAP, SIG_IGN);
	signal(SIGIOT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
*/	signal(SIGCHLD, SIG_IGN);
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
