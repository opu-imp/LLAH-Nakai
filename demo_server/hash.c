#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "def_general.h"
//#include "cv.h"
//#include "highgui.h"
#include "hash.h"
//#include "dirs.h"
#include "hist.h"
#include "block.h"
#include "extern.h"
#include "hlist2.h"

static int count;
extern long allocated_hash;

//static strHList **hash;
//static strHList2 **hash2;
//static HENTRY *hash3 = NULL;

unsigned int HashFunc( char *index, int num)
{
	int i;
	unsigned int ret = 0;

	for ( i = 0; i < eNumCom2; i++ ) {
		ret *= num;
		ret += index[i];
		ret %= kHashSize;
	}
	return ret;
}

unsigned int HashFuncArea( char *index, char *index_area, int num)
{
	int i;
	unsigned int ret = 0;

	for ( i = 0; i < eGroup2Num; i++ ) {
		ret *= eGroup2Num;
		ret += index_area[i];
		ret %= kHashSize;
	}
	for ( i = 0; i < eNumCom2; i++ ) {
		ret *= num;
		ret += index[i];
		ret %= kHashSize;
	}
	return ret;
}

unsigned int CountHashList( int index, strHList **hash )
// index番目のリストサイズを得る
{
	unsigned int count;
	strHList *hp;
	for ( count = 0, hp = hash[index]; hp != NULL; count++, hp = hp->next);
	return count;
}

unsigned int CountHashList2( int index, strHList2 **hash2 )
// index番目のリストサイズを得る
{
	unsigned int count;
	strHList2 *hp;
	for ( count = 0, hp = hash2[index]; hp != NULL; count++, hp = hp->next);
	return count;
}

int SaveHash( strHList **hash, const char *hash_dat_file_name )
// ハッシュをセーブする
{
	unsigned int i, j, n, ret;
	strHList *hp;
	char hash_fname[kMaxPathLen];

	sprintf( hash_fname, "%s%s", eDirsDir, hash_dat_file_name );
//	StartWriteBlock( eHashFileName, kBlockSize );
	ret = StartWriteBlock( hash_fname, kBlockSize );	// 公開に向けて改変
	if ( ret <= 0 )	return 0;
	for ( i = 0; i < kHashSize; i++ ) {
//		fprintf( stderr, "(%d/%d)\n", i+1, kHashSize );
		n = CountHashList( i, hash );
		if ( n > 0 ) {
			hp = hash[i];
			WriteBlock( (unsigned char *)&i, sizeof(unsigned int) );
			WriteBlock( (unsigned char *)&n, sizeof(unsigned int) );
			for ( j = 0; j < n; j++, hp = hp->next ) {
				WriteBlock( (unsigned char *)&(hp->doc), sizeof(unsigned short) );
				WriteBlock( (unsigned char *)&(hp->point), sizeof(unsigned short) );
//				WriteBlock( (unsigned char *)&(hp->pat), sizeof(unsigned char) );
				if ( eUseArea )	WriteBlock( (unsigned char *)hp->idx, sizeof(char)*(eNumCom2+eGroup2Num) );	// 面積も記録
				else			WriteBlock( (unsigned char *)hp->idx, sizeof(char)*eNumCom2 );
			}
		}
	}
	FinishWriteBlock();
	return 1;
}

int SaveHash2( strHList2 **hash2, const char *hash_dat_file_name )
// ハッシュをセーブする
{
	unsigned int i, j, k, n, ret;
	strHList2 *hp;
	char hash_fname[kMaxPathLen];

	sprintf( hash_fname, "%s%s", eDirsDir, hash_dat_file_name );
	ret = StartWriteBlock( hash_fname, kBlockSize );	// 公開に向けて改変
	if ( ret <= 0 )	return 0;
	for ( i = 0; i < kHashSize; i++ ) {
//		fprintf( stderr, "(%d/%d)\n", i+1, kHashSize );
		n = CountHashList2( i, hash2 );
		if ( n > 0 ) {
			hp = hash2[i];
			WriteBlock( (unsigned char *)&i, sizeof(unsigned int) );
			WriteBlock( (unsigned char *)&n, sizeof(unsigned int) );
			for ( j = 0; j < n; j++, hp = hp->next ) {
				WriteBlock( hp->dat, eHList2DatByte );
			}
		}
	}
	FinishWriteBlock();
	return 1;
}

