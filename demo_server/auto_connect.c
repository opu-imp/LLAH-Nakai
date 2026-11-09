#include "def_general.h"

#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include "dirs.h"
#include "auto_connect.h"
#include "hist.h"
#include "extern.h"
#include "nstr.h"

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
    std::vector<std::vector<cv::Point>> contours;

    std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
//	cv::Point *ps;
	strHist hist;
	cv::Mat pt;

	img = cv::imread(fname, cv::IMREAD_GRAYSCALE);
	if (img.empty()) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return cv::Mat();
	}
	if ( eIsJp ) {	// 日本語モードは別処理
		smt = cv::Mat::zeros(cv::Size(img.cols, img.rows), CV_8UC1);
		GetConnectedImageJp( img, smt );
		OutPutImage( smt );
		return smt;
	}
		
//	OutPutImage( img );
	if ( mode == RET_MP_MODE ) {
		bin = cv::Mat::zeros(cv::Size(img.cols*kMPExp, img.rows*kMPExp), CV_8UC1);
		inv = cv::Mat::zeros(cv::Size(img.cols*kMPExp, img.rows*kMPExp), CV_8UC1);
		smt = cv::Mat::zeros(cv::Size(img.cols*kMPExp, img.rows*kMPExp), CV_8UC1);
		pt = cv::Mat();
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
		cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する
		for( int _ci = FirstTopLevelContour( hierarchy ); _ci >= 0; _ci = hierarchy[_ci][0] ) {		// 連結成分を描画
			mom = cv::moments(contours[_ci]);
			d00 = mom.m00;
			if ( d00 < 10 ) continue;	// 小さすぎる連結成分は除外
#ifdef	WIN32
			cv::drawContours(smt, contours, _ci, cWhite, cv::FILLED, 8);	// replace cv::FILLED with 1 to see the outlines 
#else
			cv::drawContours(smt, contours, _ci, cWhite, cv::FILLED, 8);	// replace cv::FILLED with 1 to see the outlines 
#endif
	    }
//		OutPutImage( smt );
		cv::bitwise_not(smt, bin);
	} else if ( mode == RET_MP_MODE ) {	// 携帯電話付属カメラモード：リサイズしてから2値化
		cv::resize(img, smt, smt.size(), 0, 0, cv::INTER_CUBIC);
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
	} else {	// ハッシュ構築モード（電子文書）：連結成分を取り出して文字サイズを調べてから文字を太らせる
		cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cv::bitwise_not( bin, inv );
		cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する
		InitHist( &hist, 100, 0, 1000 );
	    for( int _ci = FirstTopLevelContour( hierarchy ); _ci >= 0; _ci = hierarchy[_ci][0] )
	    {
			mom = cv::moments(contours[_ci]);
			d00 = mom.m00;
			if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
			AddDataHist( &hist, sqrt(d00) );
	    }
		chsize = (int)GetMaxBin(&hist);
		ReleaseHist( &hist );
		chsize = (int)(chsize*0.1);
		chsize += (chsize + 1) % 2;
		cv::GaussianBlur(img, smt, cv::Size(chsize, chsize), 0);
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 10 );
//		OutPutImage( bin );
//
	}
	cv::bitwise_not( bin, inv );
	// 文字の大きさを調べる
	cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する
	// 連結成分を描画・重心を計算
	InitHist( &hist, 100, 0, 1000 );
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] )
    {
		mom = cv::moments(contours[ci]);
		d00 = mom.m00;
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
	cv::GaussianBlur(bin, smt, cv::Size(chsize, chsize), 0);	// OpenCV 1.0
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cv::bitwise_not( bin, smt );

#if 0
	cv::findContours(smt, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] )
    {
		double d00;
		cv::Point p;
		mom = cv::moments(contours[ci]);
		d00 = mom.m00;
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外
		p.x = (int)(mom.m10 / d00);
		p.y = (int)(mom.m01 / d00);
		cv::circle( pt, p, 4, cWhite, -1, cv::LINE_AA, 0 );
		i++;
    }
	OutPutImage( pt );
#endif


	if (!pt.empty())
	OutPutImage( smt );
	return smt;
}

