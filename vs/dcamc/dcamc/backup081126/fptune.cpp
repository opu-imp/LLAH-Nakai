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
#include "dirs.h"
#include "hist.h"
#include "proj4p.h"
#include "dscap.h"
#include "dcamc.h"
#include "init.h"
#include "fptune.h"
#include "cFpTuneReg.h"
#include "cFpTuneCap.h"

cFpTuneReg *ptr_reg = NULL;
cFpTuneCap *ptr_cap = NULL;
int show_mode = SHOW_REG_IMG, cap_show_mode = SHOW_CAP_IMG, paused = -1;
CvPoint caporig, regorig;

void TrackCallbackReg( int param )
// 登録文書のトラックバーのコールバック関数
{
	if ( ptr_reg != NULL ) {
		ptr_reg->Extract();
		switch ( show_mode ) {
			case SHOW_REG_IMG:
				cvShowImage( "Registered", ptr_reg->img );
				break;
			case SHOW_REG_SMT1:
				cvShowImage( "Registered", ptr_reg->smt1 );
				break;
			case SHOW_REG_BIN1:
				cvShowImage( "Registered", ptr_reg->bin1 );
				break;
			case SHOW_REG_SMT2:
				cvShowImage( "Registered", ptr_reg->smt2 );
				break;
			case SHOW_REG_BIN2:
				cvShowImage( "Registered", ptr_reg->bin2 );
				break;
			case SHOW_REG_PT:
				cvShowImage( "Registered", ptr_reg->pt );
				break;
			case SHOW_REG_OVER:
				cvShowImage( "Registered", ptr_reg->overlap );
				break;
		}
	}
}

void TrackCallbackCap( int param )
// キャプチャ画像のトラックバーのコールバック関数
{
	if ( ptr_cap != NULL && paused > 0 ) {
		ptr_cap->Extract();
		switch ( cap_show_mode ) {
			case SHOW_CAP_IMG:
				cvShowImage( "Capture", ptr_cap->img );
				break;
			case SHOW_CAP_BIN1:
				cvShowImage( "Capture", ptr_cap->bin1 );
				break;
			case SHOW_CAP_SMT:
				cvShowImage( "Capture", ptr_cap->smt );
				break;
			case SHOW_CAP_BIN2:
				cvShowImage( "Capture", ptr_cap->bin2 );
				break;
			case SHOW_CAP_PT:
				cvShowImage( "Capture", ptr_cap->pt );
				break;
			case SHOW_CAP_OVER:
				cvShowImage( "Capture", ptr_cap->overlap );
				break;
		}
	}
}

