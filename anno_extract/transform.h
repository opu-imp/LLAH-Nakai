#define	NEW_COR

#define	FP_ORIG	(0)
#define	FP_ANNO	(1)

#define	kTempPointFile	"point.dat"	/* �Ή��_�T�����ɍ쐬��������_�̈ꎞ�t�@�C���̃p�X */
#define	kAcr2InvType	's'	/* acr2�ŗp����s�ϗʃ^�C�v�is�ő����s�ϗʁj*/
#define	kDrawCorLineThick	(8)	/* �Ή��_��`�悷��ۂ̐��̑��� */
#define	kMinCorNum	(2)	/* �p�����[�^������s���ŏ��Ή��_�� */

int TransformAnno( cv::Mat anno, cv::Mat orig, int cluster_count, double *param );
int MakeFeaturePointsFromColorImage( cv::Point **ps, cv::Mat img, int mode );
void MakeClusterCor( int cluster_count, double avgsa[][3], double avgso[][3], int *clcor );

int OutPutPoints( char *fname, cv::Point *ps, int num, cv::Size size );
int OutPutImage( cv::Mat img );

int IsExist( char *fname );

#ifdef	COR_PRESET_MODE
int TransformAnnoPreset( cv::Mat anno, cv::Mat orig, cv::Mat anno2 );
#endif
