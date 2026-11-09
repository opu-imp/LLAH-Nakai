#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "nimg.h"

int SetPix( cv::Mat img, int x, int y, char *val )
// imgの(x,y)にvalをセットする
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// 画像の範囲を超える
	ws = (int)img.step[0];
	bpp = (int)img.elemSize();	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		img.data[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPix( cv::Mat img, int x, int y, char *val )
// imgの(x,y)をvalにセットする
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// 画像の範囲を超える
	ws = (int)img.step[0];
	bpp = (int)img.elemSize();	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		val[i] = img.data[ws*y + x*bpp + i];
	return 1;
}

void PutImage( cv::Mat src, cv::Mat dst, cv::Point origin )
// dstのoriginを起点にsrcを配置する
{
	int i, j;
	char val[3];
//	if ( origin.x + src.cols > dst.cols || origin.y + src.rows > dst.rows )	return 0;	// はみ出す
	
	for ( j = 0; j < src.rows && origin.y + j < dst.rows; j++ ) {
		for ( i = 0; i < src.cols && origin.x + i < dst.cols; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			SetPix( dst, origin.x + i, origin.y + j, val );
		}
	}
}

void CopyImageData( cv::Mat src, cv::Mat dst )
// srcの画像データをdstにコピーする
{
	int i, j;
	char val[3];

	for ( j = 0; j < src.rows && j < dst.rows; j++ ) {
		for ( i = 0; i < src.cols && i < dst.cols; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			if ( 0/*src.origin removed,cv::Mat is always top-left*/ == 0/*dst.origin removed,cv::Mat is always top-left*/ )	SetPix( dst, i, j, val );
			else	SetPix( dst, i, dst.rows - j - 1, val );	// originが異なる場合
		}
	}
}

int MergeImage( cv::Mat img1, cv::Mat img2, int mode, cv::Mat merge )
// img1とimg2をmode(MERGE_HOR:左右，MERGE_VER:上下)で並べた画像をmergeに格納する
{
	int i, j, mw, mh;
	char val[3];

//	puts("mms");
	mw = merge.cols;
	mh = merge.rows;
//	puts("mmf");

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
//			printf("%d, %d\n", i, j);
			memset( val, 0, 3 );
			if ( mode == MERGE_HOR && i >= img1.cols )	// 左右配置で画像1の幅を越える
				GetPix( img2, i - img1.cols, j, val );
			else if ( mode == MERGE_VER && j >= img1.rows )	// 上下配置で画像1の高さを超える
				GetPix( img2, i, j - img1.rows, val );
			else
				GetPix( img1, i, j, val );

			SetPix( merge, i, j, val );
		}
	}
	return 1;
}

void DrawPolygon( cv::Mat img, cv::Point *ps, int pnum, cv::Scalar color, int thickness, int line_type, int shift )
// 多角形を描画する
{
	int i;

	for ( i = 0; i < pnum; i++ ) {
		cv::line( img, ps[i], ps[(i+1)%pnum], color, thickness, line_type, shift );
	}
}

