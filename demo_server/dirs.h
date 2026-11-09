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
//#define	DRAW_FP

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

#define	cWhite	cv::Scalar(255,255,255)
#define	cBlack	cv::Scalar(0,0,0)
#define	cRed	cv::Scalar(0,0,255)
#define	cGreen	cv::Scalar(0,255,0)
#define	cBlue	cv::Scalar(255,0,0)
#define	cRandom	cv::Scalar( rand()%256, rand()%256, rand()%256 )

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
#define	CONST_MULTI_HASH_MODE	(11)	/* Construct multiple hash tables */
#define	CONST_DUAL_HASH_MODE	(12)	/* Construct dual hash */
#define	USBCAM_SERVER_DUAL_HASH_MODE	(13)
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

int FirstPoint(cv::Point *ps, int num);
int NextLine(int pt, cv::Point *ps, int num, int *nears[], int visited[]);
int RightPoint(int pt, cv::Point *ps, int num, int nears[]);
int LeftPoint(int pt, cv::Point *ps, int num, int nears[], int visited[]);
int DownPoint(int pt, cv::Point *ps, int num, int nears[]);
int IsOnRight( cv::Point *p1, cv::Point *p2 );
int IsOnLeft( cv::Point *p1, cv::Point *p2 );
int IsOnDown( cv::Point *p1, cv::Point *p2 );
void Calc1NNDiscriptor(cv::Point *ps, int num, int *nears[], char disc[]);
int IsConnected1NN(int p1, int p2, int *nears[]);
int IsConnectedWord(cv::Point p1, cv::Point p2, double *dist);
//double GetPointsDistance(cv::Point p1, cv::Point p2);
//long double GetPointsAngle(cv::Point p1, cv::Point p2);
void CalcCRDiscriptorLine(cv::Point *ps, int num, int *nears[]);
int FirstFirstRightPointCR(int pt, cv::Point *ps, int *nears[]);
int FirstRightPointCR(int pt, cv::Point *ps, int *nears[], double ang0);
int RightPointCR(int pt, int pt0, cv::Point *ps, int nears[], long double ang0 );
int NextLineCR(int pt, cv::Point *ps, int *nears[], double ang0);
double CalcCR( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 );
void JInvariantTest(void);
double CalcJI( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 );
#ifndef FIRST_TOP_LEVEL_CONTOUR_DEFINED
#define FIRST_TOP_LEVEL_CONTOUR_DEFINED
// Index of the first top-level contour (parent == -1). Iterating
//   for ( ci = FirstTopLevelContour( h ); ci >= 0; ci = h[ci][0] )
// visits the same contours as the OpenCV 1.x/2.x  first_contour->h_next  chain.
inline int FirstTopLevelContour( const std::vector<cv::Vec4i> &hierarchy )
{
	for ( size_t k = 0; k < hierarchy.size(); k++ )	if ( hierarchy[k][3] < 0 )	return (int)k;
	return -1;
}

// cvFindContours (OpenCV 1.x/2.x) cleared the 1-pixel image border before
// tracing, so components touching the border lost their edge pixels.
// cv::findContours (OpenCV >= 3.2) pads the image instead. Call this right
// before cv::findContours to get the same contours as the original code
// (which also modified the image in place).
inline void ClearImageBorder( cv::Mat img )
{
	cv::rectangle( img, cv::Rect( 0, 0, img.cols, img.rows ), cv::Scalar::all( 0 ), 1 );
}
#endif
int CalcCentres(cv::Point **ps, std::vector<std::vector<cv::Point>> &contours, std::vector<cv::Vec4i> &hierarchy, cv::Size *size, double **areas0);
int OutPutImage(cv::Mat img);
int MakeCentresFromImage(cv::Point **ps, cv::Mat , cv::Size *size, double **areas);
void ReleaseCentres( cv::Point *ps );
void ReleaseNears( int **nears, int num );
void OutPutResult( int *s, int t, int disp, char *fname );
void OutPutResultSv( int *s, int t, int disp, char *result );
int ScreenPointsWithNears( cv::Point *ps, int num, int **nears );
void DisplayPs( void );
void CreatePointFile( char *in_fname, char *out_fname, int mode );
int RetrieveUSBCamServer( void );
int RetrieveUSBCamServerDualHash( void );
int IsSucceed( char *str1, char *str2 );
double Calc12Diff( int *score );
cv::Mat MakeFeaturePointImage( cv::Mat orig, cv::Mat connected, cv::Point *ps, int num );
void ShowScoreGraph( char *win_name, int *score, int doc_num );
int CalcTotalScore( int doc_num, int num1, int num2, int *numall1, int *numall2, int *score1, int *score2 );
