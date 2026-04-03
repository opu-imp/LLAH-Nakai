#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>			// OpneCV
// #include <highgui.h> // now in opencv2/opencv.hpp	// OpenCV
#include <windows.h>
#include <mmsystem.h>
#include <dshow.h>
#include <conio.h>
#include <qedit.h>
#include "dirs.h"
#include "hist.h"
#include "proj4p.h"
#include "dscap.h"
#include "dcamc.h"
#include "fptune.h"
#include "cFpTuneCap.h"

cFpTuneCap::cFpTuneCap( cv::Mat cap, int def_adp_block, int def_adp_sub, int def_gparam, int def_thr )
{
	cv::Size size;

	img = cap;
	size.width = img.cols;
	size.height = img.rows;
	gray = cv::Mat::zeros(size, CV_8UC1);
	bin1 = cv::Mat::zeros(size, CV_8UC1);
	smt = cv::Mat::zeros(size, CV_8UC1);
	bin2 = cv::Mat::zeros(size, CV_8UC1);
	inv = cv::Mat::zeros(size, CV_8UC1);
	pt = cv::Mat::zeros(size, CV_8UC1);
	overlap = cv::Mat::zeros(size, CV_8UC3);
	cor = cv::Mat::zeros(size, CV_8UC3);
	adp_block = def_adp_block;
	adp_sub = def_adp_sub;
	gparam = def_gparam;
	thr = def_thr;
	pfs = NULL;
	// コンストラクト時に特徴点抽出はしない
}

cFpTuneCap::~cFpTuneCap(void)
{ // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
	if ( pfs != NULL )	free( pfs );
}

int cFpTuneCap::Extract( void )
// キャプチャ画像から特徴点抽出
{
	int i, num;
	cv::Point point;
	cv::Size size;

	// グレイスケールに変換
	cv::cvtColor( img, gray, cv::COLOR_BGR2GRAY );
	// 適応二値化
	cv::adaptiveThreshold( gray, bin1, 255, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, adp_block*2+1, adp_sub );
	// ガウシアン
	cv::GaussianBlur(bin1, smt, cv::Size(gparam*2+1, gparam*2+1), 0);
	// 二値化
	cv::threshold( smt, bin2, thr, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin2, inv );
	// 特徴点抽出
	if ( pfs != NULL )	free( pfs );
	num = MakeFloatCentresFromImage( &pfs, inv, &size, NULL );
	// 特徴点の描画
	cvSet( pt, CV_RGB( 255, 255, 255 ), NULL );
	cvCopy( img, overlap, 0 );
	for ( i = 0; i < num; i++ ) {
		point.x = (int)pfs[i].x;
		point.y = (int)pfs[i].y;
		cv::circle( pt, point, kTuneCapPtRadius, CV_RGB( 255, 0, 0 ), -1, cv::LINE_AA, 0 );
		cv::circle( overlap, point, kTuneCapPtRadius, CV_RGB( 255, 0, 0 ), -1, cv::LINE_AA, 0 );
	}

	return 1;
}

int cFpTuneCap::DrawCor( int pcor[][2], cv::Point2f *corpfs, cv::Point cororig )
// 対応関係を描画
{
	cv::Point p1, p2;

	cvCopy( img, cor, 0 );

	if ( pfs == NULL || corpfs == NULL )	return 0;
	p1.x = (int)pfs[0].x;
	p1.y = (int)pfs[0].y;
	p2.x = cororig.x + (int)corpfs[0].x;
	p2.y = cororig.y + (int)corpfs[0].y;

	cv::line( cor, p1, p2, CV_RGB( 0, 0, 255 ), 2, cv::LINE_AA, 0 );

	return 1;
}
