#include "def_general.h"

#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>			// OpneCV
// #include <highgui.h> // now in opencv2/opencv.hpp	// OpenCV
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

int DetectHarrisCam( char *fname, int mode )
// カメラ画像にHarris（もしくは英文文書画像からの特徴点抽出）を適用
{
	int key, paused = -1;
	long buff_size;
	unsigned char *img_buff;
	cv::Mat img_cap, *img_fp, *img_show, *img_reg, *img_reg_res, *img_reg_fp, *img_reg_smt, *img_reg_show;
	cv::Point *ps = NULL, *ps0 = NULL, *regps = NULL;
	cv::Point *frps[kMaxFramePoints];	/* FRame PointS : 複数フレームの特徴点集合 */
	int frnum[kMaxFramePoints];	/* FRame NUMbers : frpsの特徴点数 */
	int **frcors[kMaxFramePoints];	/* FRame CORrespondenceS : 対応関係の集合の集合 */
	int *reffrps[kMaxFramePoints];	/* REFined FRame PointS : 厳選された特徴点集合（の集合） */
	int **frcors_tmp, *reffrps_tmp;	/* frcors,reffrpsをずらすときの一時変数 */
	int reffrnum[kMaxFramePoints];	/* REFined FRame NUMbers : reffrpsの特徴点数 */
	int ref_score[kMaxPointNum];	/* 特徴点選別の際のスコア */
	strProjParam frparam[kMaxFramePoints];	/* FRame PARAMeters : 過去のフレームの特徴点を現在のフレームに合わせるためのパラメータ */
	strProjParam zero_param;
	cv::Point currefps[kMaxPointNum];	/* CURrent REFined PointS : reffrpsを統合し，現在のフレームの特徴点として用いるもの */
	int currefnum;	/* currefpsの特徴点数 */
	int used[kMaxFramePoints][kMaxPointNum];	/* currefpsにおいてフレームの点が使用されたか否か */
	int i, j, k, num = 0, num0 = NULL, regnum = 0, **nears = NULL, score[kMaxDocNum], pcor[kMaxPointNum];
	cv::Size img_size, img_show_size, img_reg_size, img_reg_show_size;
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
	InitDirectShowCap( eCamConfNum, &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
	buff_size = img_size.width * img_size.height * 3;
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
	img_fp = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
	img_show_size = cv::Size( (int)(img_size.width * kHarrisShowCamMag), (int)(img_size.height * kHarrisShowCamMag) );
	img_show = cv::Mat::zeros(img_show_size, CV_8UC3);	// 表示する縮小画像を作成
	// 登録側の処理
	img_reg = cv::imread( fname, cv::IMREAD_COLOR);
	if ( img_reg == NULL )	return 0;
	img_reg_size = cv::Size( (int)(img_reg.cols * kHarrisRegMag), (int)(img_reg.rows * kHarrisRegMag) );
	img_reg_res = cv::Mat::zeros(img_reg_size, CV_8UC3);
	cv::resize( img_reg, img_reg_res, CV_INTER_CUBIC );
	img_reg_fp = cv::Mat::zeros(img_reg_size, CV_8UC3);
	img_reg_smt = cv::Mat::zeros(img_reg_size, CV_8UC3);
	img_reg_show_size = cv::Size( (int)(img_reg_size.width * kHarrisShowRegMag), (int)(img_reg_size.height * kHarrisShowRegMag) );
	img_reg_show = cv::Mat::zeros(img_reg_show_size, CV_8UC3);


// 特徴点抽出クラスを作成
	StartDirectShowCap( &dsc );	// キャプチャを開始
	cv::namedWindow( "Capture", cv::WINDOW_AUTOSIZE );	// ウィンドウを作成
	cv::namedWindow( "CaptureParam", cv::WINDOW_AUTOSIZE );	// ウィンドウを作成
	cv::namedWindow( "Registered", cv::WINDOW_AUTOSIZE );
	cv::namedWindow( "RegisteredParam", cv::WINDOW_AUTOSIZE );
	if ( mode == DHC_HARRIS ) {
		cv::createTrackbar( "QualityLevel", "CaptureParam", &(gHarrisCamParam.QualityLevel), kQualityLevelMax, TrackCallbackHarrisCap );
		cv::createTrackbar( "MinDistance", "CaptureParam", &(gHarrisCamParam.MinDistance), kMinDistanceMax, TrackCallbackHarrisCap );
		cv::createTrackbar( "BlockSize", "CaptureParam", &(gHarrisCamParam.BlockSize), kBlockSizeMax, TrackCallbackHarrisCap );
		cv::createTrackbar( "UseHarris", "CaptureParam", &(gHarrisCamParam.UseHarris), kUseHarrisMax, TrackCallbackHarrisCap );
		cv::createTrackbar( "k", "CaptureParam", &(gHarrisCamParam.HarrisParamK), kHarrisParamKMax, TrackCallbackHarrisCap );
		cv::createTrackbar( "QualityLevel", "RegisteredParam", &(gHarrisRegParam.QualityLevelReg), kQualityLevelRegMax, TrackCallbackHarrisReg );
		cv::createTrackbar( "MinDistance", "RegisteredParam", &(gHarrisRegParam.MinDistanceReg), kMinDistanceRegMax, TrackCallbackHarrisReg );
		cv::createTrackbar( "BlockSize", "RegisteredParam", &(gHarrisRegParam.BlockSizeReg), kBlockSizeRegMax, TrackCallbackHarrisReg );
		cv::createTrackbar( "UseHarris", "RegisteredParam", &(gHarrisRegParam.UseHarrisReg), kUseHarrisRegMax, TrackCallbackHarrisReg );
		cv::createTrackbar( "k", "RegisteredParam", &(gHarrisRegParam.HarrisParamKReg), kHarrisParamKRegMax, TrackCallbackHarrisReg );
	}

	// ウィンドウを移動
	cv::moveWindow( "Capture", 0, 30 );
	cv::moveWindow( "CaptureParam", 2200, 0 );
	cv::moveWindow( "Registered", 1280, 0 );
	cv::moveWindow( "RegisteredParam", 2200, 400 );
	/********** ループ処理 **********/
	for ( int cntcnt=0; ;cntcnt++ ) {
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
			switch ( mode ) {
				case DHC_HARRIS:
					// Harrisモード
					DetectGoodFeatures( img_cap, img_fp, &frps[0], &frnum[0] );
					break;
				case DHC_ENG:
					// 英文モード
					DetectEngDocPoints( img_cap, img_fp, &frps[0], &frnum[0] );
					break;
				case DHC_JP:
					// 日本語モード
					DetectJpDocPoints( img_cap, img_fp, &frps[0], &frnum[0] );
					break;
				default:
					break;
			}
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
				cv::circle( img_fp, currefps[i], 3, CV_RGB(0,255,0), -1, cv::LINE_AA, 0 );
			}
			// 描画テスト3（reffrpsのみ描画）
			/*
			for ( i = 0; i < reffrnum[0]; i++ ) {
				cv::circle( img_fp, frps[0][reffrps[0][i]], 3, CV_RGB(0,255,0), -1, cv::LINE_AA, 0 );
			}*/
			// 描画テスト2（特徴点のスコアに基づいて色をつける）
			/*
			for ( i = 0; i < frnum[0]; i++ ) {
				cv::circle( img_fp, frps[0][i], 3, CV_RGB( (int)(255.0L-(double)ref_score[i]*(255.0L/(double)(kCompareFrames-1))), (int)((double)ref_score[i]*(255.0L/(double)(kCompareFrames-1))), 0), -1, cv::LINE_AA, 0 );
			}*/
			// 描画テスト1（前のフレームおよび前々のフレームの対応点から線を引く）
			/*
			for ( i = 0; i < frnum[0]; i++ ) {
				if ( frcors[0][0][i] >= 0 ) {
					cv::line( img_fp, frps[0][i], frps[1][frcors[0][0][i]], CV_RGB(0,255,255), 2, cv::LINE_AA, 0 );
				}
				if ( frcors[0][1][i] >= 0 ) {
					cv::line( img_fp, frps[0][i], frps[2][frcors[0][1][i]], CV_RGB(0,255,0), 2, cv::LINE_AA, 0 );
				}
			}*/
			// テスト：前のフレームのみ使う
/*			if ( frps[0] != NULL && frps[1] != NULL && frnum[0] > 4 && frnum[1] > 4 ) {
				FindCorPoint( frps[0], frnum[0], frps[1], frnum[1], pcor );
				for ( i = 0; i < frnum[0]; i++ ) {
					if ( pcor[i] >= 0 ) {
						cv::line( img_fp, frps[0][i], frps[1][pcor[i]], CV_RGB( 0, 255, 255 ), 2, cv::LINE_AA, 0 );
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
						cv::line( img_fp, ps[i], ps0[pcor[i]], CV_RGB( 0, 255, 255 ), 2, cv::LINE_AA, 0 );
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
						cv::line( img_fp, ps[i], ps0[pcor[i]], CV_RGB( 0, 255, 255 ), 2, cv::LINE_AA, 0 );
					}
				}
			} */
			// 登録画像の処理
			switch ( mode ) {
				case DHC_HARRIS:
					cv::GaussianBlur(img_reg_res, img_reg_smt, cv::Size(5, 5), 0);
					DetectGoodFeaturesReg( img_reg_smt, img_reg_fp, &regps, &regnum );
					break;
				case DHC_ENG:
					DetectEngDocPointsReg( img_reg_res, img_reg_fp, &regps, &regnum );
					break;
				case DHC_JP:
					DetectJpDocPointsReg( img_reg_res, img_reg_fp, &regps, &regnum );
					break;
			}
			// 表示
			cv::resize( img_fp, img_show, CV_INTER_CUBIC );
			cv::resize( img_reg_fp, img_reg_show, CV_INTER_CUBIC );
			cv::imshow( "Capture", img_show );
//			cv::imwrite( "harris.jpg", img_har );
			cv::imshow( "Registered", img_reg_show );
		}
		key = cv::waitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'c':	// 対応点探索
					PauseDirectShowCap( &dsc );
					ConstructHashSub( regps, regnum );
					MakeNearsFromCentres( currefps, currefnum, &nears );
					RetrieveNN5( currefps, currefnum, nears, img_size, score, pcor );
					for ( i = 0; i < currefnum; i++ ) {
						if ( pcor[i] >= 0 ) {
							cv::circle( img_fp, currefps[i], 5, CV_RGB(255,255,0), -1, cv::LINE_AA, 0 );
							cv::circle( img_reg_fp, regps[pcor[i]], 5, CV_RGB(255,255,0), cv::LINE_AA, 0 );
						}
					}
					cv::resize( img_fp, img_show, CV_INTER_CUBIC );
					cv::resize( img_reg_fp, img_reg_show, CV_INTER_CUBIC );
					cv::imshow( "Capture", img_show );
					cv::imshow( "Registered", img_reg_show );
					for ( key = cv::waitKey(1); key < 0; key = cv::waitKey(1) );
					ResumeDirectShowCap( &dsc );
					break;
				case 's':	// 保存
					cv::imwrite( "cap_fp.bmp", img_fp );
					cv::imwrite( "reg_fp.bmp", img_reg_fp );
					break;
				case 'q':	// 終了
					goto end_camharris;
			}
		}

	}
