//#define	CR_INTER
//#define	CR_AREA
//#define	AFFINE
#define	SIMILAR
//#define	INCLUDE_CENTRE
#define	DISC_NUM	(10)
#define	DB_DOCS	(10000)
//#define	COM97
//#define	COM98
//#define	COM87
//#define	COM76
//#define	COM54
#define	COM87

#ifdef	CR_INTER /* Interpoint CR version */
#ifdef	INCLUDE_CENTRE
#define	kNumCom1	(8)
#define	kNumCom2	(35)
#define	kGroup1Num	(8)
#define	kGroup2Num	(7)
#define	kGroup3Num	(4)
#else
#define	kNumCom1	(8)
#define	kNumCom2	(21)
#define	kGroup1Num	(8)
#define	kGroup2Num	(7)
#define	kGroup3Num	(5)
#endif
#endif	/* for CR_INTER */

#ifdef	CR_AREA /* Area CR version */
#ifdef	INCLUDE_CENTRE
#define	kNumCom1	(8)
#define	kNumCom2	(35)
#define	kGroup1Num	(8)
#define	kGroup2Num	(7)
#define	kGroup3Num	(4)

#else	/* Not include centre */

#ifdef	COM76 /* 7-6-5 */
#define	kNumCom1	(7)
#define	kNumCom2	(6)
#define	kGroup1Num	(7)
#define	kGroup2Num	(6)
#define	kGroup3Num	(5)
#endif

#ifdef	COM87 /* 8-7-5 */
#define	kNumCom1	(8)
#define	kNumCom2	(21)
#define	kGroup1Num	(8)
#define	kGroup2Num	(7)
#define	kGroup3Num	(5)
#endif

#ifdef	COM98 /* 9-8-5 */
#define	kNumCom1	(9)
#define	kNumCom2	(56)
#define	kGroup1Num	(9)
#define	kGroup2Num	(8)
#define	kGroup3Num	(5)
#endif

#ifdef	COM97 /* 9-7-5 */
#define	kNumCom1	(36)
#define	kNumCom2	(21)
#define	kGroup1Num	(9)
#define	kGroup2Num	(7)
#define	kGroup3Num	(5)
#endif

#ifdef	COM107 /* 10-7-5 */
#define	kNumCom1	(120)
#define	kNumCom2	(21)
#define	kGroup1Num	(10)
#define	kGroup2Num	(7)
#define	kGroup3Num	(5)
#endif
#endif	/* for INCLUDE_CENTRE */

#endif	/* for CR_AREA */

#ifdef	AFFINE

#ifdef	INCLUDE_CENTRE
#define	kNumCom1	(8)
#define	kNumCom2	(35)
#define	kGroup1Num	(8)
#define	kGroup2Num	(7)
#define	kGroup3Num	(3)

#else
#ifdef	COM76 /* 7-6-4 */
#define	kNumCom1	(7)
#define	kNumCom2	(15)
#define	kGroup1Num	(7)
#define	kGroup2Num	(6)
#define	kGroup3Num	(4)
#endif

#ifdef	COM87	/* 8-7-4 */
#define	kNumCom1	(8)
#define	kNumCom2	(35)
#define	kGroup1Num	(8)
#define	kGroup2Num	(7)
#define	kGroup3Num	(4)
#endif

#ifdef	COM97	/* 9-7-4 */
#define	kNumCom1	(36)
#define	kNumCom2	(35)
#define	kGroup1Num	(9)
#define	kGroup2Num	(7)
#define	kGroup3Num	(4)
#endif

#ifdef	COM98	/* 9-8-4 */
#define	kNumCom1	(9)
#define	kNumCom2	(70)
#define	kGroup1Num	(9)
#define	kGroup2Num	(8)
#define	kGroup3Num	(4)
#endif	/* for COM98 */
#endif	/* for INCLUDE_CENTRE */
#endif	/* for AFFINE */

