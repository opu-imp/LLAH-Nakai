#define	NEW_COR

#define	FP_ORIG	(0)
#define	FP_ANNO	(1)

#define	kTempPointFile	"point.dat"	/* �Ή��_�T�����ɍ쐬��������_�̈ꎞ�t�@�C���̃p�X */
#define	kAcr2InvType	's'	/* acr2�ŗp����s�ϗʃ^�C�v�is�ő����s�ϗʁj*/
#define	kDrawCorLineThick	(8)	/* �Ή��_��`�悷��ۂ̐��̑��� */
#define	kMinCorNum	(2)	/* �p�����[�^������s���ŏ��Ή��_�� */

int TransformAnno( IplImage *anno, IplImage *orig, int cluster_count, double *param );
int MakeFeaturePointsFromColorImage( CvPoint **ps, IplImage *img, int mode );
void MakeClusterCor( int cluster_count, double avgsa[][3], double avgso[][3], int *clcor );

int OutPutPoints( char *fname, CvPoint *ps, int num, CvSize size );
int OutPutImage( IplImage *img );

int IsExist( char *fname );

#ifdef	COR_PRESET_MODE
int TransformAnnoPreset( IplImage *anno, IplImage *orig, IplImage *anno2 );
#endif
