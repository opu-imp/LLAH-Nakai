//#define	VIDEO_OUTPUT
//#define	DRAW_COR
//#define	AR
//#define	RAPTOR
//#define	GRIFFON
//#define	MASAMUNE

//#define	THUMB_OUT

#define	kMaxDocNameLen	(20)	/* �摜���̍ő咷 */
#define	kMaxPointNum	5000
#define	kMaxPointNumServer	(2500)	/* �T�[�o���ł̍ő�����_�� */
#define	kSendBuffSize	(sizeof(CvSize) + sizeof(int) + sizeof(CvPoint) * kMaxPointNumServer)	/* ���M���̃o�b�t�@�̃T�C�Y */
#define	kSendBuffSizeAreas	(sizeof(CvSize) + sizeof(int) + (sizeof(CvPoint) + sizeof(unsigned short)) * kMaxPointNumServer)	/* ���M���̃o�b�t�@�̃T�C�Y�i�ʐς����M�o�[�W�����j */
#define	kRecvBuffSize	(kMaxDocNameLen + sizeof(strProjParam) + sizeof(CvSize))

//#define PORT_NO	12345
//#define	kPort	12345
//#define	kPointPort	65431
//#define	kResultPort	65432
//#define	kPointPort	65433
//#define	kResultPort	65434
//#define	SERVER_NAME	"kana"
//#define	SERVER_NAME	"arena"
//#define	kServerName	"kana"
//#define	kServerName	"mana"

#define	kTCP	(1)
#define	kUDP	(2)

#define	kMaxNameLen	(20)	/* �N���C�A���g���̍ő咷�� */
#define	SEND_FLAG	0
//#define	RECV_FLAG	MSG_WAITALL
#define	RECV_FLAG	0

#define	kVideoFileName	"video.avi"
#define	kVideoAspectRatio	(1.5)

#ifdef	GRIFFON
#define	kThumbDir	"C:\\data\\didb\\thumb\\"
#else
//#define	kThumbDir	"\\\\leo\\nakai\\didb\\thumb\\"	/* �o�^�����̏k���摜�̃f�B���N�g�� */
//#define	kThumbDir	"E:\\data\\didb\\thumb\\"
#define	kThumbDir	"C:\\data\\didb\\thumb\\"
//#define	kThumbDir	"C:\\data\\didb\\iwfhr04_thumb\\"
#endif
#define	kThumbSuffix	"jpg"	/* �k���摜�̊g���q */
#define	kThumbScale	(0.5)	/* �T���l�C���̃X�P�[�� */
#define	kResizeScaleImg	(0.5)	/* �r�f�I�p�k���̃X�P�[�� */
#define	kResizeScaleThumb	(0.5)
#define	kVideoFps	(7)

/* �ȉ��͌����E�o�^�����̂��߂̒�` */
// �s�ϗʃ^�C�v
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

// �f�t�H���g�̃p�����[�^
#define	kDefaultGroup1Num	(8)
#define	kDefaultGroup2Num	(6)
#define	kDefaultGroup3Num	(4)
#define	kDefaultInvType	AFFINE
#define	kDefaultDiscNum	(15)
#define	kDefaultClusters	(5)
#define	kDefaultResizeMode	(0)
//#define	kDefaultOutPutDir	"out/"
#define	kDefaultOutPutDir	"out0831/"
#define	kDefaultDiscFileName	"disc.txt"
#define	kAcrPath	"acr2/acr2"
#define	kDefaultAcr2Path	"acr2/acr2"
#define	kMaxParamValNum	(8)	/* �ϊ��i����or�A�t�B���j�p�����[�^�̍ő吔 */


IplImage *LoadThumb( char *fname );
void DrawThumb( char *doc_name, char *doc_name_prev, IplImage **thumb0, IplImage **thumb_draw0, strProjParam param, strProjParam zero_param, CvSize cap_size, CvSize res_size );
void DrawCor2( CvPoint corps[][2], int corpsnum, IplImage *img_cap, char *doc_name );
int GotoSnapshotMode( strDirectShowCap *dsc, unsigned char *img_buff, long buff_size, IplImage *img_cap, IplImage *thumb );
int GotoPasteMode( IplImage *img_cap, IplImage *thumb, CvPoint *p1, CvPoint *p2 );
void OnMouseThumb( int event, int x, int y, int flags, void *param );
int SaveAR( char *doc_name, IplImage *img_cap, CvSize size, CvPoint *p1, CvPoint *p2 );
void DrawPoints2( CvPoint *ps, int num, CvSize img_size );
void DrawParam( IplImage *img_cap, IplImage *thumb, strProjParam param, strProjParam zero_param );
int CaptureMovie( char *movfile );
int DecomposeMovie( char *mov_file );
void Buff2ImageData( unsigned char *buff, IplImage *img );
void ConvMovie( void );
IplImage *DrawPointsOverlap( CvPoint *ps, int num, CvSize *img_size, IplImage *img );
