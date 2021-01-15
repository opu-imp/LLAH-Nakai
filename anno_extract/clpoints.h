//#define	CL_OUTPUT_IMAGE

#define	FP_PDF	(0)
#define	FP_SCAN	(1)

#define	kMaxClusters	(10)

#define	kClScale	(0.5)
#define	kMinConSize	(20)
#define	kMaxConSize	(1000)
#define	kClCmpStep	(4)

void ColorClusteringAndMakePoints( IplImage *src, int cluster_count, int mode, int *nums, CvPoint **clps, double avgs[][3], char *dir, char *base );
int NearestCluster(unsigned char *pixels, double avgs[][3], int cluster_count );
int SaveClusterPoints( char *fname, int cluster_count, int *nums, CvPoint **clps, double avgs[][3] );
int LoadClusterPoints( char *fname, int *cluster_count, int *nums, CvPoint **clps, double avgs[][3] );
void SavePointFile( char *fname, CvPoint *ps, int num, CvSize size );

#ifndef	cColors
#define	cWhite	CV_RGB( 255,255,255 )
#define	cBlack	CV_RGB( 0,0,0 )
#define	cRed	CV_RGB( 255,0,0 )
#define	cGreen	CV_RGB( 0,255,0 )
#define	cBlue	CV_RGB( 0,0,255 )
#define	cRandom	CV_RGB( rand()%256, rand()%256, rand()%256 )
#define	cColors
#endif
