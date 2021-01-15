//#define	ANNEX_OUTPUT_IMAGE
////#define	COR_PRESET_MODE
//#define	OUTPUT_COR_IMAGE

//#define	CHK_COR

#define	cWhite	CV_RGB( 255,255,255 )
#define	cBlack	CV_RGB( 0,0,0 )
#define	cRed	CV_RGB( 255,0,0 )
#define	cGreen	CV_RGB( 0,255,0 )
#define	cBlue	CV_RGB( 0,0,255 )
#define	cRandom	CV_RGB( rand()%256, rand()%256, rand()%256 )

#define	kOrigNum	(3)	/* origの番号 */
#define	kAnnoNum	(3)	/* annoの番号 */

// 不変量タイプ
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

// デフォルトのパラメータ
#define	kDefaultGroup1Num	(5)
#define	kDefaultGroup2Num	(5)
#define	kDefaultGroup3Num	(3)
#define	kDefaultInvType	SIMILAR
#define	kDefaultDiscNum	(50)
#define	kDefaultClusters	(5)
#define	kDefaultResizeMode	(0)
//#define	kDefaultOutPutDir	"out/"
#define	kDefaultOutPutDir	"./"
#define	kDefaultDiscFileName	"disc.txt"
#define	kAcrPath	"acr2/acr2"
#define	kDefaultOrigMode	(0)
#define	kDefaultTransMode	SIMILAR

#define	kDefaultResizeMainOrig	(0.5)
#define	kDefaultResizeMainAnno	(0.5)
#define	kDefaultOrigErodeIterMain	(15)
#define	kDefaultOrigGaussParamMain	(7)
#define	kDefaultAnnoGaussParamMain	(3)
#define	kDefaultBinThrOrig	(100)
#define	kDefaultBinThrScan	(80)
#define	kDefaultClosingIter	(10)
#define	kDefaultMinAreaMain	(30)
#define	kDefaultDilMaskIter	(5)

#define	kDefaultAcr2Path	"acr2/acr2"

#define	kDefaultClScale	(0.5)
#define	kDefaultErodeClIter	(1)
#define	kDefaultClCmpStep	(4)
#define	kDefaultKMeansMaxIter	(10)
#define	kDefaultKMeansEpsilon	(1.0)
#define	kDefaultMinAreaCl	(20)
//#define	kDefaultMaxAreaCl	(1000)
#define	kDefaultMaxAreaCl	(10000000000)	// 大きすぎるものを除かないバージョン

#define	kDefaultDiffNear	(4)
#define	kDefaultDiffLeaveThr	(200000)
#define	kDefaultDiffEraseThr	(20000)

#define	kInitFileName	"annex.ini"

#define	kResizeScale	(0.5)

#define	kMaxParamValNum	(8)	/* 変換（相似orアフィン）パラメータの最大数 */

int FindFromDefaultDir( char *fname, char *init_fn, char *out_fn );
int InitAnnex( int argc, char *argv[], int *clcptr );
void MaskImage( IplImage *img, IplImage *mask, IplImage *dst );
