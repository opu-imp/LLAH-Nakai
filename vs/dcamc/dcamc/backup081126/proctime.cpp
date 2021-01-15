#include "def_general.h"
#ifdef	LINUX
#include <sys/resource.h>
#include <sys/time.h>
#endif
#ifdef	WIN
#include <windows.h>
#include <mmsystem.h>
#endif
#include "proctime.h"

TIME_COUNT GetProcTimeSec( void )
{
#ifdef	LINUX_TIME
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return (TIME_COUNT)RU.ru_utime.tv_sec + (TIME_COUNT)(RU.ru_utime.tv_usec*1e-6);
#endif
#ifdef	WIN_TIME
	return timeGetTime()*1000;
#endif
}

TIME_COUNT GetProcTimeMiliSec( void )
{
#ifdef	LINUX_TIME
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return (TIME_COUNT)(RU.ru_utime.tv_sec*1000) + (TIME_COUNT)(RU.ru_utime.tv_usec*1e-3);
#endif
#ifdef	WIN_TIME
	return timeGetTime();
#endif
}

TIME_COUNT GetProcTimeMicroSec( void )
{
#ifdef	LINUX_TIME
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return (TIME_COUNT)RU.ru_utime.tv_sec*1000000 + (TIME_COUNT)RU.ru_utime.tv_usec;
#endif
#ifdef	WIN_TIME
	return (TIME_COUNT)(timeGetTime() / 1000);
#endif
}
