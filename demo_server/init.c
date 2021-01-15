#include "def_general.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "cv.h"
#include "highgui.h"

#include "disc.h"
#include "dirs.h"
#include "houghl.h"
#include "hist.h"
#include "hash.h"
#include "nn5.h"
#include "cluster.h"
//#include "perminv.h"
#include "proj4p.h"
#include "projrecov.h"
#include "f_points.h"
#include "auto_connect.h"
#include "proctime.h"
#include "score.h"
#include "init.h"
#include "nstr.h"
#include "extern.h"
#include "ncr.h"

#ifdef WIN32
double log2( double x )
// 自然対数から2の対数を求める
{
	return log( x ) / log( 2.0L );
}
#endif

int IsExist( char *fname )
{
	FILE *fp;
	
	if ( ( fp = fopen(fname, "r") ) == NULL ) {
		return 0;
	}
	fclose(fp);
	return 1;
}

int AnalyzeArgAndSetExtern2( int argc, char *argv[] )
// 引数解析の関数2
{
	int argi;

	// 初期化
	eEntireMode = RETRIEVE_MODE;	// 静止画検索モードをデフォルトにする
//	strcpy( eDirsDir, kDfltDirsDir );
	strcpy( eHashSrcDir, kDfltHashSrcDir );
	strcpy( eHashSrcSuffix, kDfltHashSrcSuffix );
	strcpy( ePointDatFileName, kDfltPointDatFileName );
	eHashGaussMaskSize = kDfltHashGaussMaskSize;
	eConnectHashImageThr = kConnectHashImageThr;
	eInvType = kDfltInvType;
	eGroup1Num = kDfltGroup1Num;
	eGroup2Num = kDfltGroup2Num;
	eDiscNum = kDfltDiscNum;
	strcpy( eDiscFileName, kDfltDiscFileName );
	eDocNumForMakeDisc = kDfltDocNumForMakeDisc;
	strcpy( eHashDatFileName, kDfltHashDatFileName );
	eTerminate = kDfltTerminate;
	eIncludeCentre = kDfltIncludeCentre;
	ePropMakeNum = kDfltPropMakeNum;
	strcpy( eConfigFileName, kDfltConfigFileName );
	strcpy( eCopyright_2006_TomohiroNakai_IMP_OPU, kCopyright );
	eExperimentMode = 0;
	eCompressHash = 0;
	eRotateOnce = 0;
	eRemoveCollision = 0;
	eMaxHashCollision = kMaxHashCollision;
	eNoCompareInv = 0;
	eUseArea = 0;
	eIsJp = 0;
	eNoHashList = 0;
	eVariousAutoMaskSize = 0;
	eDualHashMode = 0;
	eVariousAutoMaskSizeDual = 0;

	for ( argi = 1; argi < argc && *(argv[argi]) == '-'; argi++ ) {
		switch ( *(argv[argi]+1) ) {	// '-'の次の文字で分岐
			case 'r':	// 複比を使用
				eInvType = CR_AREA;
				break;
			case 'a':	// アフィン不変量を使用
				eInvType = AFFINE;
				break;
			case 's':	// 相似不変量を使用
				eInvType = SIMILAR;
				break;
			case 'n':	// nを指定
				if ( ++argi < argc )	eGroup1Num = atoi( argv[argi] );
				break;
			case 'm':	// mを指定
				if ( ++argi < argc )	eGroup2Num = atoi( argv[argi] );
				break;
			case 'd':	// 離散化レベル数を指定
				if ( ++argi < argc )	eDiscNum = atoi( argv[argi] );
				break;
			case 'c':	// ハッシュの構築
				eEntireMode = CONST_HASH_MODE;
				if ( ++argi < argc )	strcpy( eHashSrcDir, argv[argi] );	// argiを進めて文字列をコピー．argc-1を超えた場合は何もしない
				if ( ++argi < argc )	strcpy( eHashSrcSuffix, argv[argi] );	// 拡張子も同時に指定
				break;
			case 'W':	// デュアルハッシュ構築モード
				eEntireMode = CONST_DUAL_HASH_MODE;
				if ( ++argi < argc )	strcpy( eHashSrcDir, argv[argi] );	// argiを進めて文字列をコピー．argc-1を超えた場合は何もしない
				if ( ++argi < argc )	strcpy( eHashSrcSuffix, argv[argi] );	// 拡張子も同時に指定
				break;
			case 'C':	// 複数ハッシュの構築
				/********************** 未実装 *********************/
				if ( ++argi >= argc ) {	// 引数が足りない
					fprintf( stderr, "error: arguments insufficient\n" );
					break;
				}
				eHashNum = atoi( argv[argi] );	// 引数からハッシュの数を取得
				if ( eHashNum <= 1 || eHashNum > kMaxHashNum ) {	// ハッシュの数が不正
					fprintf( stderr, "error: number of hash is wrong\n" );
					eHashNum = 0;
					break;
				}
				eEntireMode = CONST_MULTI_HASH_MODE;	// 複数ハッシュ構築モード
				break;
				/********************** 未実装 *********************/
			case 'h':	// ハッシュ等のディレクトリを指定
				if ( ++argi < argc )	strcpy( eDirsDir, argv[argi] );
				break;
//			case 'f':	// ハッシュの画像ファイルの拡張子を指定
//				if ( ++argi < argc )	strcpy( eHashSrcSuffix, argv[argi] );
//				break;
			case 'S':	// USBカメラサーバモード
				eEntireMode = USBCAM_SERVER_MODE;
				break;
			case 'T':	// デュアルハッシュでUSBカメラサーバモード
				eEntireMode = USBCAM_SERVER_DUAL_HASH_MODE;
				break;
			case 'x':	// point.datを使ってハッシュを構築するモード
				eEntireMode = CONST_HASH_PF_MODE;
				break;
			case 'e':	// 実験モード
				eExperimentMode = 1;
				break;
			case 'z':	// ハッシュ圧縮モード
				eCompressHash = 1;
				break;
			case 'y':	// 回転の総当たりを行わない
				eRotateOnce = 1;
				break;
			case 'w':	// 衝突の多い項目を除去
				eRemoveCollision = 1;
				if ( ++argi < argc )	eMaxHashCollision = atoi( argv[argi] );
				break;
			case 'v':	// 特徴量の比較なし -w 1 を推奨
				eNoCompareInv = 1;
				break;
			case 'u':	// 特徴量計算に面積を利用
				eUseArea = 1;
				break;
			case 'J':	// 日本語モード
				eIsJp = 1;
				break;
			case 't':	// リストなし
				eNoHashList = 1;
				break;
			case 'l':	// 簡易型多言語モード（特徴点抽出のパラメータを変更）
				if ( ++argi >= argc )	break;
				if ( !strcmp( argv[argi], "arabic" ) )	eHashGaussMaskSize = 11;
				else if ( !strcmp( argv[argi], "chinese" ) )	eHashGaussMaskSize = /*11単語*/3;
				else if ( !strcmp( argv[argi], "french" ) )	eHashGaussMaskSize = 5;
				else if ( !strcmp( argv[argi], "hindi" ) )	eHashGaussMaskSize = 5;
				else if ( !strcmp( argv[argi], "japanese" ) )	eHashGaussMaskSize = 3;
				else if ( !strcmp( argv[argi], "korean" ) )	eHashGaussMaskSize = /*15*/3;
				else if ( !strcmp( argv[argi], "lhao" ) )	eHashGaussMaskSize = 9;
				else if ( !strcmp( argv[argi], "russian" ) )	eHashGaussMaskSize = 5;
				else if ( !strcmp( argv[argi], "spanish" ) )	eHashGaussMaskSize = 5;
				else if ( !strcmp( argv[argi], "tamil" ) )	eHashGaussMaskSize = 5;
				else if ( !strcmp( argv[argi], "thai" ) )	eHashGaussMaskSize = /*11*/3;
				break;
			case 'V':	// 多言語ハッシュ構築モード（プレフィックスでマスクサイズを自動的に決定）
				eVariousAutoMaskSize = 1;
				break;
			case 'U':	// デュアルハッシュでの多言語自動ハッシュサイズ
				eVariousAutoMaskSizeDual = 1;
				break;
			case 'g':	// ガウシアンフィルタのサイズを指定
				if ( ++argi >= argc )	break;
				eHashGaussMaskSize = atoi( argv[argi] );
				break;
			case 'b':	// 2値化の閾値を指定
				if ( ++argi >= argc )	break;
				eConnectHashImageThr = atoi( argv[argi] );
				break;
			default:	// 未知の引数
				fprintf( stderr, "warning: %c is an unknown argument\n",  *(argv[argi]+1) );
				break;
		}
	}
	// ディレクトリの末尾に（なければ）スラッシュを追加
	AddSlash( eHashSrcDir, kMaxPathLen );
	AddSlash( eDirsDir, kMaxPathLen );
	// 完全なパスを作成
	sprintf( eHashFileName, "%s%s", eDirsDir, eHashDatFileName );
	// fを指定
	switch ( eInvType ) {
		case CR_AREA:
			eGroup3Num = 5;
			break;
		case CR_INTER:	// 使われていないはずだが，一応
			eGroup3Num = 5;
			break;
		case AFFINE:
			eGroup3Num = 4;
			break;
		case SIMILAR:
			eGroup3Num = 3;
			break;
		default:
			return 0;
	}
	if ( eGroup1Num < eGroup2Num || eGroup2Num < eGroup3Num ) {
		fprintf( stderr, "error: illegal n or m\n" );
		return 0;
	}
	// 組み合わせ数を計算
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	// ハッシュ圧縮モードでの必要バイト数などを計算
	eDocBit = (int)ceil( log2(kMaxDocNum) );	// docのビット数
	ePointBit = (int)ceil( log2(kMaxPointNum) );	// pointのビット数
	eRBit = (int)ceil( log2(eDiscNum) );	// 不変量1つのビット数
	eOBit = (int)ceil( log2(eGroup2Num) );	// 面積特徴量（順序）1つのビット数
	if ( eNoCompareInv ) {
		eHList2DatByte = (int)ceil( ((double)( eDocBit + ePointBit )) / 8.0L );	// リストのデータ部のバイト数
	} else {
		if ( eUseArea )	eHList2DatByte = (int)ceil( ((double)( eDocBit + ePointBit + eRBit*eNumCom2 + eOBit*eGroup2Num )) / 8.0L );	// リストのデータ部のバイト数
		else			eHList2DatByte = (int)ceil( ((double)( eDocBit + ePointBit + eRBit*eNumCom2 )) / 8.0L );	// リストのデータ部のバイト数
	}
	// リストなしモードで既定バイト数を上回らないかチェック
	if ( eNoHashList && (int)ceil( ((double)( eDocBit + ePointBit )) / 8.0L ) > sizeof( HENTRY ) ) {
		fprintf( stderr, "error: kMaxDocNum and/or kMaxPointNum is too large to store in HENTRY(%d bytes).\n", sizeof( HENTRY ) );
		return 0;
	}

	return argi;
}

