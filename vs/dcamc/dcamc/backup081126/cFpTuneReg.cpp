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
#include "cFpTuneReg.h"

cFpTuneReg::cFpTuneReg( char *fname, int def_gparam1, int def_adp_block, int def_adp_sub, int def_gparam2, int def_thr )
{
	CvSize size;

	if ( (img = cvLoadImage(fname, 0)) == NULL ) {
		fprintf(stderr, "画像ファイル%sが開けません\n", fname);
		return;
	}
	size.width = img->width;
	size.height = img->height;
	smt1 = cvCreateImage( size, 8, 1 );
	bin1 = cvCreateImage( size, 8, 1 );
	smt2 = cvCreateImage( size, 8, 1 );
	bin2 = cvCreateImage( size, 8, 1 );
	inv = cvCreateImage( size, 8, 1 );
	pt  = cvCreateImage( size, 8, 1 );
	overlap = cvCreateImage( size, 8, 1 );
	cor = cvCreateImage( size, 8, 1 );

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
{
	cvReleaseImage( &smt1 );
	cvReleaseImage( &bin1 );
	cvReleaseImage( &smt2 );
	cvReleaseImage( &bin2 );
	cvReleaseImage( &inv );
	cvReleaseImage( &pt );
	cvReleaseImage( &overlap );
	cvReleaseImage( &cor );
	if ( pfs != NULL )	free( pfs );
}

int cFpTuneReg::Extract( void )
{
	int i, num;
	CvPoint point;
	CvSize size;

	// ガウシアン（一回目）
	cvSmooth( img, smt1, CV_GAUSSIAN, gparam1*2+1, 0, 0 );
	// 二値化（一回目）
//	cvThreshold( smt1, bin1, thr1, 255, CV_THRESH_BINARY );
	cvAdaptiveThreshold( smt1, bin1, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_ADAPTIVE_THRESH_MEAN_C, adp_block*2+1, adp_sub );
	// ガウシアン（二回目）
	cvSmooth( bin1, smt2, CV_GAUSSIAN, gparam2*2+1, 0, 0 );
	// 二値化（二回目）
	cvThreshold( smt2, bin2, thr, 255, CV_THRESH_BINARY );
	// 反転（特徴点抽出のため）
	cvNot( bin2, inv );
	// 連結画像からの特徴点抽出
	if ( pfs != NULL )	free( pfs );
	num = MakeFloatCentresFromImage( &pfs, inv, &size, NULL );
	// 特徴点の描画
	cvSet( pt, CV_RGB( 255, 255, 255 ), NULL );
	cvCopy( img, overlap, 0 );
	for ( i = 0; i < num; i++ ) {
		point.x = (int)pfs[i].x;
		point.y = (int)pfs[i].y;
		cvCircle( pt, point, kTuneRegPtRadius, CV_RGB( 255, 0, 0 ), -1, CV_AA, 0 );
		cvCircle( overlap, point, kTuneRegPtRadius, CV_RGB( 255, 0, 0 ), -1, CV_AA, 0 );
	}

	return 1;
}

int cFpTuneReg::DrawCor( int pcor[][2], CvPoint2D32f *corpfs, CvPoint cororig )
// 対応関係を描画
{
	CvPoint p1, p2;

	cvCopy( img, cor, 0 );

	if ( pfs == NULL || corpfs == NULL )	return 0;
	p1.x = (int)pfs[0].x;
	p1.y = (int)pfs[0].y;
	p2.x = cororig.x + (int)corpfs[0].x;
	p2.y = cororig.y + (int)corpfs[0].y;

	cvLine( cor, p1, p2, CV_RGB( 0, 0, 0 ), 2, CV_AA, 0 );

	return 1;
}
