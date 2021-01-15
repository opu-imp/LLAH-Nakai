#include "def_general.h"

#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "dirs.h"
#include "auto_connect.h"
#include "hist.h"
#include "extern.h"

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
	if ( eIsJp ) {	// 日本語モードは別処理
		smt = cvCreateImage( cvSize(img->width, img->height), 8, 1 );
		GetConnectedImageJp( img, smt );
		OutPutImage( smt );
		return smt;
	}
		
//	OutPutImage( img );
	if ( mode == RET_MP_MODE ) {
		bin = cvCreateImage( cvSize(img->width*kMPExp, img->height*kMPExp), 8, 1 );
		inv = cvCreateImage( cvSize(img->width*kMPExp, img->height*kMPExp), 8, 1 );
		smt = cvCreateImage( cvSize(img->width*kMPExp, img->height*kMPExp), 8, 1 );
		pt = NULL;
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
		if ( contours != NULL )	cvClearSeq( contours );
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
		cvSmooth( img, smt, CV_GAUSSIAN, chsize, 0, 0, 0);
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 201, 10 );
//		OutPutImage( bin );
		if ( contours != NULL )	cvClearSeq( contours );
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
	if ( mode == CONST_HASH_MODE || mode == ADD_HASH_MODE ) {	// PDFから得た画像
		if ( eIsJp )	chsize = (int)((double)chsize * 0.5);	// 日本語モード
		else			chsize *= 2;
	}
	else if ( mode == RET_MP_MODE )	{	// 撮影画像
		chsize *= 3;
	}
//	chsize /= 2;
	chsize += (chsize + 1) % 2;
	cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0, 0 );	// OpenCV 1.0
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
		cvCircle( pt, p, 4, cWhite, -1, CV_AA, 0 );
		i++;
    }
	OutPutImage( pt );
#endif

	cvReleaseImage( &bin );
	cvReleaseImage( &inv );
	cvReleaseImage( &img );
	if ( pt != NULL ) cvReleaseImage( &pt );
	if ( contours != NULL )	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	OutPutImage( smt );
	return smt;
}

IplImage *GetConnectedImage2( const char *fname, int mode )
// fnameから結合画像を作成する2
{
	int i/*, num*/, chsize;
	double d00;
	IplImage *img, *bin, *inv, *smt;
	CvSize img_size;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
//	CvPoint *ps;
	strHist hist;
	IplImage *pt;

	if ( (img= cvLoadImage(fname, 0)) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return NULL;
	}
#ifdef	RESO_LIMIT
	if ( mode == RETRIEVE_MODE && img->width * img->height > 1280 * 960 ) {
		fprintf( stderr, "error: resolution of query image is limited to 1.3 mega pixels\n" );
		cvReleaseImage( &img );
		return NULL;
	}
#endif
//	OutPutImage( img );
	img_size.width = img->width; img_size.height = img->height;
	bin = cvCreateImage( img_size, 8, 1 );
	inv = cvCreateImage( img_size, 8, 1 );
	smt = cvCreateImage( img_size, 8, 1 );

	if ( mode == RETRIEVE_MODE ) {	// 検索モード
//		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 4 );
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
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
		if ( contours != NULL )	cvClearSeq( contours );
//		OutPutImage( smt );
		cvNot(smt, bin);
		cvNot( bin, inv );
		// 文字の大きさを調べる
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
		// 連結成分を描画・重心を計算
		InitHist( &hist, 1000, 0, 1000 );
	    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
	    {
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
			AddDataHist( &hist, sqrt(d00) );
	    }

		chsize = (int)GetMaxBin(&hist);
		ReleaseHist( &hist );
//		chsize += (chsize + 1) % 2;
//		chsize = chsize * 2 + 1;	// 12/21変更　高解像度カメラを用いた拡大撮影での結果を精査して
		chsize += (chsize + 1) % 2;	// 08/05/30変更　MIRU08再実験のため戻す．精度向上
//		printf( "chsize : %d\n", chsize );
		cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0, 0 );	// OpenCV 1.0
//		OutPutImage( smt );
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 4 );
//		OutPutImage( bin );
		cvNot( bin, inv );
	} else {	// ハッシュ構築モード（電子文書）：閾値固定モード
		cvSmooth( img, smt, CV_GAUSSIAN, eHashGaussMaskSize, 0, 0, 0 );	// OpenCV 1.0
//		OutPutImage( smt );
		cvThreshold( smt, bin, kConnectHashImageThr, kImgMaxVal, CV_THRESH_BINARY );
//		OutPutImage( bin );
		cvNot( bin, inv );
	}

	cvReleaseImage( &bin );
	cvReleaseImage( &smt );
	cvReleaseImage( &img );

