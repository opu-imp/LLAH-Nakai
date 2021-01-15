#include "def_general.h"
#include <stdio.h>
#include <math.h>
#include "cv.h"
#include "highgui.h"

#include "f_points.h"
#include "dirs.h"

int MakeFeaturePointsFromImage(CvPoint **ps, IplImage *img, CvSize *size)
// 閉領域から特徴点を得る
{
	int i, num = 0;
    CvMemStorage* storage = cvCreateMemStorage(0);
    IplImage *bin, *p;
	CvSeq *contours = 0;

	size->width = img->width;
	size->height = img->height;

	bin = cvCreateImage( cvSize(size->width, size->height), 8, 1 );
//	cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 31, 10 );
	cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
	OutPutImage( bin );
	cvFloodFill( bin, cvPoint(300,300), cBlack, cvScalarAll(0), cvScalarAll(0), NULL, 4, NULL );
	OutPutImage( bin );
    cvFindContours( bin, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する

	num = CalcCentres(ps, contours, size, NULL);	// 各連結成分の重心を計算する

	p = cvCreateImage( cvSize(size->width, size->height), 8, 1 );
	for ( i = 0; i < num; i++ ) {
		cvCircle( p, (*ps)[i], 8, cWhite, -1, 8, 0 );
	}
//	printf("%d\n", num);
	OutPutImage( p );
	if ( contours != NULL )	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	return num;
}