end_camharris:
	cv::destroyWindow( "Capture" );	// ウィンドウを破棄
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
	cv::Mat img_cap, *img_fp, *img_reg, *img_reg_res, *img_reg_fp, *img_reg_smt;
	cv::Mat img_anno, *img_anno_res, *img_anno_fp, *img_anno_smt;
	cv::Point *ps = NULL, *ps0 = NULL, *regps = NULL;
	cv::Point *frps[kMaxFramePoints];	/* FRame PointS : 複数フレームの特徴点集合 */
	int frnum[kMaxFramePoints];	/* FRame NUMbers : frpsの特徴点数 */
	int **frcors[kMaxFramePoints];	/* FRame CORrespondenceS : 対応関係の集合の集合 */
	int *reffrps[kMaxFramePoints];	/* REFined FRame PointS : 厳選された特徴点集合（の集合） */
	int **frcors_tmp, *reffrps_tmp;	/* frcors,reffrpsをずらすときの一時変数 */
	int reffrnum[kMaxFramePoints];	/* REFined FRame NUMbers : reffrpsの特徴点数 */
	int ref_score[kMaxPointNum];	/* 特徴点選別の際のスコア */
	strProjParam frparam[kMaxFramePoints];	/* FRame PARAMeters : 過去のフレームの特徴点を現在のフレームに合わせるためのパラメータ */
	strProjParam zero_param;
	cv::Point currefps[kMaxPointNum];	/* CURrent REFined PointS : reffrpsを統合し，現在のフレームの特徴点として用いるもの */
	int currefnum;	/* currefpsの特徴点数 */
	int used[kMaxFramePoints][kMaxPointNum];	/* currefpsにおいてフレームの点が使用されたか否か */
	int i, j, k, num = 0, num0 = NULL, regnum = 0, **nears = NULL, score[kMaxDocNum], pcor[kMaxPointNum];
	cv::Size img_size;
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
	img_reg = cv::imread( fname, cv::IMREAD_COLOR);
	if ( img_reg == NULL )	return 0;
	img_reg_res = cv::Mat::zeros( cv::Size( (int)(img_reg.cols * kHarrisTestMag), (int)(img_reg.rows * kHarrisTestMag) ), CV_8UC3 );
	cv::resize( img_reg, img_reg_res, CV_INTER_CUBIC );
	img_reg_fp = cv::Mat::zeros(cv::Size( img_reg_res.cols, img_reg_res.rows ), CV_8UC3);
	img_reg_smt = cv::Mat::zeros(cv::Size( img_reg_res.cols, img_reg_res.rows ), CV_8UC3);
	// 書き込み側の処理
	img_anno = cv::imread( fname_anno, cv::IMREAD_COLOR);
	if ( img_anno == NULL )	return 0;
	img_anno_res = cv::Mat::zeros( cv::Size( (int)(img_anno.cols * kHarrisTestMag), (int)(img_anno.rows * kHarrisTestMag) ), CV_8UC3 );
	cv::resize( img_anno, img_anno_res, CV_INTER_CUBIC );
	img_anno_fp = cv::Mat::zeros(cv::Size( img_anno_res.cols, img_anno_res.rows ), CV_8UC3);
	img_anno_smt = cv::Mat::zeros(cv::Size( img_anno_res.cols, img_anno_res.rows ), CV_8UC3);

	img_size.width = img_anno.cols;
	img_size.height = img_anno.rows;


