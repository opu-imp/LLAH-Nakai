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

#define	kHarrisRegMag	(0.4)	/* �o�^�摜�̏k���� */
#define	kHarrisTestMag	(1.0)

/* RANSAC�ɂ��Ή��_�T�� */
#define	kMaxCor	(10000)
#define	kMaxDistFindCor	(10.0)

/* �t���[���ԑΉ� */
#define	kMaxFramePoints	(10)	/* �����_��ۑ�����t���[���� */
#define	kCompareFrames	(5)		/* ��r����t���[���� */
#define	kRefineScore	(3)		/* �I�ʂ��邳���̃X�R�A */
#define	kMergeFrames	(3)		/* �ŏI�I�ȓ����_�𓝍�����t���[���� */

int DetectHarrisCam( char *fname );
int DetectHarrisTest( char *fname, char *fname_anno );
void DetectHarris( IplImage *src, IplImage *dst );
void DetectGoodFeatures( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum );
void DetectGoodFeaturesReg( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum   );
void FindCorPoint( CvPoint *ps, int num, CvPoint *corps, int cornum, int *pcor, strProjParam *param );
