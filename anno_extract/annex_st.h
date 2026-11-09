typedef struct _strSimParam {
	double theta;
	double s;
	double tx;
	double ty;
} strSimParam;

typedef struct _strAffParam {
	double a1;
	double b1;
	double c1;
	double a2;
	double b2;
	double c2;
} strAffParam;

#define	GetPointsDistance(p1, p2)	(sqrt((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y)))
#define	kBgVal	(255)

void SimilarTransformation( cv::Mat src, cv::Mat dst, double *param );
void SimPoint( cv::Point *p1, cv::Point *p2, double *param );
void CalcSimParam( cv::Point p11, cv::Point p12, cv::Point p21, cv::Point p22, double *param );
void AffineTransformation( cv::Mat src, cv::Mat dst, double *param );
void AffPoint( cv::Point *p1, cv::Point *p2, double *param );
void CalcAffParam( cv::Point p11, cv::Point p12, cv::Point p13, cv::Point p21, cv::Point p22, cv::Point p23, double *param );
