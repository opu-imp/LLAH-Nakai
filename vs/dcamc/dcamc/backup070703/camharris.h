typedef struct _strHarrisCamParam {
	int QualityLevel;
	int MinDistance;
	int BlockSize;
	int UseHarris;
	int HarrisParamK;
} strHarrisCamParam;

typedef struct _strHarrisRegParam {
	int QualityLevelReg;
	int MinDistanceReg;
	int BlockSizeReg;
	int UseHarrisReg;
	int HarrisParamKReg;
} strHarrisRegParam;

#define	kDefaultQualityLevel	(9)
#define	kDefaultMinDistance		(10)
#define	kDefaultBlockSize		(0)
#define	kDefaultUseHarris		(1)
#define	kDefaultHarrisParamK	(4)

#define	kQualityLevelMax	(100)
#define	kMinDistanceMax		(100)
#define	kBlockSizeMax		(20)
#define	kUseHarrisMax		(1)
#define	kHarrisParamKMax	(10)

#define	kDefaultQualityLevelReg	(9)
#define	kDefaultMinDistanceReg		(10)
#define	kDefaultBlockSizeReg		(0)
#define	kDefaultUseHarrisReg		(1)
#define	kDefaultHarrisParamKReg	(4)

#define	kQualityLevelRegMax	(100)
#define	kMinDistanceRegMax		(100)
#define	kBlockSizeRegMax		(20)
#define	kUseHarrisRegMax		(1)
#define	kHarrisParamKRegMax	(10)

#define	kHarrisRegMag	(1.0/*0.4*/)	/* �o�^�摜�̏k���� */
#define	kHarrisTestMag	(1.0)
#define	kHarrisShowRegMag	(1.0)	/* �E�B���h�E�ɕ\������摜�̏k���� */
#define	kHarrisShowCamMag	(1.0)

/* RANSAC�ɂ��Ή��_�T�� */
#define	kMaxCor	(10000)
#define	kMaxDistFindCor	(10.0)

/* �t���[���ԑΉ� */
#if 0
#define	kMaxFramePoints	(10)	/* �����_��ۑ�����t���[���� */
#define	kCompareFrames	(5)		/* ��r����t���[���� */
#define	kRefineScore	(3)		/* �I�ʂ���ۂ̃X�R�A */
#define	kMergeFrames	(3)		/* �ŏI�I�ȓ����_�𓝍�����t���[���� */
#endif

#if 1	// �Ή������Ȃ��ꍇ
#define	kMaxFramePoints	(1)	/* �����_��ۑ�����t���[���� */
#define	kCompareFrames	(1)		/* ��r����t���[���� */
#define	kRefineScore	(0)		/* �I�ʂ���ۂ̃X�R�A */
#define	kMergeFrames	(1)		/* �ŏI�I�ȓ����_�𓝍�����t���[���� */
#endif

/* DetectHarrisCam�̃��[�h */
#define	DHC_HARRIS	(1)
#define	DHC_ENG		(2)
#define	DHC_JP		(3)

int DetectHarrisCam( char *fname, int mode );
int DetectHarrisTest( char *fname, char *fname_anno );
void DetectHarris( IplImage *src, IplImage *dst );
void DetectGoodFeatures( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum );
void DetectGoodFeaturesReg( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum   );
void FindCorPoint( CvPoint *ps, int num, CvPoint *corps, int cornum, int *pcor, strProjParam *param );
void DetectEngDocPoints( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum );
void DetectEngDocPointsReg( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum  );
void DetectJpDocPoints( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum );
void DetectJpDocPointsReg( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum  );