int SaveHash3( HENTRY *hash3, const char *hash_dat_file_name )
// ハッシュをセーブする（ハッシュリストなしバージョン）
{
	unsigned int i, ret;
	char hash_fname[kMaxPathLen];

	sprintf( hash_fname, "%s%s", eDirsDir, hash_dat_file_name );
	ret = StartWriteBlock( hash_fname, kBlockSize );	// 公開に向けて改変
	if ( ret <= 0 )	return 0;
	for ( i = 0; i < kHashSize; i++ ) {
		if ( hash3[i] != kFreeEntry && hash3[i] != kInvalidEntry ) {	// 登録有り
			WriteBlock( (unsigned char *)&i, sizeof(unsigned int) );
			WriteBlock( (unsigned char *)&(hash3[i]), sizeof(HENTRY) );
		}
	}
	FinishWriteBlock();
	return 1;
}

strHList **LoadHash( int num, const char *hash_dat_file_name )
// ハッシュをロードする
{
	unsigned int i, index, n;
	unsigned short doc, point;
//	unsigned char pat;
	char *idx;
	strHList **hash = NULL;
	char hash_fname[kMaxPathLen];

	if ( eUseArea )	idx = (char *)calloc(eNumCom2+eGroup2Num, sizeof(char));	// インデックス
	else			idx = (char *)calloc(eNumCom2, sizeof(char));	// インデックス
	sprintf( hash_fname, "%s%s", eDirsDir, hash_dat_file_name );
	if ( StartReadBlock(hash_fname, kBlockSize) == 0 )	return NULL;
	hash = InitHash();
	if ( hash == NULL )	return NULL;	// 確保に失敗
	while ( ReadBlock((unsigned char *)&index, sizeof(unsigned int)) != 0 ) {
		ReadBlock((unsigned char *)&n, sizeof(unsigned int));
		for ( i = 0; i < n; i++ ) {
			ReadBlock((unsigned char *)&doc, sizeof(unsigned short));
			ReadBlock((unsigned char *)&point, sizeof(unsigned short));
//			ReadBlock((unsigned char *)&pat, sizeof(unsigned char));
			if ( eUseArea ) {
				ReadBlock((unsigned char *)idx, sizeof(char)*(eGroup2Num + eNumCom2));
				AddHash(idx, idx + eNumCom2, num, doc, point, hash);
			} else {
				ReadBlock((unsigned char *)idx, sizeof(char)*eNumCom2);
				AddHash(idx, NULL, num, doc, point, hash);
			}
		}
	}
	FinishReadBlock();
	free(idx);
	
	return hash;
}

strHList2 **LoadHash2( int num, const char *hash_dat_file_name )
// ハッシュをロードする2
{
	unsigned int i, index, n;
	unsigned long doc, point;
	char *idx;
	unsigned char *dat;
	strHList2 *hp, **hash2;
	char hash_fname[kMaxPathLen];

	sprintf( hash_fname, "%s%s", eDirsDir, hash_dat_file_name );
	if ( StartReadBlock(hash_fname, kBlockSize) == 0 )	return NULL;
	hash2 = InitHash2();
	if ( hash2 == NULL )	return NULL;	// 確保に失敗
	while ( ReadBlock((unsigned char *)&index, sizeof(unsigned int)) != 0 ) {
		ReadBlock((unsigned char *)&n, sizeof(unsigned int));
		for ( i = 0; i < n; i++ ) {
			hp = (strHList2 *)malloc(sizeof(*hp));
			hp->dat = (unsigned char *)calloc( eHList2DatByte, sizeof(unsigned char) );
			ReadBlock( hp->dat, eHList2DatByte );
			hp->next = hash2[index];
			hash2[index] = hp;

			count++;
			allocated_hash += sizeof(*hp);
			allocated_hash += sizeof(unsigned char) * eHList2DatByte;
		}
	}
	/*
	idx = (char *)calloc(eNumCom2, sizeof(char));	// インデックス
	dat = (unsigned char *)calloc( eHList2DatByte, sizeof(unsigned char) );

	if ( StartReadBlock(eHashFileName, kBlockSize) == 0 )	return 0;
	InitHash2();
	while ( ReadBlock((unsigned char *)&index, sizeof(unsigned int)) != 0 ) {
		ReadBlock((unsigned char *)&n, sizeof(unsigned int));
		for ( i = 0; i < n; i++ ) {
			ReadBlock( dat, eHList2DatByte );
			ReadHList2Dat( dat, &doc, &point, idx );
			AddHash2( idx, num, doc, point );
		}
	}
	*/
	FinishReadBlock();
//	free(idx);
//	free(dat);
	
	return hash2;
}

