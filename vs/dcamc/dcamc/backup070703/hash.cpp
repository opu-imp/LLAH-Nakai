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

static int count;
extern long allocated_hash;

static strHList **hash;

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

unsigned int CountHashList( int index )
// index番目のリストサイズを得る
{
	unsigned int count;
	strHList *hp;
	for ( count = 0, hp = hash[index]; hp != NULL; count++, hp = hp->next);
	return count;
}

void SaveHash( void )
// ハッシュをセーブする
{
	unsigned int i, j, n;
	strHList *hp;

	StartWriteBlock( eHashFileName, kBlockSize );
	for ( i = 0; i < kHashSize; i++ ) {
		n = CountHashList( i );
		if ( n > 0 ) {
			hp = hash[i];
			WriteBlock( (unsigned char *)&i, sizeof(unsigned int) );
			WriteBlock( (unsigned char *)&n, sizeof(unsigned int) );
			for ( j = 0; j < n; j++, hp = hp->next ) {
				WriteBlock( (unsigned char *)&(hp->doc), sizeof(unsigned short) );
				WriteBlock( (unsigned char *)&(hp->point), sizeof(unsigned short) );
//				WriteBlock( (unsigned char *)&(hp->pat), sizeof(unsigned char) );
				WriteBlock( (unsigned char *)hp->idx, sizeof(char)*eNumCom2 );
			}
		}
	}
	FinishWriteBlock();
}

int LoadHash( int num )
// ハッシュをロードする
{
	unsigned int i, index, n;
	unsigned short doc, point;
//	unsigned char pat;
	char *idx;

	idx = (char *)calloc(eNumCom2, sizeof(char));
	if ( StartReadBlock(eHashFileName, kBlockSize) == 0 )	return 0;
	InitHash();
	while ( ReadBlock((unsigned char *)&index, sizeof(unsigned int)) != 0 ) {
		ReadBlock((unsigned char *)&n, sizeof(unsigned int));
		for ( i = 0; i < n; i++ ) {
			ReadBlock((unsigned char *)&doc, sizeof(unsigned short));
			ReadBlock((unsigned char *)&point, sizeof(unsigned short));
//			ReadBlock((unsigned char *)&pat, sizeof(unsigned char));
			ReadBlock((unsigned char *)idx, sizeof(char)*eNumCom2);
			AddHash(idx, num, doc, point);
		}
	}
	FinishReadBlock();
	free(idx);
	
	return 1;
}

int ChkHash( void )
// ハッシュをチェックする
{
	unsigned long i, ent, total_ent, len, pow_sum;
	strHList *hl;
	
	for ( i = 0, ent = 0, total_ent = 0, pow_sum = 0; i < kHashSize; i++ ) {
//		printf("%d\n", i);
		if ( (hl = hash[i]) != NULL ) {
			ent++;
			for ( len = 0; hl != NULL; hl = hl->next ) {
				total_ent++;
				len++;
			}
			pow_sum += len*len;
		}
	}
	printf("%ld\t%lf\t%lf\t%lf\n", total_ent, (double)total_ent/(double)ent, (double)ent/(double)kHashSize, (double)total_ent/(double)kHashSize);
//	printf("Total entry : %d\n", total_ent);
//	printf("Entry rate : %f\nAverage entry : %f\n", (double)ent/(double)kHashSize, (double)total_ent/(double)ent);
//	printf("Variance of length : %f\n", (double)pow_sum / (double)ent - ((double)total_ent / (double)ent) * ((double)total_ent / (double)ent));
//	printf("%d\n", count);
	return 0;
}


strHList *ReadHash( char *index, int num )
// ハッシュのindex番目を返す
{
	unsigned int hindex;
	hindex = HashFunc( index, num );
	return hash[hindex];
}

int SearchHash( unsigned int index, unsigned int doc )
// ハッシュのindex番目からdocを探す．見つかれば真，見つからなければ偽を返す
{
	strHList *hp;

	for ( hp = hash[index]; hp != NULL; hp = hp->next) {
		if ( hp->doc == doc )	return 1;
	}

	return 0;
}

int AddHash( char *index, int num, unsigned short doc, unsigned short point )
// ハッシュに登録する
{
	unsigned int hindex;
	strHList *hp;

	hindex = HashFunc( index, num );
	hp = (strHList *)malloc(sizeof(*hp));
	hp->idx = (char *)calloc( eNumCom2, sizeof(char) );
	if ( hp == NULL )	return 0;
	hp->next = hash[hindex];
	hp->doc = doc;
	hp->point = point;
//	hp->pat = pat;
//	hp->idx = idx;
	memcpy(hp->idx, index, eNumCom2);
	hash[hindex] = hp;
	count++;
//	allocated_hash += sizeof(*hp);
//	allocated_hash += eNumCom2;
	return 1;
}

void PrintHash( void )
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

void InitHash( void )
// ハッシュを初期化する（解放はしない）
{
	int i;
	count = 0;
	hash = (strHList **)malloc( sizeof(strHList *)*kHashSize );
	for ( i = 0; i < kHashSize; i++ )	hash[i] = NULL;
}
