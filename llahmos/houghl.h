#define	kCloseToLine	(2.0)	/* IsCloseToLine‚ÅŽg‚¤ */
#define	kLinePoints	(50)	/*CalcHLList‚ÅŽg‚¤ */

#define	kUseCR	(1)
#define	kUseJI	(2)

#define	kSeqOrder	(1)
#define	kAllAssort	(2)

typedef struct _strPtList {
	cv::Point pt;
	struct _strPtList *next;
} strPtList;

typedef struct _strLineForCR{
	double rho;
	double theta;
	strPtList *first;
	int ptnum;
} strLineForCR;

int CalcHoughLinesImg(cv::Mat dst, strLineForCR **line_cr);
void CalcHoughLineCRHist(cv::Point *ps, int num,  cv::Size size);
void ResisterPointToLines( cv::Point pt, strLineForCR *lines, int line_num );
int IsCloseToLine( cv::Point pt, strLineForCR *line );
void AddPoint( cv::Point pt, strLineForCR *line );
//void CalcHLHist( strHist *hist, strLineForCR *lines, int line_num );