// 特徴点抽出クラスを作成
	cv::namedWindow( "Capture", cv::WINDOW_AUTOSIZE );	// ウィンドウを作成
	cv::createTrackbar( "QualityLevel", "Capture", &(gHarrisCamParam.QualityLevel), kQualityLevelMax, TrackCallbackHarrisCap );
	cv::createTrackbar( "MinDistance", "Capture", &(gHarrisCamParam.MinDistance), kMinDistanceMax, TrackCallbackHarrisCap );
	cv::createTrackbar( "BlockSize", "Capture", &(gHarrisCamParam.BlockSize), kBlockSizeMax, TrackCallbackHarrisCap );
	cv::createTrackbar( "UseHarris", "Capture", &(gHarrisCamParam.UseHarris), kUseHarrisMax, TrackCallbackHarrisCap );
	cv::createTrackbar( "k", "Capture", &(gHarrisCamParam.HarrisParamK), kHarrisParamKMax, TrackCallbackHarrisCap );

	cv::namedWindow( "Registered", cv::WINDOW_AUTOSIZE );
	cv::createTrackbar( "QualityLevel", "Registered", &(gHarrisRegParam.QualityLevelReg), kQualityLevelRegMax, TrackCallbackHarrisReg );
	cv::createTrackbar( "MinDistance", "Registered", &(gHarrisRegParam.MinDistanceReg), kMinDistanceRegMax, TrackCallbackHarrisReg );
	cv::createTrackbar( "BlockSize", "Registered", &(gHarrisRegParam.BlockSizeReg), kBlockSizeRegMax, TrackCallbackHarrisReg );
	cv::createTrackbar( "UseHarris", "Registered", &(gHarrisRegParam.UseHarrisReg), kUseHarrisRegMax, TrackCallbackHarrisReg );
	cv::createTrackbar( "k", "Registered", &(gHarrisRegParam.HarrisParamKReg), kHarrisParamKRegMax, TrackCallbackHarrisReg );
	/********** ループ処理 **********/
	for ( ; ; ) {
		if ( paused < 0 ) {	// ポーズされていない
			// 登録画像の処理
//			cv::GaussianBlur(img_reg_res, img_reg_smt, cv::Size(5, 5), 0);
			DetectGoodFeaturesReg( img_reg_res, img_reg_fp, &regps, &regnum );
			// 書き込み画像の処理
//			cv::GaussianBlur(img_anno_res, img_anno_smt, cv::Size(5, 5), 0);
			DetectGoodFeatures( img_anno_res, img_anno_fp, &ps, &num );
			// 表示
			cv::imshow( "Capture", img_anno_fp );
//			cv::imwrite( "harris.jpg", img_har );
			cv::imshow( "Registered", img_reg_fp );
		}
		key = cv::waitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'c':	// 対応点探索
					ConstructHashSub( regps, regnum );
					MakeNearsFromCentres( ps, num, &nears );
					RetrieveNN5( ps, num, nears, img_size, score, pcor );
					for ( i = 0; i < num; i++ ) {
						if ( pcor[i] >= 0 ) {
							cv::circle( img_anno_fp, ps[i], 5, CV_RGB(255,255,0), -1, cv::LINE_AA, 0 );
							cv::circle( img_reg_fp, regps[pcor[i]], 5, CV_RGB(255,255,0), cv::LINE_AA, 0 );
						}
					}
					cv::imshow( "Capture", img_anno_fp );
					cv::imshow( "Registered", img_reg_fp );
					for ( key = cv::waitKey(1); key < 0; key = cv::waitKey(1) );
					break;
				case 's':	// 保存
					cv::imwrite( "cap_fp.bmp", img_anno_fp );
					cv::imwrite( "reg_fp.bmp", img_reg_fp );
					break;
				case 'q':	// 終了
					goto end_camharris;
			}
		}

	}