cv::Mat GetConnectedImage2( const char *fname, int mode, int gaussian_parameter )
// fnameから結合画像を作成する2
{
	int i/*, num*/, chsize, mask_size;
	double d00;
	cv::Mat img, bin, inv, smt;
	cv::Size img_size;
    std::vector<std::vector<cv::Point>> contours;

    std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
//	cv::Point *ps;
	strHist hist;
	cv::Mat pt;

	img = cv::imread(fname, cv::IMREAD_GRAYSCALE);
	if (img.empty()) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return cv::Mat();
	}
#ifdef	RESO_LIMIT
	if ( mode == RETRIEVE_MODE && img.cols * img.rows > 1280 * 960 ) {
		fprintf( stderr, "error: resolution of query image is limited to 1.3 mega pixels\n" );
		return cv::Mat();
	}
#endif
//	OutPutImage( img );
	img_size.width = img.cols; img_size.height = img.rows;
	bin = cv::Mat::zeros(img_size, CV_8UC1);
	inv = cv::Mat::zeros(img_size, CV_8UC1);
	smt = cv::Mat::zeros(img_size, CV_8UC1);

	if ( mode == RETRIEVE_MODE ) {	// 検索モード
//		cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 4 );
		// 09/03/04 ゴミがやたら出るので修正（3degの60）
		cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
		OutPutImage( bin );
		cv::bitwise_not(bin, inv);	// CBDAR以後に追加したノイズ除去処理
//		OutPutImage( inv );
		cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する
		for( int _ci = FirstTopLevelContour( hierarchy ); _ci >= 0; _ci = hierarchy[_ci][0] ) {		// 連結成分を描画
			mom = cv::moments(contours[_ci]);
			d00 = mom.m00;
			if ( d00 < 10 ) continue;	// 小さすぎる連結成分は除外
#ifdef	WIN32
			cv::drawContours(smt, contours, _ci, cWhite, cv::FILLED, 8);	// replace cv::FILLED with 1 to see the outlines 
#else
			cv::drawContours(smt, contours, _ci, cWhite, cv::FILLED, 8);	// replace cv::FILLED with 1 to see the outlines 
#endif
	    }
//		OutPutImage( smt );
		cv::bitwise_not(smt, bin);
		cv::bitwise_not( bin, inv );
		// 文字の大きさを調べる
		cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する
		// 連結成分を描画・重心を計算
		InitHist( &hist, 1000, 0, 1000 );
	    i = 0;
	    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] )
	    {
			mom = cv::moments(contours[ci]);
			d00 = mom.m00;
			if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
			AddDataHist( &hist, sqrt(d00) );
	    }

		chsize = (int)GetMaxBin(&hist);
		ReleaseHist( &hist );
//		chsize += (chsize + 1) % 2;
//		chsize = chsize * 2 + 1;	// 12/21変更　高解像度カメラを用いた拡大撮影での結果を精査して
		chsize += (chsize + 1) % 2;	// 08/05/30変更　MIRU08再実験のため戻す．精度向上
//		printf( "chsize : %d\n", chsize );
		cv::GaussianBlur(bin, smt, cv::Size(chsize, chsize), 0);	// OpenCV 1.0
//		OutPutImage( smt );
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 4 );
//		OutPutImage( bin );
		cv::bitwise_not( bin, inv );
	} else {	// ハッシュ構築モード（電子文書）：閾値固定モード
		if ( eVariousAutoMaskSize ) {
			mask_size = VariousLangMaskSize( fname );
			cv::GaussianBlur(img, smt, cv::Size(mask_size, mask_size), 0);	// OpenCV 1.0
		}
		else {
			if ( gaussian_parameter <= 0 ) {	// パラメータが0以下ならコピー
				img.copyTo(smt);
			} else {
				cv::GaussianBlur(img, smt, cv::Size(gaussian_parameter, gaussian_parameter), 0);	// OpenCV 1.0
			}
		}
//		OutPutImage( smt );
		cv::threshold( smt, bin, eConnectHashImageThr, kImgMaxVal, cv::THRESH_BINARY );
//		OutPutImage( bin );
		cv::bitwise_not( bin, inv );
	}



	OutPutImage( inv );
	return inv;
}

