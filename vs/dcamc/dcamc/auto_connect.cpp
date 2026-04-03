#include "def_general.h"

#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
// #include <highgui.h> // now in opencv2/opencv.hpp
#include "extern.h"
#include "dirs.h"
#include "auto_connect.h"
#include "hist.h"

#define RET_MODE
//#define DIC_MODE
//#define	MP_MODE
#define	DRAW_POINT

cv::Mat GetConnectedImage( char *fname, int mode )
// fnameから結合画像を作成する
{
	int i/*, num*/, chsize;
	double d00;
	cv::Mat img, *bin, *inv, *smt;
    // CvMemStorage removed (not needed in OpenCV 4)
	CvSeq *contours = 0, *con0;
	CvMoments mom;
//	cv::Point *ps;
	strHist hist;
	cv::Mat pt;

	if ( (img= cv::imread(fname, cv::IMREAD_GRAYSCALE)) == NULL ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return NULL;
	}
//	OutPutImage( img );
	if ( mode == RET_MP_MODE ) {
		bin = cv::Mat::zeros(cv::Size(img.cols*kMPExp, img.rows*kMPExp), CV_8UC1);
		inv = cv::Mat::zeros(cv::Size(img.cols*kMPExp, img.rows*kMPExp), CV_8UC1);
		smt = cv::Mat::zeros(cv::Size(img.cols*kMPExp, img.rows*kMPExp), CV_8UC1);
	} else {
		bin = cv::Mat::zeros(cv::Size(img.cols, img.rows), CV_8UC1);
		inv = cv::Mat::zeros(cv::Size(img.cols, img.rows), CV_8UC1);
		smt = cv::Mat::zeros(cv::Size(img.cols, img.rows), CV_8UC1);
		pt = cv::Mat::zeros(cv::Size(img.cols, img.rows), CV_8UC1);
	}

	if ( mode == RETRIEVE_MODE ) {	// 検索モード：とりあえず2値化
		cv::adaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cv::bitwise_not(bin, inv);	// CBDAR以後に追加したノイズ除去処理
//		OutPutImage( inv );
		cv::findContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する
		for( con0 = contours; con0 != 0; con0 = con0->h_next ) {		// 連結成分を描画
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < 10 ) continue;	// 小さすぎる連結成分は除外
#ifdef	WIN
			cv::drawContours(smt, con0, cWhite, cWhite, -1, cv::FILLED, 8, cv::Point(0,0) );	// replace cv::FILLED with 1 to see the outlines 
#else
			cv::drawContours(smt, con0, cWhite, cWhite, -1, cv::FILLED, 8);	// replace cv::FILLED with 1 to see the outlines 

#endif
	    }
		cvClearSeq( contours );
//		OutPutImage( smt );
		cv::bitwise_not(smt, bin);
	} else if ( mode == RET_MP_MODE ) {	// 携帯電話付属カメラモード：リサイズしてから2値化
		cv::resize( img, smt, CV_INTER_CUBIC );
		cv::adaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
	} else {	// ハッシュ構築モード（電子文書）：連結成分を取り出して文字サイズを調べてから文字を太らせる
		cv::adaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cv::bitwise_not( bin, inv );
		cv::findContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する
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
		cv::GaussianBlur(img, smt, cv::Size(chsize, chsize), 0);
		cv::adaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 10 );
//		OutPutImage( bin );
		cvClearSeq( contours );
// // cvReleaseMemStorage removed
	}
	cv::bitwise_not( bin, inv );
	// 文字の大きさを調べる
	cv::findContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する
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
	cv::GaussianBlur(bin, smt, cv::Size(chsize, chsize), 0);
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cv::bitwise_not( bin, smt );

#if 0
	cv::findContours( smt, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		cv::Point p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外
		p.x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		p.y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
		cv::circle( pt, p, 8, cWhite, -1, 1, 0 );
		i++;
    }
	OutPutImage( pt );
#endif // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
	cvClearSeq( contours ); // cvReleaseMemStorage removed

//	OutPutImage( smt );
	return smt;
}

