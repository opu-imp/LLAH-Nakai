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

void SimilarTransformation( IplImage *src, IplImage *dst, double *param );
void SimPoint( CvPoint *p1, CvPoint *p2, double *param );
void CalcSimParam( CvPoint p11, CvPoint p12, CvPoint p21, CvPoint p22, double *param );
void AffineTransformation( IplImage *src, IplImage *dst, double *param );
void AffPoint( CvPoint *p1, CvPoint *p2, double *param );
void CalcAffParam( CvPoint p11, CvPoint p12, CvPoint p13, CvPoint p21, CvPoint p22, CvPoint p23, double *param );
