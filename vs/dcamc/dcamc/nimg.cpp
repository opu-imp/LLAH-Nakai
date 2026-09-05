#include <stdio.h>
#include <opencv2/opencv.hpp>			// OpneCV
// #include <highgui.h> // now in opencv2/opencv.hpp	// OpenCV
#include "nimg.h"

int SetPix( cv::Mat img, int x, int y, char *val )
// imgの(x,y)にvalをセットする
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// 画像の範囲を超える
	ws = img.step[0];
	bpp = (int)((img.depth() & 0x0ff) / 8) * img.channels();	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		img.data[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPix( cv::Mat img, int x, int y, char *val )
// imgの(x,y)をvalにセットする
{
	int i, ws, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;	// 画像の範囲を超える
	ws = img.step[0];
	bpp = (int)((img.depth() & 0x0ff) / 8) * img.channels();	// ピクセルあたりのバイト数
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
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge.cols;
	mh = merge.rows;
	if ( mode == MERGE_HOR ) {	// 左右に並べるモードで，
		if ( mw > (img1.cols + img2.cols) )	// 左右に並べた長さより幅が大きいとき
			orig_x = (int)((double)(mw - (img1.cols+img2.cols)) / 2.0L);	// 余白の半分をorig_xにする
		if ( mh > (std::max)(img1.rows, img2.rows) )	// 大きいほうの高さより高さが大きいとき
			orig_y = (int)((double)(mh - (std::max)(img1.rows, img2.rows)) / 2.0L);
	} else if ( mode == MERGE_VER ) {	// 上下に並べるモードで，
		if ( mw > (std::max)(img1.cols, img2.cols) )	// 大きいほうの幅よりも全体の幅が大きいとき
			orig_x = (int)((double)(mw - (std::max)(img1.cols, img2.cols)) / 2.0L);
		if ( mh > (img1.rows + img2.rows) )	// 上下に並べた長さより全体の長さが大きいとき
			orig_y = (int)((double)(mh - (img1.rows+img2.rows)) / 2.0L);
	}

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

			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
	return 1;
}

int MergeImage3( cv::Mat img1, cv::Mat img2, cv::Mat img3, cv::Mat merge )
// img1とimg2とimg3を並べた画像をmergeに格納する
// CVPRのビデオ用
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge.cols;
	mh = merge.rows;
	if ( mw > (img1.cols + img2.cols) )	// 左右に並べた長さより幅が大きいとき
		orig_x = (int)((double)(mw - (img1.cols+img2.cols)) / 2.0L);	// 余白の半分をorig_xにする
	if ( mh > (std::max)(img1.rows, img2.rows) )	// 大きいほうの高さより高さが大きいとき
		orig_y = (int)((double)(mh - (std::max)(img1.rows, img2.rows)) / 2.0L);

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
			memset( val, 0, 3 );
			if ( i >= img1.cols ) { // 画像1の幅を越える
				if ( j >= img2.rows )	// 画像2の高さを越える
					GetPix( img3, i - img1.cols, j - img2.rows, val );
				else
					GetPix( img2, i - img1.cols, j, val );
			} else {
				GetPix( img1, i, j, val );
			}
			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
//	cv::rectangle( merge, cv::Point( orig_x + img1.cols, orig_y + img2.rows ), cv::Point( orig_x + img1.cols + img3.cols - 1, orig_y + img2.rows + img3.rows - 1 ), CV_RGB(0,0,0), 1, cv::LINE_AA, 0 );
	cv::line( merge, cv::Point( orig_x + img1.cols, orig_y + img2.rows ), cv::Point( merge.cols, orig_y + img2.rows ), CV_RGB(0,0,0), 1, 8, 0 );
	return 1;
}

int MergeImage4( cv::Mat img1, cv::Mat img2, cv::Mat img3, cv::Mat img4, cv::Mat merge )
// img1とimg2とimg3とimg4を並べた画像をmergeに格納する
// PRMU0902（多言語）のビデオ用
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge.cols;
	mh = merge.rows;
	if ( mw > (img1.cols + img2.cols) )	// 左右に並べた長さより幅が大きいとき
		orig_x = (int)((double)(mw - (img1.cols+img2.cols)) / 2.0L);	// 余白の半分をorig_xにする
	if ( mh > (std::max)(img1.rows, img2.rows) )	// 大きいほうの高さより高さが大きいとき
		orig_y = (int)((double)(mh - (std::max)(img1.rows, img2.rows)) / 2.0L);

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
			memset( val, 0, 3 );
			if ( i >= img1.cols ) { // 画像1の幅を越える
				if ( j >= img2.rows + img3.rows )	// 画像2の高さ＋画像3の高さを超える
					GetPix( img4, i - img1.cols, j - img2.rows - img3.rows, val );	// 画像4のピクセルを見る
				else if ( j >= img2.rows )	// 画像2の高さを越える
					GetPix( img3, i - img1.cols, j - img2.rows, val );	// 画像3のピクセルを見る
				else
					GetPix( img2, i - img1.cols, j, val );	// 画像2のピクセルを見る
			} else {
				GetPix( img1, i, j, val );	// 画像1のピクセルを見る
			}
			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
//	cv::rectangle( merge, cv::Point( orig_x + img1.cols, orig_y + img2.rows ), cv::Point( orig_x + img1.cols + img3.cols - 1, orig_y + img2.rows + img3.rows - 1 ), CV_RGB(0,0,0), 1, cv::LINE_AA, 0 );
	cv::line( merge, cv::Point( orig_x + img1.cols, orig_y + img2.rows ), cv::Point( merge.cols, orig_y + img2.rows ), CV_RGB(0,0,0), 1, 8, 0 );
	cv::line( merge, cv::Point( orig_x + img1.cols, orig_y + img2.rows + img3.rows ), cv::Point( merge.cols, orig_y + img2.rows + img3.rows ), CV_RGB(0,0,0), 1, 8, 0 );

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

int CropImage( cv::Mat src, cv::Mat dst, cv::Point pos )
// posを起点としてsrcからdstへ切り出しを行う．端から向こうは黒で埋める
{
	int i, j;
	char val[3];

	for ( j = 0; j < dst.rows; j++ ) {
		for ( i = 0; i < dst.cols; i++ ) {
			memset( val, 0, 3 );	// クリア
			GetPix( src, pos.x + i, pos.y + j, val );
			SetPix( dst, i, j, val );
		}
	}
	return 1;
}
