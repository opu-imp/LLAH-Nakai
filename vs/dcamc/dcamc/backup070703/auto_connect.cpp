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
// fname���猋���摜���쐬����
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
		fprintf(stderr, "�摜�t�@�C��%s���J���܂���\n", fname);
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

	if ( mode == RETRIEVE_MODE ) {	// �������[�h�F�Ƃ肠����2�l��
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cvNot(bin, inv);	// CBDAR�Ȍ�ɒǉ������m�C�Y��������
//		OutPutImage( inv );
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
		for( con0 = contours; con0 != 0; con0 = con0->h_next ) {		// �A��������`��
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < 10 ) continue;	// ����������A�������͏��O
#ifdef	WIN
			cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8, cvPoint(0,0) );	// replace CV_FILLED with 1 to see the outlines 
#else
			cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8);	// replace CV_FILLED with 1 to see the outlines 

#endif
	    }
		cvClearSeq( contours );
//		OutPutImage( smt );
		cvNot(smt, bin);
	} else if ( mode == RET_MP_MODE ) {	// �g�ѓd�b�t���J�������[�h�F���T�C�Y���Ă���2�l��
		cvResize( img, smt, CV_INTER_CUBIC );
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
	} else {	// �n�b�V���\�z���[�h�i�d�q�����j�F�A�����������o���ĕ����T�C�Y�𒲂ׂĂ��當���𑾂点��
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
//		OutPutImage( bin );
		cvNot( bin, inv );
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
		InitHist( &hist, 100, 0, 1000 );
	    for( con0 = contours; con0 != 0 ; con0 = con0->h_next )
	    {
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 <= 0.5 ) continue;	// ����������A�������͏��O
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
	// �����̑傫���𒲂ׂ�
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
	// �A��������`��E�d�S���v�Z
	InitHist( &hist, 100, 0, 1000 );
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// ����������A�������͏��O
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
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����

	// �A��������`��E�d�S���v�Z
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		CvPoint p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O
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
// �P��̎��������e�X�g
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
		fprintf(stderr, "�摜�t�@�C��%s���J���܂���\n", fname);
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
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
	InitHist( &hist, 100, 0, 1000 );
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// ����������A�������͏��O
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
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����

	// �_�̐����J�E���g�i�b��j
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	num = i;

	// �A��������`��E�d�S���v�Z
	InitHist( &hist, 100, 0, 1000 );
//	InitHist( &hist, 200, 0, 20 );
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;

		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// ����������A�������͏��O
//		printf("%f\n", d00);
		AddDataHist( &hist, sqrt(d00) );
		i++;
    }
	num = i;	// �A���������̍X�V

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
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����

	// �A��������`��E�d�S���v�Z
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
//		CvScalar color;

		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O

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

	// �摜�̍쐬
	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
//	point = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// �J�n�ʒu�����킹��i�Ȃ��Ə㉺���]����j
//	gray->origin = src->origin;
//	bin->origin = src->origin;
//	inv->origin = src->origin;
//	point->origin = src->origin;
	// �O���C�X�P�[���ɕϊ�
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// �K����l��
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
//	cvNot( bin, inv );
	cvSmooth( bin, gray, CV_GAUSSIAN, 7, 0, 0 );
//	cvSaveImage( "smooth.bmp", gray );
	cvThreshold( gray, bin, 200, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );
	// �A�������̏d�S���v�Z
	/*
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
		CvPoint p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O
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


	// �O���C�X�P�[���ɕϊ�
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// �K����l��
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
	// �K�E�V�A��
	cvSmooth( bin, smt, CV_GAUSSIAN, eJpGParam*2+1, 0, 0 );
	// ��l��
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

	// �摜�̍쐬
	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// �O���C�X�P�[���ɕϊ�
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// �K����l��
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


	// �O���C�X�P�[���ɕϊ�
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// �K����l��
//	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eJpAdpBlock*2+1, eJpAdpSub );
	// �K�E�V�A��
//	cvSmooth( bin, smt, CV_GAUSSIAN, eJpGParam*2+1, 0, 0 );
	// ��l��
	cvThreshold( gray, bin, eJpThr, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );

	cvReleaseImage( &gray );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

	return inv;
}

IplImage *GetConnectedImageReg( IplImage *img )
// src���猋���摜���쐬����
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
