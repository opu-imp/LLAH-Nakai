#include <stdio.h>
#include <stdlib.h>
#include "block.h"

unsigned char *block;
int ptr;
int max;
FILE *fp;

void StartWriteBlock( char *fname, int bsize )
// ブロックの書き込みを開始する
{
	block = (unsigned char *)malloc(bsize);
	ptr = 0;
	max = bsize;
	fp = fopen(fname, "wb");
}

void WriteBlock( unsigned char *dat, int size )
// ブロックに書き込む
{
	int i;

	for ( i = 0; i < size; i++ ) {
		if ( ptr >= max ) {
			fwrite( block, 1, max, fp );
			ptr = 0;
		}
		*(block+ptr) = *(dat+i);
		ptr++;
	}
}

void FinishWriteBlock( void )
// ブロックへの書き込みを終了する
{
	if ( ptr > 0 ) {
		fwrite( block, 1, ptr, fp );
	}
	free(block);
	ptr = 0;
	max = 0;
	fclose(fp);
}

int StartReadBlock( char *fname, int bsize )
// ブロックからの読み込みを開始する
{
	block = (unsigned char *)malloc(bsize);
	ptr = 0;
	max = bsize;
	if ( ( fp = fopen(fname, "rb") ) == NULL )	return 0;
	max = fread( block, 1, max, fp );
	return 1;
}

int ReadBlock( unsigned char *dat, int size )
// ブロックに読み込む．読み込みが成功すれば1，失敗すれば（ファイルの終わりに到達すれば）0を返す．
{
	int i;

	for ( i = 0; i < size; i++ ) {
		if ( ptr >= max ) {
			if ( ( max = fread( block, 1, max, fp ) ) == 0 )	return 0;
			ptr = 0;
		}
		*(dat+i) = *(block+ptr);
		ptr++;
	}

	return 1;
}

void FinishReadBlock( void )
// ブロックからの読み込みを終了する
{
	free(block);
	ptr = 0;
	max = 0;
	fclose(fp);
}
