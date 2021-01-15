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

#define	kHarrisRegMag	(1.0/*0.4*/)	/* 登録画像の縮小率 */
#define	kHarrisTestMag	(1.0)
#define	kHarrisShowRegMag	(1.0)	/* ウィンドウに表示する画像の縮小率 */
#define	kHarrisShowCamMag	(1.0)

/* RANSACによる対応点探索 */
#define	kMaxCor	(10000)
#define	kMaxDistFindCor	(10.0)

/* フレーム間対応 */
#if 0
#define	kMaxFramePoints	(10)	/* 特徴点を保存するフレーム数 */
#define	kCompareFrames	(5)		/* 比較するフレーム数 */
#define	kRefineScore	(3)		/* 選別する際のスコア */
#define	kMergeFrames	(3)		/* 最終的な特徴点を統合するフレーム数 */
#endif

#if 1	// 対応を取らない場合
#define	kMaxFramePoints	(1)	/* 特徴点を保存するフレーム数 */
#define	kCompareFrames	(1)		/* 比較するフレーム数 */
#define	kRefineScore	(0)		/* 選別する際のスコア */
#define	kMergeFrames	(1)		/* 最終的な特徴点を統合するフレーム数 */
#endif

/* DetectHarrisCamのモード */
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