#ifdef	SIMILAR
#ifdef	COM54
#define	kNumCom1	(5)
#define	kNumCom2	(4)
#define	kGroup1Num	(5)
#define	kGroup2Num	(4)
#define	kGroup3Num	(3)
#endif	/* for COM54 */
#ifdef	COM64
#define	kNumCom1	(15)
#define	kNumCom2	(4)
#define	kGroup1Num	(6)
#define	kGroup2Num	(4)
#define	kGroup3Num	(3)
#endif	/* for COM64 */
#ifdef	COM65
#define	kNumCom1	(6)
#define	kNumCom2	(10)
#define	kGroup1Num	(6)
#define	kGroup2Num	(5)
#define	kGroup3Num	(3)
#endif	/* for COM65*/
#ifdef	COM74
#define	kNumCom1	(35)
#define	kNumCom2	(4)
#define	kGroup1Num	(7)
#define	kGroup2Num	(4)
#define	kGroup3Num	(3)
#endif	/* for COM74 */
#ifdef	COM75
#define	kNumCom1	(21)
#define	kNumCom2	(10)
#define	kGroup1Num	(7)
#define	kGroup2Num	(5)
#define	kGroup3Num	(3)
#endif	/* for COM75 */
#ifdef	COM76
#define	kNumCom1	(7)
#define	kNumCom2	(20)
#define	kGroup1Num	(7)
#define	kGroup2Num	(6)
#define	kGroup3Num	(3)
#endif	/* for COM76 */
#ifdef	COM84
#define	kNumCom1	(70)
#define	kNumCom2	(4)
#define	kGroup1Num	(8)
#define	kGroup2Num	(4)
#define	kGroup3Num	(3)
#endif	/* for COM84 */
#ifdef	COM85
#define	kNumCom1	(56)
#define	kNumCom2	(10)
#define	kGroup1Num	(8)
#define	kGroup2Num	(5)
#define	kGroup3Num	(3)
#endif	/* for COM85 */
#ifdef	COM86
#define	kNumCom1	(28)
#define	kNumCom2	(20)
#define	kGroup1Num	(8)
#define	kGroup2Num	(6)
#define	kGroup3Num	(3)
#endif	/* for COM86 */
#ifdef	COM87
#define	kNumCom1	(8)
#define	kNumCom2	(35)
#define	kGroup1Num	(8)
#define	kGroup2Num	(7)
#define	kGroup3Num	(3)
#endif	/* for COM87 */
#endif	/* for SIMILAR */

typedef struct _strHList {
	struct _strHList *next;
	unsigned short doc;
	unsigned short point;
//	unsigned char pat;
	char idx[kNumCom2];
} strHList;

#define	USE_CON
//#define	USE_ENC
//#define USE_JP

// 8-6のとき
//#define	kNumCom1	(28)
//#define	kNumCom2	(6)
// 10-8のとき
//#define	kNumCom1	(45)
//#define	kNumCom2	(56)
// 9-7のとき
//#define	kNumCom1	(36)
//#define	kNumCom2	(21)

//#define	kMaxNN5CR	(4.0)	/* 5点複比の最大値（最小値は0.0とする） */
//#define	kErrorNN5CR	(0.25)	/* 5点複比での許容誤差．プラスマイナス．ハッシュテーブルのアドレス計算に使う */
//#define	kHashNum	((unsigned int)(kMaxNN5CR / (kErrorNN5CR*2.0)))	/* ハッシュの数字の最大値．Max=4.0,Error=0.25なら8になる */
//#define	kHashDig	(kHashNum+1)	/* ハッシュの桁数字 */
//#define	kCRNum	(5)	/* 複比をいくつ計算するか */
//#if	kCRNum == 1
//#define	kHashCRMax	(kHashDig)	/* １つの複比の最大値 */
//#elif	kCRNum == 5
//#define	kHashCRMax	(kHashDig * kHashDig * kHashDig * kHashDig * kHashDig)	/* ５つの複比での最大値 */
//#endif
//#define	kHashSize	(kHashCRMax * kNumCom2)	/* ハッシュテーブルの大きさ */
//#define	kHashSize	(1024*1024*256)	/* ハッシュテーブルの大きさ */
#define	kHashSize	(1024*1024*128)	/* ハッシュテーブルの大きさ */

