#include <stdio.h>
#include <math.h>
#include <cv.h>			// OpneCV
#include <highgui.h>	// OpenCV
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

cFpTuneCap::cFpTuneCap( IplImage *cap, int def_adp_block, int def_adp_sub, int def_gparam, int def_thr )
{
	CvSize size;

	img = cap;
	size.width = img->width;
	size.height = img->height;
	gray = cvCreateImage( size, 8, 1 );
	bin1 = cvCreateImage( size, 8, 1 );
	smt = cvCreateImage( size, 8, 1 );
	bin2 = cvCreateImage( size, 8, 1 );
	inv = cvCreateImage( size, 8, 1 );
	pt = cvCreateImage( size, 8, 1 );
	overlap = cvCreateImage( size, 8, 3 );
	cor = cvCreateImage( size, 8, 3 );
	adp_block = def_adp_block;
	adp_sub = def_adp_sub;
	gparam = def_gparam;
	thr = def_thr;
	pfs = NULL;
	// コンストラクト時に特徴点抽出はしない
}

cFpTuneCap::~cFpTuneCap(void)
{
	cvReleaseImage( &gray );
	cvReleaseImage( &bin1 );
	cvReleaseImage( &smt );
	cvReleaseImage( &bin2 );
	cvReleaseImage( &inv );
	cvReleaseImage( &pt );
	cvReleaseImage( &overlap );
	cvReleaseImage( &cor );
	if ( pfs != NULL )	free( pfs );
}

int cFpTuneCap::Extract( void )
// キャプチャ画像から特徴点抽出
{
	int i, num;
	CvPoint point;
	CvSize size;

	// グレイスケールに変換
	cvCvtColor( img, gray, CV_BGR2GRAY );
	// 適応二値化
	cvAdaptiveThreshold( gray, bin1, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, adp_block*2+1, adp_sub );
	// ガウシアン
	cvSmooth( bin1, smt, CV_GAUSSIAN, gparam*2+1, 0, 0 );
	// 二値化
	cvThreshold( smt, bin2, thr, 255, CV_THRESH_BINARY );
	cvNot( bin2, inv );
	// 特徴点抽出
	if ( pfs != NULL )	free( pfs );
	num = MakeFloatCentresFromImage( &pfs, inv, &size, NULL );
	// 特徴点の描画
	cvSet( pt, CV_RGB( 255, 255, 255 ), NULL );
	cvCopy( img, overlap, 0 );
	for ( i = 0; i < num; i++ ) {
		point.x = (int)pfs[i].x;
		point.y = (int)pfs[i].y;
		cvCircle( pt, point, kTuneCapPtRadius, CV_RGB( 255, 0, 0 ), -1, CV_AA, 0 );
		cvCircle( overlap, point, kTuneCapPtRadius, CV_RGB( 255, 0, 0 ), -1, CV_AA, 0 );
	}

	return 1;
}

int cFpTuneCap::DrawCor( int pcor[][2], CvPoint2D32f *corpfs, CvPoint cororig )
// 対応関係を描画
{
	CvPoint p1, p2;

	cvCopy( img, cor, 0 );

	if ( pfs == NULL || corpfs == NULL )	return 0;
	p1.x = (int)pfs[0].x;
	p1.y = (int)pfs[0].y;
	p2.x = cororig.x + (int)corpfs[0].x;
	p2.y = cororig.y + (int)corpfs[0].y;

	cvLine( cor, p1, p2, CV_RGB( 0, 0, 255 ), 2, CV_AA, 0 );

	return 1;
}