HENTRY *LoadHash3( int num, const char *hash_dat_file_name )
// ハッシュをロードする3（リストなしモード）
{
	unsigned int i, index;
	int doc, point;
	HENTRY he, *hash3;
	char hash_fname[kMaxPathLen];

	sprintf( hash_fname, "%s%s", eDirsDir, hash_dat_file_name );
	if ( StartReadBlock(eHashFileName, kBlockSize) == 0 )	return NULL;
	hash3 = InitHash3();
	if ( hash3 == NULL )	return NULL;	// 確保に失敗
	while ( ReadBlock((unsigned char *)&index, sizeof(unsigned int)) != 0 ) {
		ReadBlock( (unsigned char *)&he, sizeof(HENTRY) );
		hash3[index] = he;
		count++;
	}
	FinishReadBlock();
	
	return hash3;
}

int ChkHash( strHList **hash )
// ハッシュをチェックする
{
	unsigned long i, ent, total_ent, len, pow_sum;
	unsigned long total_one_num = 0, total_two_num = 0, total_two_len = 0;
	strHList *hl;
	
	for ( i = 0, ent = 0, total_ent = 0, pow_sum = 0; i < kHashSize; i++ ) {
//		printf("%d\n", i);
		if ( (hl = hash[i]) != NULL ) {	// リスト長が1以上
			ent++;
			for ( len = 0; hl != NULL; hl = hl->next ) {	// リストを全部調べる
				total_ent++;
				len++;
			}
			pow_sum += len*len;
			if ( len == 1 )	{
				total_one_num++;
			}
			else {
				total_two_num++;
				total_two_len += len;
			}
		}
	}
	printf("%ld\t%lf\t%lf\t%lf\n", total_ent, (double)total_ent/(double)ent, (double)ent/(double)kHashSize, (double)total_ent/(double)kHashSize);
	printf("Hash size: %d\n# of one: %d\n# of more than one: %d\nTotal length of more than two: %d\n", kHashSize, total_one_num, total_two_num, total_two_len );
//	printf("Total entry : %d\n", total_ent);
//	printf("Entry rate : %f\nAverage entry : %f\n", (double)ent/(double)kHashSize, (double)total_ent/(double)ent);
//	printf("Variance of length : %f\n", (double)pow_sum / (double)ent - ((double)total_ent / (double)ent) * ((double)total_ent / (double)ent));
//	printf("%d\n", count);
	return 0;
}

int ChkHash2( strHList2 **hash2 )
// ハッシュをチェックする（圧縮バージョン）
{
	unsigned long i, ent, total_ent, len, pow_sum;
	unsigned long total_one_num = 0, total_two_num = 0, total_two_len = 0;
	strHList2 *hl;
	
	for ( i = 0, ent = 0, total_ent = 0, pow_sum = 0; i < kHashSize; i++ ) {
//		printf("%d\n", i);
		if ( (hl = hash2[i]) != NULL ) {	// リスト長が1以上
			ent++;
			for ( len = 0; hl != NULL; hl = hl->next ) {	// リストを全部調べる
				total_ent++;
				len++;
			}
			pow_sum += len*len;
			if ( len == 1 )	{
				total_one_num++;
			}
			else {
				total_two_num++;
				total_two_len += len;
			}
		}
	}
	printf("%ld\t%lf\t%lf\t%lf\n", total_ent, (double)total_ent/(double)ent, (double)ent/(double)kHashSize, (double)total_ent/(double)kHashSize);
	printf("Hash size: %d\n# of one: %d\n# of more than one: %d\nTotal length of more than two: %d\n", kHashSize, total_one_num, total_two_num, total_two_len );
//	printf("Total entry : %d\n", total_ent);
//	printf("Entry rate : %f\nAverage entry : %f\n", (double)ent/(double)kHashSize, (double)total_ent/(double)ent);
//	printf("Variance of length : %f\n", (double)pow_sum / (double)ent - ((double)total_ent / (double)ent) * ((double)total_ent / (double)ent));
//	printf("%d\n", count);
	return 0;
}