int AnalyzeArgAndSetExtern( int argc, char *argv[], int *p_emode, int *p_rmode, int *p_fmode, int *p_pmode )
// 引数を解析し、パラメータ（extern変数）を設定する
// p_emode: 全体のモード（登録、検索など）
// p_rmode: 射影変換の補正をするか否か
// p_fmode: 特徴点は何を使うか（単語の重心、囲まれた領域の重心、ポイントファイル）
// p_pmode: 結合画像が用意されているかどうか（現在06/01/12では形骸化）
{
	int argi, pi, n, m, inv_type, disc_num, inc_centre, vec_chk, use_area, dbdocs, start_num;
	char hash_path[1024], dbcor_fn[1024], pn_fn[1024], hash_fn[1024], dm_path[1024], dirs_dir[1024], point_dir[1024], prop_path[1024];
	char cmd[1024];
	double prop = 0.0;
	
	// default
	n = 8;
	m = 7;
	inv_type = CR_AREA;
	disc_num = 17;
	inc_centre = 0;
	use_area = 0;
	vec_chk = 1;
	*dm_path = '\0';
	*prop_path = '\0';
	*point_dir = '\0';
	*dbcor_fn = '\0';
	*pn_fn = '\0';
	strcpy(dirs_dir, "/home/nakai/dirs/sharp/");
	strcpy( hash_path, "/home/nakai/didb/test1000/*.bmp" );
	eIsJp = 0;
	// 日本語モード関係
	eJpGParam1 = 1;
	eJpAdpBlock = 50;
	eJpAdpSub = 10;
	eJpGParam2 = 1;
	eJpThr = 200;
	// 打ち切り関係
	eTerminate = TERM_NON;
	eTermVNum = kDefaultTermVNum;

	srand( (unsigned int)time(NULL) );	// 乱数の初期化
/****** 引数一覧 ******
-n [数値]:	nの値
-m [数値]:	mの値
-d [数値]:	離散化レベル数
-[r/i/a/s]:	面積による複比/交点による複比/アフィン不変量/相似不変量
-j:	中心点を含む
-E:	面積を用いる
-M:	携帯電話モード
-c:	ハッシュ構築モード
-C [ハッシュのパス]:	ハッシュ構築モード（パス指定）
-P [比例定数計算用のパス]:	比例定数計算パス指定
-t:	テストモード
-R:	射影変換の補正も行う
-e:	囲まれた領域を特徴点に用いる
-p:	単語の連結された画像を利用する
-l:	単語の連結された画像を残す
-A:	ハッシュ追加モード
-f [点ファイルのディレクトリ] [対応関係ファイル] [特徴点数ファイル]:	ハッシュの構築に点ファイルを用いる
-q:	検索質問の特徴点ファイル作成モード
-g:	登録画像の特徴点ファイル作成モード
-G [登録画像の検索パス] [特徴点ファイルのディレクトリ] [対応関係ファイル] [特徴点数ファイル] [開始番号]:	登録画像の特徴点ファイル作成モード2
-D [離散化ファイル作成に用いるパス]:	離散化ファイルの作成に用いるパスを指定
-I [DIRSディレクトリ]:	各種設定ファイルのディレクトリを指定
-N:	特徴点ベクトルのチェックなし
-h:	ハッシュのチェックモード
-S:	USBカメラサーバモード
-T [TCPポート番号]:	TCPポートの指定
-J:	日本語モード
-v [数値]:	打ち切りモードとパラメータ
****** 引数一覧 ******/
	for ( argi = 1; argi < argc && *(argv[argi]) == '-'; argi++ ) {
		for ( pi = 1; *(argv[argi]+pi) != '\0'; pi++ ) {
			switch ( *(argv[argi]+pi) ) {
				case 'n' :	// nの設定
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					n = atoi(argv[argi]+pi);
					if ( n == 0 )	return -1;
					goto next_arg;
					break;
				case 'm' :	// mの設定
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					m = atoi(argv[argi]+pi);
					if ( m == 0 )	return -1;
					goto next_arg;
					break;
				case 'd' :	// 離散化レベルの設定
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					disc_num = atoi(argv[argi]+pi);
					if ( disc_num == 0 )	return -1;
					goto next_arg;
					break;
				case 'r' :	// CR_AREA
					inv_type = CR_AREA;
					break;
				case 'i' :	// CR_INTER
					inv_type = CR_INTER;
					break;
				case 'a' :	// AFFINE
					inv_type = AFFINE;
					break;
				case 's' :	// SIMILAR
					inv_type = SIMILAR;
					break;
				case 'j' :	// 中心点を含むか
					inc_centre = 1;
					break;
				case 'E' :	// 面積を用いるか
					use_area = 1;
					break;
				case 'M' :	// 携帯電話モード
					*p_emode = RET_MP_MODE;
					break;
				case 'c' :	// ハッシュ構築モード
					*p_emode = CONST_HASH_MODE;
					break;
				case 'C' :	// ハッシュ構築モード（パス指定）
					*p_emode = CONST_HASH_MODE;
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( hash_path, argv[argi] );
					goto next_arg;
					break;
				case 'P' :	// 比例定数計算パス指定
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( prop_path, argv[argi] );
					goto next_arg;
					break;
				case 't' :	// テストモード
					*p_emode = TEST_MODE;
					break;
				case 'R' :	// 射影変換の補正も行う
					*p_rmode = RECOVER_MODE;
					break;
				case 'e' :	// 囲まれた領域を特徴点に用いる
					*p_fmode = ENCLOSED_MODE;
					break;
				case 'p' :	// 単語の連結された画像を利用する
					*p_pmode = PREPARED_MODE;
					break;
				case 'l' :	// 単語の連結された画像を残す
					*p_pmode = LEAVE_MODE;
					break;
				case 'A' :	// ハッシュ追加モード
					*p_emode = ADD_HASH_MODE;
					break;
				case 'f' :	// 点ファイルを用いる（点ファイルのディレクトリと対応ファイル名を指定）
					*p_fmode = USEPF_MODE;
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( point_dir, argv[argi++] );
					strcpy( dbcor_fn, argv[argi++] );
					strcpy( pn_fn, argv[argi] );
					goto next_arg;
					break;
				case 'q' :	// Create point file of query image
					*p_emode = CREATE_QPF_MODE;
					break;
				case 'g' :	// Create point file of registered image
					*p_emode = CREATE_RPF_MODE;
					break;
				case 'G' :	// 特徴点抽出モード２
					*p_emode = CREATE_RPF_MODE2;
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( hash_path, argv[argi++] );
					strcpy( point_dir, argv[argi++] );
					strcpy( dbcor_fn, argv[argi++] );
					strcpy( pn_fn, argv[argi++] );
					start_num = atoi( argv[argi] );
					goto next_arg;
					break;
					
				case 'D' :	// discrete file making path
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( dm_path, argv[argi] );
					goto next_arg;
				case 'I' :	// DIRS Dir
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( dirs_dir, argv[argi] );
					strcpy( eDirsDir, dirs_dir );
					goto next_arg;
					break;
				case 'N' :	// No check for vector
					vec_chk = 0;
					break;
				case 'h' : // Check hash mode
					*p_emode = CHK_HASH_MODE;
					break;
				case 'S' :	// USB Camera Server mode
					*p_emode = USBCAM_SERVER_MODE;
					break;
				case 'T' :	// TCP Port
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					eTCPPort = atoi(argv[argi]+pi);
					goto next_arg;
					break;
				case 'J' :	// 日本語モード
					eIsJp = 1;
					break;
				case 'v' :	// 打ち切りモード
					eTerminate = TERM_VNUM;	// とりあえずこのモードで固定
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					eTermVNum = atoi(argv[argi]+pi);	// パラメータを指定
					goto next_arg;
					break;
				default :
					fprintf(stderr, "%c : 無効なパラメータです\n", *(argv[argi]+pi));
					break;
			}
		}
		next_arg:
			;
	}
	AddSlash( dirs_dir, 1024 );	// スラッシュがなければ追加
	if ( *point_dir == '\0' ) {	// 点ファイルのディレクトリが指定されていなければ
		sprintf( point_dir, "%spoint/", dirs_dir );	// dirs_dir下にpointディレクトリを作成
	} else {
		AddSlash( point_dir, 1024 );	// スラッシュがなければ追加
	}
	if ( *p_emode == CONST_HASH_MODE ) {	// ハッシュ構築モードのとき
#ifndef	WIN32	// DOSのmkdirはパスのデリミタが'\\'でないとエラーになるので，Windowsでは無効にした
		sprintf(cmd, "mkdir %s %s", kMkdirNoMesOpt, dirs_dir);	// ディレクトリがなければ作成
		system(cmd);
		sprintf(cmd, "mkdir %s %s", kMkdirNoMesOpt, point_dir);
		system(cmd);
#endif
	}
	if ( *p_emode == RETRIEVE_MODE || *p_emode == USBCAM_SERVER_MODE )	LoadSetting( &inv_type, &n, &m, &disc_num, point_dir, dbcor_fn, pn_fn, &prop, &dbdocs );
	else	SaveSetting( inv_type, n, m, disc_num, point_dir, dbcor_fn, pn_fn );
	
	if ( *dbcor_fn == '\0' ) {	// 対応ファイルが指定されていなければ
		sprintf( dbcor_fn, "%sdbcor.dat", dirs_dir );
	}
	if ( *pn_fn == '\0' ) {	// 特徴点数ファイルが指定されていなければ
		sprintf( pn_fn, "%spnum.txt", dirs_dir );
	}
	sprintf( hash_fn, "%shash.dat", dirs_dir );
	if ( *p_emode == CREATE_QPF_MODE || *p_emode == CREATE_RPF_MODE ) {
		eUseArea = use_area;
		return argi;	// 特徴点を作るだけならSetExternは不要
	}
	if ( SetExtern( *p_emode, n, m, inv_type, disc_num, prop, inc_centre, use_area, vec_chk, hash_path, dirs_dir, point_dir, ".dat", \
		dbcor_fn, "disc/", dm_path, prop_path, pn_fn, hash_fn, dbdocs, start_num ) )	return argi;
	else	return -1;
}