#define	kMaxDocNum	(10240)	/* 登録できる最大ドキュメント数 */
#define	kExactDocNum	(10240)	/* 実際に登録されているドキュメント数 */
//#define	kMaxPointNum	(1024)	/* 点の最大数 */
#define	kMaxPointNum	(1024)	/* 点の最大数 */
#define	kMaxLineLen	(102400)
#define	kBlockSize	(1024)

#define	kCheckPPVarNum	(10)

#ifdef USE_JP
#define	kHashSrcPath	"c:\\nakai\\dirs_db\\j\\*.bmp"
#define	kHashSrcDir	"c:\\nakai\\dirs_db\\j\\"
#define	kHashFileName	"c:\\nakai\\vs\\cvtest\\debug\\hash_p_j.dat"
#define	kPFPrefix	"c:\\nakai\\vs\\cvtest\\debug\\point_p_j\\"
#define	kPFSuffix	".dat"
#define	kDBCorFileName	"c:\\nakai\\vs\\cvtest\\debug\\dbcor_j.dat"
#endif

#ifdef USE_ENC
#define	kHashSrcPath	"c:\\nakai\\dirs_db\\01\\bmp\\*.bmp"
#define	kHashSrcDir	"c:\\nakai\\dirs_db\\01\\bmp\\"
#define	kHashFileName	"c:\\nakai\\vs\\cvtest\\debug\\hash_p.dat"
#define	kPFPrefix	"c:\\nakai\\vs\\cvtest\\debug\\point_p\\"
#define	kPFSuffix	".dat"
#define	kDBCorFileName	"c:\\nakai\\vs\\cvtest\\debug\\dbcor.dat"
#endif

#ifdef USE_CON

#if	(DB_DOCS == 10)	/* Location of images for DB */
#define	kHashSrcPath	"/home/nakai/didb/test10/*.bmp"
#define	kHashSrcDir	"/home/nakai/didb/test10/"
#define	kPFPrefix	"/home/nakai/dirs/das/point/10/"
#define	kPFSuffix	".dat"
#define	kDBCorFileName	"/home/nakai/dirs/das/dbcor/10/dbcor.dat"
#elif	(DB_DOCS == 100)
#define	kHashSrcPath	"/home/nakai/didb/test100/*.bmp"
#define	kHashSrcDir	"/home/nakai/didb/test100/"
#define	kPFPrefix	"/home/nakai/dirs/das/point/100/"
#define	kPFSuffix	".dat"
#define	kDBCorFileName	"/home/nakai/dirs/das/dbcor/100/dbcor.dat"
#elif	(DB_DOCS == 1000)
#define	kHashSrcPath	"/home/nakai/didb/test1000/*.bmp"
#define	kHashSrcDir	"/home/nakai/didb/test1000/"
#define	kPFPrefix	"/home/nakai/dirs/das/point/1k/"
#define	kPFSuffix	".dat"
#define	kDBCorFileName	"/home/nakai/dirs/das/dbcor/1k/dbcor.dat"
#elif	(DB_DOCS == 10000)
#define	kHashSrcPath	"/home/nakai/didb/*.bmp"
#define	kHashSrcDir	"/home/nakai/didb/"
#define	kPFPrefix	"/home/nakai/dirs/das/point/10k/"
#define	kPFSuffix	".dat"
#define	kDBCorFileName	"/home/nakai/dirs/das/dbcor/10k/dbcor.dat"
#else
#define	kHashSrcPath	"/home/nakai/didb/*.bmp"
#define	kHashSrcDir	"/home/nakai/didb/"
#define	kPFPrefix	"/home/nakai/dirs/das/point/10k/"
#define	kPFSuffix	".dat"
#define	kDBCorFileName	"/home/nakai/dirs/das/dbcor/10k/dbcor.dat"
#endif	/* for DB_DOCS */

