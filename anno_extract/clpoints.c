#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <vector>

#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "clpoints.h"
#include "proctime.h"

void ColorClusteringAndMakePoints( cv::Mat src, int cluster_count, int mode, int *nums, cv::Point **clps, double avgs[][3], char *dir, char *base )
// �F�N���X�^�����O���ĘA�������̏d�S�����߂�
{
	cv::Scalar color_tab[kMaxClusters];
	cv::Mat sml, smt, climgs[kMaxClusters], climg, climgs2[kMaxClusters];
	cv::Mat dst_small;
	int i, j, sample_count, w, h, wstep, cl, clcnt[kMaxClusters], start;
	int i_small, j_small, sample_count_small, w_small, h_small, wstep_smt;
	char fname[kMaxPathLen];
	double clsum[kMaxClusters][3], d00;
	cv::Mat points_small, clusters_small;

    std::vector<std::vector<cv::Point>> contours;


    std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
#ifdef CL_OUTPUT_IMAGE
	color_tab[0] = cv::Scalar(0,0,255);
	color_tab[1] = cv::Scalar(0,255,0);
	color_tab[2] = cv::Scalar(255,0,0);
	color_tab[3] = cv::Scalar(255,0,255);
	color_tab[4] = cv::Scalar(0,255,255);
	color_tab[5] = cv::Scalar(255,255,0);
	color_tab[6] = cv::Scalar(0,0,128);
	color_tab[7] = cv::Scalar(0,128,0);
	color_tab[8] = cv::Scalar(128,0,0);
	color_tab[9] = cv::Scalar(128,0,128);
#endif

//	sml = src;
	// �摜�̏k��
	sml = cv::Mat::zeros(cv::Size( src.cols * eClScale, src.rows * eClScale ), CV_8UC3);
	cv::resize( src, sml, sml.size(), 0, 0, cv::INTER_NEAREST );
	w = sml.cols;
	h = sml.rows;
	// �_�̐��̎Z�o
	sample_count = w * h;
	// �摜�̃X���[�W���O�ierode�j
	start = GetProcTimeMiliSec();
	smt = cv::Mat::zeros(sml.size(), CV_8UC3);
//	cv::GaussianBlur(sml, smt, cv::Size(7, 7), 0);
	cv::erode( sml, smt, cv::Mat(), cv::Point(-1,-1), eErodeClIter );
	printf("Smoothing image: %d msec\n", GetProcTimeMiliSec() - start);
	// �N���X�^�����O�E��U�k����
	start = GetProcTimeMiliSec();
	w_small = (int)((double)w / (double)eClCmpStep + 0.9999);
	h_small = (int)((double)h / (double)eClCmpStep + 0.9999);
	sample_count_small = w_small * h_small;
	points_small = cv::Mat(sample_count_small, 1, CV_32FC3 , cv::Scalar(0));
	clusters_small = cv::Mat(sample_count_small, 1, CV_32SC1 , cv::Scalar(0));
	wstep = (int)smt.step[0];
	// eClCmpStep���ƂɃf�[�^�𒊏o���Apoints_small�ɐݒ肷��
	// 1/eClCmpStep�ɏk�����ꂽ�摜�̃N���X�^�����O�Ɠ����̏���
	for ( j = 0, j_small = 0; j < h; j+=eClCmpStep, j_small++ ) {
		for ( i = 0, i_small = 0; i < w; i+=eClCmpStep, i_small++ ) {
			points_small.ptr<float>(0)[(w_small*j_small+i_small)*3] = (unsigned char)smt.data[(wstep*j)+i*3];
			points_small.ptr<float>(0)[(w_small*j_small+i_small)*3+1] = (unsigned char)smt.data[(wstep*j)+i*3+1];
			points_small.ptr<float>(0)[(w_small*j_small+i_small)*3+2] = (unsigned char)smt.data[(wstep*j)+i*3+2];
		}
	}
	// �N���X�^�����O���s��
	cv::kmeans( points_small, cluster_count, clusters_small, cv::TermCriteria( cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, eKMeansMaxIter, eKMeansEpsilon ), 10, cv::KMEANS_RANDOM_CENTERS );
	printf("Clustering: %d msec\n", GetProcTimeMiliSec() - start );
#ifdef	CL_OUTPUT_IMAGE	// �N���X�^�����O���ʂ̕\��
//	fprintf( stderr, "Drawing result of clustering...\n" );
	dst_small = cv::Mat::zeros(cv::Size( w_small, h_small ), CV_8UC3);
	dst_small.setTo(cv::Scalar::all(0));
	
	wstep = (int)dst_small.step[0];
	for ( j = 0; j < h_small; j++ ) {
		for ( i = 0; i < w_small; i++ ) {
			dst_small.data[wstep*j+i*3] = (unsigned char)(color_tab[clusters_small.at<int>(w_small*j+i)].val[0]);
			dst_small.data[wstep*j+i*3+1] = (unsigned char)(color_tab[clusters_small.at<int>(w_small*j+i)].val[1]);
			dst_small.data[wstep*j+i*3+2] = (unsigned char)(color_tab[clusters_small.at<int>(w_small*j+i)].val[2]);
		}
	}
	sprintf( fname, "%s%s01acl.jpg", dir, base );
	cv::imwrite( fname, dst_small );

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
			cl = clusters_small.at<int>(w_small*j+i);
			wstep = (int)smt.step[0];
			clsum[cl][0] += (unsigned char)smt.data[(wstep*j*kClCmpStep)+i*kClCmpStep*3];
			clsum[cl][1] += (unsigned char)smt.data[(wstep*j*kClCmpStep)+i*kClCmpStep*3+1];
			clsum[cl][2] += (unsigned char)smt.data[(wstep*j*kClCmpStep)+i*kClCmpStep*3+2];
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
	dst_small = cv::Mat::zeros(cv::Size( w_small, h_small ), CV_8UC3);
	dst_small.setTo(cv::Scalar::all(0));
	
	wstep = (int)dst_small.step[0];
	for ( j = 0; j < h_small; j++ ) {
		for ( i = 0; i < w_small; i++ ) {
			dst_small.data[wstep*j+i*3] = (unsigned char)avgs[clusters_small.at<int>(w_small*j+i)][0];
			dst_small.data[wstep*j+i*3+1] = (unsigned char)avgs[clusters_small.at<int>(w_small*j+i)][1];
			dst_small.data[wstep*j+i*3+2] = (unsigned char)avgs[clusters_small.at<int>(w_small*j+i)][2];
		}
	}
	sprintf( fname, "%s%s01aclavg.jpg", dir, base );
	cv::imwrite( fname, dst_small );
#endif

	// �ł��߂����ϒl�̃N���X�^�Ɋ��蓖�Ă邱�ƂŃN���X�^���Ƃ̉摜���쐬
//	fprintf( stderr, "Making images of clusters...\n" );
	start = GetProcTimeMiliSec();
	for ( i = 0; i < cluster_count; i++ ) {
		climgs[i] = cv::Mat::zeros(sml.size(), CV_8UC1);
		climgs[i].setTo(cv::Scalar::all(0));
	}
	wstep_smt = (int)smt.step[0];
	for ( j = 0; j < h; j++ ) {
		for ( i = 0; i < w; i++ ) {
//			climg = climgs[clusters->data.i[w*j+i]];
			climg = climgs[NearestCluster((unsigned char *)&smt.data[wstep_smt*j+i*3], avgs, cluster_count)];
			wstep = (int)climg.step[0];
			climg.data[wstep*j+i] = 255;
			
		}
	}
	printf("Making images of clusters: %d msec\n", GetProcTimeMiliSec() - start);

#ifdef	CL_OUTPUT_IMAGE	// �N���X�^���Ƃ̉摜���o��
	for ( i = 0; i < cluster_count; i++ ) {
		cv::Mat cland = cv::Mat::zeros(sml.size(), CV_8UC3);
		cv::Mat clmask = cv::Mat::zeros(sml.size(), CV_8UC3);
		cv::Mat clnot = cv::Mat::zeros(sml.size(), CV_8UC3);
		cv::bitwise_not( sml, clnot );	// ���摜�𔽓]
		cv::cvtColor( climgs[i], clmask, cv::COLOR_GRAY2BGR );	// �}�X�N���쐬
		cland.setTo(cv::Scalar::all(0));	// AND�摜�̏�����
		cv::bitwise_and( clnot, clmask, cland );	// AND
		cv::bitwise_not( cland, clnot );	// ���]
		sprintf( fname, "%s%s02cl%d.jpg", dir, base, i );
		cv::imwrite( fname, climgs[i] );
		sprintf( fname, "%s%s02cland%d.jpg", dir, base, i );
		cv::imwrite( fname, clnot );


	}
#endif
	// ���������� or �傫�߂���A������������
//	fprintf( stderr, "Cleaning cluster images...\n" );
	start = GetProcTimeMiliSec();
#ifdef	CL_OUTPUT_IMAGE
	for ( i = 0; i < cluster_count; i++ ) {
		climgs2[i] = cv::Mat::zeros(sml.size(), CV_8UC1);
		climgs2[i].setTo(cv::Scalar::all(0));
	}
#endif
	for ( i = 0; i < cluster_count; i++ ) {
		// �܂��͐��𐔂���
		nums[i] = 0;
	    cv::findContours(climgs[i], contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE );	// �A�������𒊏o����
		for( int _ci = FirstTopLevelContour( hierarchy ); _ci >= 0; _ci = hierarchy[_ci][0] ) {
			mom = cv::moments(contours[_ci]);
			d00 = mom.m00;
			if ( d00 < eMinAreaCl || d00 > eMaxAreaCl ) continue;	// ����������A�������͏��O
			nums[i]++;
		}
		clps[i] = (cv::Point *)calloc( nums[i], sizeof(cv::Point) );
		j = 0;
		for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ) {
			mom = cv::moments(contours[ci]);
			d00 = mom.m00;
			if ( d00 < eMinAreaCl || d00 > eMaxAreaCl ) continue;	// ����������A�������͏��O
			clps[i][j].x = (int)(mom.m10 / d00 / eClScale );	// �����_�̍��W�̓��T�C�Y�O�̂��̂ɂ���
			clps[i][j].y = (int)(mom.m01 / d00 / eClScale );
			j++;
#ifdef	CL_OUTPUT_IMAGE
			cv::drawContours( climgs2[i], contours, ci, cGreen, cv::FILLED, kDrawContourLineType );	// replace cv::FILLED with 1 to see the outlines
#endif
		}
#ifdef	CL_OUTPUT_IMAGE
		sprintf( fname, "%s%s03con%d.jpg", dir, base, i );
		cv::imwrite( fname, climgs2[i] );
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

int SaveClusterPoints( char *fname, int cluster_count, int *nums, cv::Point **clps, double avgs[][3] )
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

int LoadClusterPoints( char *fname, int *cluster_count, int *nums, cv::Point **clps, double avgs[][3] )
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
		clps[i] = (cv::Point *)calloc( nums[i], sizeof(cv::Point) );
		for ( j = 0; j < nums[i]; j++ ) {
			fgets( line, kMaxLineLen, fp );
			sscanf( line, "%d,%d", &(clps[i][j].x), &(clps[i][j].y) );
		}
	}
	fclose( fp );
	return 1;
}

void SavePointFile( char *fname, cv::Point *ps, int num, cv::Size size )
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

