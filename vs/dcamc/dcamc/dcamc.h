//#define	VIDEO_OUTPUT	/* 撮影画像・検索結果・特徴点を動画として出力する */
//#define	POINT_OUTPUT	/* 特徴点を静止画像として出力する（上のVIDEO_OUTPUTが必須） */
//#define	DRAW_COR
//#define	AR
//#define	RAPTOR
//#define	GRIFFON
//#define	MASAMUNE

//#define	VIDEO_RECORD

//#define	THUMB_OUT

//#define	DRAW_HIST
//#define	DRAW_FLAG
//#define	GAUSS_VARIABLE

#define	kMaxDocNameLen	(100)	/* 画像名の最大長 */
#define	kMaxPointNum	5000
//#define	kMaxPointNumServer	(2500)	/* サーバ側での最大特徴点数 */
#define	kMaxPointNumServer	(5000)	/* サーバ側での最大特徴点数 */
#define	kSendBuffSize	(sizeof(cv::Size) + sizeof(int) + sizeof(cv::Point) * kMaxPointNumServer)	/* 送信時のバッファのサイズ */
#define	kSendBuffSizeAreas	(sizeof(cv::Size) + sizeof(int) + (sizeof(cv::Point) + sizeof(unsigned short)) * kMaxPointNumServer)	/* 送信時のバッファのサイズ（面積も送信バージョン） */
#define	kRecvBuffSize	(kMaxDocNameLen + sizeof(strProjParam) + sizeof(cv::Size))

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

#define	kMaxNameLen	(20)	/* クライアント名の最大長さ */
#define	SEND_FLAG	0
//#define	RECV_FLAG	MSG_WAITALL
#define	RECV_FLAG	0

#define	kVideoFileName	"video.avi"
#define	kVideoAspectRatio	(1.5)

#ifdef	GRIFFON
#define	kThumbDir	"C:\\data\\didb\\thumb\\"
#else
//#define	kThumbDir	"\\\\leo\\nakai\\didb\\thumb\\"	/* 登録文書の縮小画像のディレクトリ */
//#define	kThumbDir	"E:\\data\\didb\\thumb\\"
#define	kThumbDir	"C:\\data\\didb\\thumb\\"
//#define	kThumbDir	"C:\\data\\didb\\iwfhr04_thumb\\"
#endif
#define	kThumbSuffix	"jpg"	/* 縮小画像の拡張子 */
#define	kThumbScale	(0.5)	/* サムネイルのスケール */
#define	kResizeScaleImg	(0.5)	/* ビデオ用縮小のスケール */
#define	kResizeScaleThumb	(0.5)

#define	kFlagDir	"C:\\data\\didb\\flag\\"
#define	kFlagSuffix	"jpg"

//#define	kVideoFps	(7)
#define	kVideoFps	(3)

/* 以下は検索・登録処理のための定義 */
// 不変量タイプ
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

// デフォルトのパラメータ
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
#define	kMaxParamValNum	(8)	/* 変換（相似orアフィン）パラメータの最大数 */


cv::Mat LoadThumb( char *fname );
void DrawThumb( char *doc_name, char *doc_name_prev, cv::Mat *thumb0, cv::Mat *thumb_draw0, strProjParam param, strProjParam zero_param, cv::Size cap_size, cv::Size res_size );
void DrawCor2( cv::Point corps[][2], int corpsnum, cv::Mat img_cap, char *doc_name );
int GotoSnapshotMode( strDirectShowCap *dsc, unsigned char *img_buff, long buff_size, cv::Mat img_cap, cv::Mat thumb );
int GotoPasteMode( cv::Mat img_cap, cv::Mat thumb, cv::Point *p1, cv::Point *p2 );
void OnMouseThumb( int event, int x, int y, int flags, void *param );
int SaveAR( char *doc_name, cv::Mat img_cap, cv::Size size, cv::Point *p1, cv::Point *p2 );
void DrawPoints2( cv::Point *ps, int num, cv::Size img_size );
void DrawParam( cv::Mat img_cap, cv::Mat thumb, strProjParam param, strProjParam zero_param );
int CaptureMovie( char *movfile );
int DecomposeMovie( char *mov_file );
void Buff2ImageData( unsigned char *buff, cv::Mat img );
void ConvMovie( void );
cv::Mat DrawPointsOverlap( cv::Point *ps, int num, cv::Size *img_size, cv::Mat img );
cv::Mat LoadFlagImage( char *doc_name );
void MakeDispImage( cv::Mat img_disp, cv::Mat disp_cap, cv::Mat disp_con, cv::Point *disp_ps, int disp_num );
cv::Mat GetFlagImage( char *doc_name );
int RetrieveDualHash( void );
