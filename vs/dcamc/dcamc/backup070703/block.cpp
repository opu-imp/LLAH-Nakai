#include <stdio.h>
#include <stdlib.h>
#include "block.h"

unsigned char *block;
int ptr;
int max;
FILE *fp;

void StartWriteBlock( char *fname, int bsize )
// �u���b�N�̏������݂��J�n����
{
	block = (unsigned char *)malloc(bsize);
	ptr = 0;
	max = bsize;
	fp = fopen(fname, "wb");
}

void WriteBlock( unsigned char *dat, int size )
// �u���b�N�ɏ�������
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
// �u���b�N�ւ̏������݂��I������
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
// �u���b�N����̓ǂݍ��݂��J�n����
{
	block = (unsigned char *)malloc(bsize);
	ptr = 0;
	max = bsize;
	if ( ( fp = fopen(fname, "rb") ) == NULL )	return 0;
	max = fread( block, 1, max, fp );
	return 1;
}

int ReadBlock( unsigned char *dat, int size )
// �u���b�N�ɓǂݍ��ށD�ǂݍ��݂����������1�C���s����΁i�t�@�C���̏I���ɓ��B����΁j0��Ԃ��D
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
// �u���b�N����̓ǂݍ��݂��I������
{
	free(block);
	ptr = 0;
	max = 0;
	fclose(fp);
}