#define	kPNFileName	"/home/nakai/dirs/das/pnum.txt"

#ifdef	CR_AREA
#if		(DB_DOCS == 10)
#define	kHashFileName	"/home/nakai/dirs/das/cr10/hash.dat"
#elif	(DB_DOCS == 100)
#define	kHashFileName	"/home/nakai/dirs/das/cr100/hash.dat"
#elif	(DB_DOCS == 1000)
#define	kHashFileName	"/home/nakai/dirs/das/cr1k/hash.dat"
#elif	(DB_DOCS == 10000)
#define	kHashFileName	"/home/nakai/dirs/das/cr10k/hash.dat"
#endif	/* for DB_DOCS */
#endif	/* for CR_AREA */

#ifdef	AFFINE
#if		(DISC_NUM == 5)
#if		(DB_DOCS == 10)
#define	kHashFileName	"/home/nakai/dirs/das/af5_10/hash.dat"
#elif	(DB_DOCS == 100)
#define	kHashFileName	"/home/nakai/dirs/das/af5_100/hash.dat"
#elif	(DB_DOCS == 1000)
#define	kHashFileName	"/home/nakai/dirs/das/af5_1k/hash.dat"
#elif	(DB_DOCS == 10000)
#define	kHashFileName	"/home/nakai/dirs/das/af5_10k/hash.dat"
#endif	/* for DB_DOCS */

#elif	(DISC_NUM == 10)
#ifdef	COM87
#if		(DB_DOCS == 10)
#define	kHashFileName	"/home/nakai/dirs/das/af10/hash.dat"
#elif	(DB_DOCS == 100)
#define	kHashFileName	"/home/nakai/dirs/das/af100/hash.dat"
#elif	(DB_DOCS == 1000)
#define	kHashFileName	"/home/nakai/dirs/das/af1k/hash.dat"
#elif	(DB_DOCS == 10000)
#define	kHashFileName	"/home/nakai/dirs/das/af10k/hash.dat"
#endif	/* for DB_DOCS */
#endif	/* for COM87 */
#ifdef	COM76
#if		(DB_DOCS == 10)
#define	kHashFileName	"/home/nakai/dirs/das/af764_10/hash.dat"
#elif	(DB_DOCS == 100)
#define	kHashFileName	"/home/nakai/dirs/das/af764_100/hash.dat"
#elif	(DB_DOCS == 1000)
#define	kHashFileName	"/home/nakai/dirs/das/af764_1k/hash.dat"
#elif	(DB_DOCS == 10000)
#define	kHashFileName	"/home/nakai/dirs/das/af764_10k/hash.dat"
#endif	/* for DB_DOCS */
#endif	/* for COM76 */
#endif	/* for DISC_NUM */

#endif	/* for AFINE */

#endif	/* for USE_CON */

#ifdef	CR_AREA
#define	kDiscFileName	"a10disc.txt"
#define	kAdjScoreConst	(0.022851)
#endif

#ifdef	AFFINE
#if		(DISC_NUM == 5)
#define	kDiscFileName	"af5disc.txt"
#define	kAdjScoreConst	(0.014)
#elif	(DISC_NUM == 10)
#define	kDiscFileName	"af10disc.txt"
#define	kAdjScoreConst	(0)
#else
#define	kDiscFileName	"af5disc.txt"
#define	kAdjScoreConst	(0.014)
#endif
#endif


strHList *ReadHash( char *index, int num );
void InitHash( void );
int AddHash( char *index, int num, unsigned short doc, unsigned short point );
int SearchHash( unsigned int index, unsigned int doc );
void PrintHash( void );
void SaveHash( void );
int LoadHash( int num );
int ChkHash( void );

static strHList **hash;
