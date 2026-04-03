#define	kMinValCr	(0.0001)	/* 0�ɂ�鏜�Z������邽�߁A�\���������Ƃ��ɗp����l */

double CalcCR5ByArea( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
double CalcArea( cv::Point p1, cv::Point p2, cv::Point p3 );
double CalcAffineInv( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 );
long double GetPointsAngle(cv::Point p1, cv::Point p2);
double CalcAngleFromThreePoints( cv::Point p1, cv::Point p2, cv::Point p3 );
double CalcSimilarInv( cv::Point p1, cv::Point p2, cv::Point p3 );
