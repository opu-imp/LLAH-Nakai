#include "def_general.h"

#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "extern.h"
#include "dirs.h"
#include "auto_connect.h"
#include "hist.h"

#define RET_MODE
//#define DIC_MODE
//#define	MP_MODE
#define	DRAW_POINT

IplImage *GetConnectedImage( char *fname, int mode )
// fnameから結合画像を作成する
{
	int i/*, num*/, chsize;
	double d00;
	IplImage *img, *bin, *inv, *smt;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
//	CvPoint *ps;
	strHist hist;
	IplImage *pt;

	if ( (img= cvLoadImage(fname, 0)) == NULL ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return NULL;
	}
//	OutPutImage( img );
	if ( mode == RET_MP_MODE ) {
		bin = cvCreateImage( cvSize(img->width*kMPExp, img->height*kMPExp), 8, 1 );
		inv = cvCreateImage( cvSize(img->width*kMPExp, img->height*kMPExp), 8, 1 );
		smt = cvCreateImage( cvSize(img->width*kMPExp, img->height*kMPExp), 8, 1 );
	} else {
		bin = cvCreateImage( cvSize(img->width, img->height), 8, 1 );
		inv = cvCreateImage( cvSize(img->width, img->height), 8, 1 );
		smt = cvCreateImage( cvSize(img->width, img->height), 8, 1 );
		pt = cvCreateImage( cvSize(img->width, img->height), 8, 1 );
	}

	if ( mode == RETRIEVE_MODE ) {	// 検索モード：とりあえず2値化
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cvNot(bin, inv);	// CBDAR以後に追加したノイズ除去処理
//		OutPutImage( inv );
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
		for( con0 = contours; con0 != 0; con0 = con0->h_next ) {		// 連結成分を描画
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < 10 ) continue;	// 小さすぎる連結成分は除外
#ifdef	WIN
			cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8, cvPoint(0,0) );	// replace CV_FILLED with 1 to see the outlines 
#else
			cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8);	// replace CV_FILLED with 1 to see the outlines 

#endif
	    }
		cvClearSeq( contours );
//		OutPutImage( smt );
		cvNot(smt, bin);
	} else if ( mode == RET_MP_MODE ) {	// 携帯電話付属カメラモード：リサイズしてから2値化
		cvResize( img, smt, CV_INTER_CUBIC );
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
	} else {	// ハッシュ構築モード（電子文書）：連結成分を取り出して文字サイズを調べてから文字を太らせる
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cvNot( bin, inv );
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
		InitHist( &hist, 100, 0, 1000 );
	    for( con0 = contours; con0 != 0 ; con0 = con0->h_next )
	    {
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
			AddDataHist( &hist, sqrt(d00) );
	    }
		chsize = (int)GetMaxBin(&hist);
		ReleaseHist( &hist );
		chsize = (int)(chsize*0.1);
		chsize += (chsize + 1) % 2;
		cvSmooth( img, smt, CV_GAUSSIAN, chsize, 0, 0 );
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 201, 10 );
//		OutPutImage( bin );
		cvClearSeq( contours );
//		cvReleaseMemStorage( &storage );
	}
	cvNot( bin, inv );
	// 文字の大きさを調べる
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
	// 連結成分を描画・重心を計算
	InitHist( &hist, 100, 0, 1000 );
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
		AddDataHist( &hist, sqrt(d00) );
    }

	chsize = (int)GetMaxBin(&hist);
	ReleaseHist( &hist );
	if ( mode == CONST_HASH_MODE || mode == ADD_HASH_MODE ) chsize *= 2;
	else if ( mode == RET_MP_MODE )	chsize *= 3;
//	chsize /= 2;
	chsize += (chsize + 1) % 2;
	cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0 );
//	OutPutImage( smt );
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cvNot( bin, smt );

#if 0
	cvFindContours( smt, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		CvPoint p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外
		p.x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		p.y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
		cvCircle( pt, p, 8, cWhite, -1, 1, 0 );
		i++;
    }
	OutPutImage( pt );
#endif

	cvReleaseImage( &bin );
	cvReleaseImage( &inv );
	cvReleaseImage( &img );
	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

//	OutPutImage( smt );
	return smt;
}

int AutoConnectTest( char *fname, CvSize *size )
// 単語の自動結合テスト
{
	int i, num, chsize;
	IplImage *img, *bin, *inv, *smt, *pt;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	CvPoint p;
	strHist hist;
//	CvScalar color;

	if ( (img= cvLoadImage(fname, 0)) == NULL ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return 0;
	}

	size->width = img->width;
	size->height = img->height;
#ifdef MP_MODE
	size->width *= 9;
	size->height *= 9;
#endif
	bin = cvCreateImage( cvSize(size->width, size->height), 8, 1 );
	inv = cvCreateImage( cvSize(size->width, size->height), 8, 1 );
	smt = cvCreateImage( cvSize(size->width, size->height), 8, 1 );
	pt = cvCreateImage( cvSize(size->width, size->height), 8, 1 );

#ifdef RET_MODE
	cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
#endif
#ifdef DIC_MODE
	cvNot( img, inv );
//	OutPutImage( inv );
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
	InitHist( &hist, 100, 0, 1000 );
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
		AddDataHist( &hist, sqrt(d00) );
		i++;
    }
	chsize = (int)GetMaxBin(&hist);
	ReleaseHist( &hist );
	chsize = (int)chsize*0.1;
	chsize += (chsize + 1) % 2;
	cvSmooth( img, smt, CV_GAUSSIAN, chsize, 0, 0 );
