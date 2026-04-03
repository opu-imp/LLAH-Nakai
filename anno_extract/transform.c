#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "hash.h"
#include "annex.h"
#include "annex_st.h"
#include "clpoints.h"
#include "transform.h"
#include "disc.h"
#include "nn5j.h"
#include "estimate.h"
#include "nstr.h"
#include "proctime.h"
#include "nimg.h"

void OutputCor( cv::Mat anno, cv::Mat orig, cv::Point **clpsa, cv::Point **clpso, int *clcor, int pcors[kMaxClusters][kMaxPointNum], int cluster_count, int *numsa, char *basea, char *baseo );
void OutputCorCl( cv::Point **clpsa, cv::Point **clpso, int *numsa, int *numso, char *basea, char *baseo, int cluster_count, int *clcor, int pcors[kMaxClusters][kMaxPointNum] );

int TransformAnno( cv::Mat anno, cv::Mat orig, int cluster_count, double *param )
// anno��orig�ɕϊ����鑊���ϊ��p�����[�^�����߂�
{
	int i, j, numso[kMaxClusters], numsa[kMaxClusters], clcor[kMaxClusters], **nears, score[kMaxDocNum];
	int pcors[kMaxClusters][kMaxPointNum], cor[kMaxPointNum*kMaxClusters][4], cor_num = 0;
	int start, start_ret, ret_sum;
	cv::Point *clpso[kMaxClusters], *clpsa[kMaxClusters];
	double avgso[kMaxClusters][3], avgsa[kMaxClusters][3];
	char cmd[kMaxCmdLineLen], diro[kMaxPathLen], dira[kMaxPathLen], baseo[kFileNameLen], basea[kFileNameLen];
	char pfno[kMaxPathLen], pfna[kMaxPathLen], fname[kMaxPathLen];
	cv::Mat cor_img;
	cv::Size sizeo, sizea;
	cv::Point p1[3], p2[3];
//	strSimParam param;
	
	// ���摜�Ə������݉摜��������_�𒊏o�i���o�[�W�����j
/*	numo = MakeFeaturePointsFromColorImage( &pso, orig, FP_PDF );
	numa = MakeFeaturePointsFromColorImage( &psa, anno, FP_SCAN );
	OutPutPoints( pso, numo, orig.size() );
	OutPutPoints( psa, numa, anno.size() );
*/

	/************* ���摜�Ə������݉摜��������_�𒊏o *************/
//	sizeo = cv::Size( orig.cols * kClScale, orig.rows * kClScale );
//	sizea = cv::Size( anno.cols * kClScale, anno.rows * kClScale );
	sizeo = orig.size();
	sizea = anno.size();
	// �_�t�@�C�����̍쐬
	GetDir( eOrigFileName, kMaxPathLen, diro );
	GetDir( eAnnoFileName, kMaxPathLen, dira );
	GetBasename( eOrigFileName, kMaxPathLen, baseo );
	GetBasename( eAnnoFileName, kMaxPathLen, basea );
	sprintf( pfno, "%s%s.txt", eOutPutDir, baseo );
	sprintf( pfna, "%s%s.txt", eOutPutDir, basea );
	if ( IsExist( pfno ) ) {	// �_�t�@�C�������ɑ���
		LoadClusterPoints( pfno, &cluster_count, numso, clpso, avgso );
	}
	else {	// �F�N���X�^�����O���s���A�A�������̏d�S������_�Ƃ���
		ColorClusteringAndMakePoints( orig, cluster_count, FP_SCAN, numso, clpso, avgso, eOutPutDir, baseo );
		SaveClusterPoints( pfno, cluster_count, numso, clpso, avgso );
	}
	if ( IsExist( pfna ) ) {	// �_�t�@�C�������ɑ���
		LoadClusterPoints( pfna, &cluster_count, numsa, clpsa, avgsa );
	}
	else {	// �F�N���X�^�����O���s���A�A�������̏d�S������_�Ƃ���
		ColorClusteringAndMakePoints( anno, cluster_count, FP_SCAN, numsa, clpsa, avgsa, eOutPutDir,basea );
		SaveClusterPoints( pfna, cluster_count, numsa, clpsa, avgsa );
	}
	
	// �����_�̏o�͊m�F
#ifdef	CL_OUTPUT_IMAGE
	for ( i = 0; i < cluster_count; i++ ) {
		sprintf( fname, "%s%s04pt%d.jpg", eOutPutDir, baseo, i );
		OutPutPoints( fname, clpso[i], numso[i], sizeo );
		sprintf( fname, "%s%s04pt%d.jpg", eOutPutDir, basea, i );
		OutPutPoints( fname, clpsa[i], numsa[i], sizea );
	}
#endif
	// ���摜�A�������݉摜�̊e�N���X�^�̓����_���̏o��
	printf("original image\n");
	for ( i = 0; i < cluster_count; i++ ) {
		printf( "cluster %d : %d points\n", i, numso[i] );
	}
	printf("annotated image\n");
	for ( i = 0; i < cluster_count; i++ ) {
		printf( "cluster %d : %d points\n", i, numsa[i] );
	}
	// �N���X�^�̑Ή��֌W�̍쐬
	MakeClusterCor( cluster_count, avgsa, avgso, clcor );
	// �N���X�^�ԑΉ��֌W�̊m�F
/*	for ( i = 0; i < cluster_count; i++ ) {
		printf("%d(%lf, %lf, %lf) <-> %d(%lf, %lf, %lf)\n", i, avgsa[i][0], avgsa[i][1], avgsa[i][2], clcor[i], avgso[clcor[i]][0], avgso[clcor[i]][1], avgso[clcor[i]][2] );
	}
*/	/************* �N���X�^���ƂɑΉ��_�����߂� *************/
	start = GetProcTimeMiliSec();
	ret_sum = 0;
	for ( i = 0; i < cluster_count; i++ ) {	// i��anno���̔ԍ�
		// acr2��p���Č��摜�̓����_���痣�U���t�@�C�����쐬�ieAcr2Path��acr2���K�v�j
		SavePointFile( kTempPointFile, clpso[clcor[i]], numso[clcor[i]], sizeo );
		sprintf( cmd, "%s %c %d %d %d %s %s", eAcr2Path, kAcr2InvType, eGroup1Num, eGroup2Num, eDiscNum, eDiscFileName, kTempPointFile );
		system( cmd );
		// ���摜�̓����_����n�b�V�����\�z
		ConstructHashSub( clpso[clcor[i]], numso[clcor[i]] );
		// �������݉摜�̓����_����n�b�V��������
		start_ret = GetProcTimeMiliSec();
		MakeNearsFromCentres( clpsa[i], numsa[i], &nears );
		RetrieveNN5( clpsa[i], numsa[i], nears, sizea, score, pcors[i] );
		ret_sum += GetProcTimeMiliSec() - start_ret;
//		printf("%d\n", score[0] );
	}
	printf("Making point cor: %d msec\n", GetProcTimeMiliSec() - start );
	printf("(Retrieval: %d msec)\n", ret_sum );
	// �Ή��_�̏o�͊m�F
#ifdef	CL_OUTPUT_IMAGE
	cor_img = cv::Mat::zeros( cv::Size( sizeo.width + sizea.width, (sizeo.height > sizea.height) ? sizeo.height : sizea.height ), CV_8UC1 );
	for ( i = 0; i < cluster_count; i++ ) {
		sprintf( fname, "%s%s05cor%d.jpg", eOutPutDir, basea, i );
		cor_img.setTo(cv::Scalar::all(0));
		for ( j = 0; j < numsa[i]; j++ ) {
			if ( pcors[i][j] >= 0 ) {
				cv::line( cor_img, clpsa[i][j], cv::Point( clpso[clcor[i]][pcors[i][j]].x + sizea.width, clpso[clcor[i]][pcors[i][j]].y ), cWhite, kDrawCorLineThick, cv::LINE_AA, 0 );
			}
		}
	cv::imwrite( fname, cor_img );
	}
#endif
#ifdef	OUTPUT_COR_IMAGE
	OutputCor( anno, orig, clpsa, clpso, clcor, pcors, cluster_count, numsa, basea, baseo );
	OutputCorCl( clpsa, clpso, numsa, numso, basea, baseo , cluster_count, clcor, pcors );

#endif
	/************* �Ή��_�����ɑ����ϊ��p�����[�^�𐄒� *************/
	start = GetProcTimeMiliSec();
	// �Ή��֌W��cor�Ɋi�[����
	for ( i = 0; i < cluster_count; i++ ) {
		for ( j = 0; j < numsa[i]; j++ ) {
			if ( pcors[i][j] >= 0 ) {
				cor[cor_num][0] = i;
				cor[cor_num][1] = j;
				cor[cor_num][2] = clcor[i];
				cor[cor_num][3] = pcors[i][j];
				cor_num++;
			}
		}
	}
	// �Ή��_���瑊���ϊ��p�����[�^�𐄒肷��
	if ( cor_num < kMinCorNum ) {
		fprintf( stderr, "number of cor is too small, default param used\n" );
		// �Ή��_�����Ȃ�����ꍇ�̏���
		param[0] = 0.0;
		param[1] = 1.0;
		param[2] = 0.0;
		param[3] = 0.0;
	}
	else {
//		EstimateParam( clpsa, clpso, cor, cor_num, param, eTransMode );
		EstimateParamRANSAC( clpsa, clpso, cor, cor_num, param, eTransMode );
	}
//	for ( i = 0; i < 6; i++ ) {
//		printf("param[%d]: %lf\n", i, param[i]);
//	}
	printf("Estimate param: %d msec\n", GetProcTimeMiliSec() - start );
	return 1;
}

