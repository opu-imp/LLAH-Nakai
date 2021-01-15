#define	kCloseToLine	(2.0)	/* IsCloseToLine‚ÅŽg‚¤ */
#define	kLinePoints	(50)	/*CalcHLList‚ÅŽg‚¤ */

#define	kUseCR	(1)
#define	kUseJI	(2)

#define	kSeqOrder	(1)
#define	kAllAssort	(2)

typedef struct _strPtList {
	CvPoint pt;
	struct _strPtList *next;
} strPtList;

typedef struct _strLineForCR{
	double rho;
	double theta;
	strPtList *first;
	int ptnum;
} strLineForCR;

int CalcHoughLinesImg(IplImage *dst, strLineForCR **line_cr);
void CalcHoughLineCRHist(CvPoint *ps, int num,  CvSize size);
void ResisterPointToLines( CvPoint pt, strLineForCR *lines, int line_num );
int IsCloseToLine( CvPoint pt, strLineForCR *line );
void AddPoint( CvPoint pt, strLineForCR *line );
//void CalcHLHist( strHist *hist, strLineForCR *lines, int line_num );
