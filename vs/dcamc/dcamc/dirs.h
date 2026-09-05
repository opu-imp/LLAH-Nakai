//#define	CON_1NN
#define	CON_WORD

#ifdef	USE_PIC
	#define	DRAW_PIC
	#define	SHOW_PIC
	#define	SAVE_PIC
#endif
//#define	DRAW_BY_NUM	/* DrawPointで数字を表示する*/

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
#define	kMinPointsToCalcParam	(10)	/* 射影変換パラメータを計算する際の最小対応点数（4以上にすること）*/

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
#define	CHK_HASH_MODE	(6)	/* Check and analyse the hash table */
#define	CREATE_RPF_MODE	(7)	/* Create point file of registered image */
#define	RET_USBCAM_MODE	(8)
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

// 携帯電話の拡大率
#define	kMPExp	9

#define	GetPointsDistance(p1, p2)	(sqrt((double)((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y))))

void NearestPoint( int n, cv::Point *ps, int num, int nears[] );
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
// Index of the first top-level contour (parent == -1). Iterating
//   for ( ci = FirstTopLevelContour( h ); ci >= 0; ci = h[ci][0] )
// visits the same contours as the OpenCV 1.x/2.x  first_contour->h_next  chain.
inline int FirstTopLevelContour( const std::vector<cv::Vec4i> &hierarchy )
{
	for ( size_t k = 0; k < hierarchy.size(); k++ )	if ( hierarchy[k][3] < 0 )	return (int)k;
	return -1;
}
int CalcCentres(cv::Point **ps, const std::vector<std::vector<cv::Point>> &contours, const std::vector<cv::Vec4i> &hierarchy, cv::Size *size, double **areas0);
int CalcFloatCentres(cv::Point2f **pfs, const std::vector<std::vector<cv::Point>> &contours, const std::vector<cv::Vec4i> &hierarchy, cv::Size *size, double **areas0);
int OutPutImage(cv::Mat img);
int MakeCentresFromImage(cv::Point **ps, cv::Mat , cv::Size *size, double **areas);
int MakeFloatCentresFromImage(cv::Point2f **pfs, cv::Mat , cv::Size *size, double **areas);
void MakeNearsFromCentres( cv::Point *ps, int num, int ***nears0 );
void ReleaseCentres( cv::Point *ps );
void ReleaseNears( int **nears, int num );
void OutPutResult( int *s, int t, int disp, char *fname );
int ScreenPointsWithNears( cv::Point *ps, int num, int **nears );
void DisplayPs( void );
void CreatePointFile( char *in_fname, char *out_fname, int mode );
cv::Mat DrawPoints( cv::Point *ps, int num, cv::Size *img_size );
