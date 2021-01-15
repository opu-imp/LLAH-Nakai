#include "def_general.h"

#include <stdio.h>
#include <math.h>
#include <cv.h>			// OpneCV
#include <highgui.h>	// OpenCV
#include <windows.h>
#include <mmsystem.h>
#include <dshow.h>
#include <conio.h>
#include <qedit.h>
#include "extern.h"
#include "dirs.h"
//#include "hist.h"
#include "proj4p.h"
#include "dscap.h"
#include "dcamc.h"
#include "init.h"
#include "camharris.h"
#include "proctime.h"
#include "disc.h"
#include "hash.h"
#include "nn5dcamc.h"
#include "projrecov.h"
#include "auto_connect.h"

strHarrisCamParam gHarrisCamParam = {
	kDefaultQualityLevel,
	kDefaultMinDistance,
	kDefaultBlockSize,
	kDefaultUseHarris,
	kDefaultHarrisParamK
};

strHarrisRegParam gHarrisRegParam = {
	kDefaultQualityLevelReg,
	kDefaultMinDistanceReg,
	kDefaultBlockSizeReg,
	kDefaultUseHarrisReg,
	kDefaultHarrisParamKReg
};

void TrackCallbackHarrisCap( int param )
// �L���v�`���摜�̃g���b�N�o�[�̃R�[���o�b�N�֐�
{
	return;
}

void TrackCallbackHarrisReg( int param )
// �o�^�摜�̃g���b�N�o�[�̃R�[���o�b�N�֐�
{
	return;
}

int InitHarrisCam( void )
// ������
{
	int i, num, den;

	/********** ������ **********/
	eGroup1Num = kDefaultGroup1Num;
	eGroup2Num = kDefaultGroup2Num;
	eGroup3Num = kDefaultGroup3Num;
	eInvType = kDefaultInvType;
	eDiscNum = kDefaultDiscNum;
	if ( eGroup1Num < eGroup2Num || eGroup2Num < eGroup3Num ) {	// n, m�̒l���`�F�b�N
		fprintf( stderr, "Error: illegal n or m\n" );
		return 0;
	}
	// nCm�̐ݒ�
	for ( i = 0, num = 1, den = 1; i < eGroup2Num; i++ ) {
		num *= eGroup1Num - i;
		den *= i + 1;
	}
	eNumCom1 = (int)(num / den);
	// mCf�̐ݒ�
	for ( i = 0, num = 1, den = 1; i < eGroup3Num; i++ ) {
		num *= eGroup2Num - i;
		den *= i + 1;
	}
	eNumCom2 = (int)(num / den);
	strcpy( eDiscFileName, kDefaultDiscFileName );

	return 1;
}