end_camharris:
	cv::destroyWindow( "Capture" );	// ウィンドウを破棄
	return 1;
}

void DetectHarris( cv::Mat src, cv::Mat dst )
// Harrisを適用する
{
	double minVal = 0.0, maxVal = 0.0, scale, shift, min = 0, max = 255;
	cv::Mat img_har, *img_gray, *img_out;
	cv::Size img_size;

	img_size.width = src.cols;
	img_size.height = src.rows;

	img_gray = cv::Mat::zeros(img_size, CV_8SC1);	// 画像を作成
	img_har = cv::Mat::zeros(img_size, CV_32FC1);	// 画像を作成
	img_out = cv::Mat::zeros(img_size, CV_8UC1);	// 画像を作成

	// Harrisを適用
	cv::cvtColor( src, img_gray, cv::COLOR_RGB2GRAY );	// グレイスケールに変換
	cvCornerHarris( img_gray, img_har, 3, 5, 0.04 );	// Harrisを適用
	cvMinMaxLoc( img_har, &minVal, &maxVal, NULL, NULL, 0 );
	scale = (max - min)/(maxVal - minVal);
	shift = -minVal * scale + min;
	cv::convertScaleAbs( img_har, img_out, scale, shift );
	cv::cvtColor( img_out, dst, cv::COLOR_GRAY2RGB ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
}
//#define	CENTROID
#define	SECOND_ORDER_MOMENT
#define	kMaxNoiseArea	(10)
int CalcCentresCH(cv::Point **ps0, CvSeq *contours, cv::Size *size, double **areas0)
// 重心を計算する
// 06/01/12	面積の計算を追加
{
	int i, num;
	CvSeq *con0;
	CvMoments mom;
	cv::Point *ps;


	// 点の数をカウント（暫定）
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// ゴミ等で連結成分が多すぎるとき（本来は大きさを調べたりすべき）
	// 点を入れる配列を確保（numの値は正確ではないが，大きめということで）
	ps = (cv::Point *)calloc(num, sizeof(cv::Point));
	*ps0 = ps;
	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 && i < num ; con0 = con0->h_next )
    {
		double d00;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < kMaxNoiseArea ) continue;	// 小さすぎる連結成分は除外

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
	num = i;	// 連結成分数の更新

	return num;
}

