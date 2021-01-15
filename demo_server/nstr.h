#ifdef	WIN32
#define	kDelimiterChar	'\\'
#else
#define	kDelimiterChar	'/'
#endif
#define	kDelimiterChar1	'/'
#define	kDelimiterChar2	'\\'

int AddSlash( char *path, int len );
int GetSuffix( char *path, int len, char *suff );
int GetBasename( char *path, int len, char *basename );
int GetBasename2( char *path, int len, char *basename );
int GetDir( char *path, int len, char *dir );
