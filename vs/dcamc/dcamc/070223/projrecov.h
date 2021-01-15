#define	kNearParamThr	(1000.0L)
#define	kErrorThrMeanParam	(0.00001)	/* MeanParam�ɂ�����v�Z�ł��؂�̂��߂̃G���[��臒l */
#define	kNumVectorMeanParam	(30)	/* MeanParam�ɂ����鎟��̌v�Z�ɗp����x�N�g���̐� */

// RANSAC�ł̃p�����[�^
#define	kRANSACTry	(100)	/* �p�����[�^�̌v�Z�� */
#define	kRANSACThr	(5.0)	/* �����i�̂Q��j��臒l */
#define	kRANSACMinScore	(10)	/* �ŏ��X�R�A */
#define	kRANSACRefineNum	(100)	/* ��萸�x�̍����p�����[�^�����߂�ۂ̐� */

// CalcProjParamTop�ł̃��[�h
#define	PROJ_NORMAL	(1)
#define	PROJ_REVERSE	(2)
// CalcProjParamTop�ł̐���@
#define	PARAM_IWAMURA	(1)
#define	PARAM_RANSAC	(2)

void RPTMain( char *fname1, char *fname2 );
void RecoverPTTest( IplImage *img1, IplImage *img2, CvPoint *p1, CvPoint *p2 );
void RecovPTSub( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, CvSize size, IplImage *img );
double GetPPVar( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num );
void CalcProjParamTop( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param, int mode, int method );
int GetAppropriateParamRANSAC( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param, int mode );
int EvaluateParamRANSAC( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param, int mode );
void GetRefinedParamRANSAC( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param_src, strProjParam *param_dst, int mode );