int MakeCentresFromImageCH(cv::Point **ps, cv::Mat img, cv::Size *size, double **areas)
// 画像のファイル名を与えて重心を計算し，重心の数を返す
// 06/01/12	面積の計算を追加
{
	int num, ret;
    // CvMemStorage removed (not needed in OpenCV 4)
	CvSeq *contours = 0;

	size.cols = img.cols;
	size.rows = img.rows;
	
    ret = cv::findContours( img, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0) );	// 連結成分を抽出する
	if ( ret <= 0 ) {	// 連結成分が見つからなかった // cvReleaseMemStorage removed
		return 0;
	}
	num = CalcCentresCH(ps, contours, size, areas);	// 各連結成分の重心を計算する

	cvClearSeq( contours ); // cvReleaseMemStorage removed

	return num;
}

void DetectEngDocPoints( cv::Mat src, cv::Mat dst, cv::Point **pps, int *pnum )
// 英文モードで特徴点を抽出する
{
	int i, num;
	cv::Point *ps;
	cv::Size size;
	cv::Mat img_con;

	img_con = cv::Mat::zeros(src.size(), CV_8UC1);
	GetConnectedImageCam( src, img_con, eCamGaussMaskSize );
	size.width = img_con.cols;
	size.height = img_con.rows;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// 特徴点画像の描画
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cv::circle( dst, ps[i], 6, CV_RGB(255,255,255), -1, cv::LINE_AA, 0 );	// 黒白
		cv::circle( dst, ps[i], 4, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );	// 黒白
	} // cvReleaseImage removed (cv::Mat auto-manages memory)
	if ( num > 0 ) {	// 特徴点が1つでも抽出されたら
		*pps = ps;
		*pnum = num;
	}
}