//	OutPutImage( smt );
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 201, 10 );
#endif
#ifdef MP_MODE
	cvResize( img, smt, CV_INTER_CUBIC );
//	OutPutImage( smt );
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );

#endif
//	OutPutImage( bin );
	cvNot( bin, inv );
//	OutPutImage( inv );

	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する

	// 点の数をカウント（暫定）
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	num = i;

	// 連結成分を描画・重心を計算
	InitHist( &hist, 100, 0, 1000 );
//	InitHist( &hist, 200, 0, 20 );
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;

		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
//		printf("%f\n", d00);
		AddDataHist( &hist, sqrt(d00) );
		i++;
    }
	num = i;	// 連結成分数の更新

//	printf("%d\n", num);
//	printf("%f\n", GetMaxBin(&hist));
	OutPutHist( &hist, 200, 100 );
	chsize = (int)GetMaxBin(&hist);
#ifdef DIC_MODE
	chsize *= 2;
#endif
#ifdef MP_MODE
	chsize *= 3;
#endif
	chsize += (chsize + 1) % 2;
	cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0 );
//	OutPutImage( smt );
//	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 201, 5 );
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cvNot( bin, inv );
#ifdef DRAW_POINT
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
//		CvScalar color;

		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外

		p.x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		p.y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);

		cvCircle( pt, p, 8, cWhite, -1, 1, 0 );
		i++;
    }

//	OutPutImage( pt );
#endif
	return num;
}

IplImage *GetConnectedImageCam( IplImage *src )
{
	int i;
	IplImage *gray, *bin, *inv, *point;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;

	// 画像の作成
	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
//	point = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// 開始位置を合わせる（ないと上下反転する）
//	gray->origin = src->origin;
//	bin->origin = src->origin;
//	inv->origin = src->origin;
//	point->origin = src->origin;
	// グレイスケールに変換
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// 適応二値化
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
//	cvNot( bin, inv );
	cvSmooth( bin, gray, CV_GAUSSIAN, 7, 0, 0 );
//	cvSaveImage( "smooth.bmp", gray );
	cvThreshold( gray, bin, 200, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );
	// 連結成分の重心を計算
	/*
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		CvPoint p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外
		p.x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		p.y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
		cvCircle( point, p, 3, CV_RGB(255, 255, 255), -1, 1, 0 );
		i++;
    }
*/
	
//	cvThreshold( gray, bin, 100, CV_THRESH_BINARY, CV_THRESH_BINARY );
//	cvSmooth( src, bin, CV_GAUSSIAN, 3, 0, 0 );
//	cvSaveImage( "bin.bmp", bin);
	
	cvReleaseImage( &gray );
	cvReleaseImage( &bin );

	return inv;
}

IplImage *GetConnectedImageCamJp( IplImage *src )
{
	IplImage *gray, *bin, *smt, *inv;

	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	smt = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );


	// グレイスケールに変換
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// 適応二値化
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
	// ガウシアン
	cvSmooth( bin, smt, CV_GAUSSIAN, eJpGParam*2+1, 0, 0 );
	// 二値化
	cvThreshold( smt, bin, eJpThr, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );

	cvReleaseImage( &gray );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

	return inv;
}

IplImage *GetConnectedImageCamJp2( IplImage *src )
{
	IplImage *gray, *bin, *inv;

	// 画像の作成
	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// グレイスケールに変換
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// 適応二値化
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 12 );
//	cvNot( bin, inv );
	cvSmooth( bin, gray, CV_GAUSSIAN, 3, 0, 0 );
//	cvSaveImage( "smooth.bmp", gray );
	cvThreshold( gray, bin, 200, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );
	
	cvReleaseImage( &gray );
	cvReleaseImage( &bin );

	return inv;
}

IplImage *GetConnectedImageRegJp( IplImage *src )
{
	IplImage *gray, *bin, *smt, *inv;

	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	smt = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );


	// グレイスケールに変換
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// 適応二値化
//	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
	// ガウシアン
//	cvSmooth( bin, smt, CV_GAUSSIAN, eJpGParam*2+1, 0, 0 );
	// 二値化
	cvThreshold( gray, bin, eJpThr, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );

	cvReleaseImage( &gray );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

	return inv;
}

IplImage *GetConnectedImageReg( IplImage *img )
// srcから結合画像を作成する
{
	int i/*, num*/, chsize;
	double d00;
	IplImage *gray, *bin, *inv, *smt;
	CvSize img_size;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	strHist hist;
	IplImage *pt;

//	OutPutImage( img );
	img_size.width = img->width; img_size.height = img->height;
	gray = cvCreateImage( img_size, 8, 1 );
	bin = cvCreateImage( img_size, 8, 1 );
	inv = cvCreateImage( img_size, 8, 1 );
	smt = cvCreateImage( img_size, 8, 1 );

	cvCvtColor( img, gray, CV_RGB2GRAY );
	cvSmooth( gray, smt, CV_GAUSSIAN, /*15*/7, 0, 0, 0 );	// OpenCV 1.0
//	OutPutImage( smt );
	cvThreshold( smt, bin, 220, 255, CV_THRESH_BINARY );
//	OutPutImage( bin );
	cvNot( bin, inv );

	cvReleaseImage( &bin );
	cvReleaseImage( &smt );
	cvReleaseImage( &gray );

//	OutPutImage( inv );
	return inv;
}
