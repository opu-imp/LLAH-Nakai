#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def_general.h"
#include "init.h"
#include "extern.h"

int ReadIniFile( void )
// iniファイルを読む
{
	char line[kMaxLineLen], *tok;
	FILE *fp;

	// デフォルト値の設定
	eCamNum = kDefaultCamNum;
	eCamConfNum = kDefaultCamConfNum;
	eTCPPort = kDefaultTCPPort;
	eProtocol = kDefaultProtocol;
	ePointPort = kDefaultPointPort;
	eResultPort = kDefaultResultPort;
	strcpy( eClientName, kDefaultClientName );
	strcpy( eServerName, kDefaultServerName );
	strcpy( eThumbDir, kDefaultThumbDir );
	strcpy( eThumbSuffix, kDefaultThumbSuffix );
	eThumbScale = kDefaultThumbScale;

	if ( ( fp = fopen( kIniFileName, "r" ) ) == NULL ) {	// iniファイルがない
		fprintf( stderr, "warning: %s cannot be opened\n", kIniFileName );
		return 0;
	}
	for ( ; fgets( line, kMaxLineLen, fp ) != NULL;  ) {	// iniファイルを行ごとに処理
//		puts( line );
		tok = strtok( line, " =\t\n" );	// strtokで行を分解
		if ( tok == NULL )	continue;	// トークンなし
		if ( *tok == '#' )	continue;	// 行頭が#ならコメントとみなす
		if ( strcmp( tok, "CamNum" ) == 0 ) {	// CamNumと一致
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eCamNum = atoi(tok);
		}
		else if ( strcmp( tok, "CamConfNum" ) == 0 ) {	// CamConfNumと一致
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eCamConfNum = atoi(tok);
		}
		else if ( strcmp( tok, "TCPPort" ) == 0 ) {
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
		else if ( strcmp( tok, "ThumbDir" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	strcpy( eThumbDir, tok );
		}
		else if ( strcmp( tok, "ThumbSuffix" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	strcpy( eThumbSuffix, tok );
		}
		else if ( strcmp( tok, "ThumbScale" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eThumbScale = atof( tok );
		}
	}
	fclose( fp );

	return 1;
}

void InterpretArguments( int argc, char *argv[] )
// 引数を解釈し，グローバル変数を設定する
{
	int argi, pi;

	eEntireMode = CAM_RET_MODE;
	eIsJp = 0;
	// デフォルト
	eJpAdpBlock = 50;
	eJpAdpSub = 10;
	eJpGParam = 1;
	eJpThr = 200;

	strcpy( eCopyright, kCopyright );

	for ( argi = 1; argi < argc; argi++ ) {
		pi = 0;
		if ( argv[argi][pi++] == '-' ) {
			switch ( argv[argi][pi] ) {
#ifndef	FUNC_LIMIT
				case 'v' :	// 動画の保存モード
					eEntireMode = CAP_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					break;
				case 'm' :	// 動画入力モード
					eEntireMode = INPUT_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					break;
				case 'd' :	// 動画の分解
					eEntireMode = DECOMPOSE_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					break;
				case 't' :	// 特徴点抽出パラメータチューニングモード
					eEntireMode = TUNE_FP_MODE;
					if ( ++argi < argc )	strcpy( eTuneFpRegFileName, argv[argi] );
					break;
#endif
				case 'c':	// カメラの情報をチェックするモード
					eEntireMode = CHK_CAM_MODE;
					break;
#ifndef	FUNC_LIMIT
				case 'J':	// 日本語モード
					eIsJp = 1;
					break;
				case 'o':	// 動画形式変換モード
					eEntireMode = CONV_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					if ( ++argi < argc )	strcpy( eConvMovieFileName, argv[argi] );
					break;
				case 'h':	// Harrisモード
					eEntireMode = CAM_HARRIS_MODE;
					if ( ++argi < argc )	strcpy( eCamHarrisRegFileName, argv[argi] );
					break;
				case 'H':	// Harrisのテストモード
					eEntireMode = HARRIS_TEST_MODE;
					if ( ++argi < argc )	strcpy( eHarrisTestOrigFileName, argv[argi] );
					if ( ++argi < argc )	strcpy( eHarrisTestAnnoFileName, argv[argi] );
#endif
				default :
					break;
			}
		}
	}
}
