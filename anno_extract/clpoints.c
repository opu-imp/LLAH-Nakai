#include <stdio.h>

#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "clpoints.h"
#include "proctime.h"

void ColorClusteringAndMakePoints( IplImage *src, int cluster_count, int mode, int *nums, CvPoint **clps, double avgs[][3], char *dir, char *base )
// �F�N���X�^�����O���ĘA�������̏d�S�����߂�
{
	CvScalar color_tab[kMaxClusters];
	IplImage *sml, *smt, *climgs[kMaxClusters], *climg, *climgs2[kMaxClusters];
	IplImage *dst_small;
	int i, j, sample_count, w, h, wstep, cl, clcnt[kMaxClusters], start;
	int i_small, j_small, sample_count_small, w_small, h_small, wstep_smt;
	char fname[kMaxPathLen];
	double clsum[kMaxClusters][3], d00;
	CvMat *points_small, *clusters_small;

    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
#ifdef CL_OUTPUT_IMAGE
	color_tab[0] = CV_RGB(255,0,0);
	color_tab[1] = CV_RGB(0,255,0);
	color_tab[2] = CV_RGB(0,0,255);
	color_tab[3] = CV_RGB(255,0,255);
	color_tab[4] = CV_RGB(255,255,0);
	color_tab[5] = CV_RGB(0,255,255);
	color_tab[6] = CV_RGB(128,0,0);
	color_tab[7] = CV_RGB(0,128,0);
	color_tab[8] = CV_RGB(0,0,128);
	color_tab[9] = CV_RGB(128,0,128);
#endif

//	sml = src;
	// �摜�̏k��
	sml = cvCreateImage( cvSize( src->width * eClScale, src->height * eClScale ), IPL_DEPTH_8U, 3 );
	cvResize( src, sml, CV_INTER_NN );
	w = sml->width;
	h = sml->height;
	// �_�̐��̎Z�o
	sample_count = w * h;
	// �摜�̃X���[�W���O�ierode�j
	start = GetProcTimeMiliSec();
	smt = cvCreateImage( cvGetSize( sml ), 8, 3 );
//	cvSmooth( sml, smt, CV_GAUSSIAN, 7, 0, 0 );
	cvErode( sml, smt, NULL, eErodeClIter );
	printf("Smoothing image: %d msec\n", GetProcTimeMiliSec() - start);
	// �N���X�^�����O�E��U�k����
	start = GetProcTimeMiliSec();
	w_small = (int)((double)w / (double)eClCmpStep + 0.9999);
	h_small = (int)((double)h / (double)eClCmpStep + 0.9999);
	sample_count_small = w_small * h_small;
	points_small = cvCreateMat( sample_count_small, 1, CV_32FC3 );
	clusters_small = cvCreateMat( sample_count_small, 1, CV_32SC1 );
	wstep = smt->widthStep;
	// eClCmpStep���ƂɃf�[�^�𒊏o���Apoints_small�ɐݒ肷��
	// 1/eClCmpStep�ɏk�����ꂽ�摜�̃N���X�^�����O�Ɠ����̏���
	for ( j = 0, j_small = 0; j < h; j+=eClCmpStep, j_small++ ) {
		for ( i = 0, i_small = 0; i < w; i+=eClCmpStep, i_small++ ) {
			points_small->data.fl[(w_small*j_small+i_small)*3] = (unsigned char)smt->imageData[(wstep*j)+i*3];
			points_small->data.fl[(w_small*j_small+i_small)*3+1] = (unsigned char)smt->imageData[(wstep*j)+i*3+1];
			points_small->data.fl[(w_small*j_small+i_small)*3+2] = (unsigned char)smt->imageData[(wstep*j)+i*3+2];
		}
	}
	// �N���X�^�����O���s��
	cvKMeans2( points_small, cluster_count, clusters_small, cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, eKMeansMaxIter, eKMeansEpsilon ));
	printf("Clustering: %d msec\n", GetProcTimeMiliSec() - start );
#ifdef	CL_OUTPUT_IMAGE	// �N���X�^�����O���ʂ̕\��
//	fprintf( stderr, "Drawing result of clustering...\n" );
	dst_small = cvCreateImage( cvSize( w_small, h_small ), 8, 3 );
	cvZero( dst_small );
	
	wstep = dst_small->widthStep;
	for ( j = 0; j < h_small; j++ ) {
		for ( i = 0; i < w_small; i++ ) {
			dst_small->imageData[wstep*j+i*3] = (unsigned char)(color_tab[clusters_small->data.i[w_small*j+i]].val[0]);
			dst_small->imageData[wstep*j+i*3+1] = (unsigned char)(color_tab[clusters_small->data.i[w_small*j+i]].val[1]);
			dst_small->imageData[wstep*j+i*3+2] = (unsigned char)(color_tab[clusters_small->data.i[w_small*j+i]].val[2]);
		}
	}
	sprintf( fname, "%s%s01acl.jpg", dir, base );
	cvSaveImage( fname, dst_small );
	cvReleaseImage( &dst_small );