int GetConnectedImageJp( cv::Mat src, cv::Mat dst )
// 日本語モード
{
	cv::Mat smt, bin;
	cv::Size size;

	size.width = src.cols;
	size.height = src.rows;

	smt = cv::Mat::zeros(size, CV_8UC1);
	bin = cv::Mat::zeros(size, CV_8UC1);

	// ガウシアン（一回目）
	cv::GaussianBlur(src, smt, cv::Size(eJpGParam1*2+1, eJpGParam1*2+1), 0);
	// 二値化（一回目）
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::ADAPTIVE_THRESH_MEAN_C, eJpAdpBlock*2+1, eJpAdpSub );
	// ガウシアン（二回目）
	cv::GaussianBlur(bin, smt, cv::Size(eJpGParam2*2+1, eJpGParam2*2+1), 0);
	// 二値化（二回目）
	cv::threshold( smt, bin, eJpThr, 255, cv::THRESH_BINARY );
	// 反転（特徴点抽出のため）
	cv::bitwise_not( bin, dst );

}

cv::Mat GetConnectedImageJp2( const char *fname, int mode )
// 特徴点抽出（日本語モード）上のは何だっけ？
{
	cv::Mat img, smt, bin, inv;
	cv::Size img_size;

	img = cv::imread(fname, cv::IMREAD_GRAYSCALE);
	if (img.empty()) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return cv::Mat();
	}
//	OutPutImage( img );
	img_size.width = img.cols; img_size.height = img.rows;
	bin = cv::Mat::zeros(img_size, CV_8UC1);
	inv = cv::Mat::zeros(img_size, CV_8UC1);
	smt = cv::Mat::zeros(img_size, CV_8UC1);

	if ( mode == RETRIEVE_MODE ) {	// 検索モード
		// 適応二値化
		cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 12 );
		cv::GaussianBlur(bin, smt, cv::Size(3/* 1 */, 3/* 1 */), 0);	// 1は解像度が低い場合
		cv::threshold( smt, bin, 200, 255, cv::THRESH_BINARY );
		cv::bitwise_not( bin, inv );
	}
	else { // ハッシュ構築モード
		// ガウシアン
//		cv::GaussianBlur(img, smt, cv::Size(7, 7), 0);	// OpenCV 1.0
		// 07/07/24
		cv::GaussianBlur(img, smt, cv::Size(3, 3), 0);	// OpenCV 1.0
		// 2値化
		cv::threshold( smt, bin, 228, 255, cv::THRESH_BINARY );
		// 反転
		cv::bitwise_not( bin, inv );
	}
	// 開放



	OutPutImage( inv );

	return inv;
}

int AutoConnectTest( char *fname, cv::Size *size )
// 単語の自動結合テスト
{
	int i, num, chsize;
	cv::Mat img, bin, inv, smt, pt;
    std::vector<std::vector<cv::Point>> contours;

    std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
	cv::Point p;
	strHist hist;
//	cv::Scalar color;

	img = cv::imread(fname, cv::IMREAD_GRAYSCALE);
	if (img.empty()) {
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
	cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する
	InitHist( &hist, 100, 0, 1000 );
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] )
    {
		double d00;
		mom = cv::moments(contours[ci]);
		d00 = mom.m00;
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
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 10 );
#endif
#ifdef MP_MODE
	cv::resize(img, smt, smt.size(), 0, 0, cv::INTER_CUBIC);
//	OutPutImage( smt );
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );

#endif
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv );
//	OutPutImage( inv );

	cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する

	// 点の数をカウント（暫定）
	i = 0;
	for ( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ) i++;
	num = i;

	// 連結成分を描画・重心を計算
	InitHist( &hist, 100, 0, 1000 );