int DetectHarrisCam( char *fname, int mode )
// �J�����摜��Harris�i�������͉p�������摜����̓����_���o�j��K�p
{
	int key, paused = -1;
	long buff_size;
	unsigned char *img_buff;
	IplImage *img_cap, *img_fp, *img_show, *img_reg, *img_reg_res, *img_reg_fp, *img_reg_smt, *img_reg_show;
	CvPoint *ps = NULL, *ps0 = NULL, *regps = NULL;
	CvPoint *frps[kMaxFramePoints];	/* FRame PointS : �����t���[���̓����_�W�� */
	int frnum[kMaxFramePoints];	/* FRame NUMbers : frps�̓����_�� */
	int **frcors[kMaxFramePoints];	/* FRame CORrespondenceS : �Ή��֌W�̏W���̏W�� */
	int *reffrps[kMaxFramePoints];	/* REFined FRame PointS : ���I���ꂽ�����_�W���i�̏W���j */
	int **frcors_tmp, *reffrps_tmp;	/* frcors,reffrps�����炷�Ƃ��̈ꎞ�ϐ� */
	int reffrnum[kMaxFramePoints];	/* REFined FRame NUMbers : reffrps�̓����_�� */
	int ref_score[kMaxPointNum];	/* �����_�I�ʂ̍ۂ̃X�R�A */
	strProjParam frparam[kMaxFramePoints];	/* FRame PARAMeters : �ߋ��̃t���[���̓����_�����݂̃t���[���ɍ��킹�邽�߂̃p�����[�^ */
	strProjParam zero_param;
	CvPoint currefps[kMaxPointNum];	/* CURrent REFined PointS : reffrps�𓝍����C���݂̃t���[���̓����_�Ƃ��ėp������� */
	int currefnum;	/* currefps�̓����_�� */
	int used[kMaxFramePoints][kMaxPointNum];	/* currefps�ɂ����ăt���[���̓_���g�p���ꂽ���ۂ� */
	int i, j, k, num = 0, num0 = NULL, regnum = 0, **nears = NULL, score[kMaxDocNum], pcor[kMaxPointNum];
	CvSize img_size, img_show_size, img_reg_size, img_reg_show_size;
	strDirectShowCap dsc;
	TIME_COUNT start, end;
	strPoint pt, ptd;

	/********** ������ **********/
	
	InitHarrisCam();
	for ( i = 0; i < kMaxFramePoints; i++ ) {
		frps[i] = NULL;
		frnum[i] = 0;
		frcors[i] = (int **)calloc( kMaxFramePoints, sizeof(int *) );
		for ( j = 0; j < kMaxFramePoints; j++ ) {
			frcors[i][j] = (int *)calloc( kMaxPointNum, sizeof(int) );
		}
		reffrps[i] = (int *)calloc( kMaxPointNum, sizeof(int) );
		reffrnum[i] = 0;
		memset( &frparam[i], 0, sizeof(strProjParam) );
	}
	zero_param.a1 = 0.0L;
	zero_param.a2 = 0.0L;
	zero_param.a3 = 0.0L;
	zero_param.b1 = 0.0L;
	zero_param.b2 = 0.0L;
	zero_param.b3 = 0.0L;
	zero_param.c1 = 0.0L;
	zero_param.c2 = 0.0L;
	/********** �L���v�`���摜���̏��� **********/
	if ( ReadIniFile() == 0 ) {	// ini�t�@�C���̓ǂݍ��݂Ɏ��s
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 0;
	}
	InitDirectShowCap( &dsc, &(img_size.width), &(img_size.height) );	// USB�J������������
	buff_size = img_size.width * img_size.height * 3;
	img_buff = (unsigned char *)malloc( buff_size );	// �摜�̃o�b�t�@���m��
	img_cap = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// �摜���쐬
	img_fp = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// �摜���쐬
	img_show_size = cvSize( (int)(img_size.width * kHarrisShowCamMag), (int)(img_size.height * kHarrisShowCamMag) );
	img_show = cvCreateImage( img_show_size, IPL_DEPTH_8U, 3 );	// �\������k���摜���쐬
	// �o�^���̏���
	img_reg = cvLoadImage( fname, 1 );
	if ( img_reg == NULL )	return 0;
	img_reg_size = cvSize( (int)(img_reg->width * kHarrisRegMag), (int)(img_reg->height * kHarrisRegMag) );
	img_reg_res = cvCreateImage( img_reg_size, IPL_DEPTH_8U, 3 );
	cvResize( img_reg, img_reg_res, CV_INTER_CUBIC );
	img_reg_fp = cvCreateImage( img_reg_size, IPL_DEPTH_8U, 3 );
	img_reg_smt = cvCreateImage( img_reg_size, IPL_DEPTH_8U, 3 );
	img_reg_show_size = cvSize( (int)(img_reg_size.width * kHarrisShowRegMag), (int)(img_reg_size.height * kHarrisShowRegMag) );
	img_reg_show = cvCreateImage( img_reg_show_size, IPL_DEPTH_8U, 3 );


// �����_���o�N���X���쐬
	StartDirectShowCap( &dsc );	// �L���v�`�����J�n
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// �E�B���h�E���쐬
	cvNamedWindow( "CaptureParam", CV_WINDOW_AUTOSIZE );	// �E�B���h�E���쐬
	cvNamedWindow( "Registered", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "RegisteredParam", CV_WINDOW_AUTOSIZE );
	if ( mode == DHC_HARRIS ) {
		cvCreateTrackbar( "QualityLevel", "CaptureParam", &(gHarrisCamParam.QualityLevel), kQualityLevelMax, TrackCallbackHarrisCap );
		cvCreateTrackbar( "MinDistance", "CaptureParam", &(gHarrisCamParam.MinDistance), kMinDistanceMax, TrackCallbackHarrisCap );
		cvCreateTrackbar( "BlockSize", "CaptureParam", &(gHarrisCamParam.BlockSize), kBlockSizeMax, TrackCallbackHarrisCap );
		cvCreateTrackbar( "UseHarris", "CaptureParam", &(gHarrisCamParam.UseHarris), kUseHarrisMax, TrackCallbackHarrisCap );
		cvCreateTrackbar( "k", "CaptureParam", &(gHarrisCamParam.HarrisParamK), kHarrisParamKMax, TrackCallbackHarrisCap );
		cvCreateTrackbar( "QualityLevel", "RegisteredParam", &(gHarrisRegParam.QualityLevelReg), kQualityLevelRegMax, TrackCallbackHarrisReg );
		cvCreateTrackbar( "MinDistance", "RegisteredParam", &(gHarrisRegParam.MinDistanceReg), kMinDistanceRegMax, TrackCallbackHarrisReg );
		cvCreateTrackbar( "BlockSize", "RegisteredParam", &(gHarrisRegParam.BlockSizeReg), kBlockSizeRegMax, TrackCallbackHarrisReg );
		cvCreateTrackbar( "UseHarris", "RegisteredParam", &(gHarrisRegParam.UseHarrisReg), kUseHarrisRegMax, TrackCallbackHarrisReg );
		cvCreateTrackbar( "k", "RegisteredParam", &(gHarrisRegParam.HarrisParamKReg), kHarrisParamKRegMax, TrackCallbackHarrisReg );
	}

	// �E�B���h�E���ړ�
	cvMoveWindow( "Capture", 0, 30 );
	cvMoveWindow( "CaptureParam", 2200, 0 );
	cvMoveWindow( "Registered", 1280, 0 );
	cvMoveWindow( "RegisteredParam", 2200, 400 );
	/********** ���[�v���� **********/
	for ( int cntcnt=0; ;cntcnt++ ) {
		if ( paused < 0 ) {	// �|�[�Y����Ă��Ȃ�
			// �L���v�`��
			CaptureDirectShowCap( &dsc, img_buff, buff_size );
			// �o�b�t�@���摜�ɕϊ�
			Buff2ImageData( img_buff, img_cap );
			// Harris��K�p
//			DetectHarris( img_cap, img_fp );
			// frps�Cfrnum�Cfrcors�Creffrps�Creffrnum�����炷
			if ( frps[kMaxFramePoints-1] != NULL )	free( frps[kMaxFramePoints-1] );
			frcors_tmp = frcors[kMaxFramePoints-1];	// ������frcors_tmp�ɓ����
			reffrps_tmp = reffrps[kMaxFramePoints-1];
			for ( i = kMaxFramePoints - 1; i >= 1; i-- ) {
				frps[i] = frps[i-1];
				frnum[i] = frnum[i-1];
				frcors[i] = frcors[i-1];
				reffrps[i] = reffrps[i-1];
				reffrnum[i] = reffrnum[i-1];
			}
			frps[0] = NULL;
			frnum[0] = 0;
			frcors[0] = frcors_tmp;
			reffrps[0] = reffrps_tmp;
			reffrnum[0] = 0;
			// �����_�𒊏o���Cfrps[0]�ɓ����
			switch ( mode ) {
				case DHC_HARRIS:
					// Harris���[�h
					DetectGoodFeatures( img_cap, img_fp, &frps[0], &frnum[0] );
					break;
				case DHC_ENG:
					// �p�����[�h
					DetectEngDocPoints( img_cap, img_fp, &frps[0], &frnum[0] );
					break;
				case DHC_JP:
					// ���{�ꃂ�[�h
					DetectJpDocPoints( img_cap, img_fp, &frps[0], &frnum[0] );
					break;
				default:
					break;
			}
			// �ȑO�̃t���[���Ƃ̑Ή��֌W�����߂�
			for ( i = 1; i < kCompareFrames; i++ ) {
				FindCorPoint( frps[0], frnum[0], frps[i], frnum[i], frcors[0][i-1], &frparam[i-1] );
			}
			// �Ή��_�����t���[���������߂�
			for ( i = 0; i < frnum[0]; i++ ) {
				ref_score[i] = 0;
				for ( j = 0; j < kCompareFrames-1; j++ ) {
					if ( frcors[0][j][i] >= 0 )	ref_score[i]++;
				}
			}
			// �Ή��_����p����reffrps�����߂�
			reffrnum[0] = 0;
			for ( i = 0; i < frnum[0]; i++ ) {
				if ( ref_score[i] >= kRefineScore-1 ) {
					reffrps[0][reffrnum[0]++] = i;
				}
			}
			// currefps�̍쐬
			// used�̏�����
			for ( i = 0; i < kMaxFramePoints; i++ ) {
				for ( j = 0; j < kMaxPointNum; j++ ) {
					used[i][j] = 0;
				}
			}
			currefnum = 0;
			for ( i = 0; i < kMergeFrames; i++ ) {	// ���݂̃t���[�����܂߂�kMergeFrames�t���[������C
				for ( j = 0; j < reffrnum[i]; j++ ) {	// reffrps�̓_���C
					if ( i != 0 && memcmp( &frparam[i-1], &zero_param, sizeof(strProjParam) ) == 0 )	continue;	// ���݂̃t���[���łȂ��C�p�����[�^����Ɏ��s���Ă����ꍇ
					if ( !used[i][reffrps[i][j]] ) {	// used��0�Ȃ�΁C
						if ( i == 0 ) {						// i==0�Ȃ�
							currefps[currefnum++] = frps[i][reffrps[i][j]];	// currefps�ɂ��̂܂܁C
						} else {	// �����łȂ����
							pt.x = frps[i][reffrps[i][j]].x;
							pt.y = frps[i][reffrps[i][j]].y;
							ProjTrans( &pt, &ptd, &frparam[i-1] );	// �ϊ����{����
							currefps[currefnum].x = ptd.x;	// currefps�Ɋi�[
							currefps[currefnum].y = ptd.y;
							currefnum++;
						}
						// used�Ɏg�p�ς݃t���O�𗧂Ă�
						used[i][reffrps[i][j]] = 1;	// ���̃t���[��
						for ( k = i+1; k < kMaxFramePoints; k++ ) {
							if ( frcors[i][k-i-1][reffrps[i][j]] >= 0 ) {	// �Ή��_�����Ȃ��
								used[k][frcors[i][k-i-1][reffrps[i][j]]] = 1;
							}
						}
					}
				}
			}
			// �`��e�X�g4�icurrefps�̕`��j
			for ( i = 0; i < currefnum; i++ ) {
				cvCircle( img_fp, currefps[i], 3, CV_RGB(0,255,0), -1, CV_AA, 0 );
			}
			// �`��e�X�g3�ireffrps�̂ݕ`��j
			/*
			for ( i = 0; i < reffrnum[0]; i++ ) {
				cvCircle( img_fp, frps[0][reffrps[0][i]], 3, CV_RGB(0,255,0), -1, CV_AA, 0 );
			}*/
			// �`��e�X�g2�i�����_�̃X�R�A�Ɋ�Â��ĐF������j
			/*
			for ( i = 0; i < frnum[0]; i++ ) {
				cvCircle( img_fp, frps[0][i], 3, CV_RGB( (int)(255.0L-(double)ref_score[i]*(255.0L/(double)(kCompareFrames-1))), (int)((double)ref_score[i]*(255.0L/(double)(kCompareFrames-1))), 0), -1, CV_AA, 0 );
			}*/
			// �`��e�X�g1�i�O�̃t���[������ёO�X�̃t���[���̑Ή��_������������j
			/*
			for ( i = 0; i < frnum[0]; i++ ) {
				if ( frcors[0][0][i] >= 0 ) {
					cvLine( img_fp, frps[0][i], frps[1][frcors[0][0][i]], CV_RGB(0,255,255), 2, CV_AA, 0 );
				}
				if ( frcors[0][1][i] >= 0 ) {
					cvLine( img_fp, frps[0][i], frps[2][frcors[0][1][i]], CV_RGB(0,255,0), 2, CV_AA, 0 );
				}
			}*/
			// �e�X�g�F�O�̃t���[���̂ݎg��
/*			if ( frps[0] != NULL && frps[1] != NULL && frnum[0] > 4 && frnum[1] > 4 ) {
				FindCorPoint( frps[0], frnum[0], frps[1], frnum[1], pcor );
				for ( i = 0; i < frnum[0]; i++ ) {
					if ( pcor[i] >= 0 ) {
						cvLine( img_fp, frps[0][i], frps[1][pcor[i]], CV_RGB( 0, 255, 255 ), 2, CV_AA, 0 );
					}
				}
			}*/
			/* �O�̃t���[���݂̂ƑΉ������߂�
			start = GetProcTimeMiliSec();
			if ( ps0 != NULL )	free( ps0 );
			ps0 = ps;
			num0 = num;
			DetectGoodFeatures( img_cap, img_fp, &ps, &num );
			end = GetProcTimeMiliSec();
			printf("GoodFeatures : %d ms\n", end - start );
			if ( ps != NULL && ps0 != NULL && num > 4 && num0 > 4 ) {
				FindCorPoint( ps, num, ps0, num0, pcor );
				for ( i = 0; i < num; i++ ) {
					if ( pcor[i] >= 0 ) {
						cvLine( img_fp, ps[i], ps0[pcor[i]], CV_RGB( 0, 255, 255 ), 2, CV_AA, 0 );
					}
				}
			}*/
			/* �����ɂ���đΉ��_�T�����s���D���Ȃ�d����
			if ( ps != NULL && ps0 != NULL ) {
				ConstructHashSub( ps0, num0 );
				MakeNearsFromCentres( ps, num, &nears );
				RetrieveNN5( ps, num, nears, img_size, score, pcor );
				for ( i = 0; i < num; i++ ) {
					if ( pcor[i] >= 0 ) {
						cvLine( img_fp, ps[i], ps0[pcor[i]], CV_RGB( 0, 255, 255 ), 2, CV_AA, 0 );
					}
				}
			} */
			// �o�^�摜�̏���
			switch ( mode ) {
				case DHC_HARRIS:
					cvSmooth( img_reg_res, img_reg_smt, CV_GAUSSIAN, 5, 0, 0 );
					DetectGoodFeaturesReg( img_reg_smt, img_reg_fp, &regps, &regnum );
					break;
				case DHC_ENG:
					DetectEngDocPointsReg( img_reg_res, img_reg_fp, &regps, &regnum );
					break;
				case DHC_JP:
					DetectJpDocPointsReg( img_reg_res, img_reg_fp, &regps, &regnum );
					break;
			}
			// �\��
			cvResize( img_fp, img_show, CV_INTER_CUBIC );
			cvResize( img_reg_fp, img_reg_show, CV_INTER_CUBIC );
			cvShowImage( "Capture", img_show );
//			cvSaveImage( "harris.jpg", img_har );
			cvShowImage( "Registered", img_reg_show );
		}
		key = cvWaitKey( 1 );	// �Ȃ��ƃE�B���h�E���X�V����Ȃ�
		if ( key >= 0 ) {
			switch ( key ) {
				case 'c':	// �Ή��_�T��
					PauseDirectShowCap( &dsc );
					ConstructHashSub( regps, regnum );
					MakeNearsFromCentres( currefps, currefnum, &nears );
					RetrieveNN5( currefps, currefnum, nears, img_size, score, pcor );
					for ( i = 0; i < currefnum; i++ ) {
						if ( pcor[i] >= 0 ) {
							cvCircle( img_fp, currefps[i], 5, CV_RGB(255,255,0), -1, CV_AA, 0 );
							cvCircle( img_reg_fp, regps[pcor[i]], 5, CV_RGB(255,255,0), CV_AA, 0 );
						}
					}
					cvResize( img_fp, img_show, CV_INTER_CUBIC );
					cvResize( img_reg_fp, img_reg_show, CV_INTER_CUBIC );
					cvShowImage( "Capture", img_show );
					cvShowImage( "Registered", img_reg_show );
					for ( key = cvWaitKey(1); key < 0; key = cvWaitKey(1) );
					ResumeDirectShowCap( &dsc );
					break;
				case 's':	// �ۑ�
					cvSaveImage( "cap_fp.bmp", img_fp );
					cvSaveImage( "reg_fp.bmp", img_reg_fp );
					break;
				case 'q':	// �I��
					goto end_camharris;
			}
		}

	}
end_camharris:
	cvDestroyWindow( "Capture" );	// �E�B���h�E��j��
	StopDirectShowCap( &dsc );	// �L���v�`�����I��
	ReleaseDirectShowCap( &dsc );	// DirectShow�̏��X�������[�X
	return 1;
}