int ChkHash3( HENTRY *hash3 )
// ハッシュをチェックする（リストなしバージョン）
{
	unsigned long i, ent = 0, total_ent = 0;
	unsigned long total_free_num = 0, total_col_num = 0, total_valid_num = 0;

	for ( i = 0; i < kHashSize; i++ ) {
//		printf("%d\n", i);
		switch ( hash3[i] ) {
			case kFreeEntry:
				total_free_num++;
				break;
			case kInvalidEntry:
				ent++;
				total_col_num++;
				break;
			default:
				total_valid_num++;
				break;
		}
	}
	printf("%ld\t%lf\t%lf\t%lf\n", total_ent, (double)total_ent/(double)ent, (double)ent/(double)kHashSize, (double)total_ent/(double)kHashSize);
	printf("Hash size: %d\n# of one: %d\n# of more than one: %d\nTotal length of more than two: %d\n", kHashSize, total_ent, 0, 0 );
//	printf("Total entry : %d\n", total_ent);
//	printf("Entry rate : %f\nAverage entry : %f\n", (double)ent/(double)kHashSize, (double)total_ent/(double)ent);
//	printf("Variance of length : %f\n", (double)pow_sum / (double)ent - ((double)total_ent / (double)ent) * ((double)total_ent / (double)ent));
//	printf("%d\n", count);
	return 0;
}

strHList *ReadHash( char *index, char *index_area, int num, strHList **hash )
// ハッシュのindex番目を返す
{
	unsigned int hindex;
	if ( eUseArea )	hindex = HashFuncArea( index, index_area, num );
	else			hindex = HashFunc( index, num );
	return hash[hindex];
}

strHList2 *ReadHash2( char *index, char *index_area, int num, strHList2 **hash2 )
// ハッシュのindex番目を返す
{
	unsigned int hindex;
	if ( eUseArea )	hindex = HashFuncArea( index, index_area, num );
	else			hindex = HashFunc( index, num );
	return hash2[hindex];
}

HENTRY *ReadHash3( char *index, char *index_area, int num, HENTRY *hash3 )
// ハッシュのindex番目を返す（ハッシュリストなしバージョン）
{
	unsigned int hindex;
	if ( eUseArea )	hindex = HashFuncArea( index, index_area, num );
	else			hindex = HashFunc( index, num );
	return &(hash3[hindex]);
}


strHList2 *ReadHash2Area( char *index, char *index_area, int num, strHList2 **hash2 )
// ハッシュのindex番目を返す（面積バージョン）
{
	unsigned int hindex;
	hindex = HashFuncArea( index, index_area, num );
	return hash2[hindex];
}

int SearchHash( unsigned int index, unsigned int doc, strHList **hash )
// ハッシュのindex番目からdocを探す．見つかれば真，見つからなければ偽を返す
{
	strHList *hp;

	for ( hp = hash[index]; hp != NULL; hp = hp->next) {
		if ( hp->doc == doc )	return 1;
	}

	return 0;
}

int AddHash( char *index, char *index_area, int num, unsigned short doc, unsigned short point, strHList **hash )
// ハッシュに登録する
// 07/07/05 面積特徴量の記録を追加
{
	unsigned int hindex;
	strHList *hp;

	if ( eUseArea )	hindex = HashFuncArea( index, index_area, num );	// 面積特徴量の利用
	else			hindex = HashFunc( index, num );
	hp = (strHList *)malloc(sizeof(*hp));
	if ( eUseArea )	hp->idx = (char *)calloc( eNumCom2 + eGroup2Num, sizeof(char) );
	else			hp->idx = (char *)calloc( eNumCom2, sizeof(char) );
	if ( hp == NULL )	return 0;
	hp->next = hash[hindex];
	hp->doc = doc;
	hp->point = point;
//	hp->pat = pat;
//	hp->idx = idx;
	if ( eUseArea )	{
		memcpy(hp->idx, index, eNumCom2);
		memcpy(hp->idx + eNumCom2, index_area, eGroup2Num);
	} else {
		memcpy(hp->idx, index, eNumCom2);	// 面積特徴量は記録しない
	}
	hash[hindex] = hp;
	count++;
	allocated_hash += sizeof(*hp);
	if ( eUseArea )	allocated_hash += (eNumCom2 + eGroup2Num) * sizeof(char);
	else			allocated_hash += eNumCom2 * sizeof(char);

	return 1;
}

