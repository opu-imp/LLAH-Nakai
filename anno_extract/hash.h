typedef struct _strHList {
	struct _strHList *next;
	unsigned short doc;
	unsigned short point;
//	unsigned char pat;
	char *idx;
} strHList;

#define	USE_CON
//#define	USE_ENC
//#define USE_JP


#define	kHashSize	(1024*1024*128-1)	/* ハッシュテーブルの大きさ */

#define	kMaxDocNum	(10240)	/* 登録できる最大ドキュメント数 */
//#define	kMaxPointNum	(1024)	/* 点の最大数 */
#define	kMaxPointNum	(10000)	/* 点の最大数 */
#define	kBlockSize	(1024)

#define	kCheckPPVarNum	(10)

#define	kAdjScoreConst	(0.0)	/* 875-disc10以外では不明のため */

strHList *ReadHash( char *index, int num );
void InitHash( void );
int AddHash( char *index, int num, unsigned short doc, unsigned short point );
int SearchHash( unsigned int index, unsigned int doc );
void PrintHash( void );
void SaveHash( void );
int LoadHash( int num );
int ChkHash( void );