int DetectHarrisTest( char *fname, char *fname_anno )
// �摜��Harris��K�p
{
	int key, paused = -1;
	long buff_size;
	unsigned char *img_buff;
	IplImage *img_cap, *img_fp, *img_reg, *img_reg_res, *img_reg_fp, *img_reg_smt;
	IplImage *img_anno, *img_anno_res, *img_anno_fp, *img_anno_smt;
	CvPoint *ps = NULL, *ps0 = NULL, *regps = NULL;
	CvPoint *frps[kMaxFramePoints];	/* FRame PointS : �����t���[���̓����_�W�� */
	int frnum[kMaxFramePoints];	/* FRame NUMbers : frps�̓����_�� */
	int **frcors[kMaxFramePoints];	/* FRame CORrespondenceS : �Ή��֌W�̏W���̏W�� */
	int *reffrps[kMaxFramePoints];	/* REFined FRame PointS : ���I���ꂽ�����_�W���i�̏W���j */
	int **frcors_tmp, *reffrps_tmp;	/* frcors,reffrps�����炷�Ƃ��̈ꎞ�ϐ� */
	int reffrnum[kMaxFramePoints];	/* REFined FRame NUMbers : reffrps�̓����_�� */
	int ref_score[kMaxPointNum];	/* �����_�I�ʂ̍ۂ̃X�R�A */
	strProjParam frparam[kMaxFramePoints];	/* FRame PARAMeters : �ߋ��̃t���[���̓����_�����݂̃t���[���ɍ��킹�邽�߂̃p�����[�^ */
	strProjParam zero_param;
	CvPoint currefps[kMaxPointNum];	/* CURrent REFined PointS : reffrps�𓝍����C���݂̃t���[���̓����_�Ƃ��ėp������� */
	int currefnum;	/* currefps�̓����_�� */
	int used[kMaxFramePoints][kMaxPointNum];	/* currefps�ɂ����ăt���[���̓_���g�p���ꂽ���ۂ� */
	int i, j, k, num = 0, num0 = NULL, regnum = 0, **nears = NULL, score[kMaxDocNum], pcor[kMaxPointNum];
	CvSize img_size;
	strDirectShowCap dsc;
	TIME_COUNT start, end;
	strPoint pt, ptd;

	/********** ������ **********/
	InitHarrisCam();
	zero_param.a1 = 0.0L;
	zero_param.a2 = 0.0L;
	zero_param.a3 = 0.0L;
	zero_param.b1 = 0.0L;
	zero_param.b2 = 0.0L;
	zero_param.b3 = 0.0L;
	zero_param.c1 = 0.0L;
	zero_param.c2 = 0.0L;
	
	/********** �L���v�`���摜���̏��� **********/
	if ( ReadIniFile() == 0 ) {	// ini�t�@�C���̓ǂݍ��݂Ɏ��s
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 0;
	}
	// �o�^���̏���
	img_reg = cvLoadImage( fname, 1 );
	if ( img_reg == NULL )	return 0;
	img_reg_res = cvCreateImage( cvSize( (int)(img_reg->width * kHarrisTestMag), (int)(img_reg->height * kHarrisTestMag) ), IPL_DEPTH_8U, 3 );
	cvResize( img_reg, img_reg_res, CV_INTER_CUBIC );
	img_reg_fp = cvCreateImage( cvSize( img_reg_res->width, img_reg_res->height ), IPL_DEPTH_8U, 3 );
	img_reg_smt = cvCreateImage( cvSize( img_reg_res->width, img_reg_res->height ), IPL_DEPTH_8U, 3 );
	// �������ݑ��̏���
	img_anno = cvLoadImage( fname_anno, 1 );
	if ( img_anno == NULL )	return 0;
	img_anno_res = cvCreateImage( cvSize( (int)(img_anno->width * kHarrisTestMag), (int)(img_anno->height * kHarrisTestMag) ), IPL_DEPTH_8U, 3 );
	cvResize( img_anno, img_anno_res, CV_INTER_CUBIC );
	img_anno_fp = cvCreateImage( cvSize( img_anno_res->width, img_anno_res->height ), IPL_DEPTH_8U, 3 );
	img_anno_smt = cvCreateImage( cvSize( img_anno_res->width, img_anno_res->height ), IPL_DEPTH_8U, 3 );

	img_size.width = img_anno->width;
	img_size.height = img_anno->height;


// �����_���o�N���X���쐬
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// �E�B���h�E���쐬
	cvCreateTrackbar( "QualityLevel", "Capture", &(gHarrisCamParam.QualityLevel), kQualityLevelMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "MinDistance", "Capture", &(gHarrisCamParam.MinDistance), kMinDistanceMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "BlockSize", "Capture", &(gHarrisCamParam.BlockSize), kBlockSizeMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "UseHarris", "Capture", &(gHarrisCamParam.UseHarris), kUseHarrisMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "k", "Capture", &(gHarrisCamParam.HarrisParamK), kHarrisParamKMax, TrackCallbackHarrisCap );

	cvNamedWindow( "Registered", CV_WINDOW_AUTOSIZE );
	cvCreateTrackbar( "QualityLevel", "Registered", &(gHarrisRegParam.QualityLevelReg), kQualityLevelRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "MinDistance", "Registered", &(gHarrisRegParam.MinDistanceReg), kMinDistanceRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "BlockSize", "Registered", &(gHarrisRegParam.BlockSizeReg), kBlockSizeRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "UseHarris", "Registered", &(gHarrisRegParam.UseHarrisReg), kUseHarrisRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "k", "Registered", &(gHarrisRegParam.HarrisParamKReg), kHarrisParamKRegMax, TrackCallbackHarrisReg );
	/********** ���[�v���� **********/
	for ( ; ; ) {
		if ( paused < 0 ) {	// �|�[�Y����Ă��Ȃ�
			// �o�^�摜�̏���
//			cvSmooth( img_reg_res, img_reg_smt, CV_GAUSSIAN, 5, 0, 0 );
			DetectGoodFeaturesReg( img_reg_res, img_reg_fp, &regps, &regnum );
			// �������݉摜�̏���
//			cvSmooth( img_anno_res, img_anno_smt, CV_GAUSSIAN, 5, 0, 0 );
			DetectGoodFeatures( img_anno_res, img_anno_fp, &ps, &num );
			// �\��
			cvShowImage( "Capture", img_anno_fp );
//			cvSaveImage( "harris.jpg", img_har );
			cvShowImage( "Registered", img_reg_fp );
		}
		key = cvWaitKey( 1 );	// �Ȃ��ƃE�B���h�E���X�V����Ȃ�
		if ( key >= 0 ) {
			switch ( key ) {
				case 'c':	// �Ή��_�T��
					ConstructHashSub( regps, regnum );
					MakeNearsFromCentres( ps, num, &nears );
					RetrieveNN5( ps, num, nears, img_size, score, pcor );
					for ( i = 0; i < num; i++ ) {
						if ( pcor[i] >= 0 ) {
							cvCircle( img_anno_fp, ps[i], 5, CV_RGB(255,255,0), -1, CV_AA, 0 );
							cvCircle( img_reg_fp, regps[pcor[i]], 5, CV_RGB(255,255,0), CV_AA, 0 );
						}
					}
					cvShowImage( "Capture", img_anno_fp );
					cvShowImage( "Registered", img_reg_fp );
					for ( key = cvWaitKey(1); key < 0; key = cvWaitKey(1) );
					break;
				case 's':	// �ۑ�
					cvSaveImage( "cap_fp.bmp", img_anno_fp );
					cvSaveImage( "reg_fp.bmp", img_reg_fp );
					break;
				case 'q':	// �I��
					goto end_camharris;
			}
		}

	}
end_camharris:
	cvDestroyWindow( "Capture" );	// �E�B���h�E��j��
	return 1;
}

