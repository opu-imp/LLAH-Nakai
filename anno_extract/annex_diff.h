#define	kDiffNear	(4)
#define	kLeaveAnnoThr	/*(100000000)*/	(200000)
#define	kEnoughThr	(20000)	/* 100 */
#define	kEraseThr	(20000)

void GetDiffImage0( IplImage *diff, IplImage *anno, IplImage *orig, int mode );
void GetDiffImage( IplImage *diff, IplImage *anno, IplImage *orig, int mode );
double CalcColorDist( unsigned char *c1, unsigned char *c2 );

void RGB2Lab( double *rgb, double *lab );