//	InitHist( &hist, 200, 0, 20 );
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] )
    {
		double d00;

		mom = cv::moments(contours[ci]);
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
	cv::GaussianBlur(bin, smt, cv::Size(chsize, chsize), 0);
//	OutPutImage( smt );
//	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 201, 5 );
//	OutPutImage( bin );
	cv::bitwise_not( bin, inv );
#ifdef DRAW_POINT
	cv::findContours(inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] )
    {
		double d00;
//		cv::Scalar color;

		mom = cv::moments(contours[ci]);
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

cv::Mat GetConnectedImageCam( cv::Mat src )
{
	static cv::Mat gray, bin;
	cv::Mat inv;

#ifdef	WIN32

#ifdef	RESO_LIMIT
	if ( src.cols * src.rows > 1280 * 960 ) {
		fprintf( stderr, "error: resolution of query image is limited to 1.3 mega pixels\n" );
		return cv::Mat();
	}
#endif
	// 画像の作成
	if (gray.empty())
		gray = cv::Mat::zeros(src.size(), CV_8UC1);
	if (bin.empty())
		bin = cv::Mat::zeros(src.size(), CV_8UC1);
	inv = cv::Mat::zeros(src.size(), CV_8UC1);
	// グレイスケールに変換
	cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	// 07/06/07
	cv::GaussianBlur(bin, gray, cv::Size(3, 3), 0);
	// 07/06/07
	cv::threshold( gray, bin, 240, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv );
	
//
//
#else
	inv = cv::Mat();
#endif

	return inv;
}

cv::Mat GetConnectedImageCamJp2( cv::Mat src )
{
	cv::Mat gray, bin, inv;

#ifdef WIN32
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
	cv::GaussianBlur(bin, gray, cv::Size(3, 3), 0);
//	cv::imwrite( "smooth.bmp", gray );
	cv::threshold( gray, bin, 200, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv );

#else
	inv = cv::Mat();
#endif

	return inv;
}

int VariousLangMaskSize( const char *fname )
// ファイル名からマスクサイズを自動的に決定
{
	char path[kMaxPathLen], basename[kMaxPathLen];
	char prefixes[][20] = { "arabic", "chinese", "english", "french", "hindi", "japanese", "korean", "lhao", "russian", "spanish", "tamil", "thai" };
	int i, num, sizes[] = { 5, 3, 5, 7, 5, 3, 3, 7, 5, 5, 3, 3 };

	strcpy( path, fname );
	if ( GetBasename2( path, kMaxPathLen, basename ) == 0 )	return eHashGaussMaskSize;
	num = sizeof( sizes );
	for ( i = 0; i < num; i++ ) {
		if ( strncmp( basename, prefixes[i], strlen( prefixes[i] ) ) == 0 )	return sizes[i];
	}
	return eHashGaussMaskSize;
}

int VariousLangMaskSizeDualChar( const char *fname )
// ファイル名からマスクサイズを自動的に決定（デュアルハッシュの文字の場合）
{
	char path[kMaxPathLen], basename[kMaxPathLen];
	char prefixes[][20] = { "arabic", "chinese", "english", "french", "hindi", "japanese", "korean", "lhao", "russian", "spanish", "tamil", "thai" };
	int i, num, sizes[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	strcpy( path, fname );
	if ( GetBasename2( path, kMaxPathLen, basename ) == 0 )	return eHashGaussMaskSize;
	num = sizeof( sizes );
	for ( i = 0; i < num; i++ ) {
		if ( strncmp( basename, prefixes[i], strlen( prefixes[i] ) ) == 0 )	return sizes[i];
	}
	return eHashGaussMaskSize;
}

int VariousLangMaskSizeDualWord( const char *fname )
// ファイル名からマスクサイズを自動的に決定（デュアルハッシュの単語の場合）
{
	char path[kMaxPathLen], basename[kMaxPathLen];
	char prefixes[][20] = { "arabic", "chinese", "english", "french", "hindi", "japanese", "korean", "lhao", "russian", "spanish", "tamil", "thai" };
	int i, num, sizes[] = { 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23 };

	strcpy( path, fname );
	if ( GetBasename2( path, kMaxPathLen, basename ) == 0 )	return eHashGaussMaskSize;
	num = sizeof( sizes );
	for ( i = 0; i < num; i++ ) {
		if ( strncmp( basename, prefixes[i], strlen( prefixes[i] ) ) == 0 )	return sizes[i];
	}
	return eHashGaussMaskSize;
}