#endif

	
	// �N���X�^���Ƃ̕��ϒl�����߂�
//	fprintf( stderr, "Calculating averages...\n" );
	start = GetProcTimeMiliSec();
	for ( i = 0; i < cluster_count; i++ ) {
		// ���ϒl�̏�����
		clcnt[i] = 0;
		clsum[i][0] = 0.0;
		clsum[i][1] = 0.0;
		clsum[i][2] = 0.0;
	}
	for ( j = 0; j < h_small; j++ ) {
		for ( i = 0; i < w_small; i++ ) {
			cl = clusters_small->data.i[w_small*j+i];
			wstep = smt->widthStep;
			clsum[cl][0] += (unsigned char)smt->imageData[(wstep*j*kClCmpStep)+i*kClCmpStep*3];
			clsum[cl][1] += (unsigned char)smt->imageData[(wstep*j*kClCmpStep)+i*kClCmpStep*3+1];
			clsum[cl][2] += (unsigned char)smt->imageData[(wstep*j*kClCmpStep)+i*kClCmpStep*3+2];
			clcnt[cl]++;
		}
	}
	for ( i = 0; i < cluster_count; i++ ) {
		avgs[i][0] = clsum[i][0] / clcnt[i];
		avgs[i][1] = clsum[i][1] / clcnt[i];
		avgs[i][2] = clsum[i][2] / clcnt[i];
//		printf("cluster %d: (%f, %f, %f)\n", i, avgs[i][0], avgs[i][1], avgs[i][2] );
	}
	printf("Calculating avg: %d msec\n", GetProcTimeMiliSec() - start);

#ifdef	CL_OUTPUT_IMAGE	// �N���X�^�����O���ʂ̕\���i���ϒl�o�[�W�����j
//	fprintf( stderr, "Drawing result of clustering...\n" );
	dst_small = cvCreateImage( cvSize( w_small, h_small ), 8, 3 );
	cvZero( dst_small );
	
	wstep = dst_small->widthStep;
	for ( j = 0; j < h_small; j++ ) {
		for ( i = 0; i < w_small; i++ ) {
			dst_small->imageData[wstep*j+i*3] = (unsigned char)avgs[clusters_small->data.i[w_small*j+i]][0];
			dst_small->imageData[wstep*j+i*3+1] = (unsigned char)avgs[clusters_small->data.i[w_small*j+i]][1];
			dst_small->imageData[wstep*j+i*3+2] = (unsigned char)avgs[clusters_small->data.i[w_small*j+i]][2];
		}
	}
	sprintf( fname, "%s%s01aclavg.jpg", dir, base );
	cvSaveImage( fname, dst_small );
	cvReleaseImage( &dst_small );
#endif

	// �ł��߂����ϒl�̃N���X�^�Ɋ��蓖�Ă邱�ƂŃN���X�^���Ƃ̉摜���쐬
//	fprintf( stderr, "Making images of clusters...\n" );
	start = GetProcTimeMiliSec();
	for ( i = 0; i < cluster_count; i++ ) {
		climgs[i] = cvCreateImage( cvGetSize( sml ), 8, 1 );
		cvZero( climgs[i] );
	}
	wstep_smt = smt->widthStep;
	for ( j = 0; j < h; j++ ) {
		for ( i = 0; i < w; i++ ) {
//			climg = climgs[clusters->data.i[w*j+i]];
			climg = climgs[NearestCluster((unsigned char *)&smt->imageData[wstep_smt*j+i*3], avgs, cluster_count)];
			wstep = climg->widthStep;
			climg->imageData[wstep*j+i] = 255;
			
		}
	}
	printf("Making images of clusters: %d msec\n", GetProcTimeMiliSec() - start);

#ifdef	CL_OUTPUT_IMAGE	// �N���X�^���Ƃ̉摜���o��
	for ( i = 0; i < cluster_count; i++ ) {
		IplImage *cland = cvCreateImage( cvGetSize( sml ), 8, 3 );
		IplImage *clmask = cvCreateImage( cvGetSize( sml ), 8, 3 );
		IplImage *clnot = cvCreateImage( cvGetSize( sml ), 8, 3 );
		cvNot( sml, clnot );	// ���摜�𔽓]
		cvCvtColor( climgs[i], clmask, CV_GRAY2RGB );	// �}�X�N���쐬
		cvZero( cland );	// AND�摜�̏�����
		cvAnd( clnot, clmask, cland, 0 );	// AND
		cvNot( cland, clnot );	// ���]
		sprintf( fname, "%s%s02cl%d.jpg", dir, base, i );
		cvSaveImage( fname, climgs[i] );
		sprintf( fname, "%s%s02cland%d.jpg", dir, base, i );
		cvSaveImage( fname, clnot );
		cvReleaseImage( &cland );
		cvReleaseImage( &clmask );
		cvReleaseImage( &clnot );
	}
#endif
	// ���������� or �傫�߂���A������������
//	fprintf( stderr, "Cleaning cluster images...\n" );
	start = GetProcTimeMiliSec();
