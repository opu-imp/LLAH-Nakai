#define	kNum8C6	(28)
#define	kNum6C4	(15)

#define	CalcFp2( Fdashp )	(16.0L*(0.2-(1.0L/(6.0L-Fdashp))))
#define	CalcFd2( Fdashd )	(-(double)(3.0L/8.0L)*Fdashd)

void TestPerminv( cv::Size size, cv::Point *ps, int num, int *nears[] );
double CalcRho( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
double CalcP( cv::Point p1, cv::Point p2, cv::Point p3 );
double CalcFp( double rho );
double CalcFd( double rho );
double CalcPsi( double rho );
double CalcFdashp( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
double CalcFdashd( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
void MakeCom6of8( void );
void MakeCom4of6( void );
