#include <stdio.h>
#include <cv.h>			// OpneCV
#include <highgui.h>	// OpenCV
#include "nimg.h"

int SetPix( IplImage *img, int x, int y, char *val )
// img��(x,y)��val���Z�b�g����
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// �摜�͈̔͂𒴂���
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// �s�N�Z��������̃o�C�g��
	for ( i = 0; i < bpp; i++ )
		img->imageData[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPix( IplImage *img, int x, int y, char *val )
// img��(x,y)��val�ɃZ�b�g����
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// �摜�͈̔͂𒴂���
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// �s�N�Z��������̃o�C�g��
	for ( i = 0; i < bpp; i++ )
		val[i] = img->imageData[ws*y + x*bpp + i];
	return 1;
}

void PutImage( IplImage *src, IplImage *dst, CvPoint origin )
// dst��origin���N�_��src��z�u����
{
	int i, j;
	char val[3];
//	if ( origin.x + src->width > dst->width || origin.y + src->height > dst->height )	return 0;	// �͂ݏo��
	
	for ( j = 0; j < src->height && origin.y + j < dst->height; j++ ) {
		for ( i = 0; i < src->width && origin.x + i < dst->width; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			SetPix( dst, origin.x + i, origin.y + j, val );
		}
	}
}

void CopyImageData( IplImage *src, IplImage *dst )
// src�̉摜�f�[�^��dst�ɃR�s�[����
{
	int i, j;
	char val[3];

	for ( j = 0; j < src->height && j < dst->height; j++ ) {
		for ( i = 0; i < src->width && i < dst->width; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			if ( src->origin == dst->origin )	SetPix( dst, i, j, val );
			else	SetPix( dst, i, dst->height - j - 1, val );	// origin���قȂ�ꍇ
		}
	}
}

int MergeImage( IplImage *img1, IplImage *img2, int mode, IplImage *merge )
// img1��img2��mode(MERGE_HOR:���E�CMERGE_VER:�㉺)�ŕ��ׂ��摜��merge�Ɋi�[����
{
	int i, j, mw, mh;
	char val[3];

//	puts("mms");
	mw = merge->width;
	mh = merge->height;
//	puts("mmf");

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
//			printf("%d, %d\n", i, j);
			memset( val, 0, 3 );
			if ( mode == MERGE_HOR && i >= img1->width )	// ���E�z�u�ŉ摜1�̕����z����
				GetPix( img2, i - img1->width, j, val );
			else if ( mode == MERGE_VER && j >= img1->height )	// �㉺�z�u�ŉ摜1�̍����𒴂���
				GetPix( img2, i, j - img1->height, val );
			else
				GetPix( img1, i, j, val );

			SetPix( merge, i, j, val );
		}
	}
	return 1;
}

void DrawPolygon( IplImage *img, CvPoint *ps, int pnum, CvScalar color, int thickness, int line_type, int shift )
// ���p�`��`�悷��
{
	int i;

	for ( i = 0; i < pnum; i++ ) {
		cvLine( img, ps[i], ps[(i+1)%pnum], color, thickness, line_type, shift );
	}
}