void DetectEngDocPointsReg( cv::Mat src, cv::Mat dst, cv::Point **pps, int *pnum  )
// 英文モードで特徴点を抽出する（登録側）
{
	int i, num;
	cv::Point *ps;
	cv::Size size;
	cv::Mat img_con;

	img_con = GetConnectedImageReg( src );
	size.width = img_con.cols;
	size.height = img_con.rows;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// 特徴点画像の描画
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cv::circle( dst, ps[i], 6, CV_RGB(255,255,255), -1, cv::LINE_AA, 0 );	// 黒白
		cv::circle( dst, ps[i], 4, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );	// 黒白
	} // cvReleaseImage removed (cv::Mat auto-manages memory)
	if ( num > 0 ) {	// 特徴点が1つでも抽出されたら
		*pps = ps;
		*pnum = num;
	}
}

void DetectJpDocPoints( cv::Mat src, cv::Mat dst, cv::Point **pps, int *pnum )
// 日本語モードで特徴点を抽出する
{
	int i, num;
	cv::Point *ps;
	cv::Size size;
	cv::Mat img_con;

	img_con = GetConnectedImageCamJp( src );
	size.width = img_con.cols;
	size.height = img_con.rows;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// 特徴点画像の描画
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cv::circle( dst, ps[i], 6, CV_RGB(255,255,255), -1, cv::LINE_AA, 0 );	// 黒白
		cv::circle( dst, ps[i], 4, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );	// 黒白
	} // cvReleaseImage removed (cv::Mat auto-manages memory)
	if ( num > 0 ) {	// 特徴点が1つでも抽出されたら
		*pps = ps;
		*pnum = num;
	}
}

void DetectJpDocPointsReg( cv::Mat src, cv::Mat dst, cv::Point **pps, int *pnum  )
// 英文モードで特徴点を抽出する（登録側）
{
	int i, num;
	cv::Point *ps;
	cv::Size size;
	cv::Mat img_con;

	img_con = GetConnectedImageRegJp( src );
	size.width = img_con.cols;
	size.height = img_con.rows;
	num = MakeCentresFromImageCH( &ps, img_con, &size, NULL );
	// 特徴点画像の描画
	cvCopy( src, dst, 0 );
	for ( i = 0; i < num; i++ ) {
		cv::circle( dst, ps[i], 6, CV_RGB(255,255,255), -1, cv::LINE_AA, 0 );	// 黒白
		cv::circle( dst, ps[i], 4, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );	// 黒白
	} // cvReleaseImage removed (cv::Mat auto-manages memory)
	if ( num > 0 ) {	// 特徴点が1つでも抽出されたら
		*pps = ps;
		*pnum = num;
	}
}

