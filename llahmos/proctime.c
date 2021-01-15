#include "def_general.h"
#include "proctime.h"

#ifdef	LINUX_TIME
#include <sys/resource.h>
#include <sys/time.h>
#endif
#ifdef	WIN_TIME
#include <windows.h>
#include <mmsystem.h>
#endif

int GetProcTimeSec( void )
{
#ifdef	LINUX_TIME
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return RU.ru_utime.tv_sec + (int)(RU.ru_utime.tv_usec*1e-6);
#endif
#ifdef	WIN_TIME
	return (int)(timeGetTime() / 1000);
#endif
}

int GetProcTimeMiliSec( void )
{
#ifdef	LINUX_TIME
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return (int)(RU.ru_utime.tv_sec*1000) + (int)(RU.ru_utime.tv_usec*1e-3);
#endif
#ifdef	WIN_TIME
	return timeGetTime();
#endif
}

int GetProcTimeMicroSec( void )
{
#ifdef	LINUX_TIME
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return RU.ru_utime.tv_sec*1000000 + RU.ru_utime.tv_usec;
#endif
#ifdef	WIN_TIME
	return timeGetTime()*1000;
#endif
}
