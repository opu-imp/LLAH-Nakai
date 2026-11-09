//#define	CL_OUTPUT_IMAGE

#define	FP_PDF	(0)
#define	FP_SCAN	(1)

#define	kMaxClusters	(10)

#define	kClScale	(0.5)
#define	kMinConSize	(20)
#define	kMaxConSize	(1000)
#define	kClCmpStep	(4)

void ColorClusteringAndMakePoints( cv::Mat src, int cluster_count, int mode, int *nums, cv::Point **clps, double avgs[][3], char *dir, char *base );
int NearestCluster(unsigned char *pixels, double avgs[][3], int cluster_count );
int SaveClusterPoints( char *fname, int cluster_count, int *nums, cv::Point **clps, double avgs[][3] );
int LoadClusterPoints( char *fname, int *cluster_count, int *nums, cv::Point **clps, double avgs[][3] );
void SavePointFile( char *fname, cv::Point *ps, int num, cv::Size size );

#ifndef	cColors
#define	cWhite	cv::Scalar(255,255,255)
#define	cBlack	cv::Scalar(0,0,0)
#define	cRed	cv::Scalar(0,0,255)
#define	cGreen	cv::Scalar(0,255,0)
#define	cBlue	cv::Scalar(255,0,0)
#define	cRandom	cv::Scalar( rand()%256, rand()%256, rand()%256 )
#define	cColors
#endif
