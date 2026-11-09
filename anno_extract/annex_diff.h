#define	kDiffNear	(4)
#define	kLeaveAnnoThr	/*(100000000)*/	(200000)
#define	kEnoughThr	(20000)	/* 100 */
#define	kEraseThr	(20000)

void GetDiffImage0( cv::Mat diff, cv::Mat anno, cv::Mat orig, int mode );
void GetDiffImage( cv::Mat diff, cv::Mat anno, cv::Mat orig, int mode );
double CalcColorDist( unsigned char *c1, unsigned char *c2 );

void RGB2Lab( double *rgb, double *lab );
