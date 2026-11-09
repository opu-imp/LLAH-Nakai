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
	cv::Mat img, bin, inv, smt;
    // CvMemStorage removed (not needed in OpenCV 4)
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
//	cv::Point *ps;
	strHist hist;
	cv::Mat pt;

	if ( ( img = cv::imread(fname, cv::IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION) ).empty() ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return cv::Mat();
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
		cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cv::bitwise_not(bin, inv);	// CBDAR以後に追加したノイズ除去処理
//		OutPutImage( inv );
		ClearImageBorder( inv );
		cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する
		for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){		// 連結成分を描画
			mom = cv::moments( contours[ci] );
			d00 = mom.m00;
			if ( d00 < 10 ) continue;	// 小さすぎる連結成分は除外
#ifdef	WIN
			cv::drawContours( smt, contours, ci, cWhite, cv::FILLED, 8, hierarchy, 1 );	// replace cv::FILLED with 1 to see the outlines 
#else
			cv::drawContours( smt, contours, ci, cWhite, cv::FILLED, 8, hierarchy, 1 );	// replace cv::FILLED with 1 to see the outlines 

#endif
	    }
//		OutPutImage( smt );
		cv::bitwise_not(smt, bin);
	} else if ( mode == RET_MP_MODE ) {	// 携帯電話付属カメラモード：リサイズしてから2値化
		cv::resize( img, smt, smt.size(), 0, 0, cv::INTER_CUBIC );
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
	} else {	// ハッシュ構築モード（電子文書）：連結成分を取り出して文字サイズを調べてから文字を太らせる
		cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cv::bitwise_not( bin, inv );
		ClearImageBorder( inv );
		cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する
		InitHist( &hist, 100, 0, 1000 );
	    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
			mom = cv::moments( contours[ci] );
			d00 = mom.m00;
			if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
			AddDataHist( &hist, sqrt(d00) );
	    }
		chsize = (int)GetMaxBin(&hist);
		ReleaseHist( &hist );
		chsize = (int)(chsize*0.1);
		chsize += (chsize + 1) % 2;
		cv::GaussianBlur(img, smt, cv::Size(chsize, chsize), 0, 0, cv::BORDER_REPLICATE);
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 10 );
//		OutPutImage( bin );
// // cvReleaseMemStorage removed
	}
	cv::bitwise_not( bin, inv );
	// 文字の大きさを調べる
	ClearImageBorder( inv );
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する
	// 連結成分を描画・重心を計算
	InitHist( &hist, 100, 0, 1000 );
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
		AddDataHist( &hist, sqrt(d00) );
    }

	chsize = (int)GetMaxBin(&hist);
	ReleaseHist( &hist );
	if ( mode == CONST_HASH_MODE || mode == ADD_HASH_MODE ) chsize *= 2;
	else if ( mode == RET_MP_MODE )	chsize *= 3;
//	chsize /= 2;
	chsize += (chsize + 1) % 2;
	cv::GaussianBlur(bin, smt, cv::Size(chsize, chsize), 0, 0, cv::BORDER_REPLICATE);
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cv::bitwise_not( bin, smt );

#if 0
	ClearImageBorder( smt );
	cv::findContours( smt, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
		double d00;
		cv::Point p;
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外
		p.x = (int)(mom.m10 / d00);
		p.y = (int)(mom.m01 / d00);
		cv::circle( pt, p, 8, cWhite, -1, 1, 0 );
		i++;
    }
	OutPutImage( pt );
#endif // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

//	OutPutImage( smt );
	return smt;
}

int AutoConnectTest( char *fname, cv::Size *size )
// 単語の自動結合テスト
{
	int i, num, chsize;
	cv::Mat img, bin, inv, smt, pt;
    // CvMemStorage removed (not needed in OpenCV 4)
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
	cv::Point p;
	strHist hist;
//	cv::Scalar color;

	if ( ( img = cv::imread(fname, cv::IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION) ).empty() ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return 0;
	}

	size->width = img.cols;
	size->height = img.rows;
#ifdef MP_MODE
	size->width *= 9;
	size->height *= 9;
#endif
	bin = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);
	inv = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);
	smt = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);
	pt = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);

#ifdef RET_MODE
	cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
#endif
#ifdef DIC_MODE
	cv::bitwise_not( img, inv );
//	OutPutImage( inv );
	ClearImageBorder( inv );
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する
	InitHist( &hist, 100, 0, 1000 );
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
		double d00;
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
		AddDataHist( &hist, sqrt(d00) );
		i++;
    }
	chsize = (int)GetMaxBin(&hist);
	ReleaseHist( &hist );
	chsize = (int)chsize*0.1;
	chsize += (chsize + 1) % 2;
	cv::GaussianBlur(img, smt, cv::Size(chsize, chsize), 0, 0, cv::BORDER_REPLICATE);
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 10 );
#endif
#ifdef MP_MODE
	cv::resize( img, smt, smt.size(), 0, 0, cv::INTER_CUBIC );
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );

#endif
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv );
//	OutPutImage( inv );

	ClearImageBorder( inv );
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する

	// 点の数をカウント（暫定）
	i = 0;
	for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ) i++;
	num = i;

	// 連結成分を描画・重心を計算
	InitHist( &hist, 100, 0, 1000 );