int SetExtern( int emode, int n, int m, int inv_type, int disc_num, double prop, int inc_centre, int use_area, int vec_chk, \
 char *hs_path, char *dirs_dir, char *pf_pref, char *pf_suf, char *cor_fn, \
 char *disc_dir, char *dm_path, char *prop_path, char *pn_fn, char *hash_fn, int dbdocs, int start_num )
// パラメータを設定する
{
	int i, num, den;
	char it, disc_fn[1024], cmd[1024];

	// 不変量タイプの設定
	eInvType = inv_type;
	// 中心点を含めるかどうか
	eIncludeCentre = inc_centre;
	// 面積を用いるかどうか
	eUseArea = use_area;
	// 特徴量の一致判定を行うか
	eVectorCheck = vec_chk;
	// nおよびmの設定
	eGroup1Num = n;
	eGroup2Num = m;
	// 不変量タイプに応じたeGroup3Numの設定
	switch (inv_type) {
		case CR_AREA:
			eGroup3Num = 5;
			it = 'r';
			break;
		case CR_INTER:
			eGroup3Num = 5;
			it = 'i';
			break;
		case AFFINE:
			eGroup3Num = 4;
			it = 'a';
			break;
		case SIMILAR:
			eGroup3Num = 3;
			it = 's';
			break;
		default:
			return 0;
	}
//	if ( eIncludeCentre || inv_type == SIMILAR )	// 中心点を含む
	if ( eIncludeCentre )	// 中心点を含む
		eGroup3Num--;

	// nCmの設定
	for ( i = 0, num = 1, den = 1; i < eGroup2Num; i++ ) {
		num *= eGroup1Num - i;
		den *= i + 1;
	}
	eNumCom1 = (int)(num / den);
//	if ( inv_type == SIMILAR ) {
//		eNumCom2 = eGroup2Num - 1;
//	} else {
	// mCfの設定
		for ( i = 0, num = 1, den = 1; i < eGroup3Num; i++ ) {
			num *= eGroup2Num - i;
			den *= i + 1;
		}
		eNumCom2 = (int)(num / den);
//	}
	// 量子化レベル
	eDiscNum = disc_num;
	eProp = prop;

	// 離散化ファイルの設定
	sprintf( disc_fn, "%sdisc.txt", dirs_dir );
	if ( *dm_path != '\0' ) {	// dm_pathが設定されているなら，acr2を起動してdisc.txtを生成する
		sprintf( cmd, "%s %c %d %d %d %s %s", kAcrPath, it, n, m, disc_num, disc_fn, dm_path );
		puts(cmd);
		system(cmd);
		strcpy( eDiscFileName, disc_fn );
	} else {
		if ( IsExist( disc_fn ) ) {
			strcpy( eDiscFileName, disc_fn );
		} else {
			if ( inv_type == CR_AREA || inv_type == CR_INTER ) {
				sprintf( eDiscFileName, "%scr%ddisc%d%d.txt", disc_dir, disc_num, n, m );
			} else if ( inv_type == AFFINE ) {
				sprintf( eDiscFileName, "%saf%ddisc%d%d.txt", disc_dir, disc_num, n, m );
			} else if ( inv_type == SIMILAR ) {
				sprintf( eDiscFileName, "%ssi%ddisc%d%d.txt", disc_dir, disc_num, n, m );
			} else {
				return 0;
			}
		}
	}
	// 比例定数の設定
	strcpy( ePropMakePath, prop_path );
//	if ( inv_type != SIMILAR && !IsExist( eDiscFileName ) ) {
	if ( !IsExist( eDiscFileName ) && emode != CREATE_RPF_MODE2 ) {
		fprintf(stderr, "離散化ファイル%sは存在しません\n", eDiscFileName );
		return 0;
	}

	strcpy( eDirsDir, dirs_dir );
	strcpy( eHashSrcPath, hs_path );	// ハッシュの元画像ファイルの探索パス
//	strcpy( eHashSrcDir, hs_dir );	// ハッシュの元画像ファイルのディレクトリ
	strcpy( ePFPrefix, pf_pref );	// ハッシュの点ファイルのディレクトリ
	strcpy( ePFSuffix, pf_suf );	// ハッシュの点ファイルの拡張子
	strcpy( eDBCorFileName, cor_fn );	// 対応ファイル
	strcpy( ePNFileName, pn_fn );	// 点の数のファイル
	strcpy( eHashFileName, hash_fn ); // ハッシュのファイル名
	eDbDocs = dbdocs;	// データベースのサイズ
	eCPF2StartNum = start_num;	// 特徴点抽出モード２での開始番号

	return 1;
}

