#define	DB_DOCS	(10000)

#define	kMaxNumCom1	(252)	/* eNumCom1の最大値 */
#define	kMaxNumCom2	(252)	/* eNumCom2の最大値 */
#define	kMaxGroup1Num	(10)	/* eGroup1Numの最大値 */
#define	kMaxGroup2Num	(10)		/* eGroup2Numの最大値 */
#define	kMaxGroup3Num	(5)		/* eGroup3Numの最大値 */

typedef struct _strHList {
	struct _strHList *next;
	unsigned short doc;
	unsigned short point;
//	unsigned char pat;
	char idx[kMaxNumCom2];
} strHList;

#define	USE_CON
#define	kHashSize	(1024*1024*128-1)	/* ハッシュテーブルの大きさ */

#define	kMaxDocNum	(10240)	/* 登録できる最大ドキュメント数 */
#define	kExactDocNum	(10240)	/* 実際に登録されているドキュメント数 */
//#define	kMaxPointNum	(1024)	/* 点の最大数 */
#define	kMaxPointNum	(2500)	/* 点の最大数 */
#define	kMaxLineLen	(102400)
#define	kBlockSize	(1024)

#define	kCheckPPVarNum	(10)


strHList *ReadHash( char *index, int num );
void InitHash( void );
int AddHash( char *index, int num, unsigned short doc, unsigned short point );
int SearchHash( unsigned int index, unsigned int doc );
void PrintHash( void );
void SaveHash( void );
int LoadHash( int num );
int ChkHash( void );

static strHList **hash;
