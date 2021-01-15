//#define	CON_1NN
#define	CON_WORD

#ifdef	USE_PIC
	#define	DRAW_PIC
	#define	SHOW_PIC
	#define	SAVE_PIC
#endif
//#define	DRAW_BY_NUM	/* DrawPointで数字を表示する*/
//#define	DRAW_COR
//#define	DRAW_COR_ALL

//#define	kNears	5	/* 1NN,Wordの場合*/
#define	kNears	30	/* CRの場合 */
#define	kDivX	4	/* MakeNearsFromCentresDiv用 */
#define	kDivY	5	/* MakeNearsFromCentresDiv用 */
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

#define	kLittleVal	(0.000001L)	/* CalcCR用 */
//#define	kMinPointsToCalcParam	(10)	/* 射影変換パラメータを計算する際の最小対応点数（4以上にすること）*/
#define	kMinPointsToCalcParam	(4)	/* 射影変換パラメータを計算する際の最小対応点数（4以上にすること）*/

#ifdef	WIN32
#define	kAcrPath	"C:\\doc\\src\\vs\\acr2win\\debug\\acr2win.exe"
#else
#define	kAcrPath	"/home/nakai/src/acr2/acr2"
#endif

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
#define	CHK_HASH_MODE	(6)	/* Check and analyse the hash table */
#define	CREATE_RPF_MODE	(7)	/* Create point file of registered image */
#define	USBCAM_SERVER_MODE	(8)
#define	CREATE_RPF_MODE2	(9)
#define	CONST_HASH_PF_MODE	(10)	/* Construct hash using point.dat */
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

// 不変量タイプ
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

// 不変量タイプを表す文字
#define	kInvCharCRArea	'r'
#define	kInvCharCRInter	'i'
#define	kInvCharAffine	'a'
#define	kInvCharSimilar	's'

// 打ち切り関係
#define	TERM_NON	(0)
#define	TERM_PNUM	(1)
#define	TERM_RATE	(2)
#define	TERM_VNUM	(3)

// 携帯電話の拡大率
#define	kMPExp	9

#define	GetPointsDistance(p1, p2)	(sqrt((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y)))

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

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
//long double GetPointsAngle(CvPoint p1, CvPoint p2);
void CalcCRDiscriptorLine(CvPoint *ps, int num, int *nears[]);
int FirstFirstRightPointCR(int pt, CvPoint *ps, int *nears[]);
int FirstRightPointCR(int pt, CvPoint *ps, int *nears[], double ang0);
int RightPointCR(int pt, int pt0, CvPoint *ps, int nears[], long double ang0 );
int NextLineCR(int pt, CvPoint *ps, int *nears[], double ang0);
double CalcCR( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
void JInvariantTest(void);
double CalcJI( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
int CalcCentres(CvPoint **ps, CvSeq *contours, CvSize *size, double **areas0);
int OutPutImage(IplImage *img);
int MakeCentresFromImage(CvPoint **ps, IplImage *, CvSize *size, double **areas);
void ReleaseCentres( CvPoint *ps );
void ReleaseNears( int **nears, int num );
void OutPutResult( int *s, int t, int disp, char *fname );
void OutPutResultSv( int *s, int t, int disp, char *result );
int ScreenPointsWithNears( CvPoint *ps, int num, int **nears );
void DisplayPs( void );
void CreatePointFile( char *in_fname, char *out_fname, int mode );
int RetrieveUSBCamServer( void );
int IsSucceed( char *str1, char *str2 );
double Calc12Diff( int *score );
