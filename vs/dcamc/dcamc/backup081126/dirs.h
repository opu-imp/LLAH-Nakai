//#define	CON_1NN
#define	CON_WORD

#ifdef	USE_PIC
	#define	DRAW_PIC
	#define	SHOW_PIC
	#define	SAVE_PIC
#endif
//#define	DRAW_BY_NUM	/* DrawPoint�Ő�����\������*/

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
#define	kMinPointsToCalcParam	(10)	/* �ˉe�ϊ��p�����[�^���v�Z����ۂ̍ŏ��Ή��_���i4�ȏ�ɂ��邱�Ɓj*/

#define	kLittleVal	(0.000001L)	/* CalcCR�p */

#define	cWhite	CV_RGB( 255,255,255 )
#define	cBlack	CV_RGB( 0,0,0 )
#define	cRed	CV_RGB( 255,0,0 )
#define	cGreen	CV_RGB( 0,255,0 )
#define	cBlue	CV_RGB( 0,0,255 )
#define	cRandom	CV_RGB( rand()%256, rand()%256, rand()%256 )

// �S�̂̃��[�h
#define	RETRIEVE_MODE	(0)
#define	CONST_HASH_MODE	(1)
#define	RET_MP_MODE			(2)
#define	TEST_MODE		(3)
#define	ADD_HASH_MODE	(4)
#define	CREATE_QPF_MODE	(5)	/* Create point file of query image */
#define	CHK_HASH_MODE	(6)	/* Check and analyse the hash table */
#define	CREATE_RPF_MODE	(7)	/* Create point file of registered image */
#define	RET_USBCAM_MODE	(8)
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

// �s�ϗʃ^�C�v
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

// �g�ѓd�b�̊g�嗦
#define	kMPExp	9

#define	GetPointsDistance(p1, p2)	(sqrt((double)((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y))))

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
int CalcFloatCentres(CvPoint2D32f **pfs, CvSeq *contours, CvSize *size, double **areas0);
int OutPutImage(IplImage *img);
int MakeCentresFromImage(CvPoint **ps, IplImage *, CvSize *size, double **areas);
int MakeFloatCentresFromImage(CvPoint2D32f **pfs, IplImage *, CvSize *size, double **areas);
void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 );
void ReleaseCentres( CvPoint *ps );
void ReleaseNears( int **nears, int num );
void OutPutResult( int *s, int t, int disp, char *fname );
int ScreenPointsWithNears( CvPoint *ps, int num, int **nears );
void DisplayPs( void );
void CreatePointFile( char *in_fname, char *out_fname, int mode );
IplImage *DrawPoints( CvPoint *ps, int num, CvSize *img_size );