void DetectGoodFeatures( cv::Mat src, cv::Mat dst, cv::Point **pps, int *pnum )
// cvGoodFeaturesToTrackで特徴点を抽出する
{
	cv::Mat img_gray, *img_eig, *img_temp;
	cv::Size img_size;
	cv::Point pt, *ps;
	cv::Point2f *corners = NULL;
	int i, cor_cnt, block_size;
	double quality_level, min_distance, k;

	img_size.width = src.cols;
	img_size.height = src.rows;

	img_gray = cv::Mat::zeros(img_size, CV_8UC1);
	img_eig = cv::Mat::zeros(img_size, CV_32FC1);
	img_temp = cv::Mat::zeros(img_size, CV_32FC1);

	corners = (cv::Point2f *)calloc( kMaxPointNum, sizeof(cv::Point2f) );
	cor_cnt = kMaxPointNum;
	cv::cvtColor( src, img_gray, cv::COLOR_RGB2GRAY );
	quality_level = 0.01 * (double)(gHarrisCamParam.QualityLevel + 1);
	min_distance = (double)gHarrisCamParam.MinDistance;
	block_size = gHarrisCamParam.BlockSize * 2 + 3;
	k = 0.01 * (double)gHarrisCamParam.HarrisParamK;
	cv::goodFeaturesToTrack( img_gray, img_eig, img_temp, corners, &cor_cnt, quality_level, min_distance, 0, block_size, gHarrisCamParam.UseHarris, k );
//	printf("cor_cnt : %d\n", cor_cnt);
	ps = (cv::Point *)calloc( cor_cnt, sizeof(cv::Point) );
//	dst.setTo(cv::Scalar::all(0));
	cvCopy( src, dst, 0 );
	for ( i = 0; i < cor_cnt; i++ ) {
		pt.x = (int)corners[i].x;
		pt.y = (int)corners[i].y;
//		cv::circle( dst, pt, 2, CV_RGB(255,0,0), -1, 8, 0 );	// 赤
		cv::circle( dst, pt, 6, CV_RGB(255,255,255), -1, cv::LINE_AA, 0 );	// 黒白
		cv::circle( dst, pt, 4, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );	// 黒白
		ps[i] = pt;
	} // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
	free( corners );
	*pps = ps;
	*pnum = cor_cnt;
}

void DetectGoodFeaturesReg( cv::Mat src, cv::Mat dst, cv::Point **pps, int *pnum  )
// cvGoodFeaturesToTrackで特徴点を抽出する（登録画像）
{
	cv::Mat img_gray, *img_eig, *img_temp;
	cv::Size img_size;
	cv::Point pt, *ps;
	cv::Point2f *corners = NULL;
	int i, cor_cnt, block_size;
	double quality_level, min_distance, k;

	img_size.width = src.cols;
	img_size.height = src.rows;

	img_gray = cv::Mat::zeros(img_size, CV_8UC1);
	img_eig = cv::Mat::zeros(img_size, CV_32FC1);
	img_temp = cv::Mat::zeros(img_size, CV_32FC1);

	corners = (cv::Point2f *)calloc( 10240, sizeof(cv::Point2f) );
	cor_cnt = 10240;
	cv::cvtColor( src, img_gray, cv::COLOR_RGB2GRAY );
	quality_level = 0.01 * (double)(gHarrisRegParam.QualityLevelReg + 1);
	min_distance = (double)gHarrisRegParam.MinDistanceReg;
	block_size = gHarrisRegParam.BlockSizeReg * 2 + 3;
	k = 0.01 * (double)gHarrisRegParam.HarrisParamKReg;
	cv::goodFeaturesToTrack( img_gray, img_eig, img_temp, corners, &cor_cnt, quality_level, min_distance, 0, block_size, gHarrisRegParam.UseHarrisReg, k );
//	printf("cor_cnt : %d\n", cor_cnt);
	ps = (cv::Point *)calloc( cor_cnt, sizeof(cv::Point) );
//	dst.setTo(cv::Scalar::all(0));
	cvCopy( src, dst, 0 );
	for ( i = 0; i < cor_cnt; i++ ) {
		pt.x = (int)corners[i].x;
		pt.y = (int)corners[i].y;
//		cv::circle( dst, pt, 2, CV_RGB(255,0,0), -1, 8, 0 );	// 赤
		cv::circle( dst, pt, 6, CV_RGB(255,255,255), -1, cv::LINE_AA, 0 );	// 黒白
		cv::circle( dst, pt, 4, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );	// 黒白
		ps[i] = pt;
	} // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
	free( corners );
	*pps = ps;
	*pnum = cor_cnt;
}

void FindCorPoint( cv::Point *ps, int num, cv::Point *corps, int cornum, int *pcor, strProjParam *param )
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
