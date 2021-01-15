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

int gQualityLevel = kDefaultQualityLevel;
int gMinDistance = kDefaultMinDistance;
int gBlockSize = kDefaultBlockSize;
int gUseHarris = kDefaultUseHarris;
int gHarrisParamK = kDefaultHarrisParamK;

int gQualityLevelReg = kDefaultQualityLevelReg;
int gMinDistanceReg = kDefaultMinDistanceReg;
int gBlockSizeReg = kDefaultBlockSizeReg;
int gUseHarrisReg = kDefaultUseHarrisReg;
int gHarrisParamKReg = kDefaultHarrisParamKReg;

void TrackCallbackHarrisCap( int param )
// キャプチャ画像のトラックバーのコールバック関数
{
	return;
}

void TrackCallbackHarrisReg( int param )
// 登録画像のトラックバーのコールバック関数
{
	return;
}

int InitHarrisCam( void )
// 初期化
{
	int i, num, den;

	/********** 初期化 **********/
	eGroup1Num = kDefaultGroup1Num;
	eGroup2Num = kDefaultGroup2Num;
	eGroup3Num = kDefaultGroup3Num;
	eInvType = kDefaultInvType;
	eDiscNum = kDefaultDiscNum;
	if ( eGroup1Num < eGroup2Num || eGroup2Num < eGroup3Num ) {	// n, mの値をチェック
		fprintf( stderr, "Error: illegal n or m\n" );
		return 0;
	}
	// nCmの設定
	for ( i = 0, num = 1, den = 1; i < eGroup2Num; i++ ) {
		num *= eGroup1Num - i;
		den *= i + 1;
	}
	eNumCom1 = (int)(num / den);
	// mCfの設定
	for ( i = 0, num = 1, den = 1; i < eGroup3Num; i++ ) {
		num *= eGroup2Num - i;
		den *= i + 1;
	}
	eNumCom2 = (int)(num / den);
	strcpy( eDiscFileName, kDefaultDiscFileName );

	return 1;
}

