// 不変量タイプ
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

//#define	CON_1NN
#define	CON_WORD

#ifdef	USE_PIC
	#define	DRAW_PIC
	#define	SHOW_PIC
	#define	SAVE_PIC
#endif
//#define	DRAW_BY_NUM	/* DrawPointで数字を表示する*/

#ifndef M_PI /* 円周率 (pi) */
#define M_PI (3.14159265358979323846L)
#endif

//#define	kNears	5	/* 1NN,Wordの場合*/
#define	kNears	30	/* CRの場合 */
#define	kRadRange (M_PI / 4.0)
#define	kRightRange kRadRange
#define	kLeftRange kRadRange
#define	kUpRange kRadRange
#define	kDownRange kRadRange
#define	kMaxDiscriptorSize	(4096)
#define	kWordSep	(1.4)
#define	kFFRAngle	(M_PI / 4.0)		/* FirstFirstRightPointCR用「だいたい同じ」*/
#define	kRoughlyAngle	(M_PI / 20.0)	/* FirstRightPointCR用「やや同じ」*/
#define	kNearlyAngle	(M_PI / 18.0)	/* RightPointCR用「ほとんど同じ」*/
#define	kNLAngle	(M_PI / 4.0)	/* NextLineCR用 */
//#define	kCloseAngle	(M_PI / 14400.0)	/* RightPointCR用　行頭の文字との角度*/
#define	kCloseAngle	((long double)(M_PI / 1000000000.0L))	/* RightPointCR用　行頭の文字との角度 */
#define	kAlmostAngle	(M_PI / 4.0)	/* RightPointCR用　右側にないと困るので*/

#define	kFileNameLen	(64)

#define	kLittleVal	(0.000001L)	/* CalcCR用 */

#define	cWhite	CV_RGB( 255,255,255 )
#define	cBlack	CV_RGB( 0,0,0 )
#define	cRed	CV_RGB( 255,0,0 )
#define	cGreen	CV_RGB( 0,255,0 )
#define	cBlue	CV_RGB( 0,0,255 )
#define	cRandom	CV_RGB( rand()%256, rand()%256, rand()%256 )

// 全体のモード
#define	RETRIEVE_MODE	(0)
#define	CONST_HASH_MODE	(1)
#define	RET_MP_MODE			(2)
#define	TEST_MODE		(3)
#define	ADD_HASH_MODE	(4)
#define	CREATE_QPF_MODE	(5)	/* Create point file of query image */
// 射影変換の補正をするかしないか
#define	NOT_RECOVER_MODE	(0)
#define	RECOVER_MODE		(1)
// 特徴点は何を使うか
#define	CONNECTED_MODE	(0)
#define	ENCLOSED_MODE	(1)
#define	USEPF_MODE		(2)	/* kPFPrefix, kPFSuffixの点ファイルを使う */
// 結合画像は用意されているか（残すか）
#define	NOT_PREPARED_MODE	(0)
#define	PREPARED_MODE		(1)
#define	LEAVE_MODE			(2)

// 携帯電話の拡大率
#define	kMPExp	9

#define	GetPointsDistance(p1, p2)	(sqrt((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y)))

void NearestPoint( int n, CvPoint *ps, int num, int nears[] );
int FirstPoint(CvPoint *ps, int num);
int NextLine(int pt, CvPoint *ps, int num, int *nears[], int visited[]);
int RightPoint(int pt, CvPoint *ps, int num, int nears[]);
int LeftPoint(int pt, CvPoint *ps, int num, int nears[], int visited[]);
int DownPoint(int pt, CvPoint *ps, int num, int nears[]);
int IsOnRight( CvPoint *p1, CvPoint *p2 );
int IsOnLeft( CvPoint *p1, CvPoint *p2 );
int IsOnDown( CvPoint *p1, CvPoint *p2 );
void Calc1NNDiscriptor(CvPoint *ps, int num, int *nears[], char disc[]);
int IsConnected1NN(int p1, int p2, int *nears[]);
int IsConnectedWord(CvPoint p1, CvPoint p2, double *dist);
//double GetPointsDistance(CvPoint p1, CvPoint p2);
long double GetPointsAngle(CvPoint p1, CvPoint p2);
void CalcCRDiscriptorLine(CvPoint *ps, int num, int *nears[]);
int FirstFirstRightPointCR(int pt, CvPoint *ps, int *nears[]);
int FirstRightPointCR(int pt, CvPoint *ps, int *nears[], double ang0);
int RightPointCR(int pt, int pt0, CvPoint *ps, int nears[], long double ang0 );
int NextLineCR(int pt, CvPoint *ps, int *nears[], double ang0);
double CalcCR( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
void JInvariantTest(void);
double CalcJI( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
int CalcCentres(CvPoint **ps, CvSeq *contours, CvSize *size);
int OutPutImage(IplImage *img);
int MakeCentresFromImage(CvPoint **ps, IplImage *, CvSize *size);
void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 );
void ReleaseCentres( CvPoint *ps );
void ReleaseNears( int **nears, int num );
void OutPutResult( int *s, int t, int disp, char *fname );
int ScreenPointsWithNears( CvPoint *ps, int num, int **nears );
