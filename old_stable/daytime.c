#ifdef	LINUX_TIME
#include <stdlib.h>
#include <sys/time.h>
#endif
#ifdef	WIN_TIME
#include <windows.h>
#include <mmsystem.h>
#endif
#include "daytime.h"

int GetDayTimeSec( void )
// gettimeofdat関数で時刻（秒）を取得する
{
#ifdef	LINUX_TIME
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (int)tv.tv_sec + (int)(tv.tv_usec*1e-6);
#endif
#ifdef	WIN_TIME
	return timeGetTime()*1000;
#endif
}

int GetDayTimeMicroSec( void )
// gettimeofdat関数で時刻（秒）を取得する
{
#ifdef	LINUX_TIME
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (int)tv.tv_sec*1000000 + tv.tv_usec;
#endif
#ifdef	WIN_TIME
	return (int)(timeGetTime() / 1000);
#endif
}
