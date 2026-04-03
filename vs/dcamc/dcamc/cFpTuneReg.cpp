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
#include "cFpTuneReg.h"

cFpTuneReg::cFpTuneReg( char *fname, int def_gparam1, int def_adp_block, int def_adp_sub, int def_gparam2, int def_thr )
{
	cv::Size size;

	if ( (img = cv::imread(fname, cv::IMREAD_GRAYSCALE)) == NULL ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return;
	}
	size.width = img.cols;
	size.height = img.rows;
	smt1 = cv::Mat::zeros(size, CV_8UC1);
	bin1 = cv::Mat::zeros(size, CV_8UC1);
	smt2 = cv::Mat::zeros(size, CV_8UC1);
	bin2 = cv::Mat::zeros(size, CV_8UC1);
	inv = cv::Mat::zeros(size, CV_8UC1);
	pt  = cv::Mat::zeros(size, CV_8UC1);
	overlap = cv::Mat::zeros(size, CV_8UC1);
	cor = cv::Mat::zeros(size, CV_8UC1);

	gparam1 = def_gparam1;
	adp_block = def_adp_block;
	adp_sub = def_adp_sub;
	gparam2 = def_gparam2;
	thr = def_thr;
	pfs = NULL;

	Extract();

	return;
}

cFpTuneReg::~cFpTuneReg(void)
{ // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
	if ( pfs != NULL )	free( pfs );
}

int cFpTuneReg::Extract( void )
{
	int i, num;
	cv::Point point;
	cv::Size size;

	// ガウシアン（一回目）
	cv::GaussianBlur(img, smt1, cv::Size(gparam1*2+1, gparam1*2+1), 0);
	// 二値化（一回目）
//	cv::threshold( smt1, bin1, thr1, 255, cv::THRESH_BINARY );
	cv::adaptiveThreshold( smt1, bin1, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_ADAPTIVE_THRESH_MEAN_C, adp_block*2+1, adp_sub );
	// ガウシアン（二回目）
	cv::GaussianBlur(bin1, smt2, cv::Size(gparam2*2+1, gparam2*2+1), 0);
	// 二値化（二回目）
	cv::threshold( smt2, bin2, thr, 255, cv::THRESH_BINARY );
	// 反転（特徴点抽出のため）
	cv::bitwise_not( bin2, inv );
	// 連結画像からの特徴点抽出
	if ( pfs != NULL )	free( pfs );
	num = MakeFloatCentresFromImage( &pfs, inv, &size, NULL );
	// 特徴点の描画
	cvSet( pt, CV_RGB( 255, 255, 255 ), NULL );
	cvCopy( img, overlap, 0 );
	for ( i = 0; i < num; i++ ) {
		point.x = (int)pfs[i].x;
		point.y = (int)pfs[i].y;
		cv::circle( pt, point, kTuneRegPtRadius, CV_RGB( 255, 0, 0 ), -1, cv::LINE_AA, 0 );
		cv::circle( overlap, point, kTuneRegPtRadius, CV_RGB( 255, 0, 0 ), -1, cv::LINE_AA, 0 );
	}

	return 1;
}

int cFpTuneReg::DrawCor( int pcor[][2], cv::Point2f *corpfs, cv::Point cororig )
// 対応関係を描画
{
	cv::Point p1, p2;

	cvCopy( img, cor, 0 );

	if ( pfs == NULL || corpfs == NULL )	return 0;
	p1.x = (int)pfs[0].x;
	p1.y = (int)pfs[0].y;
	p2.x = cororig.x + (int)corpfs[0].x;
	p2.y = cororig.y + (int)corpfs[0].y;

	cv::line( cor, p1, p2, CV_RGB( 0, 0, 0 ), 2, cv::LINE_AA, 0 );

	return 1;
}