int DetectHarrisCam( char *fname )
// カメラ画像にHarrisを適用
{
	int key, paused = -1;
	long buff_size;
	unsigned char *img_buff;
	IplImage *img_cap, *img_fp, *img_reg, *img_reg_res, *img_reg_fp, *img_reg_smt;
	CvPoint *ps = NULL, *ps0 = NULL, *regps = NULL;
	CvPoint *frps[kMaxFramePoints];	/* FRame PointS : 複数フレームの特徴点集合 */
	int frnum[kMaxFramePoints];	/* FRame NUMbers : frpsの特徴点数 */
	int **frcors[kMaxFramePoints];	/* FRame CORrespondenceS : 対応関係の集合の集合 */
	int *reffrps[kMaxFramePoints];	/* REFined FRame PointS : 厳選された特徴点集合（の集合） */
	int **frcors_tmp, *reffrps_tmp;	/* frcors,reffrpsをずらすときの一時変数 */
	int reffrnum[kMaxFramePoints];	/* REFined FRame NUMbers : reffrpsの特徴点数 */
	int ref_score[kMaxPointNum];	/* 特徴点選別の際のスコア */
	strProjParam frparam[kMaxFramePoints];	/* FRame PARAMeters : 過去のフレームの特徴点を現在のフレームに合わせるためのパラメータ */
	strProjParam zero_param;
	CvPoint currefps[kMaxPointNum];	/* CURrent REFined PointS : reffrpsを統合し，現在のフレームの特徴点として用いるもの */
	int currefnum;	/* currefpsの特徴点数 */
	int used[kMaxFramePoints][kMaxPointNum];	/* currefpsにおいてフレームの点が使用されたか否か */
	int i, j, k, num = 0, num0 = NULL, regnum = 0, **nears = NULL, score[kMaxDocNum], pcor[kMaxPointNum];
	CvSize img_size;
	strDirectShowCap dsc;
	TIME_COUNT start, end;
	strPoint pt, ptd;

	/********** 初期化 **********/
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
	/********** キャプチャ画像側の処理 **********/
	if ( ReadIniFile() == 0 ) {	// iniファイルの読み込みに失敗
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 0;
	}
	InitDirectShowCap( &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
	buff_size = img_size.width * img_size.height * 3;
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// 画像を作成
	img_fp = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// 画像を作成
	// 登録側の処理
	img_reg = cvLoadImage( fname, 1 );
	if ( img_reg == NULL )	return 0;
	img_reg_res = cvCreateImage( cvSize( (int)(img_reg->width * kHarrisRegMag), (int)(img_reg->height * kHarrisRegMag) ), IPL_DEPTH_8U, 3 );
	cvResize( img_reg, img_reg_res, CV_INTER_CUBIC );
	img_reg_fp = cvCreateImage( cvSize( img_reg_res->width, img_reg_res->height ), IPL_DEPTH_8U, 3 );
	img_reg_smt = cvCreateImage( cvSize( img_reg_res->width, img_reg_res->height ), IPL_DEPTH_8U, 3 );


// 特徴点抽出クラスを作成
	StartDirectShowCap( &dsc );	// キャプチャを開始
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// ウィンドウを作成
	cvCreateTrackbar( "QualityLevel", "Capture", &gQualityLevel, kQualityLevelMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "MinDistance", "Capture", &gMinDistance, kMinDistanceMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "BlockSize", "Capture", &gBlockSize, kBlockSizeMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "UseHarris", "Capture", &gUseHarris, kUseHarrisMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "k", "Capture", &gHarrisParamK, kHarrisParamKMax, TrackCallbackHarrisCap );

	cvNamedWindow( "Registered", CV_WINDOW_AUTOSIZE );
	cvCreateTrackbar( "QualityLevel", "Registered", &gQualityLevelReg, kQualityLevelRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "MinDistance", "Registered", &gMinDistanceReg, kMinDistanceRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "BlockSize", "Registered", &gBlockSizeReg, kBlockSizeRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "UseHarris", "Registered", &gUseHarrisReg, kUseHarrisRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "k", "Registered", &gHarrisParamK, kHarrisParamKRegMax, TrackCallbackHarrisReg );
	/********** ループ処理 **********/
	for ( ; ; ) {
		if ( paused < 0 ) {	// ポーズされていない
			// キャプチャ
			CaptureDirectShowCap( &dsc, img_buff, buff_size );
			// バッファを画像に変換
			Buff2ImageData( img_buff, img_cap );
			// Harrisを適用
//			DetectHarris( img_cap, img_fp );
			// frps，frnum，frcors，reffrps，reffrnumをずらす
			if ( frps[kMaxFramePoints-1] != NULL )	free( frps[kMaxFramePoints-1] );
			frcors_tmp = frcors[kMaxFramePoints-1];	// 末尾をfrcors_tmpに入れる
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
			// 特徴点を抽出し，frps[0]に入れる
			DetectGoodFeatures( img_cap, img_fp, &frps[0], &frnum[0] );
			// 以前のフレームとの対応関係を求める
			for ( i = 1; i < kCompareFrames; i++ ) {
				FindCorPoint( frps[0], frnum[0], frps[i], frnum[i], frcors[0][i-1], &frparam[i-1] );
			}
			// 対応点をもつフレーム数を求める
			for ( i = 0; i < frnum[0]; i++ ) {
				ref_score[i] = 0;
				for ( j = 0; j < kCompareFrames-1; j++ ) {
					if ( frcors[0][j][i] >= 0 )	ref_score[i]++;
				}
			}
			// 対応点数を用いてreffrpsを求める
			reffrnum[0] = 0;
			for ( i = 0; i < frnum[0]; i++ ) {
				if ( ref_score[i] >= kRefineScore-1 ) {
					reffrps[0][reffrnum[0]++] = i;
				}
			}
			// currefpsの作成
			// usedの初期化
			for ( i = 0; i < kMaxFramePoints; i++ ) {
				for ( j = 0; j < kMaxPointNum; j++ ) {
					used[i][j] = 0;
				}
			}
			currefnum = 0;
			for ( i = 0; i < kMergeFrames; i++ ) {	// 現在のフレームを含めてkMergeFramesフレームから，
				for ( j = 0; j < reffrnum[i]; j++ ) {	// reffrpsの点を，
					if ( i != 0 && memcmp( &frparam[i-1], &zero_param, sizeof(strProjParam) ) == 0 )	continue;	// 現在のフレームでなく，パラメータ推定に失敗していた場合
					if ( !used[i][reffrps[i][j]] ) {	// usedが0ならば，
						if ( i == 0 ) {						// i==0なら
							currefps[currefnum++] = frps[i][reffrps[i][j]];	// currefpsにそのまま，
						} else {	// そうでなければ
							pt.x = frps[i][reffrps[i][j]].x;
							pt.y = frps[i][reffrps[i][j]].y;
							ProjTrans( &pt, &ptd, &frparam[i-1] );	// 変換を施して
							currefps[currefnum].x = ptd.x;	// currefpsに格納
							currefps[currefnum].y = ptd.y;
							currefnum++;
						}
						// usedに使用済みフラグを立てる
						used[i][reffrps[i][j]] = 1;	// このフレーム
						for ( k = i+1; k < kMaxFramePoints; k++ ) {
							if ( frcors[i][k-i-1][reffrps[i][j]] >= 0 ) {	// 対応点をもつならば
								used[k][frcors[i][k-i-1][reffrps[i][j]]] = 1;
							}
						}
					}
				}
			}
			// 描画テスト4（currefpsの描画）
			for ( i = 0; i < currefnum; i++ ) {
				cvCircle( img_fp, currefps[i], 3, CV_RGB(0,255,0), -1, CV_AA, 0 );
			}
			// 描画テスト3（reffrpsのみ描画）
			/*
			for ( i = 0; i < reffrnum[0]; i++ ) {
				cvCircle( img_fp, frps[0][reffrps[0][i]], 3, CV_RGB(0,255,0), -1, CV_AA, 0 );
			}*/
			// 描画テスト2（特徴点のスコアに基づいて色をつける）
			/*
			for ( i = 0; i < frnum[0]; i++ ) {
				cvCircle( img_fp, frps[0][i], 3, CV_RGB( (int)(255.0L-(double)ref_score[i]*(255.0L/(double)(kCompareFrames-1))), (int)((double)ref_score[i]*(255.0L/(double)(kCompareFrames-1))), 0), -1, CV_AA, 0 );
			}*/
			// 描画テスト1（前のフレームおよび前々のフレームの対応点から線を引く）
			/*
			for ( i = 0; i < frnum[0]; i++ ) {
				if ( frcors[0][0][i] >= 0 ) {
					cvLine( img_fp, frps[0][i], frps[1][frcors[0][0][i]], CV_RGB(0,255,255), 2, CV_AA, 0 );
				}
				if ( frcors[0][1][i] >= 0 ) {
					cvLine( img_fp, frps[0][i], frps[2][frcors[0][1][i]], CV_RGB(0,255,0), 2, CV_AA, 0 );
				}
			}*/
			// テスト：前のフレームのみ使う
/*			if ( frps[0] != NULL && frps[1] != NULL && frnum[0] > 4 && frnum[1] > 4 ) {
				FindCorPoint( frps[0], frnum[0], frps[1], frnum[1], pcor );
				for ( i = 0; i < frnum[0]; i++ ) {
					if ( pcor[i] >= 0 ) {
						cvLine( img_fp, frps[0][i], frps[1][pcor[i]], CV_RGB( 0, 255, 255 ), 2, CV_AA, 0 );
					}
				}
			}*/
			/* 前のフレームのみと対応を求める
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
			/* 検索によって対応点探索を行う．かなり重たい
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
			// 登録画像の処理
			cvSmooth( img_reg_res, img_reg_smt, CV_GAUSSIAN, 5, 0, 0 );
			DetectGoodFeaturesReg( img_reg_smt, img_reg_fp, &regps, &regnum );
			// 表示
			cvShowImage( "Capture", img_fp );
//			cvSaveImage( "harris.jpg", img_har );
			cvShowImage( "Registered", img_reg_fp );
		}
		key = cvWaitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'c':	// 対応点探索
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
					cvShowImage( "Capture", img_fp );
					cvShowImage( "Registered", img_reg_fp );
					for ( key = cvWaitKey(1); key < 0; key = cvWaitKey(1) );
					ResumeDirectShowCap( &dsc );
					break;
				case 's':	// 保存
					cvSaveImage( "cap_fp.bmp", img_fp );
					cvSaveImage( "reg_fp.bmp", img_reg_fp );
					break;
				case 'q':	// 終了
					goto end_camharris;
			}
		}

	}
end_camharris:
	cvDestroyWindow( "Capture" );	// ウィンドウを破棄
	StopDirectShowCap( &dsc );	// キャプチャを終了
	ReleaseDirectShowCap( &dsc );	// DirectShowの諸々をリリース
	return 1;
}

int DetectHarrisTest( char *fname, char *fname_anno )
// 画像にHarrisを適用
{
	int key, paused = -1;
	long buff_size;
	unsigned char *img_buff;
	IplImage *img_cap, *img_fp, *img_reg, *img_reg_res, *img_reg_fp, *img_reg_smt;
	IplImage *img_anno, *img_anno_res, *img_anno_fp, *img_anno_smt;
	CvPoint *ps = NULL, *ps0 = NULL, *regps = NULL;
	CvPoint *frps[kMaxFramePoints];	/* FRame PointS : 複数フレームの特徴点集合 */
	int frnum[kMaxFramePoints];	/* FRame NUMbers : frpsの特徴点数 */
	int **frcors[kMaxFramePoints];	/* FRame CORrespondenceS : 対応関係の集合の集合 */
	int *reffrps[kMaxFramePoints];	/* REFined FRame PointS : 厳選された特徴点集合（の集合） */
	int **frcors_tmp, *reffrps_tmp;	/* frcors,reffrpsをずらすときの一時変数 */
	int reffrnum[kMaxFramePoints];	/* REFined FRame NUMbers : reffrpsの特徴点数 */
	int ref_score[kMaxPointNum];	/* 特徴点選別の際のスコア */
	strProjParam frparam[kMaxFramePoints];	/* FRame PARAMeters : 過去のフレームの特徴点を現在のフレームに合わせるためのパラメータ */
	strProjParam zero_param;
	CvPoint currefps[kMaxPointNum];	/* CURrent REFined PointS : reffrpsを統合し，現在のフレームの特徴点として用いるもの */
	int currefnum;	/* currefpsの特徴点数 */
	int used[kMaxFramePoints][kMaxPointNum];	/* currefpsにおいてフレームの点が使用されたか否か */
	int i, j, k, num = 0, num0 = NULL, regnum = 0, **nears = NULL, score[kMaxDocNum], pcor[kMaxPointNum];
	CvSize img_size;
	strDirectShowCap dsc;
	TIME_COUNT start, end;
	strPoint pt, ptd;

	/********** 初期化 **********/
	InitHarrisCam();
	zero_param.a1 = 0.0L;
	zero_param.a2 = 0.0L;
	zero_param.a3 = 0.0L;
	zero_param.b1 = 0.0L;
	zero_param.b2 = 0.0L;
	zero_param.b3 = 0.0L;
	zero_param.c1 = 0.0L;
	zero_param.c2 = 0.0L;
	
	/********** キャプチャ画像側の処理 **********/
	if ( ReadIniFile() == 0 ) {	// iniファイルの読み込みに失敗
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 0;
	}
	// 登録側の処理
	img_reg = cvLoadImage( fname, 1 );
	if ( img_reg == NULL )	return 0;
	img_reg_res = cvCreateImage( cvSize( (int)(img_reg->width * kHarrisTestMag), (int)(img_reg->height * kHarrisTestMag) ), IPL_DEPTH_8U, 3 );
	cvResize( img_reg, img_reg_res, CV_INTER_CUBIC );
	img_reg_fp = cvCreateImage( cvSize( img_reg_res->width, img_reg_res->height ), IPL_DEPTH_8U, 3 );
	img_reg_smt = cvCreateImage( cvSize( img_reg_res->width, img_reg_res->height ), IPL_DEPTH_8U, 3 );
	// 書き込み側の処理
	img_anno = cvLoadImage( fname_anno, 1 );
	if ( img_anno == NULL )	return 0;
	img_anno_res = cvCreateImage( cvSize( (int)(img_anno->width * kHarrisTestMag), (int)(img_anno->height * kHarrisTestMag) ), IPL_DEPTH_8U, 3 );
	cvResize( img_anno, img_anno_res, CV_INTER_CUBIC );
	img_anno_fp = cvCreateImage( cvSize( img_anno_res->width, img_anno_res->height ), IPL_DEPTH_8U, 3 );
	img_anno_smt = cvCreateImage( cvSize( img_anno_res->width, img_anno_res->height ), IPL_DEPTH_8U, 3 );

	img_size.width = img_anno->width;
	img_size.height = img_anno->height;


// 特徴点抽出クラスを作成
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// ウィンドウを作成
	cvCreateTrackbar( "QualityLevel", "Capture", &gQualityLevel, kQualityLevelMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "MinDistance", "Capture", &gMinDistance, kMinDistanceMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "BlockSize", "Capture", &gBlockSize, kBlockSizeMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "UseHarris", "Capture", &gUseHarris, kUseHarrisMax, TrackCallbackHarrisCap );
	cvCreateTrackbar( "k", "Capture", &gHarrisParamK, kHarrisParamKMax, TrackCallbackHarrisCap );

	cvNamedWindow( "Registered", CV_WINDOW_AUTOSIZE );
	cvCreateTrackbar( "QualityLevel", "Registered", &gQualityLevelReg, kQualityLevelRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "MinDistance", "Registered", &gMinDistanceReg, kMinDistanceRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "BlockSize", "Registered", &gBlockSizeReg, kBlockSizeRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "UseHarris", "Registered", &gUseHarrisReg, kUseHarrisRegMax, TrackCallbackHarrisReg );
	cvCreateTrackbar( "k", "Registered", &gHarrisParamK, kHarrisParamKRegMax, TrackCallbackHarrisReg );
	/********** ループ処理 **********/
	for ( ; ; ) {
		if ( paused < 0 ) {	// ポーズされていない
			// 登録画像の処理
//			cvSmooth( img_reg_res, img_reg_smt, CV_GAUSSIAN, 5, 0, 0 );
			DetectGoodFeaturesReg( img_reg_res, img_reg_fp, &regps, &regnum );
			// 書き込み画像の処理
//			cvSmooth( img_anno_res, img_anno_smt, CV_GAUSSIAN, 5, 0, 0 );
			DetectGoodFeatures( img_anno_res, img_anno_fp, &ps, &num );
			// 表示
			cvShowImage( "Capture", img_anno_fp );
//			cvSaveImage( "harris.jpg", img_har );
			cvShowImage( "Registered", img_reg_fp );
		}
		key = cvWaitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'c':	// 対応点探索
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
				case 's':	// 保存
					cvSaveImage( "cap_fp.bmp", img_anno_fp );
					cvSaveImage( "reg_fp.bmp", img_reg_fp );
					break;
				case 'q':	// 終了
					goto end_camharris;
			}
		}

	}
end_camharris:
	cvDestroyWindow( "Capture" );	// ウィンドウを破棄
	return 1;
}

void DetectHarris( IplImage *src, IplImage *dst )
// Harrisを適用する
{
	double minVal = 0.0, maxVal = 0.0, scale, shift, min = 0, max = 255;
	IplImage *img_har, *img_gray, *img_out;
	CvSize img_size;

	img_size.width = src->width;
	img_size.height = src->height;

	img_gray = cvCreateImage( img_size, IPL_DEPTH_8S, 1 );	// 画像を作成
	img_har = cvCreateImage( img_size, IPL_DEPTH_32F, 1 );	// 画像を作成
	img_out = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );	// 画像を作成

	// Harrisを適用
	cvCvtColor( src, img_gray, CV_RGB2GRAY );	// グレイスケールに変換
	cvCornerHarris( img_gray, img_har, 3, 5, 0.04 );	// Harrisを適用
	cvMinMaxLoc( img_har, &minVal, &maxVal, NULL, NULL, 0 );
	scale = (max - min)/(maxVal - minVal);
	shift = -minVal * scale + min;
	cvConvertScale( img_har, img_out, scale, shift );
	cvCvtColor( img_out, dst, CV_GRAY2RGB );

	cvReleaseImage( &img_gray );
	cvReleaseImage( &img_har );
	cvReleaseImage( &img_out );
}

