#include <stdio.h>
#include "cv.h"
#include "highgui.h"
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

void OutputCor( IplImage *anno, IplImage *orig, CvPoint **clpsa, CvPoint **clpso, int *clcor, int pcors[kMaxClusters][kMaxPointNum], int cluster_count, int *numsa, char *basea, char *baseo );
void OutputCorCl( CvPoint **clpsa, CvPoint **clpso, int *numsa, int *numso, char *basea, char *baseo, int cluster_count, int *clcor, int pcors[kMaxClusters][kMaxPointNum] );

int TransformAnno( IplImage *anno, IplImage *orig, int cluster_count, double *param )
// anno��orig�ɕϊ����鑊���ϊ��p�����[�^�����߂�
{
	int i, j, numso[kMaxClusters], numsa[kMaxClusters], clcor[kMaxClusters], **nears, score[kMaxDocNum];
	int pcors[kMaxClusters][kMaxPointNum], cor[kMaxPointNum*kMaxClusters][4], cor_num = 0;
	int start, start_ret, ret_sum;
	CvPoint *clpso[kMaxClusters], *clpsa[kMaxClusters];
	double avgso[kMaxClusters][3], avgsa[kMaxClusters][3];
	char cmd[kMaxCmdLineLen], diro[kMaxPathLen], dira[kMaxPathLen], baseo[kFileNameLen], basea[kFileNameLen];
	char pfno[kMaxPathLen], pfna[kMaxPathLen], fname[kMaxPathLen];
	IplImage *cor_img;
	CvSize sizeo, sizea;
	CvPoint p1[3], p2[3];
//	strSimParam param;
	
	// ���摜�Ə������݉摜��������_�𒊏o�i���o�[�W�����j
/*	numo = MakeFeaturePointsFromColorImage( &pso, orig, FP_PDF );
	numa = MakeFeaturePointsFromColorImage( &psa, anno, FP_SCAN );
	OutPutPoints( pso, numo, cvGetSize(orig) );
	OutPutPoints( psa, numa, cvGetSize(anno) );
*/

	/************* ���摜�Ə������݉摜��������_�𒊏o *************/
//	sizeo = cvSize( orig->width * kClScale, orig->height * kClScale );
//	sizea = cvSize( anno->width * kClScale, anno->height * kClScale );
	sizeo = cvGetSize( orig );
	sizea = cvGetSize( anno );
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
	cor_img = cvCreateImage( cvSize( sizeo.width + sizea.width, (sizeo.height > sizea.height) ? sizeo.height : sizea.height ), 8, 1 );
	for ( i = 0; i < cluster_count; i++ ) {
		sprintf( fname, "%s%s05cor%d.jpg", eOutPutDir, basea, i );
		cvZero( cor_img );
		for ( j = 0; j < numsa[i]; j++ ) {
			if ( pcors[i][j] >= 0 ) {
				cvLine( cor_img, clpsa[i][j], cvPoint( clpso[clcor[i]][pcors[i][j]].x + sizea.width, clpso[clcor[i]][pcors[i][j]].y ), cWhite, kDrawCorLineThick, CV_AA, 0 );
			}
		}
	cvSaveImage( fname, cor_img );
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
						
int MakeFeaturePointsFromColorImage( CvPoint **ps0, IplImage *img, int mode )
// �J���[�摜img����mode�œ����_�𒊏o���Aps�Ɋi�[���ē_����Ԃ��i���݂͎g�p�����j
{
	int i;
	IplImage *gray, *bin, *inv;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	CvPoint *ps;

	// �摜�̍쐬
	gray = cvCreateImage( cvGetSize( img ), IPL_DEPTH_8U, 1 );
	bin = cvCreateImage( cvGetSize( img ), IPL_DEPTH_8U, 1 );
	inv = cvCreateImage( cvGetSize( img ), IPL_DEPTH_8U, 1 );
	// �J�n�ʒu�����킹��i�Ȃ��Ə㉺���]����j
	gray->origin = img->origin;
	bin->origin = img->origin;
	inv->origin = img->origin;
	// �O���C�X�P�[���ɕϊ�
	cvCvtColor( img, gray, CV_BGR2GRAY );
	OutPutImage( gray );
	// �K����l��
//	cvAdaptiveThreshold( gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 401, 10 );
	cvThreshold( gray, bin, 128, 255, CV_THRESH_BINARY );
	cvNot( bin, inv );
	OutPutImage( inv );
	// �A�������̏d�S���v�Z
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
	// �܂����𐔂���
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
//		CvPoint p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O
		i++;
    }
    // �m�ۂ���
    ps = (CvPoint *)calloc( i, sizeof( CvPoint ) );
    *ps0 = ps;
    for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
    {
		double d00;
//		CvPoint p;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O
		ps[i].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		ps[i].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
//		cvCircle( point, p, 3, CV_RGB(255, 255, 255), -1, 1, 0 );
		i++;
    }
    
    cvReleaseMemStorage( &storage );
    cvReleaseImage( &gray );
    cvReleaseImage( &bin );
    cvReleaseImage( &inv );
    
	return i;
}
int OutPutPoints( char *fname, CvPoint *ps, int num, CvSize size )
// �����_���o�͂���
{
	int i, ret;
	IplImage *img;
	
	img = cvCreateImage( size, IPL_DEPTH_8U, 1 );
	cvZero( img );
	for ( i = 0; i < num; i++ ) {
		cvCircle( img, ps[i], 25, cWhite, -1, CV_AA, 0 );
	}
//	ret = OutPutImage( img );
	ret = cvSaveImage( fname, img );
	cvReleaseImage( &img );
	
	return ret;
}

int OutPutImage( IplImage *img )
// �摜���t�@�C���ɕۑ�����D�t�@�C�����͎����I�ɘA�ԂɂȂ�
{
	static int n = 0;
	char filename[kFileNameLen];

	sprintf(filename, "output%02d.bmp", n++);
	return cvSaveImage(filename, img);
}

#ifdef	COR_PRESET_MODE
int TransformAnnoPreset( IplImage *anno, IplImage *orig, IplImage *anno2 )
// anno��orig�ɍ��킹�Aanno2�ɏo�͂���B
// �Ԃ�l�F����I���Ȃ�1�A�ُ�I���i�Ή����Ă��Ȃ��j�Ȃ�0
// �Ή��_���\�ߗ^�����Ă���ꍇ�B���݂͎g�p�s�\
{
	strSimParam param;
	CvPoint p11, p12, p21, p22;
	
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

void OutputCor( IplImage *anno, IplImage *orig, CvPoint **clpsa, CvPoint **clpso, int *clcor, int pcors[kMaxClusters][kMaxPointNum], int cluster_count, int *numsa, char *basea, char *baseo )
// �Ή��֌W��`��
{
	int i, j;
	char mergefn[kMaxPathLen];
	IplImage *img;
	
	// �摜���쐬
	img = cvCreateImage( cvSize( anno->width + orig->width, (anno->height > orig->height) ? anno->height : orig->height ), 8, 3 );
	// ���ׂĕ`��
	MergeImage( anno, orig, MERGE_HOR, img );
	// �g��`��
	cvRectangle( img, cvPoint( 0, 0 ), cvPoint( anno->width, anno->height ), cBlack, 20, CV_AA, 0 );
	cvRectangle( img, cvPoint( anno->width, 0 ), cvPoint( anno->width + orig->width, orig->height ), cBlack, 20, CV_AA, 0 );
	// �Ή��֌W��`��
	for ( i = 0; i < cluster_count; i++ ) {
		for ( j = 0; j < numsa[i]; j++ ) {
			if ( pcors[i][j] >= 0 ) {
				cvLine( img, clpsa[i][j], cvPoint( clpso[clcor[i]][pcors[i][j]].x + anno->width, clpso[clcor[i]][pcors[i][j]].y ), cRed, kDrawCorLineThick, CV_AA, 0 );
			}
		}
	}
	sprintf( mergefn, "%s%sacor.jpg", eOutPutDir, basea );
	cvSaveImage( mergefn, img );
	cvReleaseImage( &img );
}

#if 0

#define	MERGE_HOR	(0)
#define	MERGE_VER	(1)

#define	max(a,b)	(( a > b ) ? a : b)

int MergeImage( IplImage *img1, IplImage *img2, int mode, IplImage *merge )
// img1��img2��mode(MERGE_HOR:���E�CMERGE_VER:�㉺)�ŕ��ׂ��摜��merge�Ɋi�[����
{
	int i, j, mw, mh, orig_x = 0, orig_y = 0;
	char val[3];

	mw = merge->width;
	mh = merge->height;
	if ( mode == MERGE_HOR ) {	// ���E�ɕ��ׂ郂�[�h�ŁC
		if ( mw > (img1->width + img2->width) )	// ���E�ɕ��ׂ�������蕝���傫���Ƃ�
			orig_x = (int)((double)(mw - (img1->width+img2->width)) / 2.0L);	// �]���̔�����orig_x�ɂ���
		if ( mh > max(img1->height, img2->height) )	// �傫���ق��̍�����荂�����傫���Ƃ�
			orig_y = (int)((double)(mh - max(img1->height, img2->height)) / 2.0L);
	} else if ( mode == MERGE_VER ) {	// �㉺�ɕ��ׂ郂�[�h�ŁC
		if ( mw > max(img1->width, img2->width) )	// �傫���ق��̕������S�̂̕����傫���Ƃ�
			orig_x = (int)((double)(mw - max(img1->width, img2->width)) / 2.0L);
		if ( mh > (img1->height + img2->height) )	// �㉺�ɕ��ׂ��������S�̂̒������傫���Ƃ�
			orig_y = (int)((double)(mh - (img1->height+img2->height)) / 2.0L);
	}

	for ( j = 0; j < mh; j++ ) {
		for ( i = 0; i < mw; i++ ) {
//			printf("%d, %d\n", i, j);
			memset( val, 0, 3 );
			if ( mode == MERGE_HOR && i >= img1->width )	// ���E�z�u�ŉ摜1�̕����z����
				GetPix( img2, i - img1->width, j, val );
			else if ( mode == MERGE_VER && j >= img1->height )	// �㉺�z�u�ŉ摜1�̍����𒴂���
				GetPix( img2, i, j - img1->height, val );
			else
				GetPix( img1, i, j, val );

			SetPix( merge, orig_x + i, orig_y + j, val );
		}
	}
	return 1;
}
#endif

void OutputCorCl( CvPoint **clpsa, CvPoint **clpso, int *numsa, int *numso, char *basea, char *baseo, int cluster_count, int *clcor, int pcors[][10000] )
// �N���X�^���ƂɑΉ��_��`��
{
	int i, j;
	char climgafn[kMaxPathLen], climgofn[kMaxPathLen], mergefn[kMaxPathLen];
	IplImage *climga, *climgo, *merge, *sml_merge;
	CvPoint pta, pto;

	for ( i = 0; i < cluster_count; i++ ) {
		sprintf( climgafn, "%s%s03con%d.jpg", eOutPutDir, basea, i );
		sprintf( climgofn, "%s%s03con%d.jpg", eOutPutDir, baseo, clcor[i] );
		puts( climgafn );
		puts( climgofn );
		climga = cvLoadImage( climgafn, 1 );
		climgo = cvLoadImage( climgofn, 1 );
		if ( climga == NULL || climgo == NULL )	{
			fprintf( stderr, "%s or %s cannot open\n", climgafn, climgofn );
			return;
		}
		merge = cvCreateImage( cvSize( climga->width + climgo->width, (climga->height > climgo->height) ? climga->height : climgo->height ), IPL_DEPTH_8U, 3 );
		sml_merge = cvCreateImage( cvSize( (int)(merge->width * 0.5), (int)(merge->height * 0.5) ), IPL_DEPTH_8U, 3 );
		sprintf( mergefn, "%s%sclcor%d.jpg", eOutPutDir, basea, i );
		MergeImage( climga, climgo, MERGE_HOR, merge );
		for ( j = 0; j < numsa[i]; j++ ) {
			cvCircle( merge, cvPoint( (int)(clpsa[i][j].x * eClScale), (int)(clpsa[i][j].y * eClScale)), 4, CV_RGB(255,0,0), -1, CV_AA, 0 );
		}
		for ( j = 0; j < numso[clcor[i]]; j++ ) {
			cvCircle( merge, cvPoint( climga->width + (int)(clpso[clcor[i]][j].x * eClScale), (int)(clpso[clcor[i]][j].y * eClScale)), 4, CV_RGB(255,0,0), -1, CV_AA, 0 );
		}
		for ( j = 0; j < numsa[i]; j++ ) {
			if ( pcors[i][j] >= 0 ) {
				pta = cvPoint( (int)(clpsa[i][j].x * eClScale), (int)(clpsa[i][j].y * eClScale) );
				pto = cvPoint( climga->width + (int)(clpso[clcor[i]][pcors[i][j]].x * eClScale), (int)(clpso[clcor[i]][pcors[i][j]].y * eClScale) );
				cvLine( merge, pta, pto, CV_RGB(255,255,0), 4, CV_AA, 0 );
			}
		}
		cvResize( merge, sml_merge, CV_INTER_NN );
		cvSaveImage( mergefn, sml_merge );

		cvReleaseImage( &sml_merge );
		cvReleaseImage( &merge );
		cvReleaseImage( &climga );
		cvReleaseImage( &climgo );
	}
}