//void MakeClusterCor( int cluster_count, double avgso[][3], double avgsa[][3], int *clcor )
void MakeClusterCor( int cluster_count, double avgsa[][3], double avgso[][3], int *clcor )
// �N���X�^�̑Ή��֌W�̍쐬
{
#ifdef	NEW_COR	// �V�o�[�W�����i�s�����̉\�����j
	int i, j, k, pos, min_cora, min_coro;
	double dist, diff, min_dist;
	int *left_side, *used;
	int **cor_mat;
	double **dist_mat;
	
	// �Ή��E�����}�g���N�X�̏�����
	cor_mat = (int **)calloc( cluster_count, sizeof(int *) );
	dist_mat = (double **)calloc( cluster_count, sizeof(double *) );
	for ( i = 0; i < cluster_count; i++ ) {
		cor_mat[i] = (int *)calloc( cluster_count, sizeof(int) );
		dist_mat[i] = (double *)calloc( cluster_count, sizeof(double) );
	}
	// �Ή��E�����}�g���N�X�̍쐬
	for ( i = 0; i < cluster_count; i++ ) {
		for ( j = 0; j < cluster_count; j++ ) {
			// anno��i�N���X�^��orig��j�N���X�^�̋��������߂�
			dist = 0;
			for ( k = 0; k < 3; k++ ) {
				diff = avgsa[i][k] - avgso[j][k];
				dist += diff * diff;
			}
			// �����ꏊ��T��
			for ( k = 0, pos = 0; k < j; k++, pos++ ) {
				if ( dist < dist_mat[i][k] )	break;
			}
			// ���炷
			for ( k = j; k > pos; k-- ) {
				dist_mat[i][k] = dist_mat[i][k-1];
				cor_mat[i][k] = cor_mat[i][k-1];
			}
			// �����
			dist_mat[i][pos] = dist;
			cor_mat[i][pos] = j;
		}
	}
	// �Ή��E�����}�g���N�X�̏o��
/*	for ( i = 0; i < cluster_count; i++ ) {
		for ( j = 0; j < cluster_count; j++ ) {
			printf("%d(%.0lf), ", cor_mat[i][j], dist_mat[i][j]);
		}
		printf("\n");
	}
*/
	// �Ή��E�����}�g���N�X��p���čœK�ȑΉ��֌W�����߂�
	// ���[�s��̊m�ہE������
	left_side = (int *)calloc( cluster_count, sizeof(int) );
	for ( i = 0; i < cluster_count; i++ )	left_side[i] = 0;
	// �Ή��ςݍs��̊m��
	used = (int *)calloc( cluster_count, sizeof(int) );
	for ( i = 0; i < cluster_count; i++ ) {
		min_dist = 16777216;
		for ( j = 0; j < cluster_count; j++ ) {
			if ( left_side[j] < 0 )	continue;	// ���łɑΉ��Â��Ă���
			if ( dist_mat[j][left_side[j]] < min_dist ) {	// ���[�̋������ŏ�
				min_cora = j;
				min_coro = cor_mat[j][left_side[j]];
				min_dist = dist_mat[j][left_side[j]];
			}
		}
//		printf("%d <-> %d\n", min_cora, min_coro);
		clcor[min_cora] = min_coro;
		left_side[min_cora] = -1;
		used[min_coro] = 1;
		// �g�p�ςݍs��Ɋ�Â��č��[�s����X�V
		for ( j = 0; j < cluster_count; j++ ) {
			if ( left_side[j] < 0 )	continue;
			for ( k = left_side[j]; used[cor_mat[j][k]]; k++ );
			left_side[j] = k;
		}
	}
	// 0722�p�̎b�菈�u
//	clcor[0] = 2;
//	clcor[1] = 1;
//	clcor[2] = 3;
//	clcor[3] = 0;
//	clcor[4] = 4;
				
	// �Ή��E�����}�g���N�X�̉��
	for ( i = 0; i < cluster_count; i++ ) {
		free( cor_mat[i] );
		free( dist_mat[i] );
	}
	free( cor_mat );
	free( dist_mat );
	// ���[�E�g�p�ςݍs��̉��
	free( left_side );
	free( used );
	
#else	// ���o�[�W����
	int i, j, k, min_cl, occu[kMaxClusters];
	double distance, min_distance, diff;
	
	// �Ή��ς݃N���X�^�̏�����
	for ( i = 0; i < cluster_count; i++ ) {
		occu[i] = 0;
	}
	// �N���X�^�̑Ή��t��
	for ( i = 0; i < cluster_count; i++ ) {
		min_cl = -1;
		min_distance =  16777216;
		for ( j = 0; j < cluster_count; j++ ) {
			if ( occu[j] == 1 )	continue;	// �Ή��ς�
			distance = 0;
//			printf("%d(%d, %d, %d) <-> %d(%d, %d, %d)\n", i, (int)avgso[i][0], (int)avgso[i][1], (int)avgso[i][2], j, (int)avgsa[j][0],(int)avgsa[j][1],(int)avgsa[j][2]);
			for ( k = 0; k < 3; k++ ) {
				diff = avgso[i][k] - avgsa[j][k];
//				printf("%d : %d : %d : diff : %lf\n", i, j, k, diff);
				distance += diff * diff;
//				printf("%d : %d : %d : distance : %lf\n", i, j, k, distance);
			}
//			printf("%d : %d : distance : %lf\n", i, j, distance);
			if ( distance < min_distance ) {
				min_distance = distance;
				min_cl = j;
			}
		}
//		printf("%d : min_distance : %lf\n", i, min_distance);
		clcor[i] = min_cl;
		occu[min_cl] = 1;
	}
	// 0722�p�̎b�菈�u
/*	clcor[0] = 2;
	clcor[1] = 1;
	clcor[2] = 3;
	clcor[3] = 0;
	clcor[4] = 4;
*/
#endif
}
						
