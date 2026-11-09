#define	kMinValCr	(0.0001)	/* 0による除算を避けるため、十分小さいときに用いる値 */

double CalcCR5ByArea( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
double CalcArea( cv::Point p1, cv::Point p2, cv::Point p3 );
double CalcAffineInv( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 );
long double GetPointsAngle(cv::Point p1, cv::Point p2);
double CalcAngleFromThreePoints( cv::Point p1, cv::Point p2, cv::Point p3 );
double CalcSimilarInv( cv::Point p1, cv::Point p2, cv::Point p3 );
