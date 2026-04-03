#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "nimg.h"

int SetPix( cv::Mat img, int x, int y, char *val )
// img��(x,y)��val���Z�b�g����
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// �摜�͈̔͂𒴂���
	ws = (int)img.step[0];
	bpp = (int)img.elemSize();	// �s�N�Z��������̃o�C�g��
	for ( i = 0; i < bpp; i++ )
		img.data[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPix( cv::Mat img, int x, int y, char *val )
// img��(x,y)��val�ɃZ�b�g����
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// �摜�͈̔͂𒴂���
	ws = (int)img.step[0];
	bpp = (int)img.elemSize();	// �s�N�Z��������̃o�C�g��
	for ( i = 0; i < bpp; i++ )
		val[i] = img.data[ws*y + x*bpp + i];
	return 1;
}

int SetPixU( cv::Mat img, int x, int y, unsigned char *val )
// img��(x,y)��val���Z�b�g����(unsigned)
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// �摜�͈̔͂𒴂���
	ws = (int)img.step[0];
	bpp = (int)img.elemSize();	// �s�N�Z��������̃o�C�g��
	for ( i = 0; i < bpp; i++ )
		img.data[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPixU( cv::Mat img, int x, int y, unsigned char *val )
// img��(x,y)��val�ɃZ�b�g����(unsigned)
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// �摜�͈̔͂𒴂���
	ws = (int)img.step[0];
	bpp = (int)img.elemSize();	// �s�N�Z��������̃o�C�g��
	for ( i = 0; i < bpp; i++ )
		val[i] = img.data[ws*y + x*bpp + i];
	return 1;
}

void PutImage( cv::Mat src, cv::Mat dst, cv::Point origin )
// dst��origin���N�_��src��z�u����
{
	int i, j;
	char val[3];
//	if ( origin.x + src.cols > dst.cols || origin.y + src.rows > dst.rows )	return 0;	// �͂ݏo��
	
	for ( j = 0; j < src.rows && origin.y + j < dst.rows; j++ ) {
		for ( i = 0; i < src.cols && origin.x + i < dst.cols; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			SetPix( dst, origin.x + i, origin.y + j, val );
		}
	}
}

void CopyImageData( cv::Mat src, cv::Mat dst )
// src�̉摜�f�[�^��dst�ɃR�s�[����
{
	int i, j;
	char val[3];

	for ( j = 0; j < src.rows && j < dst.rows; j++ ) {
		for ( i = 0; i < src.cols && i < dst.cols; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			if ( 0/*src.origin removed,cv::Mat is always top-left*/ == 0/*dst.origin removed,cv::Mat is always top-left*/ )	SetPix( dst, i, j, val );
			else	SetPix( dst, i, dst.rows - j - 1, val );	// origin���قȂ�ꍇ
		}
	}
}

int MergeImage( cv::Mat img1, cv::Mat img2, int mode, cv::Mat merge )
// img1��img2��mode(MERGE_HOR:���E�CMERGE_VER:�㉺)�ŕ��ׂ��摜��merge�Ɋi�[����
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge.cols;
	mh = merge.rows;
	if ( mode == MERGE_HOR ) {	// ���E�ɕ��ׂ郂�[�h�ŁC
		if ( mw > (img1.cols + img2.cols) )	// ���E�ɕ��ׂ�������蕝���傫���Ƃ�
			orig_x = (int)((double)(mw - (img1.cols+img2.cols)) / 2.0L);	// �]���̔�����orig_x�ɂ���
		if ( mh > std::max(img1.rows, img2.rows) )	// �傫���ق��̍�����荂�����傫���Ƃ�
			orig_y = (int)((double)(mh - std::max(img1.rows, img2.rows)) / 2.0L);
	} else if ( mode == MERGE_VER ) {	// �㉺�ɕ��ׂ郂�[�h�ŁC
		if ( mw > std::max(img1.cols, img2.cols) )	// �傫���ق��̕������S�̂̕����傫���Ƃ�
			orig_x = (int)((double)(mw - std::max(img1.cols, img2.cols)) / 2.0L);
		if ( mh > (img1.rows + img2.rows) )	// �㉺�ɕ��ׂ��������S�̂̒������傫���Ƃ�
			orig_y = (int)((double)(mh - (img1.rows+img2.rows)) / 2.0L);
	}

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
//			printf("%d, %d\n", i, j);
			memset( val, 0, 3 );
			if ( mode == MERGE_HOR && i >= img1.cols )	// ���E�z�u�ŉ摜1�̕����z����
				GetPix( img2, i - img1.cols, j, val );
			else if ( mode == MERGE_VER && j >= img1.rows )	// �㉺�z�u�ŉ摜1�̍����𒴂���
				GetPix( img2, i, j - img1.rows, val );
			else
				GetPix( img1, i, j, val );

			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
	return 1;
}

int MergeImage3( cv::Mat img1, cv::Mat img2, cv::Mat img3, cv::Mat merge )
// img1��img2��img3����ׂ��摜��merge�Ɋi�[����
// CVPR�̃r�f�I�p
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge.cols;
	mh = merge.rows;
	if ( mw > (img1.cols + img2.cols) )	// ���E�ɕ��ׂ�������蕝���傫���Ƃ�
		orig_x = (int)((double)(mw - (img1.cols+img2.cols)) / 2.0L);	// �]���̔�����orig_x�ɂ���
	if ( mh > std::max(img1.rows, img2.rows) )	// �傫���ق��̍�����荂�����傫���Ƃ�
		orig_y = (int)((double)(mh - std::max(img1.rows, img2.rows)) / 2.0L);

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
			memset( val, 0, 3 );
			if ( i >= img1.cols ) { // �摜1�̕����z����
				if ( j >= img2.rows )	// �摜2�̍������z����
					GetPix( img3, i - img1.cols, j - img2.rows, val );
				else
					GetPix( img2, i - img1.cols, j, val );
			} else {
				GetPix( img1, i, j, val );
			}
			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
//	cv::rectangle( merge, cv::Point( orig_x + img1.cols, orig_y + img2.rows ), cv::Point( orig_x + img1.cols + img3.cols - 1, orig_y + img2.rows + img3.rows - 1 ), cv::Scalar(0,0,0), 1, cv::LINE_AA, 0 );
	cv::line( merge, cv::Point( orig_x + img1.cols, orig_y + img2.rows ), cv::Point( merge.cols, orig_y + img2.rows ), cv::Scalar(0,0,0), 1, 8, 0 );
	return 1;
}

void DrawPolygon( cv::Mat img, cv::Point *ps, int pnum, cv::Scalar color, int thickness, int line_type, int shift )
// ���p�`��`�悷��
{
	int i;

	for ( i = 0; i < pnum; i++ ) {
		cv::line( img, ps[i], ps[(i+1)%pnum], color, thickness, line_type, shift );
	}
}

int CropImage( cv::Mat src, cv::Mat dst, cv::Point pos )
// pos���N�_�Ƃ���src����dst�֐؂�o�����s���D�[����������͍��Ŗ��߂�
{
	int i, j;
	char val[3];

	for ( j = 0; j < dst.rows; j++ ) {
		for ( i = 0; i < dst.cols; i++ ) {
			memset( val, 0, 3 );	// �N���A
			GetPix( src, pos.x + i, pos.y + j, val );
			SetPix( dst, i, j, val );
		}
	}
	return 1;
}