int MakeFeaturePointsFromColorImage( cv::Point **ps0, cv::Mat img, int mode )
// �J���[�摜img����mode�œ����_�𒊏o���Aps�Ɋi�[���ē_����Ԃ��i���݂͎g�p�����j
{
	int i;
	cv::Mat gray, bin, inv;
    std::vector<std::vector<cv::Point>> contours;

    cv::Vec4i hierarchy_unused;  // for hierarchy if needed
	cv::Moments mom;
	cv::Point *ps;

	// �摜�̍쐬
	gray = cv::Mat::zeros(img.size(), CV_8UC1);
	bin = cv::Mat::zeros(img.size(), CV_8UC1);
	inv = cv::Mat::zeros(img.size(), CV_8UC1);
	// �J�n�ʒu�����킹��i�Ȃ��Ə㉺���]����j
	// origin not needed in cv::Mat (always top-left)
	// origin not needed in cv::Mat (always top-left)
	// origin not needed in cv::Mat (always top-left)
	// �O���C�X�P�[���ɕϊ�
	cv::cvtColor( img, gray, cv::COLOR_BGR2GRAY );
	OutPutImage( gray );
	// �K����l��
//	cv::adaptiveThreshold( gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 401, 10 );
	cv::threshold( gray, bin, 128, 255, cv::THRESH_BINARY );
	cv::bitwise_not( bin, inv );
	OutPutImage( inv );
	// �A�������̏d�S���v�Z
	cv::findContours(inv, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// �A�������𒊏o����
	// �܂����𐔂���
    for( i = 0; i < (int)contours.size(); i++ )
    {
		double d00;
//		cv::Point p;
		mom = cv::moments(contours[i]);
		d00 = mom.m00;
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O
		i++;
    }
    // �m�ۂ���
    ps = (cv::Point *)calloc( i, sizeof( cv::Point ) );
    *ps0 = ps;
    for( i = 0; i < (int)contours.size(); i++ )
    {
		double d00;
//		cv::Point p;
		mom = cv::moments(contours[i]);
		d00 = mom.m00;
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O
		ps[i].x = (int)(mom.m10 / d00);
		ps[i].y = (int)(mom.m01 / d00);
//		cv::circle( point, p, 3, cv::Scalar(255,255,255), -1, 1, 0 );
		i++;
    }


    
	return i;
}
int OutPutPoints( char *fname, cv::Point *ps, int num, cv::Size size )
// �����_���o�͂���
{
	int i, ret;
	cv::Mat img;
	
	img = cv::Mat::zeros(size, CV_8UC1);
	img.setTo(cv::Scalar::all(0));
	for ( i = 0; i < num; i++ ) {
		cv::circle( img, ps[i], 25, cWhite, -1, cv::LINE_AA, 0 );
	}
//	ret = OutPutImage( img );
	ret = cv::imwrite( fname, img );
	
	return ret;
}

int OutPutImage( cv::Mat img )
// �摜���t�@�C���ɕۑ�����D�t�@�C�����͎����I�ɘA�ԂɂȂ�
{
	static int n = 0;
	char filename[kFileNameLen];

	sprintf(filename, "output%02d.bmp", n++);
	return cv::imwrite(filename, img);
}

#ifdef	COR_PRESET_MODE
int TransformAnnoPreset( cv::Mat anno, cv::Mat orig, cv::Mat anno2 )
// anno��orig�ɍ��킹�Aanno2�ɏo�͂���B
// �Ԃ�l�F����I���Ȃ�1�A�ُ�I���i�Ή����Ă��Ȃ��j�Ȃ�0
// �Ή��_���\�ߗ^�����Ă���ꍇ�B���݂͎g�p�s�\
{
	strSimParam param;
	cv::Point p11, p12, p21, p22;
	
/* �K�v�H�킩���	
	param.theta = 0.2;
	param.s = 10.0;
	param.tx = 10.0;
	param.ty = 1.0;
*/
	switch ( kOrigNum ) {
	case ( 1 ):
	//	orig01.bmp
		p11.x = 6212;
		p11.y = 591;
		p12.x = 800;
		p12.y = 4307;
	//	anno01.bmp
		p21.x = 166;
		p21.y = 217;
		p22.x = 1056;
		p22.y = 1518;
		break;
	case ( 2 ):
	//	orig02.bmp
		p11.x = 6448;
		p11.y = 241;
		p12.x = 234;
		p12.y = 4478;
	//	anno02.bmp
		p21.x = 80;
		p21.y = 164;
		p22.x = 1100;
		p22.y = 1655;
		break;
	case ( 3 ):
	//	0301orig.bmp
		p11.x = 300;
		p11.y = 380;
		p12.x = 1559;
		p12.y = 2674;
	//	03*02anno.bmp
		p21.x = 297;
		p21.y = 383;
		p22.x = 1555;
		p22.y = 2680;
		break;
	case ( 5 ):
	//	0501orig.bmp
		p11.x = 352;
		p11.y = 449;
		p12.x = 2232;
		p12.y = 3116;
	//	05*02anno.bmp
		p21.x = 349;
		p21.y = 459;
		p22.x = 2227;
		p22.y = 3127;
		break;
	default:
		fprintf(stderr, "illegal kOrigNum %d\n", kOrigNum);
		return 1;
	}
	
	// �Ή�����2�_���瑊���ϊ��p�����[�^�����߂�
	CalcSimParam( p11, p12, p21, p22, &param );
	printf("s = %lf\ntheta = %lf\ntx = %lf\nty = %lf\n", param.s, param.theta, param.tx, param.ty );
	// �p�����[�^�Ɋ�Â��đ����ϊ����s��
	SimilarTransformation( anno, anno2, &param );
	
	return 1;
}

#endif

int IsExist( char *fname )
// �t�@�C�������݂��邩�m�F����
{
	FILE *fp;
	
	if ( ( fp = fopen( fname, "r" ) ) == NULL )	return 0;
	fclose( fp );
	return 1;
}

void OutputCor( cv::Mat anno, cv::Mat orig, cv::Point **clpsa, cv::Point **clpso, int *clcor, int pcors[kMaxClusters][kMaxPointNum], int cluster_count, int *numsa, char *basea, char *baseo )
// �Ή��֌W��`��
{
	int i, j;
	char mergefn[kMaxPathLen];
	cv::Mat img;
	
	// �摜���쐬
	img = cv::Mat::zeros( cv::Size( anno.cols + orig.cols, (anno.rows > orig.rows) ? anno.rows : orig.rows ), CV_8UC3 );
	// ���ׂĕ`��
	MergeImage( anno, orig, MERGE_HOR, img );
	// �g��`��
	cv::rectangle( img, cv::Point( 0, 0 ), cv::Point( anno.cols, anno.rows ), cBlack, 20, cv::LINE_AA, 0 );
	cv::rectangle( img, cv::Point( anno.cols, 0 ), cv::Point( anno.cols + orig.cols, orig.rows ), cBlack, 20, cv::LINE_AA, 0 );
	// �Ή��֌W��`��
	for ( i = 0; i < cluster_count; i++ ) {
		for ( j = 0; j < numsa[i]; j++ ) {
			if ( pcors[i][j] >= 0 ) {
				cv::line( img, clpsa[i][j], cv::Point( clpso[clcor[i]][pcors[i][j]].x + anno.cols, clpso[clcor[i]][pcors[i][j]].y ), cRed, kDrawCorLineThick, cv::LINE_AA, 0 );
			}
		}
	}
	sprintf( mergefn, "%s%sacor.jpg", eOutPutDir, basea );
	cv::imwrite( mergefn, img );
}

#if 0

#define	MERGE_HOR	(0)
#define	MERGE_VER	(1)

#define	max(a,b)	(( a > b ) ? a : b)

int MergeImage( cv::Mat img1, cv::Mat img2, int mode, cv::Mat merge )
// img1��img2��mode(MERGE_HOR:���E�CMERGE_VER:�㉺)�ŕ��ׂ��摜��merge�Ɋi�[����
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge.cols;
	mh = merge.rows;
	if ( mode == MERGE_HOR ) {	// ���E�ɕ��ׂ郂�[�h�ŁC
		if ( mw > (img1.cols + img2.cols) )	// ���E�ɕ��ׂ�������蕝���傫���Ƃ�
			orig_x = (int)((double)(mw - (img1.cols+img2.cols)) / 2.0L);	// �]���̔�����orig_x�ɂ���
		if ( mh > max(img1.rows, img2.rows) )	// �傫���ق��̍�����荂�����傫���Ƃ�
			orig_y = (int)((double)(mh - max(img1.rows, img2.rows)) / 2.0L);
	} else if ( mode == MERGE_VER ) {	// �㉺�ɕ��ׂ郂�[�h�ŁC
		if ( mw > max(img1.cols, img2.cols) )	// �傫���ق��̕������S�̂̕����傫���Ƃ�
			orig_x = (int)((double)(mw - max(img1.cols, img2.cols)) / 2.0L);
		if ( mh > (img1.rows + img2.rows) )	// �㉺�ɕ��ׂ��������S�̂̒������傫���Ƃ�
			orig_y = (int)((double)(mh - (img1.rows+img2.rows)) / 2.0L);
	}

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
//			printf("%d, %d\n", i, j);
			memset( val, 0, 3 );
			if ( mode == MERGE_HOR && i >= img1.cols )	// ���E�z�u�ŉ摜1�̕����z����
				GetPix( img2, i - img1.cols, j, val );
			else if ( mode == MERGE_VER && j >= img1.rows )	// �㉺�z�u�ŉ摜1�̍����𒴂���
				GetPix( img2, i, j - img1.rows, val );
			else
				GetPix( img1, i, j, val );

			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
	return 1;
}
#endif