int AutoConnectTest( char *fname, cv::Size *size )
// 単語の自動結合テスト
{
	int i, num, chsize;
	cv::Mat img, *bin, *inv, *smt, *pt;
    // CvMemStorage removed (not needed in OpenCV 4)
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	cv::Point p;
	strHist hist;
//	cv::Scalar color;

	if ( (img= cv::imread(fname, cv::IMREAD_GRAYSCALE)) == NULL ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return 0;
	}

	size.cols = img.cols;
	size.rows = img.rows;
#ifdef MP_MODE
	size.cols *= 9;
	size.rows *= 9;
#endif
	bin = cv::Mat::zeros(cv::Size(size.cols, size.rows), CV_8UC1);
	inv = cv::Mat::zeros(cv::Size(size.cols, size.rows), CV_8UC1);
	smt = cv::Mat::zeros(cv::Size(size.cols, size.rows), CV_8UC1);
	pt = cv::Mat::zeros(cv::Size(size.cols, size.rows), CV_8UC1);

#ifdef RET_MODE
	cv::adaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
#endif
#ifdef DIC_MODE
	cv::bitwise_not( img, inv );
//	OutPutImage( inv );
	cv::findContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する
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
	cv::GaussianBlur(img, smt, cv::Size(chsize, chsize), 0);
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 10 );
#endif
#ifdef MP_MODE
	cv::resize( img, smt, CV_INTER_CUBIC );
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );

#endif
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv );
//	OutPutImage( inv );

	cv::findContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する

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
	cv::GaussianBlur(bin, smt, cv::Size(chsize, chsize), 0);
//	OutPutImage( smt );
//	cv::adaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
	cv::adaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv );
#ifdef DRAW_POINT
	cv::findContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
//		cv::Scalar color;

		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外

		p.x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		p.y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);

		cv::circle( pt, p, 8, cWhite, -1, 1, 0 );
		i++;
    }

//	OutPutImage( pt );
#endif
	return num;
}

cv::Mat GetConnectedImageCam( cv::Mat src, cv::Mat inv, int cam_gauss_mask_size )
{
	int i;
	static cv::Mat gray = NULL, *bin = NULL, *img_hist = NULL;
//	cv::Mat inv;
	strHist hist;
    // CvMemStorage removed (not needed in OpenCV 4)
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	double d00;

	// 画像の作成
	if ( gray == NULL )
		gray = cv::Mat::zeros(src.size(), CV_8UC1);
	if ( bin == NULL )
		bin = cv::Mat::zeros(src.size(), CV_8UC1);
//	inv = cv::Mat::zeros(src.size(), CV_8UC1);
#ifdef	DRAW_HIST
	if ( img_hist == NULL )
		img_hist = cv::Mat::zeros(cv::Size( 1000, 200 ), CV_8UC1);
#endif
//	point = cv::Mat::zeros(src.size(), CV_8UC1);
	// 開始位置を合わせる（ないと上下反転する）
//	0/*gray.origin removed,cv::Mat is always top-left*/ = 0/*src.origin removed,cv::Mat is always top-left*/;
//	0/*bin.origin removed,cv::Mat is always top-left*/ = 0/*src.origin removed,cv::Mat is always top-left*/;
//	0/*inv.origin removed,cv::Mat is always top-left*/ = 0/*src.origin removed,cv::Mat is always top-left*/;
//	0/*point.origin removed,cv::Mat is always top-left*/ = 0/*src.origin removed,cv::Mat is always top-left*/;
	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
	cv::adaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	// 08/09/11 文字サイズの調査
	cv::bitwise_not( bin, inv );
#ifdef	DRAW_HIST
	cv::findContours( inv, storage, &contours, sizeof(CvContour), CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する
	// 連結成分を描画・重心を計算
	InitHist( &hist, 500, 0, 200 );
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next ) {
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 2 ) continue;	// 小さすぎる連結成分は除外
		AddDataHist( &hist, sqrt(d00) );
    }
	DrawHist( img_hist, &hist );
	cv::imshow( "Hist", img_hist );
	ReleaseHist( &hist );
	if ( contours )	cvClearSeq( contours ); // cvReleaseMemStorage removed
#endif
// 07/06/07
//	cv::adaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 31, 10 );
//	cv::bitwise_not( bin, inv );
//	cv::GaussianBlur(bin, gray, cv::Size(7, 7), 0);
	// 07/06/07
