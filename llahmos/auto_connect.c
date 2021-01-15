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
	if ( eIsJp ) {	// ���{�ꃂ�[�h�͕ʏ���
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
		if ( contours != NULL )	cvClearSeq( contours );
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
		cvSmooth( img, smt, CV_GAUSSIAN, chsize, 0, 0, 0);
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 201, 10 );
//		OutPutImage( bin );
		if ( contours != NULL )	cvClearSeq( contours );
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
	if ( mode == CONST_HASH_MODE || mode == ADD_HASH_MODE ) {	// PDF���瓾���摜
		if ( eIsJp )	chsize = (int)((double)chsize * 0.5);	// ���{�ꃂ�[�h
		else			chsize *= 2;
	}
	else if ( mode == RET_MP_MODE )	{	// �B�e�摜
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
// fname���猋���摜���쐬����2
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

	if ( mode == RETRIEVE_MODE ) {	// �������[�h
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 7 );
//		OutPutImage( bin );
		cvNot(bin, inv);	// CBDAR�Ȍ�ɒǉ������m�C�Y��������
//		OutPutImage( inv );
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
		for( con0 = contours; con0 != 0; con0 = con0->h_next ) {		// �A��������`��
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < 70 ) continue;	// ����������A�������͏��O
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
		// �����̑傫���𒲂ׂ�
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
		// �A��������`��E�d�S���v�Z
		InitHist( &hist, 1000, 0, 1000 );
	    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
	    {
			cvMoments(con0, &mom, 1);
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 <= 0.5 ) continue;	// ����������A�������͏��O
			AddDataHist( &hist, sqrt(d00) );
	    }

		chsize = (int)GetMaxBin(&hist);
		ReleaseHist( &hist );
//		chsize += (chsize + 1) % 2;
//		chsize = chsize * 2 + 1;	// 12/21�ύX�@���𑜓x�J������p�����g��B�e�ł̌��ʂ𐸍�����
		chsize = chsize * 2 + 1;	// 08/05/16�ύX�@���U�C�L���O�̏ꍇ
//		printf( "chsize : %d\n", chsize );
		cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0, 0 );	// OpenCV 1.0
//		OutPutImage( smt );
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 4 );
//		OutPutImage( bin );
		cvNot( bin, inv );
	} else {	// �n�b�V���\�z���[�h�i�d�q�����j�F臒l�Œ胂�[�h
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
// ���{�ꃂ�[�h
{
	IplImage *smt, *bin;
	CvSize size;

	size.width = src->width;
	size.height = src->height;

	smt = cvCreateImage( size, 8, 1 );
	bin = cvCreateImage( size, 8, 1 );

	// �K�E�V�A���i���ځj
	cvSmooth( src, smt, CV_GAUSSIAN, eJpGParam1*2+1, 0, 0, 0 );
	// ��l���i���ځj
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_ADAPTIVE_THRESH_MEAN_C, eJpAdpBlock*2+1, eJpAdpSub );
	// �K�E�V�A���i���ځj
	cvSmooth( bin, smt, CV_GAUSSIAN, eJpGParam2*2+1, 0, 0, 0 );
	// ��l���i���ځj
	cvThreshold( smt, bin, eJpThr, 255, CV_THRESH_BINARY );
	// ���]�i�����_���o�̂��߁j
	cvNot( bin, dst );

	cvReleaseImage( &smt );
	cvReleaseImage( &bin );
}

IplImage *GetConnectedImageJp2( const char *fname, int mode )
// �����_���o�i���{�ꃂ�[�h�j��͉̂��������H
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

	if ( mode == RETRIEVE_MODE ) {	// �������[�h
		// �K����l��
		cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 12 );
		cvSmooth( bin, smt, CV_GAUSSIAN, 3/* 1 */, 0, 0, 0 );	// 1�͉𑜓x���Ⴂ�ꍇ
		cvThreshold( smt, bin, 200, 255, CV_THRESH_BINARY );
		cvNot( bin, inv );
	}
	else { // �n�b�V���\�z���[�h
		// �K�E�V�A��
//		cvSmooth( img, smt, CV_GAUSSIAN, 7, 0, 0, 0 );	// OpenCV 1.0
		// 07/07/24
		cvSmooth( img, smt, CV_GAUSSIAN, 3, 0, 0, 0 );	// OpenCV 1.0
		// 2�l��
		cvThreshold( smt, bin, 228, 255, CV_THRESH_BINARY );
		// ���]
		cvNot( bin, inv );
	}
	// �J��
	cvReleaseImage( &img );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

//	OutPutImage( inv );

	return inv;
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
	cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0, 0 );
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
	static IplImage *gray = NULL, *bin = NULL;
	IplImage *inv;

#ifdef	WIN

#ifdef	RESO_LIMIT
	if ( src->width * src->height > 1280 * 960 ) {
		fprintf( stderr, "error: resolution of query image is limited to 1.3 mega pixels\n" );
		return NULL;
	}
#endif
	// �摜�̍쐬
	if ( gray == NULL )
		gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	if ( bin == NULL )
		bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// �O���C�X�P�[���ɕϊ�
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// �K����l��
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
	// �摜�̍쐬
	gray = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
	// �O���C�X�P�[���ɕϊ�
	cvCvtColor( src, gray, CV_BGR2GRAY );
	// �K����l��
//	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 12 );
	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 141, 12 );	// MIRU07 ���ʂ�΍�
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