void DetectHarris( IplImage *src, IplImage *dst )
// Harris��K�p����
{
	double minVal = 0.0, maxVal = 0.0, scale, shift, min = 0, max = 255;
	IplImage *img_har, *img_gray, *img_out;
	CvSize img_size;

	img_size.width = src->width;
	img_size.height = src->height;

	img_gray = cvCreateImage( img_size, IPL_DEPTH_8S, 1 );	// �摜���쐬
	img_har = cvCreateImage( img_size, IPL_DEPTH_32F, 1 );	// �摜���쐬
	img_out = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );	// �摜���쐬

	// Harris��K�p
	cvCvtColor( src, img_gray, CV_RGB2GRAY );	// �O���C�X�P�[���ɕϊ�
	cvCornerHarris( img_gray, img_har, 3, 5, 0.04 );	// Harris��K�p
	cvMinMaxLoc( img_har, &minVal, &maxVal, NULL, NULL, 0 );
	scale = (max - min)/(maxVal - minVal);
	shift = -minVal * scale + min;
	cvConvertScale( img_har, img_out, scale, shift );
	cvCvtColor( img_out, dst, CV_GRAY2RGB );

	cvReleaseImage( &img_gray );
	cvReleaseImage( &img_har );
	cvReleaseImage( &img_out );
}
//#define	CENTROID
#define	SECOND_ORDER_MOMENT
#define	kMaxNoiseArea	(10)
int CalcCentresCH(CvPoint **ps0, CvSeq *contours, CvSize *size, double **areas0)
// �d�S���v�Z����
// 06/01/12	�ʐς̌v�Z��ǉ�
{
	int i, num;
	CvSeq *con0;
	CvMoments mom;
	CvPoint *ps;


	// �_�̐����J�E���g�i�b��j
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// �S�~���ŘA����������������Ƃ��i�{���͑傫���𒲂ׂ��肷�ׂ��j
	// �_������z����m�ہinum�̒l�͐��m�ł͂Ȃ����C�傫�߂Ƃ������ƂŁj
	ps = (CvPoint *)calloc(num, sizeof(CvPoint));
	*ps0 = ps;
	// �A��������`��E�d�S���v�Z
    for( i = 0, con0 = contours; con0 != 0 && i < num ; con0 = con0->h_next )
    {
		double d00;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < kMaxNoiseArea ) continue;	// ����������A�������͏��O

#ifdef	CENTROID
		ps[i].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		ps[i].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
#endif
#ifdef	SECOND_ORDER_MOMENT
		ps[i].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00 + cvGetNormalizedCentralMoment( &mom, 2, 0 ) );
		ps[i].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00 + cvGetNormalizedCentralMoment( &mom, 0, 2 ) );
