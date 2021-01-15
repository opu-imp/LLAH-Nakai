#ifdef	WIN32
#define	WIN_TIME
#else
#define	LINUX_TIME
#endif

int GetProcTimeSec( void );
int GetProcTimeMiliSec( void );
int GetProcTimeMicroSec( void );
