#ifdef	WIN
#define	WIN_TIME
#else
#define	LINUX_TIME
#endif

#ifdef	WIN
typedef	DWORD	TIME_COUNT;
#else
typedef	int	TIME_COUNT;
#endif

TIME_COUNT GetProcTimeSec( void );
TIME_COUNT GetProcTimeMiliSec( void );
TIME_COUNT GetProcTimeMicroSec( void );