void DetectGoodFeatures( IplImage *src, IplImage *dst, CvPoint **pps, int *pnum )
// cvGoodFeaturesToTrackで特徴点を抽出する
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
	quality_level = 0.01 * (double)(gQualityLevel + 1);
	min_distance = (double)gMinDistance;
	block_size = gBlockSize * 2 + 3;
	k = 0.01 * (double)gHarrisParamK;
	cvGoodFeaturesToTrack( img_gray, img_eig, img_temp, corners, &cor_cnt, quality_level, min_distance, 0, block_size, gUseHarris, k );
//	printf("cor_cnt : %d\n", cor_cnt);
	ps = (CvPoint *)calloc( cor_cnt, sizeof(CvPoint) );
//	cvZero( dst );
	cvCopy( src, dst, 0 );
	for ( i = 0; i < cor_cnt; i++ ) {
		pt.x = (int)corners[i].x;
		pt.y = (int)corners[i].y;
//		cvCircle( dst, pt, 2, CV_RGB(255,0,0), -1, 8, 0 );	// 赤
		cvCircle( dst, pt, 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// 黒白
		cvCircle( dst, pt, 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// 黒白
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
// cvGoodFeaturesToTrackで特徴点を抽出する（登録画像）
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
	quality_level = 0.01 * (double)(gQualityLevelReg + 1);
	min_distance = (double)gMinDistanceReg;
	block_size = gBlockSizeReg * 2 + 3;
	k = 0.01 * (double)gHarrisParamKReg;
	cvGoodFeaturesToTrack( img_gray, img_eig, img_temp, corners, &cor_cnt, quality_level, min_distance, 0, block_size, gUseHarris, k );
//	printf("cor_cnt : %d\n", cor_cnt);
	ps = (CvPoint *)calloc( cor_cnt, sizeof(CvPoint) );
//	cvZero( dst );
	cvCopy( src, dst, 0 );
	for ( i = 0; i < cor_cnt; i++ ) {
		pt.x = (int)corners[i].x;
		pt.y = (int)corners[i].y;
//		cvCircle( dst, pt, 2, CV_RGB(255,0,0), -1, 8, 0 );	// 赤
		cvCircle( dst, pt, 6, CV_RGB(255,255,255), -1, CV_AA, 0 );	// 黒白
		cvCircle( dst, pt, 4, CV_RGB(0,0,0), -1, CV_AA, 0 );	// 黒白
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
// フレーム間の対応点を見つける
{
	int i, j, pcornum = 0;
	int cor[kMaxCor][2];	// 対応関係
	strPoint pt1, pt2d, pt2;

	// 対応関係を初期化
	for ( i = 0; i < num; i++ ) {
		pcor[i] = -1;
	}
	if ( ps == NULL || num < 4 || corps == NULL || cornum < 4 )	return;
	// 対応してそうなものを全部くっつける
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
