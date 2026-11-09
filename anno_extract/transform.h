#define	NEW_COR

#define	FP_ORIG	(0)
#define	FP_ANNO	(1)

#define	kTempPointFile	"point.dat"	/* 対応点探索時に作成する特徴点の一時ファイルのパス */
#define	kAcr2InvType	's'	/* acr2で用いる不変量タイプ（sで相似不変量）*/
#define	kDrawCorLineThick	(8)	/* 対応点を描画する際の線の太さ */
#define	kMinCorNum	(2)	/* パラメータ推定を行う最小対応点数 */

int TransformAnno( cv::Mat anno, cv::Mat orig, int cluster_count, double *param );
int MakeFeaturePointsFromColorImage( cv::Point **ps, cv::Mat img, int mode );
void MakeClusterCor( int cluster_count, double avgsa[][3], double avgso[][3], int *clcor );

int OutPutPoints( char *fname, cv::Point *ps, int num, cv::Size size );
int OutPutImage( cv::Mat img );

int IsExist( char *fname );

#ifdef	COR_PRESET_MODE
int TransformAnnoPreset( cv::Mat anno, cv::Mat orig, cv::Mat anno2 );
#endif