//	OutPutImage( inv );
	return inv;
}

int GetConnectedImageJp( IplImage *src, IplImage *dst )
// 日本語モード
{
	IplImage *smt, *bin;
	CvSize size;

	size.width = src->width;
	size.height = src->height;

	smt = cvCreateImage( size, 8, 1 );
	bin = cvCreateImage( size, 8, 1 );

	// ガウシアン（一回目）
	cvSmooth( src, smt, CV_GAUSSIAN, eJpGParam1*2+1, 0, 0, 0 );
	// 二値化（一回目）
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_ADAPTIVE_THRESH_MEAN_C, eJpAdpBlock*2+1, eJpAdpSub );
	// ガウシアン（二回目）
	cvSmooth( bin, smt, CV_GAUSSIAN, eJpGParam2*2+1, 0, 0, 0 );
	// 二値化（二回目）
	cvThreshold( smt, bin, eJpThr, 255, CV_THRESH_BINARY );
	// 反転（特徴点抽出のため）
	cvNot( bin, dst );

	cvReleaseImage( &smt );
	cvReleaseImage( &bin );
}

IplImage *GetConnectedImageJp2( const char *fname, int mode )
// 特徴点抽出（日本語モード）上のは何だっけ？
{
	IplImage *img, *smt, *bin, *inv;
	CvSize img_size;

	if ( (img= cvLoadImage(fname, 0)) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return NULL;
	}
//	OutPutImage( img );
	img_size.width = img->width; img_size.height = img->height;
	bin = cvCreateImage( img_size, 8, 1 );
	inv = cvCreateImage( img_size, 8, 1 );
	smt = cvCreateImage( img_size, 8, 1 );

	if ( mode == RETRIEVE_MODE ) {	// 検索モード
		// 適応二値化
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 12 );
		cvSmooth( bin, smt, CV_GAUSSIAN, 3/* 1 */, 0, 0, 0 );	// 1は解像度が低い場合
		cvThreshold( smt, bin, 200, 255, CV_THRESH_BINARY );
		cvNot( bin, inv );
	}
	else { // ハッシュ構築モード
		// ガウシアン
//		cvSmooth( img, smt, CV_GAUSSIAN, 7, 0, 0, 0 );	// OpenCV 1.0
		// 07/07/24
		cvSmooth( img, smt, CV_GAUSSIAN, 3, 0, 0, 0 );	// OpenCV 1.0
		// 2値化
		cvThreshold( smt, bin, 228, 255, CV_THRESH_BINARY );
		// 反転
		cvNot( bin, inv );
	}
	// 開放
	cvReleaseImage( &img );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

//	OutPutImage( inv );

	return inv;
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
	cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0, 0 );
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
	static IplImage *gray = NULL, *bin = NULL;
	IplImage *inv;

#ifdef	WIN

#ifdef	RESO_LIMIT
	if ( src->width * src->height > 1280 * 960 ) {
		fprintf( stderr, "error: resolution of query image is limited to 1.3 mega pixels\n" );
		return NULL;
	}
#endif
	// 画像の作成
	if ( gray == NULL )
		gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	if ( bin == NULL )
		bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// グレイスケールに変換
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// 適応二値化
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
	// 07/06/07
	cvSmooth( bin, gray, CV_GAUSSIAN, 3, 0, 0, 0 );
	// 07/06/07
	cvThreshold( gray, bin, 240, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );
	
//	cvReleaseImage( &gray );
//	cvReleaseImage( &bin );
#else
	inv = NULL;
#endif

	return inv;
}

IplImage *GetConnectedImageCamJp2( IplImage *src )
{
	IplImage *gray, *bin, *inv;

#ifdef WIN
	// 画像の作成
	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// グレイスケールに変換
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// 適応二値化
//	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 12 );
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 141, 12 );	// MIRU07 裏写り対策
//	cvNot( bin, inv );
	cvSmooth( bin, gray, CV_GAUSSIAN, 3, 0, 0, 0 );
//	cvSaveImage( "smooth.bmp", gray );
	cvThreshold( gray, bin, 200, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );
	
	cvReleaseImage( &gray );
	cvReleaseImage( &bin );
#else
	inv = NULL;
#endif

	return inv;
}
