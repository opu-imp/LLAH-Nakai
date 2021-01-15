#include <stdio.h>
#include <cv.h>			// OpneCV
#include <highgui.h>	// OpenCV
#include "nimg.h"

int SetPix( IplImage *img, int x, int y, char *val )
// imgの(x,y)にvalをセットする
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// 画像の範囲を超える
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		img->imageData[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPix( IplImage *img, int x, int y, char *val )
// imgの(x,y)をvalにセットする
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// 画像の範囲を超える
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		val[i] = img->imageData[ws*y + x*bpp + i];
	return 1;
}

int SetPixU( IplImage *img, int x, int y, unsigned char *val )
// imgの(x,y)にvalをセットする(unsigned)
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// 画像の範囲を超える
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		img->imageData[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPixU( IplImage *img, int x, int y, unsigned char *val )
// imgの(x,y)をvalにセットする(unsigned)
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// 画像の範囲を超える
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		val[i] = img->imageData[ws*y + x*bpp + i];
	return 1;
}

void PutImage( IplImage *src, IplImage *dst, CvPoint origin )
// dstのoriginを起点にsrcを配置する
{
	int i, j;
	char val[3];
//	if ( origin.x + src->width > dst->width || origin.y + src->height > dst->height )	return 0;	// はみ出す
	
	for ( j = 0; j < src->height && origin.y + j < dst->height; j++ ) {
		for ( i = 0; i < src->width && origin.x + i < dst->width; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			SetPix( dst, origin.x + i, origin.y + j, val );
		}
	}
}

void CopyImageData( IplImage *src, IplImage *dst )
// srcの画像データをdstにコピーする
{
	int i, j;
	char val[3];

	for ( j = 0; j < src->height && j < dst->height; j++ ) {
		for ( i = 0; i < src->width && i < dst->width; i++ ) {
			memset( val, 0, 3 );
			GetPix( src, i, j, val );
			if ( src->origin == dst->origin )	SetPix( dst, i, j, val );
			else	SetPix( dst, i, dst->height - j - 1, val );	// originが異なる場合
		}
	}
}

int MergeImage( IplImage *img1, IplImage *img2, int mode, IplImage *merge )
// img1とimg2をmode(MERGE_HOR:左右，MERGE_VER:上下)で並べた画像をmergeに格納する
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge->width;
	mh = merge->height;
	if ( mode == MERGE_HOR ) {	// 左右に並べるモードで，
		if ( mw > (img1->width + img2->width) )	// 左右に並べた長さより幅が大きいとき
			orig_x = (int)((double)(mw - (img1->width+img2->width)) / 2.0L);	// 余白の半分をorig_xにする
		if ( mh > max(img1->height, img2->height) )	// 大きいほうの高さより高さが大きいとき
			orig_y = (int)((double)(mh - max(img1->height, img2->height)) / 2.0L);
	} else if ( mode == MERGE_VER ) {	// 上下に並べるモードで，
		if ( mw > max(img1->width, img2->width) )	// 大きいほうの幅よりも全体の幅が大きいとき
			orig_x = (int)((double)(mw - max(img1->width, img2->width)) / 2.0L);
		if ( mh > (img1->height + img2->height) )	// 上下に並べた長さより全体の長さが大きいとき
			orig_y = (int)((double)(mh - (img1->height+img2->height)) / 2.0L);
	}

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
//			printf("%d, %d\n", i, j);
			memset( val, 0, 3 );
			if ( mode == MERGE_HOR && i >= img1->width )	// 左右配置で画像1の幅を越える
				GetPix( img2, i - img1->width, j, val );
			else if ( mode == MERGE_VER && j >= img1->height )	// 上下配置で画像1の高さを超える
				GetPix( img2, i, j - img1->height, val );
			else
				GetPix( img1, i, j, val );

			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
	return 1;
}

int MergeImage3( IplImage *img1, IplImage *img2, IplImage *img3, IplImage *merge )
// img1とimg2とimg3を並べた画像をmergeに格納する
// CVPRのビデオ用
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge->width;
	mh = merge->height;
	if ( mw > (img1->width + img2->width) )	// 左右に並べた長さより幅が大きいとき
		orig_x = (int)((double)(mw - (img1->width+img2->width)) / 2.0L);	// 余白の半分をorig_xにする
	if ( mh > max(img1->height, img2->height) )	// 大きいほうの高さより高さが大きいとき
		orig_y = (int)((double)(mh - max(img1->height, img2->height)) / 2.0L);

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
			memset( val, 0, 3 );
			if ( i >= img1->width ) { // 画像1の幅を越える
				if ( j >= img2->height )	// 画像2の高さを越える
					GetPix( img3, i - img1->width, j - img2->height, val );
				else
					GetPix( img2, i - img1->width, j, val );
			} else {
				GetPix( img1, i, j, val );
			}
			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
//	cvRectangle( merge, cvPoint( orig_x + img1->width, orig_y + img2->height ), cvPoint( orig_x + img1->width + img3->width - 1, orig_y + img2->height + img3->height - 1 ), CV_RGB(0,0,0), 1, CV_AA, 0 );
	cvLine( merge, cvPoint( orig_x + img1->width, orig_y + img2->height ), cvPoint( merge->width, orig_y + img2->height ), CV_RGB(0,0,0), 1, 8, 0 );
	return 1;
}

void DrawPolygon( IplImage *img, CvPoint *ps, int pnum, CvScalar color, int thickness, int line_type, int shift )
// 多角形を描画する
{
	int i;

	for ( i = 0; i < pnum; i++ ) {
		cvLine( img, ps[i], ps[(i+1)%pnum], color, thickness, line_type, shift );
	}
}

int CropImage( IplImage *src, IplImage *dst, CvPoint pos )
// posを起点としてsrcからdstへ切り出しを行う．端から向こうは黒で埋める
{
	int i, j;
	char val[3];

	for ( j = 0; j < dst->height; j++ ) {
		for ( i = 0; i < dst->width; i++ ) {
			memset( val, 0, 3 );	// クリア
			GetPix( src, pos.x + i, pos.y + j, val );
			SetPix( dst, i, j, val );
		}
	}
	return 1;
}