int SaveSetting( int inv_type, int n, int m, int d, char *point_dir, char *dbcor_fn, char *pn_fn )
{
	char it, set_fn[1024];
	FILE *fp;
	
	sprintf(set_fn, "%ssetting.txt", eDirsDir);
	if ( ( fp = fopen(set_fn, "w") ) == NULL ) return 0;
	switch ( inv_type ) {
		case CR_AREA:
			it = 'r';
			break;
		case CR_INTER:
			it = 'i';
			break;
		case AFFINE:
			it = 'a';
			break;
		case SIMILAR:
			it = 's';
			break;
		default:
			return 0;
	}

	fprintf(fp, "%c %d %d %d\n", it, n, m, d);
	fprintf(fp, "%s\n%s\n%s\n", point_dir, dbcor_fn, pn_fn);
	fclose(fp);
	return 1;
}

void RemoveLastCR( char *str, int len )
// 文字列最後の改行を削除
{
	int i;
	
	for ( i = 0; str[i] != '\0' && i < len; i++ );
	if ( i > 0 && str[i-1] == '\n' )	str[i-1] = '\0';
}

int LoadSetting( int *inv_type, int *n, int *m, int *d, char *point_dir, char *dbcor_fn, char *pn_fn, double *p, int *dbdocs )
{
	char it, set_fn[1024], line[1024];
	FILE *fp;
	
	sprintf(set_fn, "%ssetting.txt", eDirsDir);
	if ( ( fp = fopen(set_fn, "r") ) == NULL ) return 0;
	fgets( line, 1024, fp );
	sscanf( line, "%c %d %d %d", &it, n, m, d);
	fgets( point_dir, 1024, fp );
	RemoveLastCR( point_dir, 1024 );
	fgets( dbcor_fn, 1024, fp );
	RemoveLastCR( dbcor_fn, 1024 );
	fgets( pn_fn, 1024, fp );
	RemoveLastCR( pn_fn, 1024 );
	fgets( line, 1024, fp );
	sscanf( line, "%lf", p);
	fgets( line, 1024, fp );
	sscanf( line, "%d", dbdocs );
	switch ( it ) {
		case 'r':
			*inv_type = CR_AREA;
			break;
		case 'i':
			*inv_type = CR_INTER;
			break;
		case 'a':
			*inv_type = AFFINE;
			break;
		case 's':
			*inv_type = SIMILAR;
			break;
		default:
			return 0;
	}
	fclose(fp);
	return 1;
}