void OutputCorCl( cv::Point **clpsa, cv::Point **clpso, int *numsa, int *numso, char *basea, char *baseo, int cluster_count, int *clcor, int pcors[][10000] )
// �N���X�^���ƂɑΉ��_��`��
{
	int i, j;
	char climgafn[kMaxPathLen], climgofn[kMaxPathLen], mergefn[kMaxPathLen];
	cv::Mat climga, climgo, merge, sml_merge;
	cv::Point pta, pto;

	for ( i = 0; i < cluster_count; i++ ) {
		sprintf( climgafn, "%s%s03con%d.jpg", eOutPutDir, basea, i );
		sprintf( climgofn, "%s%s03con%d.jpg", eOutPutDir, baseo, clcor[i] );
		puts( climgafn );
		puts( climgofn );
		climga = cv::imread(climgafn, cv::IMREAD_COLOR);
		climgo = cv::imread(climgofn, cv::IMREAD_COLOR);
		if (climga.empty() || climgo.empty())	{
			fprintf( stderr, "%s or %s cannot open\n", climgafn, climgofn );
			return;
		}
		merge = cv::Mat::zeros( cv::Size( climga.cols + climgo.cols, (climga.rows > climgo.rows) ? climga.rows : climgo.rows ), CV_8UC3 );
		sml_merge = cv::Mat::zeros( cv::Size( (int)(merge.cols * 0.5), (int)(merge.rows * 0.5) ), CV_8UC3 );
		sprintf( mergefn, "%s%sclcor%d.jpg", eOutPutDir, basea, i );
		MergeImage( climga, climgo, MERGE_HOR, merge );
		for ( j = 0; j < numsa[i]; j++ ) {
			cv::circle( merge, cv::Point( (int)(clpsa[i][j].x * eClScale), (int)(clpsa[i][j].y * eClScale)), 4, cv::Scalar(0,0,255), -1, cv::LINE_AA, 0 );
		}
		for ( j = 0; j < numso[clcor[i]]; j++ ) {
			cv::circle( merge, cv::Point( climga.cols + (int)(clpso[clcor[i]][j].x * eClScale), (int)(clpso[clcor[i]][j].y * eClScale)), 4, cv::Scalar(0,0,255), -1, cv::LINE_AA, 0 );
		}
		for ( j = 0; j < numsa[i]; j++ ) {
			if ( pcors[i][j] >= 0 ) {
				pta = cv::Point( (int)(clpsa[i][j].x * eClScale), (int)(clpsa[i][j].y * eClScale) );
				pto = cv::Point( climga.cols + (int)(clpso[clcor[i]][pcors[i][j]].x * eClScale), (int)(clpso[clcor[i]][pcors[i][j]].y * eClScale) );
				cv::line( merge, pta, pto, cv::Scalar(0,255,255), 4, cv::LINE_AA, 0 );
			}
		}
		cv::resize( merge, sml_merge, sml_merge.size(), 0, 0, cv::INTER_NEAREST );
		cv::imwrite( mergefn, sml_merge );



	}
}
