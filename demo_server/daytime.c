#ifdef	LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif
#ifdef	WIN32
#include <windows.h>
#include <mmsystem.h>
#endif
#include "daytime.h"

int GetDayTimeSec( void )
// gettimeofdat関数で時刻（秒）を取得する
{
#ifdef	LINUX
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (int)tv.tv_sec + (int)(tv.tv_usec*1e-6);
#endif
#ifdef	WIN32
	return timeGetTime()*1000;
#endif
}

int GetDayTimeMicroSec( void )
// gettimeofdat関数で時刻（秒）を取得する
{
#ifdef	LINUX
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (int)tv.tv_sec*1000000 + tv.tv_usec;
#endif
#ifdef	WIN32
	return (int)(timeGetTime() / 1000);
#endif
}
