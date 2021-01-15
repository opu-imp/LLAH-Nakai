#define	GLOBAL_DEFINE

#include <stdio.h>
#include <unistd.h>
#include "cv.h"
#include "highgui.h"
#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "annex.h"
#include "annex_st.h"
#include "transform.h"
#include "annex_diff.h"
#include "clpoints.h"
#include "nstr.h"
#include "proctime.h"

unsigned char max3( unsigned char d1, unsigned char d2, unsigned char d3 )
// 3つの最大値を返す
{
	if ( d1 > d2 ) {
		if ( d1 > d3 )	return d1;
		else	return d3;
	}
	else {
		if ( d2 > d3 )	return d2;
		else	return d3;
	}
}

void RGB2Gray( IplImage *diff, IplImage *gray )
// カラー画像をグレイスケールに変換する（RGBの最大値をセットする）
{
	int i, j, w, h, wstep_diff, wstep_gray;
	unsigned char *diff_dat;
	
	w = diff->width;
	h = diff->height;
	wstep_diff = diff->widthStep;
	wstep_gray = gray->widthStep;
	
	for ( j = 0; j < h; j++ ) {
		for ( i = 0; i < w; i++ ) {
			diff_dat = (unsigned char *)&diff->imageData[j*wstep_diff+i*3];
			gray->imageData[j*wstep_gray+i] = max3( diff_dat[0], diff_dat[1], diff_dat[2] );
		}
	}
}


int FindFromDefaultDir( char *fname, char *init_fn, char *out_fn )
// カレントディレクトリおよびinit_fnに記述されているディレクトリからfnameを探し、パスを作成してout_fnに入れる
// 見つからなければ0を返す
{
	char line[kMaxLineLen];
	FILE *fp, *fp_init;
	
	if ( ( fp = fopen( fname, "r" ) ) != NULL ) {	// そのままで存在
		strcpy( out_fn, fname );
		fclose( fp );
		return 1;
	}
	if ( fname[0] == '/' ) {	// 先頭が'/'（ディレクトリが指定済み）
		return 0;
	}
	if ( ( fp_init = fopen( init_fn, "r" ) ) == NULL ) {	// init_fnが開けない
		return 0;
	}
	while ( fgets( line, kMaxLineLen, fp_init ) != NULL ) {	// 一行読み込む
		if ( line[strlen( line ) - 1] == '\n' ) {	// 末尾に改行があったら
			line[strlen( line ) - 1] = '\0';
		}
		strcat(line, fname);	// パスを作成
		if ( ( fp = fopen( line, "r" ) ) != NULL ) {	// 作成したファイル名で存在
			strcpy( out_fn, line );
			fclose( fp );
			fclose( fp_init );
			return 1;
		}
	}
	return 0;
}