int TuneFeaturePointParam( char *fname )
// 特徴点抽出パラメータの手動チューニングツール
{
	int key;
	long buff_size;
	unsigned char *img_buff;
	IplImage *img_cap;
	CvSize img_size;
	strDirectShowCap dsc;
	// 登録画像の特徴点抽出クラス
	cFpTuneReg c_reg( fname, kDefaultTuneRegGParam1, kDefaultTuneRegAdpBlock, kDefaultTuneRegAdpSub, kDefaultTuneRegGParam2, kDefaultTuneRegThr );
	// キャプチャ画像の特徴点抽出クラス
//	cFpTuneCap *ptr_cap;

	/********** 登録画像側の処理 **********/
	ptr_reg = &c_reg;
	cvNamedWindow( "Registered", CV_WINDOW_AUTOSIZE );
	cvCreateTrackbar( "GParam1", "Registered", &c_reg.gparam1, kTuneGParam1Max, TrackCallbackReg );
	cvCreateTrackbar( "AdpBlock", "Registered", &c_reg.adp_block, kTuneAdpBlockMax, TrackCallbackReg );
	cvCreateTrackbar( "AdpSub", "Registered", &c_reg.adp_sub, kTuneAdpSubMax, TrackCallbackReg );
	cvCreateTrackbar( "GParam2", "Registered", &c_reg.gparam2, kTuneGParam2Max, TrackCallbackReg );
	cvCreateTrackbar( "Thr", "Registered", &c_reg.thr, kTuneThrMax, TrackCallbackReg );
//	cvCreateTrackbar( "Mode", "Registered", &show_mode, SHOW_REG_MAX, TrackCallbackReg );
	cvShowImage( "Registered", c_reg.img );	// 画像を表示
	/********** キャプチャ画像側の処理 **********/
	if ( ReadIniFile() == 0 ) {	// iniファイルの読み込みに失敗
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 1;
	}
	InitDirectShowCap( &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
	buff_size = img_size.width * img_size.height * 3;
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// 画像を作成
	// 特徴点抽出クラスを作成
	ptr_cap = new cFpTuneCap( img_cap, kDefaultTuneCapAdpBlock, kDefaultTuneCapAdpSub, kDefaultTuneCapGParam, kDefaultTuneCapThr );
	StartDirectShowCap( &dsc );	// キャプチャを開始
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// ウィンドウを作成
	cvCreateTrackbar( "AdpBlock", "Capture", &ptr_cap->adp_block, kTuneCapAdpBlockMax, TrackCallbackCap );
	cvCreateTrackbar( "AdpSub", "Capture", &ptr_cap->adp_sub, kTuneCapAdpSubMax, TrackCallbackCap );
	cvCreateTrackbar( "GParam", "Capture", &ptr_cap->gparam, kTuneCapGParamMax, TrackCallbackCap );
	cvCreateTrackbar( "Thr", "Capture", &ptr_cap->thr, kTuneCapThrMax, TrackCallbackCap );
	// ウィンドウを配置（この場所から移動されないものとする）
	cvMoveWindow( "Capture", 0, kTaskBarHeight );
	cvMoveWindow( "Registered", kCvWindowLeftMargin + kCvWindowRightMargin + ptr_cap->img->width, kTaskBarHeight );
	// 原点の相対的な座標
	regorig.x = ptr_cap->img->width + kCvWindowLeftMargin + kCvWindowRightMargin;
	regorig.y = 0;
	caporig.x = - kCvWindowLeftMargin - kCvWindowRightMargin - ptr_cap->img->width;
	caporig.y = 0;
	/********** ループ処理 **********/
	for ( ; ; ) {
		if ( paused < 0 ) {	// ポーズされていない
			// キャプチャ
			CaptureDirectShowCap( &dsc, img_buff, buff_size );
			// バッファを画像に変換
			Buff2ImageData( img_buff, img_cap );
			ptr_cap->Extract();
			ptr_cap->DrawCor( NULL, ptr_reg->pfs, regorig );
			ptr_reg->DrawCor( NULL, ptr_cap->pfs, caporig );
			// 表示
			switch ( cap_show_mode ) {
				case SHOW_CAP_IMG:
					cvShowImage( "Capture", ptr_cap->img );
					break;
				case SHOW_CAP_BIN1:
					cvShowImage( "Capture", ptr_cap->bin1 );
					break;
				case SHOW_CAP_SMT:
					cvShowImage( "Capture", ptr_cap->smt );
					break;
				case SHOW_CAP_BIN2:
					cvShowImage( "Capture", ptr_cap->bin2 );
					break;
				case SHOW_CAP_PT:
					cvShowImage( "Capture", ptr_cap->pt );
					break;
				case SHOW_CAP_OVER:
					cvShowImage( "Capture", ptr_cap->overlap );
					break;
				case SHOW_CAP_COR:
					cvShowImage( "Capture", ptr_cap->cor );
					break;
			}
			if ( show_mode == SHOW_REG_COR )	cvShowImage( "Registered", ptr_reg->cor );
		}
		key = cvWaitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				// 登録画像のキー
				case 'a':	// 元画像
					show_mode = SHOW_REG_IMG;
					cvShowImage( "Registered", c_reg.img );
					break;
				case 's':	// ガウシアン１
					show_mode = SHOW_REG_SMT1;
					cvShowImage( "Registered", c_reg.smt1 );
					break;
				case 'd':	// 二値化１
					show_mode = SHOW_REG_BIN1;
					cvShowImage( "Registered", c_reg.bin1 );
					break;
				case 'f':	// ガウシアン２
					show_mode = SHOW_REG_SMT2;
					cvShowImage( "Registered", c_reg.smt2 );
					break;
				case 'g':	// 二値化２
					show_mode = SHOW_REG_BIN2;
					cvShowImage( "Registered", c_reg.bin2 );
					break;
				case 'h':	// 特徴点
					show_mode = SHOW_REG_PT;
					cvShowImage( "Registered", c_reg.pt );
					break;
				case 'j':	// 重ね合わせ
					show_mode = SHOW_REG_OVER;
					cvShowImage( "Registered", c_reg.overlap );
					break;
				case 'k':	// 対応関係
					show_mode = SHOW_REG_COR;
					cvShowImage( "Registered", c_reg.cor );
					break;
				// キャプチャ画像のキー
				case 'z':	// 元画像
					cap_show_mode = SHOW_CAP_IMG;
					cvShowImage( "Capture", ptr_cap->img );
					break;
				case 'x':	// 二値化１
					cap_show_mode = SHOW_CAP_BIN1;
					cvShowImage( "Capture", ptr_cap->bin1 );
					break;
				case 'c':	// ガウシアン
					cap_show_mode = SHOW_CAP_SMT;
					cvShowImage( "Capture", ptr_cap->smt );
					break;
				case 'v':	// 二値化２
					cap_show_mode = SHOW_CAP_BIN2;
					cvShowImage( "Capture", ptr_cap->bin2 );
					break;
				case 'b':	// 特徴点
					cap_show_mode = SHOW_CAP_PT;
					cvShowImage( "Capture", ptr_cap->pt );
					break;
				case 'n':	// 重ね合わせ
					cap_show_mode = SHOW_CAP_OVER;
					cvShowImage( "Capture", ptr_cap->overlap );
					break;
				case 'm':	// 対応関係
					cap_show_mode = SHOW_CAP_COR;
					cvShowImage( "Capture", ptr_cap->cor );
					break;
				// その他
				case 'p':	// 一時停止
					if ( paused < 0 ) // ポーズされていない
						PauseDirectShowCap( &dsc );
					else
						ResumeDirectShowCap( &dsc );
					paused *= -1;
					break;
				case 'q':	// 終了
					goto end_tune;
			}
		}

	}
end_tune:
	delete ptr_cap;
	cvDestroyWindow( "Capture" );	// ウィンドウを破棄
	cvDestroyWindow( "Registered" );	// ウィンドウを破棄
	StopDirectShowCap( &dsc );	// キャプチャを終了
	ReleaseDirectShowCap( &dsc );	// DirectShowの諸々をリリース
	return 1;
}