int AddHash2( char *index, char *index_area, int num, unsigned short doc, unsigned short point, strHList2 **hash2 )
// ハッシュに登録する2（圧縮モード）
{
	unsigned int hindex;
	strHList2 *hp;

	if ( eUseArea )	hindex = HashFuncArea( index, index_area, num );	// 面積特徴量の利用
	else			hindex = HashFunc( index, num );
	hp = (strHList2 *)malloc(sizeof(*hp));
	if ( hp == NULL )	return 0;
	hp->dat = MakeHList2Dat( doc, point, index, index_area );
	if ( hp->dat == NULL )	return 0;
	hp->next = hash2[hindex];
	hash2[hindex] = hp;
	count++;
	allocated_hash += sizeof(*hp);
	allocated_hash += sizeof(unsigned char) * eHList2DatByte;

	return 1;
}

HENTRY MakeHashEntry( int doc, int point )
// ハッシュのエントリをビット操作で作成
{
	return ((HENTRY)doc) << ePointBit | ((HENTRY)point);
}

void ReadHashEntry( HENTRY *phe, int *p_doc, int *p_point )
// ハッシュのエントリからビット操作でIDを取得
{
	*p_point = (int)(*phe & ((1 << ePointBit) - 1));
	*p_doc = (int)((*phe >> ePointBit) & ((1 << eDocBit) - 1));
}

int AddHash3( char *index, char *index_area, int num, int doc, int point, HENTRY *hash3 )
// ハッシュに登録する3（ハッシュリストなしバージョン）
{
	unsigned int hindex;

	if ( eUseArea )	hindex = HashFuncArea( index, index_area, num );	// 面積特徴量の利用
	else			hindex = HashFunc( index, num );
	switch ( hash3[hindex] ) {
		case kInvalidEntry:	// 衝突により登録不可
			break;
		case kFreeEntry:	// 空き
			hash3[hindex] = MakeHashEntry( doc, point );
			break;
		default:	// 登録済み（衝突発生）
			hash3[hindex] = kInvalidEntry;
			break;
	}

	return 1;
}

void PrintHash( strHList **hash )
// ハッシュのデータを画面に出力する
{
	unsigned int i;
	int j;
	strHList *hp;

	for ( i = 0; i < kHashSize; i++ ) {
		hp = hash[i];
		if ( hash[i] != NULL ) {
            printf("%d : ", i);
			do {
				printf("%d ", hp->doc);
				for ( j = 0; j < eNumCom2; j++ ) printf("%02x", hp->idx[j]);
				printf(", ");
				hp = hp->next;
			} while ( hp != NULL );
		printf("\n");
		}
	}
}

void PrintHash2( strHList2 **hash2 )
// ハッシュのデータを画面に出力する
{
	unsigned int i;
	int j;
	strHList2 *hp;
	char *r, *o;
	unsigned long doc, point;

	r = (char *)calloc( eNumCom2, sizeof(char) );
	o = (char *)calloc( eGroup2Num, sizeof(char) );

	for ( i = 0; i < kHashSize; i++ ) {
		hp = hash2[i];
		if ( hash2[i] != NULL ) {
            printf("%d : ", i);
			do {
				ReadHList2Dat( hp->dat, &doc, &point, r, o );
				printf("%d %d ", doc, point);
				for ( j = 0; j < eNumCom2; j++ ) printf("%02x", r[j]);
				if ( eUseArea ) {
					for ( j = 0; j < eGroup2Num; j++ )	printf("%02x", o[j]);
				}
				printf(", ");
				hp = hp->next;
			} while ( hp != NULL );
		printf("\n");
		}
	}
	free( r );
	free( o );
}

strHList **InitHash( void )
// ハッシュを確保1
{
	int i;
	strHList **hash = NULL;

	count = 0;
	hash = (strHList **)malloc( sizeof(strHList *)*kHashSize );
	if ( hash == NULL )	return NULL;
	for ( i = 0; i < kHashSize; i++ )	hash[i] = NULL;
	return hash;
}

void ReleaseHash( strHList **hash )
// ハッシュを解放1
{
	int i;
	strHList *hp, *next;

	if ( hash == NULL )	return;
	for ( i = 0; i < kHashSize; i++ ) {
		for ( hp = hash[i]; hp != NULL; hp = next ) {
			next = hp->next;
			free( hp->idx );
			free( hp );
		}
	}
	free( hash );
}

strHList2 **InitHash2( void )
// ハッシュを確保2
{
	int i;
	strHList2 **hash2 = NULL;

	count = 0;
	hash2 = (strHList2 **)malloc( sizeof(strHList2 *)*kHashSize );
	if ( hash2 == NULL )	return NULL;
	for ( i = 0; i < kHashSize; i++ )	hash2[i] = NULL;
	return hash2;
}