int InitAnnex( int argc, char *argv[], int *clcptr )
// コマンドラインから各種パラメータを設定する（annex用）
{
	int result, i, num, den;
	
	// デフォルト値に設定
	eGroup1Num = kDefaultGroup1Num;
	eGroup2Num = kDefaultGroup2Num;
	eGroup3Num = kDefaultGroup3Num;
	eInvType = kDefaultInvType;
	eDiscNum = kDefaultDiscNum;
	*clcptr = kDefaultClusters;
	eResizeMode = kDefaultResizeMode;
	eOrigMode = kDefaultOrigMode;
	eTransMode = kDefaultTransMode;
	strcpy( eOutPutDir, kDefaultOutPutDir );
	eResizeMainOrig = kDefaultResizeMainOrig;
	eResizeMainAnno = kDefaultResizeMainAnno;
	eBinThr = -1;
	eClosingIter = kDefaultClosingIter;
	eMinAreaMain = kDefaultMinAreaMain;
	eDilMaskIter = kDefaultDilMaskIter;
	strcpy( eAcr2Path, kDefaultAcr2Path );
	eClScale = kDefaultClScale;
	eErodeClIter = kDefaultErodeClIter;
	eClCmpStep = kDefaultClCmpStep;
	eKMeansMaxIter = kDefaultKMeansMaxIter;
	eKMeansEpsilon = kDefaultKMeansEpsilon;
	eMinAreaCl = kDefaultMinAreaCl;
	eMaxAreaCl = kDefaultMaxAreaCl;
	eDiffNear = kDefaultDiffNear;
	eDiffLeaveThr = kDefaultDiffLeaveThr;
	eDiffEraseThr = kDefaultDiffEraseThr;
	eOrigErodeIterMain = kDefaultOrigErodeIterMain;
	eOrigGaussParamMain = kDefaultOrigGaussParamMain;
	eAnnoGaussPramMain = kDefaultAnnoGaussParamMain;

	// パラメータ解析
	while ( ( result = getopt( argc, argv, "Ac:d:D:e:E:g:G:h:H:i:I:l:L:m:M:n:N:Oo:p:P:R:St:T:z:Z:" ) ) != -1 ) {
		switch ( result ) {
			case 'n':	// パラメータn
				eGroup1Num = atoi( optarg );
				break;
			case 'm':	// パラメータm
				eGroup2Num = atoi( optarg );
				break;
			case 'd':	// パラメータk（離散化レベル数）
				eDiscNum = atoi( optarg );
				break;
			case 'c':	// クラスタ数
				*clcptr = atoi( optarg );
				break;
			case 'S':	// リサイズモード（現在は無効）
				eResizeMode = 1;
				break;
			case 'O':	// 比較対象をオリジナルPDFから得た画像とするモード
				eOrigMode = 1;
				break;
			case 'A':	// TransModeをアフィン変換にする
				eTransMode = AFFINE;
				break;
			case 'o':	// 出力ディレクトリの設定
				strcpy( eOutPutDir, optarg );
				AddSlash( eOutPutDir, kMaxPathLen );
				break;
			case 'z':	// mainで縮小する際の倍率（元画像）
				eResizeMainOrig = atof( optarg );
				break;
			case 'Z':	// mainで縮小する際の倍率（書き込み画像）
				eResizeMainAnno = atof( optarg );
				break;
			case 't':	// 二値化する際の閾値
				eBinThr = atof( optarg );
				break;
			case 'h':	// ばらばらになった連結成分を結合する際のモルフォロジ演算のパラメータ
				eClosingIter = atoi( optarg );
				break;
			case 'M':	// mainでノイズ除去する際の最小面積
				eMinAreaMain = atof( optarg );
				break;
			case 'D':	// マスクを太らせる際のパラメータ
				eDilMaskIter = atoi( optarg );
				break;
			case 'P':	// 不変量の離散化テーブルを作成するコマンドacr2のパス
				strcpy( eAcr2Path, optarg );
				break;
			case 'L':	// 特徴点抽出時に縮小する際の倍率
				eClScale = atof( optarg );
				break;
			case 'I':	// クラスタリングの前にErodeする際のパラメータ
				eErodeClIter = atoi( optarg );
				break;
			case 'T':	// クラスタリング前に縮小する際のステップ（4ならサイズは約1/4になる）
				eClCmpStep = atoi( optarg );
				break;
			case 'i':	// クラスタリング時の最大繰り返し数
				eKMeansMaxIter = atoi( optarg );
				break;
			case 'p':	// クラスタリング時の精度パラメータ
				eKMeansEpsilon = atof( optarg );
				break;
			case 'e':	// 特徴点抽出でのノイズ除去時の最小面積
				eMinAreaCl = atof( optarg );
				break;
			case 'E':	// 特徴点抽出でのノイズ除去時の最大面積
				eMaxAreaCl = atof( optarg );
				break;
			case 'N':	// 最も近い画素を探す際の範囲
				eDiffNear = atoi( optarg );
				break;
			case 'l':	// 差分を取らずにそのまま残す際の閾値
				eDiffLeaveThr = atof( optarg );
				break;
			case 'H':	// 十分に近いとみなす際の閾値
				eDiffEraseThr = atof( optarg );
				break;
			case 'R':	// 元画像がPDFの場合、元画像をerodeするパラメータ
				eOrigErodeIterMain = atoi( optarg );
				break;
			case 'G':	// 元画像がスキャンの場合、元画像にGaussianをかけるパラメータ
				eOrigGaussParamMain = atoi( optarg );
				break;
			case 'g':	// 書き込み画像にGaussianをかけるパラメータ
				eAnnoGaussPramMain = atoi( optarg );
				break;
			case ':':
				fprintf( stderr, "Error: %c needs value\n", result );
				return 0;
			case '?':
				fprintf( stderr, "Error: unknown parameter %c\n", result );
				return 0;
		}
	}
	if ( eBinThr < 0 ) {	// 変更されていない
		if ( eOrigMode )	eBinThr = kDefaultBinThrOrig;
		else	eBinThr = kDefaultBinThrScan;
	}
	if ( optind + 1 >= argc ) {	// ファイル名が足りない
		fprintf( stderr, "Usage: annex [option] [original image] [annotated image]\n" );
		return 0;
	}
	if ( !FindFromDefaultDir( argv[optind], kInitFileName, eOrigFileName ) ) {
		fprintf( stderr, "Error: %s cannot find\n", argv[optind] );
		return 0;
	}
	if ( !FindFromDefaultDir( argv[optind+1], kInitFileName, eAnnoFileName ) ) {
		fprintf( stderr, "Error: %s cannot find\n", argv[optind+1] );
		return 0;
	}
		
//	strcpy( eOrigFileName, argv[optind] );
//	strcpy( eAnnoFileName, argv[optind+1] );
	
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

	if ( *clcptr <= 1 || *clcptr > kMaxClusters ) {	// クラスタ数をチェック
		fprintf( stderr, "Error: illegal number (2-%d)\n", kMaxClusters );
		return 1;
	}
	strcpy( eDiscFileName, kDefaultDiscFileName );

//	printf("%d %d %d %d %d %s %s\n", eGroup1Num, eGroup2Num, eDiscNum, *clcptr, eResizeMode, eOrigFileName, eAnnoFileName );
	
	return 1;
}

				
int main( int argc, char *argv[] )
{
	double d00;
	int cluster_count, start_all, start;
	char fname[kMaxPathLen], base[kFileNameLen];
	// 画像データへのポインタ。実体はo***, a***で確保
	IplImage *orig, *anno, *orig_small, *anno_small, *anno_trans, *orig_erode, *anno_smth, *diff, *gray, *bin, *clean, *small_mask, *mask, *dil_mask, *anno_trans_big, *nand, *bin_erode, *bin_dil;
	// 画像データの実体で、これらを使いまわす
	// o***: 元画像側、a***: 書き込み画像側、*6**: 600dpi（元のサイズ）、*3**: 300dpi（縮小サイズ）、**3*: 3チャネル（カラー）、**1*: 1チャネル（グレースケール）
	IplImage *o631, *o632, *o633, *o634, *a631, *o331, *o332, *o333, *a331, *o311, *o312;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	double param[kMaxParamValNum];	// 相似/アフィン変換パラメータ

	start_all = GetProcTimeMiliSec();
	// 初期化
	if ( InitAnnex( argc, argv, &cluster_count ) == 0 ) {
		fprintf( stderr, "Error: initialization\n" );
		return 1;
	}
	// 画像のロード
	start = GetProcTimeMiliSec();
	if ( ( orig = cvLoadImage( eOrigFileName, 1 ) ) == NULL ) {
		fprintf( stderr, "Image file load error: %s\n", eOrigFileName );
		return 1;
	}
	if ( ( anno = cvLoadImage( eAnnoFileName, 1 ) ) == NULL ) {
		fprintf( stderr, "Image file load error: %s\n", eAnnoFileName );
		return 1;
	}
	printf("Loading image: %d msec\n", GetProcTimeMiliSec() - start );
	// 画像データの実体を作成
	o631 = orig;
	o632 = cvCreateImage( cvGetSize( o631 ), IPL_DEPTH_8U, 3 );
	o633 = cvCreateImage( cvGetSize( o631 ), IPL_DEPTH_8U, 3 );
	o634 = cvCreateImage( cvGetSize( o631 ), IPL_DEPTH_8U, 3 );
	a631 = anno;
	o331 = cvCreateImage( cvSize( (int)(o631->width * eResizeMainOrig), (int)(o631->height * eResizeMainOrig) ), IPL_DEPTH_8U, 3 );
	o332 = cvCreateImage( cvGetSize( o331 ), IPL_DEPTH_8U, 3 );
	o333 = cvCreateImage( cvGetSize( o331 ), IPL_DEPTH_8U, 3 );
	a331 = cvCreateImage( cvSize( (int)(a631->width * eResizeMainAnno), (int)(a631->height * eResizeMainAnno) ), IPL_DEPTH_8U, 3 );
	o311 = cvCreateImage( cvGetSize( o331 ), IPL_DEPTH_8U, 1 );
	o312 = cvCreateImage( cvGetSize( o331 ), IPL_DEPTH_8U, 1 );
	
#ifdef	COR_PRESET_MODE	// 対応点手動設定モード（現在は無効）
	if ( TransformAnnoPreset( anno_small, orig_small, &param_small ) == 0 ) {
		fprintf( stderr, "対応していない可能性があります。\n" );
	}
#else
	// 変換パラメータの取得
	start = GetProcTimeMiliSec();
	if ( TransformAnno( anno, orig, cluster_count, param ) == 0 ) {
		fprintf( stderr, "Warning: TransformAnno returned 0\n" );
	}
#ifdef	CHK_COR
	fprintf( stderr, "exit(CHK_COR defined)\n" );
	return 1;
#endif
	// 得られたパラメータを用いて書き込み画像を元画像に合わせる
	anno_trans_big = o634;
	if ( eTransMode == SIMILAR ) {
		SimilarTransformation( anno, anno_trans_big, param );
	}
	else if ( eTransMode == AFFINE ) {
		AffineTransformation( anno, anno_trans_big, param );
	}
	// 以降の処理の軽減のため縮小
	orig_small = o331;
//	anno_small = a331;
	cvResize( orig, orig_small, CV_INTER_NN );
//	cvResize( anno, anno_small, CV_INTER_NN );
	anno_trans = o332;
	cvResize( anno_trans_big, anno_trans, CV_INTER_NN );

//	SimilarTransformation( anno_small, anno_trans, &param_small );
	printf("Transforming image: %d msec\n", GetProcTimeMiliSec() - start );
#endif
	GetBasename( eAnnoFileName, kMaxPathLen, base );	// 処理過程の画像出力のためにファイル名を取得
#ifdef	ANNEX_OUTPUT_IMAGE	// 処理過程の画像を出力
	sprintf( fname, "%s%s06trans.jpg", eOutPutDir, base );
	cvSaveImage( fname, anno_trans );
#endif
	// 元画像をerodeし、書き込み画像をスムージングする
	start = GetProcTimeMiliSec();
	orig_erode = o333;
//	cvErode( orig_small, orig_erode, NULL, 15 );
	if ( eOrigMode ) {	// 元画像がオリジナルの場合、にじみを再現するため太らせる必要がある
		cvErode( orig_small, orig_erode, NULL, eOrigErodeIterMain );
	}
	else {	// 元画像がスキャンされたものの場合でも、ガウシアンフィルタでドットパターンをぼかす
//		cvErode( orig_small, orig_erode, NULL, 10 );
		cvSmooth( orig_small, orig_erode, CV_GAUSSIAN, eOrigGaussParamMain, 0, 0 );
	}
//		cvCopy( orig_small, orig_erode, NULL );
//	cvSaveImage( "orig_erode.jpg", orig_erode );
	anno_smth = o331;
	cvSmooth( anno_trans, anno_smth, CV_GAUSSIAN, eAnnoGaussPramMain, 0, 0 );
//	cvCopy( anno_trans, anno_smth, NULL );
//	cvSaveImage( "anno_smth.jpg", anno_smth );
	printf("Eroding image: %d msec\n", GetProcTimeMiliSec() - start);
	// 差分を取る
	start = GetProcTimeMiliSec();
	diff = o332;
	GetDiffImage( diff, anno_smth, orig_erode, 1 );
	printf("Making diff image: %d msec\n", GetProcTimeMiliSec() - start );
#ifdef	ANNEX_OUTPUT_IMAGE
	sprintf( fname, "%s%s07diff.jpg", eOutPutDir, base );
	cvSaveImage( fname, diff );
#endif
	// グレイスケールに変換する（二値化のため）
	start = GetProcTimeMiliSec();
	gray = o311;
//	cvCvtColor( diff, gray, CV_RGB2GRAY );
	RGB2Gray( diff, gray );
	// 2値化する
	bin = o312;
	
	cvThreshold( gray, bin, eBinThr, kBinMaxVal, CV_THRESH_BINARY );
//	if ( eOrigMode ) {
//		cvThreshold( gray, bin, kDefaultBinThrOrig, kBinMaxVal, CV_THRESH_BINARY );
//	}
//	else {
//		cvThreshold( gray, bin, kDefaultBinThrScan, kBinMaxVal, CV_THRESH_BINARY );
//	}

	printf("Binarizing image: %d msec\n", GetProcTimeMiliSec() - start);
#ifdef	ANNEX_OUTPUT_IMAGE
	sprintf( fname, "%s%s08bin.jpg", eOutPutDir, base );
	cvSaveImage( fname, bin );
#endif
	// モルフォロジ演算（Closing）によりばらばらになった書き込みを結合する
	bin_dil = o311;
	cvDilate( bin, bin_dil, NULL, eClosingIter );
//	cvSaveImage( "dil.jpg", bin_dil );
	bin_erode = o312;
	cvErode( bin_dil, bin_erode, NULL, eClosingIter );
//	cvSaveImage( "erode.jpg", bin_erode );
	bin = bin_erode;
	// 連結成分を抽出し、面積に基づいてノイズを除去する
	start = GetProcTimeMiliSec();
	clean = o311;
	cvSetZero( clean );
	cvFindContours( bin, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
	for( con0 = contours; con0 != 0; con0 = con0->h_next ) {		// 連結成分を描画
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );	// 面積を求める
		if ( d00 < eMinAreaMain ) continue;	// 小さすぎる連結成分は除外
		cvDrawContours(clean, con0, cWhite, cWhite, kDrawContourMaxLevel, CV_FILLED, kDrawContourLineType);
	}
	printf("Cleaning image: %d msec\n", GetProcTimeMiliSec() - start);
#ifdef	ANNEX_OUTPUT_IMAGE
	sprintf( fname, "%s%s09clean.jpg", eOutPutDir, base );
	cvSaveImage( fname, clean );
#endif
	// マスクを作成する
	start = GetProcTimeMiliSec();
	small_mask = o331;
	cvCvtColor( clean, small_mask, CV_GRAY2RGB );	// 3チャネルに変換する
	mask = o631;
	cvResize( small_mask, mask, CV_INTER_NN );
	dil_mask = o632;
	cvDilate( mask, dil_mask, NULL, eDilMaskIter );	// マスクをdilateで太らせる
	printf("Making mask: %d msec\n", GetProcTimeMiliSec() - start);
//	cvSaveImage( "dil_mask.jpg", dil_mask );
	// 600dpiのannoを作成する
	start = GetProcTimeMiliSec();
//	anno_trans_big = o631;
//	SimilarTransformation( anno, anno_trans_big, &param );
//	cvSaveImage( "anno_trans_big.jpg", anno_trans_big );
	printf("Transforming: %d msec\n", GetProcTimeMiliSec() - start);
	// マスクを用いて書き込み抽出する
	start = GetProcTimeMiliSec();
	nand = o633;
//	MaskImage( anno_trans_big, dil_mask, nand );
	cvNot( anno_trans_big, anno_trans_big );
	cvAnd( anno_trans_big, dil_mask, nand );
	cvNot( nand, nand );
	printf("And image: %d msec\n", GetProcTimeMiliSec() - start );
	sprintf( fname, "%s%s10and.jpg", eOutPutDir, base );
	cvSaveImage( fname, nand );
	printf("Total: %d msec\n", GetProcTimeMiliSec() - start_all );
	
	return 0;
}

void MaskImage( IplImage *img, IplImage *mask, IplImage *dst )
// マスク処理を行う（動作がおかしいため使用せず）
{
	int i;
	
	for ( i = 0; i < img->imageSize; i++ ) {
		dst->imageData[i] = !(unsigned char) ( !((unsigned char)img->imageData[i]) & (unsigned char)mask->imageData[i] );
		if ( dst->imageData[i] != 0 ) {
			printf("%02x ", dst->imageData[i]);
		}
	}
}