#endif
		i++;
    }
	num = i;	// �A���������̍X�V

	return num;
}

int MakeCentresFromImageCH(CvPoint **ps, IplImage *img, CvSize *size, double **areas)
// �摜�̃t�@�C������^���ďd�S���v�Z���C�d�S�̐���Ԃ�
// 06/01/12	�ʐς̌v�Z��ǉ�
{
	int num, ret;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0;

	size->width = img->width;
	size->height = img->height;
	
    ret = cvFindContours( img, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
	if ( ret <= 0 ) {	// �A��������������Ȃ�����
		cvReleaseMemStorage( &storage );
		return 0;
	}
	num = CalcCentresCH(ps, contours, size, areas);	// �e�A�������̏d�S���v�Z����

	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	return num;
}

void DetectEngDocPoints( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum )
// �p�����[�h�œ����_�𒊏o����
{
	int i, num;
	CvPoint *ps;
	CvSize size;
	IplImage *img_con;

	img_con = cvCreateImage( cvGetSize( src ), IPL_DEPTH_8U, 1 );
	GetConnectedImageCam( src, img_con );
	size.width = img_con->width;
	size.height = img_con->height;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// �����_�摜�̕`��
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cvCircle( dst, ps[i], 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// ����
		cvCircle( dst, ps[i], 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// ����
	}
	cvReleaseImage( &img_con );
	if ( num > 0 ) {	// �����_��1�ł����o���ꂽ��
		*pps = ps;
		*pnum = num;
	}
}

void DetectEngDocPointsReg( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum  )
// �p�����[�h�œ����_�𒊏o����i�o�^���j
{
	int i, num;
	CvPoint *ps;
	CvSize size;
	IplImage *img_con;

	img_con = GetConnectedImageReg( src );
	size.width = img_con->width;
	size.height = img_con->height;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// �����_�摜�̕`��
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cvCircle( dst, ps[i], 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// ����
		cvCircle( dst, ps[i], 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// ����
	}
	cvReleaseImage( &img_con );
	if ( num > 0 ) {	// �����_��1�ł����o���ꂽ��
		*pps = ps;
		*pnum = num;
	}
}

void DetectJpDocPoints( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum )
// ���{�ꃂ�[�h�œ����_�𒊏o����
{
	int i, num;
	CvPoint *ps;
	CvSize size;
	IplImage *img_con;

	img_con = GetConnectedImageCamJp( src );
	size.width = img_con->width;
	size.height = img_con->height;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// �����_�摜�̕`��
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cvCircle( dst, ps[i], 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// ����
		cvCircle( dst, ps[i], 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// ����
	}
	cvReleaseImage( &img_con );
	if ( num > 0 ) {	// �����_��1�ł����o���ꂽ��
		*pps = ps;
		*pnum = num;
	}
}

void DetectJpDocPointsReg( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum  )
// �p�����[�h�œ����_�𒊏o����i�o�^���j
{
	int i, num;
	CvPoint *ps;
	CvSize size;
	IplImage *img_con;

	img_con = GetConnectedImageRegJp( src );
	size.width = img_con->width;
	size.height = img_con->height;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// �����_�摜�̕`��
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cvCircle( dst, ps[i], 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// ����
		cvCircle( dst, ps[i], 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// ����
	}
	cvReleaseImage( &img_con );
	if ( num > 0 ) {	// �����_��1�ł����o���ꂽ��
		*pps = ps;
		*pnum = num;
	}
}

void DetectGoodFeatures( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum )
// cvGoodFeaturesToTrack�œ����_�𒊏o����
{
	IplImage *img_gray, *img_eig, *img_temp;
	CvSize img_size;
	CvPoint pt, *ps;
	CvPoint2D32f *corners = NULL;
	int i, cor_cnt, block_size;
	double quality_level, min_distance, k;

	img_size.width = src->width;
	img_size.height = src->height;

	img_gray = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );
	img_eig = cvCreateImage( img_size, IPL_DEPTH_32F, 1 );
	img_temp = cvCreateImage( img_size, IPL_DEPTH_32F, 1 );

	corners = (CvPoint2D32f *)calloc( kMaxPointNum, sizeof(CvPoint2D32f) );
	cor_cnt = kMaxPointNum;
	cvCvtColor( src, img_gray, CV_RGB2GRAY );
	quality_level = 0.01 * (double)(gHarrisCamParam.QualityLevel + 1);
	min_distance = (double)gHarrisCamParam.MinDistance;
	block_size = gHarrisCamParam.BlockSize * 2 + 3;
	k = 0.01 * (double)gHarrisCamParam.HarrisParamK;
	cvGoodFeaturesToTrack( img_gray, img_eig, img_temp, corners, &cor_cnt, quality_level, min_distance, 0, block_size, gHarrisCamParam.UseHarris, k );
//	printf("cor_cnt : %d\n", cor_cnt);
	ps = (CvPoint *)calloc( cor_cnt, sizeof(CvPoint) );
//	cvZero( dst );
	cvCopy( src, dst, 0 );
	for ( i = 0; i < cor_cnt; i++ ) {
		pt.x = (int)corners[i].x;
		pt.y = (int)corners[i].y;
//		cvCircle( dst, pt, 2, CV_RGB(255,0,0), -1, 8, 0 );	// ��
		cvCircle( dst, pt, 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// ����
		cvCircle( dst, pt, 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// ����
		ps[i] = pt;
	}
	cvReleaseImage( &img_gray );
	cvReleaseImage( &img_eig );
	cvReleaseImage( &img_temp );
	free( corners );
	*pps = ps;
	*pnum = cor_cnt;
}

void DetectGoodFeaturesReg( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum  )
// cvGoodFeaturesToTrack�œ����_�𒊏o����i�o�^�摜�j
{
	IplImage *img_gray, *img_eig, *img_temp;
	CvSize img_size;
	CvPoint pt, *ps;
	CvPoint2D32f *corners = NULL;
	int i, cor_cnt, block_size;
	double quality_level, min_distance, k;

	img_size.width = src->width;
	img_size.height = src->height;

	img_gray = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );
	img_eig = cvCreateImage( img_size, IPL_DEPTH_32F, 1 );
	img_temp = cvCreateImage( img_size, IPL_DEPTH_32F, 1 );

	corners = (CvPoint2D32f *)calloc( 10240, sizeof(CvPoint2D32f) );
	cor_cnt = 10240;
	cvCvtColor( src, img_gray, CV_RGB2GRAY );
	quality_level = 0.01 * (double)(gHarrisRegParam.QualityLevelReg + 1);
	min_distance = (double)gHarrisRegParam.MinDistanceReg;
	block_size = gHarrisRegParam.BlockSizeReg * 2 + 3;
	k = 0.01 * (double)gHarrisRegParam.HarrisParamKReg;
	cvGoodFeaturesToTrack( img_gray, img_eig, img_temp, corners, &cor_cnt, quality_level, min_distance, 0, block_size, gHarrisRegParam.UseHarrisReg, k );
//	printf("cor_cnt : %d\n", cor_cnt);
	ps = (CvPoint *)calloc( cor_cnt, sizeof(CvPoint) );
//	cvZero( dst );
	cvCopy( src, dst, 0 );
	for ( i = 0; i < cor_cnt; i++ ) {
		pt.x = (int)corners[i].x;
		pt.y = (int)corners[i].y;
//		cvCircle( dst, pt, 2, CV_RGB(255,0,0), -1, 8, 0 );	// ��
		cvCircle( dst, pt, 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// ����
		cvCircle( dst, pt, 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// ����
		ps[i] = pt;
	}
	cvReleaseImage( &img_gray );
	cvReleaseImage( &img_eig );
	cvReleaseImage( &img_temp );
	free( corners );
	*pps = ps;
	*pnum = cor_cnt;
}

void FindCorPoint( CvPoint *ps, int num, CvPoint *corps, int cornum, int *pcor, strProjParam *param )
// �t���[���Ԃ̑Ή��_��������
{
	int i, j, pcornum = 0;
	int cor[kMaxCor][2];	// �Ή��֌W
	strPoint pt1, pt2d, pt2;

	// �Ή��֌W��������
	for ( i = 0; i < num; i++ ) {
		pcor[i] = -1;
	}
	if ( ps == NULL || num < 4 || corps == NULL || cornum < 4 )	return;
	// �Ή����Ă����Ȃ��̂�S����������
	for ( i = 0; i < num; i++ ) {
		for ( j = 0; j < cornum && pcornum < kMaxCor; j++ ) {
			if ( GetPointsDistance( ps[i], corps[j] ) < kMaxDistFindCor ) {
				cor[pcornum][0] = i;
				cor[pcornum][1] = j;
				pcornum++;
			}
		}
	}
	if ( pcornum < 4 )	return;
	if ( GetAppropriateParamRANSAC( ps, corps, cor, pcornum, param, PROJ_NORMAL ) == 0 )	return;

	for ( i = 0; i < pcornum; i++ ) {
		pt1.x = ps[cor[i][0]].x;
		pt1.y = ps[cor[i][0]].y;
		pt2.x = corps[cor[i][1]].x;
		pt2.y = corps[cor[i][1]].y;

		ProjTrans( &pt2, &pt2d, param );
		if ( GetPointsDistance( pt1, pt2d ) < kRANSACThr ) {
			pcor[cor[i][0]] = cor[i][1];
		}
	}
}