int ReadIniFile( void )
// iniファイルを読む
{
	char line[kMaxLineLen], *tok;
	FILE *fp;

	// デフォルト値の設定
	eTCPPort = kDefaultTCPPort;
	eProtocol = kDefaultProtocol;
	ePointPort = kDefaultPointPort;
	eResultPort = kDefaultResultPort;
	strcpy( eClientName, kDefaultClientName );
	strcpy( eServerName, kDefaultServerName );
	strcpy( eDirsDir, kDfltDirsDir );

	if ( ( fp = fopen( kIniFileName, "r" ) ) == NULL ) {	// iniファイルがない
		fprintf( stderr, "warning: %s cannot be opened\n", kIniFileName );
		return 0;
	}
	for ( ; fgets( line, kMaxLineLen, fp ) != NULL;  ) {	// iniファイルを行ごとに処理
//		puts( line );
		tok = strtok( line, " =\t\n" );	// strtokで行を分解
		if ( tok == NULL )	continue;	// トークンなし
		if ( *tok == '#' )	continue;	// 行頭が#ならコメントとみなす
		if ( strcmp( tok, "TCPPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eTCPPort = atoi(tok);
		}
		else if ( strcmp( tok, "Protocol" ) == 0 ) {	// Protocol
			tok = strtok( NULL, " =\t\n" );
			if ( tok == NULL )	continue;	// トークンなし
			if ( strcmp( tok, "TCP" ) == 0 )	eProtocol = 1;
			else if ( strcmp( tok, "UDP" ) == 0 )	eProtocol = 2;
		}
		else if ( strcmp( tok, "PointPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	ePointPort = atoi( tok );
		}
		else if ( strcmp( tok, "ResultPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eResultPort = atoi( tok );
		}
		else if ( strcmp( tok, "ClientName" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	strcpy( eClientName, tok );
		}
		else if ( strcmp( tok, "ServerName" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	strcpy( eServerName, tok );
		}
		else if ( strcmp( tok, "DatabaseDir" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	strcpy( eDirsDir, tok );
		}
	}
	fclose( fp );

	return 1;
}

int SaveConfig( int inv_type, int group1_num, int group2_num, int disc_num, double prop, int compress_hash, int doc_bit, int point_bit, int r_bit, int o_bit, int hlist2_dat_byte, int rotate_once, int use_area, int is_jp, int no_hash_list,  const char *config_file_name )
// 設定ファイルを保存
{
	char fname[kMaxPathLen], ch;
	FILE *fp;

	// config.datのパスを作成
	sprintf( fname, "%s%s", eDirsDir, config_file_name );
	if ( ( fp = fopen( fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}
	// config.datのバージョン
	fprintf( fp, "%s\n", kConfigVerStr );
	// 不変量タイプ
	switch ( inv_type ) {
		case CR_AREA:
			ch = kInvCharCRArea;
			break;
		case CR_INTER:
			ch = kInvCharCRInter;
			break;
		case AFFINE:
			ch = kInvCharAffine;
			break;
		case SIMILAR:
			ch = kInvCharSimilar;
			break;
		default:
			return 0;
	}
	fprintf( fp, "%c\n", ch );
	// n
	fprintf( fp, "%c %d\n", 'n', group1_num );
	// m
	fprintf( fp, "%c %d\n", 'm', group2_num );
	// d
	fprintf( fp, "%c %d\n", 'd', disc_num );
	// prop
	fprintf( fp, "%c %lf\n", 'p', prop );
	// ハッシュ圧縮モードならビット数など
	if ( compress_hash )	fprintf( fp, "%c %d %d %d %d %d\n", 'z', doc_bit, point_bit, r_bit, o_bit, hlist2_dat_byte );
	// 回転一通りモード
	if ( rotate_once )	fprintf( fp, "%c\n", 'y' );
	// 面積を使用するかどうか
	if ( use_area )	fprintf( fp, "%c\n", 'u' );
	// 日本語モード
	if ( is_jp )	fprintf( fp, "%c\n", 'J' );
	// ハッシュリストなしモード
	if ( no_hash_list )	fprintf( fp, "%c\n", 't' );

	fclose( fp );
	return 1;
/* 旧バージョン
	fprintf( fp, "%c %d %d %d\n", ch, eGroup1Num, eGroup2Num, eDiscNum );
	fprintf( fp, "%lf\n%d\n", eProp, eDbDocs );
	// ハッシュ圧縮モードかどうか
	fprintf( fp, "%s\n", (eCompressHash) ? kCompressHashDiscroptor : kNormalHashDiscroptor );
	// ビット数
	fprintf( fp, "%d %d %d %d\n", eDocBit, ePointBit, eRBit, eHList2DatByte );

	fclose( fp );
	return 1;
*/
}

int LoadConfig( const char *config_file_name, int *p_inv_type, int *p_group1_num, int *p_group2_num, int *p_group3_num, int *p_num_com1, int *p_num_com2, int *p_disc_num, double *p_prop, int *p_compress_hash, int *p_doc_bit, int *p_point_bit, int *p_r_bit, int *p_o_bit, int *p_hlist2_dat_byte, int *p_rotate_once, int *p_use_area, int *p_is_jp, int *p_no_hash_list )
// 設定ファイルを読み込み
{
	char fname[kMaxPathLen], line[kMaxLineLen], ch, *fgets_ret;
	FILE *fp;

	// config.datのパスを作成して開く
	sprintf( fname, "%s%s", eDirsDir, config_file_name );
	if ( ( fp = fopen( fname, "r" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}
	fgets_ret = fgets( line, kMaxLineLen, fp );	// 一行読み込み
	// 読み込み失敗もしくはバージョン文字列と一致せず
	if ( fgets_ret == NULL || !strcmp( line, kConfigVerStr ) ) {
		fprintf( stderr, "error: %s version error\n", config_file_name );
		return 0;
	}
	// 各行を解析して設定
	// 例:	a
	//		n 7
	//		m 6
	//		d 15
	//		z 15 12 4 4
	//		y
	while ( fgets( line, kMaxLineLen, fp ) != NULL ) {
		switch ( line[0] ) {
			// 不変量タイプ
			case kInvCharCRArea:	// r
				*p_inv_type = CR_AREA;
				*p_group3_num = 5;
				break;
			case kInvCharCRInter:	// i
				*p_inv_type = CR_INTER;
				*p_group3_num = 5;
				break;
			case kInvCharAffine:	// a
				*p_inv_type = AFFINE;
				*p_group3_num = 4;
				break;
			case kInvCharSimilar:	//s
				*p_inv_type = SIMILAR;
				*p_group3_num = 3;
				break;
			case 'n':
				sscanf( line, "%c %d", &ch, p_group1_num );
				break;
			case 'm':
				sscanf( line, "%c %d", &ch, p_group2_num );
				break;
			case 'd':
				sscanf( line, "%c %d", &ch, p_disc_num );
				break;
			case 'p':
				sscanf( line, "%c %lf", &ch, p_prop );
				break;
			case 'z':
				eCompressHash = 1;
				sscanf( line, "%c %d %d %d %d %d", &ch, p_doc_bit, p_point_bit, p_r_bit, p_o_bit, p_hlist2_dat_byte );
				break;
			case 'y':
				*p_rotate_once = 1;
				break;
			case 'u':
				*p_use_area = 1;
				break;
			case 'J':
				*p_is_jp = 1;
				break;
			case 't':
				*p_no_hash_list = 1;
				break;
			default:
				break;
		}
	}
	fclose( fp );
	// 組み合わせ数を計算
	*p_num_com1 = CalcnCr( *p_group1_num, *p_group2_num );
	*p_num_com2 = CalcnCr( *p_group2_num, *p_group3_num );
	return 1;

	/* 旧バージョン
	sscanf( line, "%c %d %d %d", &ch, &eGroup1Num, &eGroup2Num, &eDiscNum );
	switch ( ch ) {	// 文字によって分岐
		case kInvCharCRArea:
			eInvType = CR_AREA;
			eGroup3Num = 5;
			break;
		case kInvCharCRInter:
			eInvType = CR_INTER;
			eGroup3Num = 5;
			break;
		case kInvCharAffine:
			eInvType = AFFINE;
			eGroup3Num = 4;
			break;
		case kInvCharSimilar:
			eInvType = SIMILAR;
			eGroup3Num = 3;
			break;
		default:
			return 0;
	}
	fgets( line, kMaxLineLen, fp );	// 一行読み込み
	sscanf( line, "%lf", &eProp );
	fgets( line, kMaxLineLen, fp );	// 一行読み込み
	sscanf( line, "%d", &eDbDocs );
	// ハッシュ圧縮モード追加
	fgets_ret = fgets( line, kMaxLineLen, fp );	// 一行読み込み
	if ( !strncmp( line, kCompressHashDiscroptor, strlen(kCompressHashDiscroptor) ) ) {
		puts("Compress Hash Mode");
		eCompressHash = 1;	// ハッシュ圧縮モード
	}
	else {
		eCompressHash = 0;
	}
	// ビット数
	fgets( line, kMaxLineLen, fp );	// 一行読み込み
	sscanf( line, "%d %d %d %d", &eDocBit, &ePointBit, &eRBit, &eHList2DatByte );

	fclose( fp );
	// 組み合わせ数を計算
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );

	return 1;
	*/
}