//	cv::GaussianBlur(bin, gray, cv::Size(3, 3), 0);
	// 08/08/25 多言語
	cv::GaussianBlur(bin, gray, cv::Size(cam_gauss_mask_size, cam_gauss_mask_size), 0);
//	cv::imwrite( "smooth.bmp", gray );
//	cv::threshold( gray, bin, 200, 255, cv::THRESH_BINARY );
	// 07/06/07
	cv::threshold( gray, bin, 240, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv );
	// 連結成分の重心を計算
	/*
	cv::findContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		cv::Point p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外
		p.x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		p.y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
		cv::circle( point, p, 3, CV_RGB(255, 255, 255), -1, 1, 0 );
		i++;
    }
*/
	
//	cv::threshold( gray, bin, 100, cv::THRESH_BINARY, cv::THRESH_BINARY );
//	cv::GaussianBlur(src, bin, cv::Size(3, 3), 0);
//	cv::imwrite( "bin.bmp", bin);
	
// // cvReleaseImage removed (cv::Mat auto-manages memory)
// // cvReleaseImage removed (cv::Mat auto-manages memory)

	return inv;
}

cv::Mat GetConnectedImageCamJp( cv::Mat src )
{
	cv::Mat gray, *bin, *smt, *inv;

	gray = cv::Mat::zeros(src.size(), CV_8UC1);
	bin = cv::Mat::zeros(src.size(), CV_8UC1);
	smt = cv::Mat::zeros(src.size(), CV_8UC1);
	inv = cv::Mat::zeros(src.size(), CV_8UC1);


	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
	cv::adaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
	// ガウシアン
	cv::GaussianBlur(bin, smt, cv::Size(eJpGParam*2+1, eJpGParam*2+1), 0);
	// 二値化
	cv::threshold( smt, bin, eJpThr, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

	return inv;
}

cv::Mat GetConnectedImageCamJp2( cv::Mat src )
{
	cv::Mat gray, *bin, *inv;

	// 画像の作成
	gray = cv::Mat::zeros(src.size(), CV_8UC1);
	bin = cv::Mat::zeros(src.size(), CV_8UC1);
	inv = cv::Mat::zeros(src.size(), CV_8UC1);
	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
//	cv::adaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 12 );
	cv::adaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 141, 12 );	// MIRU07 裏写り対策
//	cv::bitwise_not( bin, inv );
	cv::GaussianBlur(bin, gray, cv::Size(3, 3), 0);
//	cv::imwrite( "smooth.bmp", gray );
	cv::threshold( gray, bin, 200, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

	return inv;
}

cv::Mat GetConnectedImageRegJp( cv::Mat src )
{
	cv::Mat gray, *bin, *smt, *inv;

	gray = cv::Mat::zeros(src.size(), CV_8UC1);
	bin = cv::Mat::zeros(src.size(), CV_8UC1);
	smt = cv::Mat::zeros(src.size(), CV_8UC1);
	inv = cv::Mat::zeros(src.size(), CV_8UC1);


	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
//	cv::adaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
	// ガウシアン
//	cv::GaussianBlur(bin, smt, cv::Size(eJpGParam*2+1, eJpGParam*2+1), 0);
	// 二値化
	cv::threshold( gray, bin, eJpThr, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

	return inv;
}

cv::Mat GetConnectedImageReg( cv::Mat img )
// srcから結合画像を作成する
{
	int i/*, num*/, chsize;
	double d00;
	cv::Mat gray, *bin, *inv, *smt;
	cv::Size img_size;
    // CvMemStorage removed (not needed in OpenCV 4)
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	strHist hist;
	cv::Mat pt;

//	OutPutImage( img );
	img_size.width = img.cols; img_size.height = img.rows;
	gray = cv::Mat::zeros(img_size, CV_8UC1);
	bin = cv::Mat::zeros(img_size, CV_8UC1);
	inv = cv::Mat::zeros(img_size, CV_8UC1);
	smt = cv::Mat::zeros(img_size, CV_8UC1);

	cv::cvtColor( img, gray, cv::COLOR_RGB2GRAY );
	cvSmooth( gray, smt, CV_GAUSSIAN, /*15*/7, 0, 0, 0 );	// OpenCV 1.0
//	OutPutImage( smt );
	cv::threshold( smt, bin, 220, 255, cv::THRESH_BINARY );
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

//	OutPutImage( inv );
	return inv;
}
