#include "Timer.h"
//----------------------------------------------------------------------------
vector<TimerControl*> Timer::items;
//----------------------------------------------------------------------------
Timer::Timer(): Resource()
{
	c = ConnectionNumber( display );
	if( fcntl( c, F_SETFD, 1 ) == -1) {
		cerr << "* Error: Couldn't mark display connection as close-on-exec\n";
		exit(0);
	}
}
//----------------------------------------------------------------------------
void Timer::Update()
{
	int i, s = 1000000000;
	time_t now = time(0);

	for( i=items.size()-1; i>=0; i-- ) {
		int d = items[i]->GetDelay( now );
		if( d<=0 ) {
			items[i]->OnTime();
			if( !items[i]->IsOneShot() )
				d = items[i]->Reset();
			else {
				items.erase( items.begin() + i );
				continue;
			}
		}
		if( s>d ) s = d;
	}
	if( !s || s==1000000000 ) {
		usleep( 100 );
		return;
	}

	fd_set rfds;
	timeval tv = { s, 0 };
	FD_ZERO( &rfds );
	FD_SET( c, &rfds );
	if( !select( c+1, &rfds, 0, 0, &tv ) ) { // timeout
		now = time(0);
		for( i=items.size()-1; i>=0; i-- ) {
			TimerControl *t = items[i];

			if( t->GetDelay( now )<=0 ) {
				t->OnTime();
				if( t->IsOneShot() )
					items.erase( items.begin()+i );
				else
					t->Reset();
			}
		}
	}
}
//----------------------------------------------------------------------------
void Timer::Add( TimerControl *t, long d )
{
	t->SetDelay( d );
	for( int i=0; i<items.size(); i++ )
		if( t==items[i] ) return;
	items.push_back( t );
}
//----------------------------------------------------------------------------
void Timer::Remove( TimerControl *t )
{
	for( int i=0; i<items.size(); i++ )
		if( t==items[i] ) {
			items.erase( items.begin() + i );
			return;
		}
}
//----------------------------------------------------------------------------