#ifdef	CL_OUTPUT_IMAGE
	for ( i = 0; i < cluster_count; i++ ) {
		climgs2[i] = cvCreateImage( cvGetSize( sml ), 8, 1 );
		cvZero( climgs2[i] );
	}
#endif
	for ( i = 0; i < cluster_count; i++ ) {
		// �܂��͐��𐔂���
		nums[i] = 0;
	    cvFindContours( climgs[i], storage, &contours, sizeof(CvContour),
			CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
		for ( con0 = contours; con0 != 0; con0 = con0->h_next ) {
			cvMoments( con0, &mom, 1 );
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < eMinAreaCl || d00 > eMaxAreaCl ) continue;	// ����������A�������͏��O
			nums[i]++;
		}
		clps[i] = (CvPoint *)calloc( nums[i], sizeof(CvPoint) );
		for ( j=0, con0 = contours; con0 != 0; con0 = con0->h_next ) {
			cvMoments( con0, &mom, 1 );
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < eMinAreaCl || d00 > eMaxAreaCl ) continue;	// ����������A�������͏��O
			clps[i][j].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00 / eClScale );	// �����_�̍��W�̓��T�C�Y�O�̂��̂ɂ���
			clps[i][j].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00 / eClScale );
			j++;
#ifdef	CL_OUTPUT_IMAGE
			cvDrawContours( climgs2[i], con0, cWhite, cGreen, kDrawContourMaxLevel, CV_FILLED, kDrawContourLineType);	// replace CV_FILLED with 1 to see the outlines
#endif
		}
#ifdef	CL_OUTPUT_IMAGE
		sprintf( fname, "%s%s03con%d.jpg", dir, base, i );
		cvSaveImage( fname, climgs2[i] );
#endif
	}
	printf("Cleaning & making points: %d msec\n", GetProcTimeMiliSec() - start );
}

int NearestCluster( unsigned char *pixels, double avgs[][3], int cluster_count )
// �ł��߂��N���X�^��T��
{
	int i, j, min_cl;
	double dist, min_dist, diff;
	
	min_cl = -1;
	min_dist = kMaxColorDist;
	for ( i = 0; i < cluster_count; i++ ) {
		dist = 0;
		for ( j = 0; j < 3; j++ ) {
			diff = (double)pixels[j] - avgs[i][j];
			dist += diff * diff;
		}
		if ( dist < min_dist ) {
			min_dist = dist;
			min_cl = i;
		}
	}
	return min_cl;
}

int SaveClusterPoints( char *fname, int cluster_count, int *nums, CvPoint **clps, double avgs[][3] )
// �N���X�^���Ƃ̓_�f�[�^��ۑ�
{
	int i, j;
	FILE *fp;
	
	if ( ( fp = fopen( fname, "w" ) ) == NULL ) {
		fprintf( stderr, "Error: %s cannot open\n", fname );
		return 0;
	}
	fprintf( fp, "%d\n", cluster_count );
	for ( i = 0; i < cluster_count; i++ ) {
		fprintf( fp, "%lf %lf %lf\n", avgs[i][0], avgs[i][1], avgs[i][2] );
		fprintf( fp, "%d\n", nums[i] );
		for ( j = 0; j < nums[i]; j++ ) {
			fprintf( fp, "%d,%d\n", clps[i][j].x, clps[i][j].y );
		}
	}
	fclose( fp );
	return 1;
}

int LoadClusterPoints( char *fname, int *cluster_count, int *nums, CvPoint **clps, double avgs[][3] )
// �N���X�^���Ƃ̓_�f�[�^��ǂݍ���
{
	int i, j;
	char line[kMaxLineLen];
	FILE *fp;
	
	if ( ( fp = fopen( fname, "r" ) ) == NULL ) {
		fprintf( stderr, "Error: %s cannot open\n", fname );
		return 0;
	}
	fgets( line, kMaxLineLen, fp );
	sscanf( line, "%d", cluster_count );
	for ( i = 0; i < *cluster_count; i++ ) {
		fgets( line, kMaxLineLen, fp );
		sscanf( line, "%lf %lf %lf", &avgs[i][0], &avgs[i][1], &avgs[i][2] );
		fgets( line, kMaxLineLen, fp );
		sscanf( line, "%d", &nums[i] );
		clps[i] = (CvPoint *)calloc( nums[i], sizeof(CvPoint) );
		for ( j = 0; j < nums[i]; j++ ) {
			fgets( line, kMaxLineLen, fp );
			sscanf( line, "%d,%d", &(clps[i][j].x), &(clps[i][j].y) );
		}
	}
	fclose( fp );
	return 1;
}

void SavePointFile( char *fname, CvPoint *ps, int num, CvSize size )
// �_�f�[�^��ۑ�����
{
	int i;
	FILE *fp;

	fp = fopen(fname, "w");
	fprintf(fp, "%d,%d\n", size.width, size.height);	// �擪�ɃT�C�Y��ۑ�����
	for ( i = 0; i < num; i++ )
		fprintf(fp, "%d,%d\n", ps[i].x, ps[i].y);
	fclose(fp);
}

