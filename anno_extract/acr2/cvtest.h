// �s�ϗʃ^�C�v
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
//#define	DRAW_BY_NUM	/* DrawPoint�Ő�����\������*/

#ifndef M_PI /* �~���� (pi) */
#define M_PI (3.14159265358979323846L)
#endif

//#define	kNears	5	/* 1NN,Word�̏ꍇ*/
#define	kNears	30	/* CR�̏ꍇ */
#define	kRadRange (M_PI / 4.0)
#define	kRightRange kRadRange
#define	kLeftRange kRadRange
#define	kUpRange kRadRange
#define	kDownRange kRadRange
#define	kMaxDiscriptorSize	(4096)
#define	kWordSep	(1.4)
#define	kFFRAngle	(M_PI / 4.0)		/* FirstFirstRightPointCR�p�u�������������v*/
#define	kRoughlyAngle	(M_PI / 20.0)	/* FirstRightPointCR�p�u��⓯���v*/
#define	kNearlyAngle	(M_PI / 18.0)	/* RightPointCR�p�u�قƂ�Ǔ����v*/
#define	kNLAngle	(M_PI / 4.0)	/* NextLineCR�p */
//#define	kCloseAngle	(M_PI / 14400.0)	/* RightPointCR�p�@�s���̕����Ƃ̊p�x*/
#define	kCloseAngle	((long double)(M_PI / 1000000000.0L))	/* RightPointCR�p�@�s���̕����Ƃ̊p�x */
#define	kAlmostAngle	(M_PI / 4.0)	/* RightPointCR�p�@�E���ɂȂ��ƍ���̂�*/

#define	kFileNameLen	(64)

#define	kLittleVal	(0.000001L)	/* CalcCR�p */

#define	cWhite	cv::Scalar(255,255,255)
#define	cBlack	cv::Scalar(0,0,0)
#define	cRed	cv::Scalar(0,0,255)
#define	cGreen	cv::Scalar(0,255,0)
#define	cBlue	cv::Scalar(255,0,0)
#define	cRandom	cv::Scalar(rand(,rand()%256,rand()%256)%256 )

// �S�̂̃��[�h
#define	RETRIEVE_MODE	(0)
#define	CONST_HASH_MODE	(1)
#define	RET_MP_MODE			(2)
#define	TEST_MODE		(3)
#define	ADD_HASH_MODE	(4)
#define	CREATE_QPF_MODE	(5)	/* Create point file of query image */
// �ˉe�ϊ��̕␳�����邩���Ȃ���
#define	NOT_RECOVER_MODE	(0)
#define	RECOVER_MODE		(1)
// �����_�͉����g����
#define	CONNECTED_MODE	(0)
#define	ENCLOSED_MODE	(1)
#define	USEPF_MODE		(2)	/* kPFPrefix, kPFSuffix�̓_�t�@�C�����g�� */
// �����摜�͗p�ӂ���Ă��邩�i�c�����j
#define	NOT_PREPARED_MODE	(0)
#define	PREPARED_MODE		(1)
#define	LEAVE_MODE			(2)

// �g�ѓd�b�̊g�嗦
#define	kMPExp	9

#define	GetPointsDistance(p1, p2)	(sqrt((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y)))

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
long double GetPointsAngle(cv::Point p1, cv::Point p2);
void CalcCRDiscriptorLine(cv::Point *ps, int num, int *nears[]);
int FirstFirstRightPointCR(int pt, cv::Point *ps, int *nears[]);
int FirstRightPointCR(int pt, cv::Point *ps, int *nears[], double ang0);
int RightPointCR(int pt, int pt0, cv::Point *ps, int nears[], long double ang0 );
int NextLineCR(int pt, cv::Point *ps, int *nears[], double ang0);
double CalcCR( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 );
void JInvariantTest(void);
double CalcJI( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 );
int CalcCentres(cv::Point **ps, std::vector<std::vector<cv::Point>> &contours, cv::Size *size);
int OutPutImage(cv::Mat img);
int MakeCentresFromImage(cv::Point **ps, cv::Mat , cv::Size *size);
void MakeNearsFromCentres( cv::Point *ps, int num, int ***nears0 );
void ReleaseCentres( cv::Point *ps );
void ReleaseNears( int **nears, int num );
void OutPutResult( int *s, int t, int disp, char *fname );
int ScreenPointsWithNears( cv::Point *ps, int num, int **nears );
