#define	kMinValCr	(0.0001)	/* 0による除算を避けるため、十分小さいときに用いる値 */

double CalcCR5ByArea( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
double CalcArea( CvPoint p1, CvPoint p2, CvPoint p3 );
double CalcAffineInv( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
long double GetPointsAngle(CvPoint p1, CvPoint p2);
double CalcAngleFromThreePoints( CvPoint p1, CvPoint p2, CvPoint p3 );
double CalcSimilarInv( CvPoint p1, CvPoint p2, CvPoint p3 );
