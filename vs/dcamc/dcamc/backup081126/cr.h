#define	kMinValCr	(0.0001)	/* 0�ɂ�鏜�Z������邽�߁A�\���������Ƃ��ɗp����l */

double CalcCR5ByArea( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
double CalcArea( CvPoint p1, CvPoint p2, CvPoint p3 );
double CalcAffineInv( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
long double GetPointsAngle(CvPoint p1, CvPoint p2);
double CalcAngleFromThreePoints( CvPoint p1, CvPoint p2, CvPoint p3 );
double CalcSimilarInv( CvPoint p1, CvPoint p2, CvPoint p3 );