//	InitHist( &hist, 200, 0, 20 );
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
		double d00;

		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
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
	cv::GaussianBlur(bin, smt, cv::Size(chsize, chsize), 0, 0, cv::BORDER_REPLICATE);
//	OutPutImage( smt );
//	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv );
#ifdef DRAW_POINT
	ClearImageBorder( inv );
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
		double d00;
//		cv::Scalar color;

		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外

		p.x = (int)(mom.m10 / d00);
		p.y = (int)(mom.m01 / d00);

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
	static cv::Mat gray, bin, img_hist;
//	cv::Mat inv;
	strHist hist;
    // CvMemStorage removed (not needed in OpenCV 4)
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
	double d00;

	// 画像の作成
	if ( gray.empty() )
		gray = cv::Mat::zeros(src.size(), CV_8UC1);
	if ( bin.empty() )
		bin = cv::Mat::zeros(src.size(), CV_8UC1);
//	inv = cv::Mat::zeros(src.size(), CV_8UC1);
#ifdef	DRAW_HIST
	if ( img_hist.empty() )
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
	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	// 08/09/11 文字サイズの調査
	cv::bitwise_not( bin, inv );
#ifdef	DRAW_HIST
	ClearImageBorder( inv );
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する
	// 連結成分を描画・重心を計算
	InitHist( &hist, 500, 0, 200 );
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < 2 ) continue;	// 小さすぎる連結成分は除外
		AddDataHist( &hist, sqrt(d00) );
    }
	DrawHist( img_hist, &hist );
	cv::imshow( "Hist", img_hist );
	ReleaseHist( &hist );
#endif
// 07/06/07
//	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 31, 10 );
//	cv::bitwise_not( bin, inv );
//	cv::GaussianBlur(bin, gray, cv::Size(7, 7), 0);
	// 07/06/07
//	cv::GaussianBlur(bin, gray, cv::Size(3, 3), 0);
	// 08/08/25 多言語
	cv::GaussianBlur(bin, gray, cv::Size(cam_gauss_mask_size, cam_gauss_mask_size), 0, 0, cv::BORDER_REPLICATE);
//	cv::imwrite( "smooth.bmp", gray );
//	cv::threshold( gray, bin, 200, 255, cv::THRESH_BINARY );
	// 07/06/07
	cv::threshold( gray, bin, 240, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv );
	// 連結成分の重心を計算
	/*
	ClearImageBorder( inv );
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );	// 連結成分を抽出する
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ){
		double d00;
		cv::Point p;
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外
		p.x = (int)(mom.m10 / d00);
		p.y = (int)(mom.m01 / d00);
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
	cv::Mat gray, bin, smt, inv;

	gray = cv::Mat::zeros(src.size(), CV_8UC1);
	bin = cv::Mat::zeros(src.size(), CV_8UC1);
	smt = cv::Mat::zeros(src.size(), CV_8UC1);
	inv = cv::Mat::zeros(src.size(), CV_8UC1);


	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
	// ガウシアン
	cv::GaussianBlur(bin, smt, cv::Size(eJpGParam*2+1, eJpGParam*2+1), 0, 0, cv::BORDER_REPLICATE);
	// 二値化
	cv::threshold( smt, bin, eJpThr, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

	return inv;
}

cv::Mat GetConnectedImageCamJp2( cv::Mat src )
{
	cv::Mat gray, bin, inv;

	// 画像の作成
	gray = cv::Mat::zeros(src.size(), CV_8UC1);
	bin = cv::Mat::zeros(src.size(), CV_8UC1);
	inv = cv::Mat::zeros(src.size(), CV_8UC1);
	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
//	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 12 );
	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 141, 12 );	// MIRU07 裏写り対策
//	cv::bitwise_not( bin, inv );
	cv::GaussianBlur(bin, gray, cv::Size(3, 3), 0, 0, cv::BORDER_REPLICATE);
//	cv::imwrite( "smooth.bmp", gray );
	cv::threshold( gray, bin, 200, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

	return inv;
}

cv::Mat GetConnectedImageRegJp( cv::Mat src )
{
	cv::Mat gray, bin, smt, inv;

	gray = cv::Mat::zeros(src.size(), CV_8UC1);
	bin = cv::Mat::zeros(src.size(), CV_8UC1);
	smt = cv::Mat::zeros(src.size(), CV_8UC1);
	inv = cv::Mat::zeros(src.size(), CV_8UC1);


	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
//	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
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
	cv::Mat gray, bin, inv, smt;
	cv::Size img_size;
    // CvMemStorage removed (not needed in OpenCV 4)
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
	strHist hist;
	cv::Mat pt;

//	OutPutImage( img );
	img_size.width = img.cols; img_size.height = img.rows;
	gray = cv::Mat::zeros(img_size, CV_8UC1);
	bin = cv::Mat::zeros(img_size, CV_8UC1);
	inv = cv::Mat::zeros(img_size, CV_8UC1);
	smt = cv::Mat::zeros(img_size, CV_8UC1);

	cv::cvtColor( img, gray, cv::COLOR_RGB2GRAY );
	cv::GaussianBlur( gray, smt, cv::Size( 7, 7 ), 0, 0, cv::BORDER_REPLICATE );	// OpenCV 1.0
//	OutPutImage( smt );
	cv::threshold( smt, bin, 220, 255, cv::THRESH_BINARY );
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

//	OutPutImage( inv );
	return inv;
}
