#ifdef	WIN
#define	WIN_TIME
#else
#define	LINUX_TIME
#endif

#ifdef	WIN
// DWORD. nn5dcamc.cpp includes this header without windows.h; that only compiled
// while the OpenCV 1.x/2.x headers (cv.h, highgui.h) pulled windows.h in.
#include <windows.h>
typedef	DWORD	TIME_COUNT;
#else
typedef	int	TIME_COUNT;
#endif

TIME_COUNT GetProcTimeSec( void );
TIME_COUNT GetProcTimeMiliSec( void );
TIME_COUNT GetProcTimeMicroSec( void );