void ReleaseHash2( strHList2 **hash2 )
// ハッシュを解放2
{
	int i;
	strHList2 *hp, *next;

	if ( hash2 == NULL )	return;
	for ( i = 0; i < kHashSize; i++ ) {
		for ( hp = hash2[i]; hp != NULL; hp = next ) {
			next = hp->next;
			free( hp->dat );
			free( hp );
		}
	}
	free( hash2 );
}

HENTRY *InitHash3( void )
// リストなしモードでハッシュを初期化する
{
	int i;
	HENTRY *hash3 = NULL;

	if ( hash3 != NULL )	free( hash3 );
	hash3 = (HENTRY *)calloc( kHashSize, sizeof(HENTRY) );
	if ( hash3 == NULL )	return NULL;
	for ( i = 0; i < kHashSize; i++ )	hash3[i] = kFreeEntry;

	return hash3;
}

void ReleaseHash3( HENTRY *hash3 )
// ハッシュを解放3
{
	if ( hash3 != NULL )	free( hash3 );
	return;
}

void GetMinHindex( char **hindex_array, char *hindex )
// 最小のハッシュインデックスを求める
{
	int i, j, min = 0;

	for ( i = 1; i < eGroup2Num; i++ ) {
		for ( j = 0; j < eNumCom2; j++ ) {
			if ( hindex_array[min][j] < hindex_array[i][j] ) {
				break;
			} else if ( hindex_array[min][j] > hindex_array[i][j] ) {
				min = i;
				break;
			}
		}
	}
	for ( i = 0; i < eNumCom2; i++ )
		hindex[i] = hindex_array[min][i];
}

int RefineHash( strHList **hash )
// 衝突の多い項目を除去する（非圧縮バージョン）
{
	unsigned long i, len, removed, remain;
	strHList *hl, *next_hl;
	
	for ( i = 0, removed = 0, remain = 0; i < kHashSize; i++ ) {
		if ( (hl = hash[i]) != NULL ) {	// 1つ以上登録されている
			// 衝突の数をカウント
			for ( len = 0; hl != NULL; hl = hl->next ) {
				len++;
			}
			if ( len > eMaxHashCollision ) {	// 衝突が最大数を超過
				for ( hl = hash[i]; hl != NULL; ) {
					next_hl = hl->next;
					free( hl->idx );
					free( hl );
					hl = next_hl;
				}
				hash[i] = NULL;
				removed++;
			}
			else {
				remain++;
			}
		}
	}
	printf( "%d items removed.\n", removed );
	printf( "%d items remain.\n", remain );

	return 0;
}

int RefineHash2( strHList2 **hash2 )
// 衝突の多い項目を除去する（圧縮バージョン）
{
	unsigned long i, len, removed, remain;
	strHList2 *hl, *next_hl;
	
	for ( i = 0, removed = 0, remain = 0; i < kHashSize; i++ ) {
		if ( (hl = hash2[i]) != NULL ) {	// 1つ以上登録されている
			// 衝突の数をカウント
			for ( len = 0; hl != NULL; hl = hl->next ) {
				len++;
			}
			if ( len > eMaxHashCollision ) {	// 衝突が最大数を超過
				for ( hl = hash2[i]; hl != NULL; ) {
					next_hl = hl->next;
					free( hl->dat );
					free( hl );
					hl = next_hl;
				}
				hash2[i] = NULL;
				removed++;
			}
			else {
				remain++;
			}
		}
	}
	printf( "%d items removed.\n", removed );
	printf( "%d items remain.\n", remain );

	return 0;
}

int AddHashArea( char *index, char *index_area, int num, unsigned short doc, unsigned short point, strHList2 **hash2 )
// ハッシュに登録する（面積バージョン）
{
	unsigned int hindex;
	strHList2 *hp;

	hindex = HashFuncArea( index, index_area, num );
	hp = (strHList2 *)malloc(sizeof(*hp));
	if ( hp == NULL )	return 0;
	hp->dat = MakeHList2Dat( doc, point, index, index_area );
	if ( hp->dat == NULL )	return 0;
	hp->next = hash2[hindex];
	hash2[hindex] = hp;
	count++;
	allocated_hash += sizeof(*hp);
	allocated_hash += sizeof(unsigned char) * eHList2DatByte;

	return 1;
}
