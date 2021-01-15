#ifndef _LLAHDOC_HASH_H_
#define _LLAHDOC_HASH_H_

typedef struct _strHList {
	struct _strHList *next;
	unsigned short doc;
	unsigned short point;
//	unsigned char pat;
	char *idx;
} strHList;

typedef struct _strHList2 {
	struct _strHList2 *next;
	unsigned char *dat;
} strHList2;

#define	USE_CON
//#define	USE_ENC
//#define USE_JP


#define	kHashSize	(1024*1024*128-1)	/* ハッシュテーブルの大きさ */
//#define	kHashSize	(1024*1024*1280-1)	/* ハッシュテーブルの大きさ */

#define	kMaxDocNum	(20600)	/* 登録できる最大ドキュメント数 */
//#define	kMaxPointNum	(1024)	/* 点の最大数 */
#define	kMaxPointNum	(2500)	/* 点の最大数 */
#define	kBlockSize	(1024)

#define	kCheckPPVarNum	(10)

#define	kAdjScoreConst	(0.0)	/* 875-disc10以外では不明のため */

#define	kMaxHashCollision	(10)	/* 最大衝突数 */

/* リストなしモードでのハッシュエントリの形式 */
// 帰るときはkFreeEntryとkInvalidEntryも合わせて
typedef	unsigned int	HENTRY;	/* unsigned int: 4バイト */
#define	kFreeEntry		(0xffffffff)	/* 登録なし */
#define	kInvalidEntry	(0xfffffffe)	/* 衝突により無効 */

strHList *ReadHash( char *index, char *index_area, int num, strHList **hash );
strHList2 *ReadHash2( char *index, char *index_area, int num, strHList2 **hash2 );
strHList2 *ReadHash2Area( char *index, char *index_area, int num, strHList2 **hash2 );
HENTRY *ReadHash3( char *index, char *index_area, int num, HENTRY *hash3 );
strHList **InitHash( void );
strHList2 **InitHash2( void );
HENTRY *InitHash3( void );
void ReleaseHash( strHList **hash );
void ReleaseHash2( strHList2 **hash2 );
void ReleaseHash3( HENTRY *hash3 );
int AddHash( char *index, char *index_area, int num, unsigned short doc, unsigned short point, strHList **hash );
int AddHash2( char *index, char *index_area, int num, unsigned short doc, unsigned short point, strHList2 **hash2 );
int AddHashArea( char *index, char *index_area, int num, unsigned short doc, unsigned short point, strHList2 **hash2 );
int AddHash3( char *index, char *index_area, int num, int doc, int point, HENTRY *hash3 );
int SearchHash( unsigned int index, unsigned int doc, strHList **hash );
void PrintHash( strHList **hash );
void PrintHash2( strHList2 **hash2 );
int SaveHash( strHList **hash );
int SaveHash2( strHList2 **hash2 );
int SaveHash3( HENTRY *hash3 );
strHList **LoadHash( int num );
strHList2 **LoadHash2( int num );
HENTRY *LoadHash3( int num );
int ChkHash( strHList **hash );
int ChkHash2( strHList2 **hash2 );
int ChkHash3( HENTRY *hash3 );
void GetMinHindex( char **hindex_array, char *hindex );
int RefineHash( strHList **hash );
int RefineHash2( strHList2 **hash2 );
HENTRY MakeHashEntry( int doc, int point );
void ReadHashEntry( HENTRY *phe, int *p_doc, int *p_point );

#endif