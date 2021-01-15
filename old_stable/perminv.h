#define	kNum8C6	(28)
#define	kNum6C4	(15)

#define	CalcFp2( Fdashp )	(16.0L*(0.2-(1.0L/(6.0L-Fdashp))))
#define	CalcFd2( Fdashd )	(-(double)(3.0L/8.0L)*Fdashd)

void TestPerminv( CvSize size, CvPoint *ps, int num, int *nears[] );
double CalcRho( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
double CalcP( CvPoint p1, CvPoint p2, CvPoint p3 );
double CalcFp( double rho );
double CalcFd( double rho );
double CalcPsi( double rho );
double CalcFdashp( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
double CalcFdashd( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
void MakeCom6of8( void );
void MakeCom4of6( void );
