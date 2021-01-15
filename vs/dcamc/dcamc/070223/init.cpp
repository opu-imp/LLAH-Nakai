#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def_general.h"
#include "init.h"
#include "extern.h"

int ReadIniFile( void )
// ini�t�@�C����ǂ�
{
	char line[kMaxLineLen], *tok;
	FILE *fp;

	// �f�t�H���g�l�̐ݒ�
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

	if ( ( fp = fopen( kIniFileName, "r" ) ) == NULL ) {	// ini�t�@�C�����Ȃ�
		fprintf( stderr, "warning: %s cannot be opened\n", kIniFileName );
		return 0;
	}
	for ( ; fgets( line, kMaxLineLen, fp ) != NULL;  ) {	// ini�t�@�C�����s���Ƃɏ���
//		puts( line );
		tok = strtok( line, " =\t\n" );	// strtok�ōs�𕪉�
		if ( tok == NULL )	continue;	// �g�[�N���Ȃ�
		if ( *tok == '#' )	continue;	// �s����#�Ȃ�R�����g�Ƃ݂Ȃ�
		if ( strcmp( tok, "CamNum" ) == 0 ) {	// CamNum�ƈ�v
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eCamNum = atoi(tok);
		}
		else if ( strcmp( tok, "CamConfNum" ) == 0 ) {	// CamConfNum�ƈ�v
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eCamConfNum = atoi(tok);
		}
		else if ( strcmp( tok, "TCPPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eTCPPort = atoi(tok);
		}
		else if ( strcmp( tok, "Protocol" ) == 0 ) {	// Protocol
			tok = strtok( NULL, " =\t\n" );
			if ( tok == NULL )	continue;	// �g�[�N���Ȃ�
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
// ���������߂��C�O���[�o���ϐ���ݒ肷��
{
	int argi, pi;

	eEntireMode = CAM_RET_MODE;
	eIsJp = 0;
	// �f�t�H���g
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
				case 'v' :	// ����̕ۑ����[�h
					eEntireMode = CAP_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					break;
				case 'm' :	// ������̓��[�h
					eEntireMode = INPUT_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					break;
				case 'd' :	// ����̕���
					eEntireMode = DECOMPOSE_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					break;
				case 't' :	// �����_���o�p�����[�^�`���[�j���O���[�h
					eEntireMode = TUNE_FP_MODE;
					if ( ++argi < argc )	strcpy( eTuneFpRegFileName, argv[argi] );
					break;
#endif
				case 'c':	// �J�����̏����`�F�b�N���郂�[�h
					eEntireMode = CHK_CAM_MODE;
					break;
#ifndef	FUNC_LIMIT
				case 'J':	// ���{�ꃂ�[�h
					eIsJp = 1;
					break;
				case 'o':	// ����`���ϊ����[�h
					eEntireMode = CONV_MOVIE_MODE;
					if ( ++argi < argc )	strcpy( eMovieFileName, argv[argi] );
					if ( ++argi < argc )	strcpy( eConvMovieFileName, argv[argi] );
					break;
				case 'h':	// Harris���[�h
					eEntireMode = CAM_HARRIS_MODE;
					if ( ++argi < argc )	strcpy( eCamHarrisRegFileName, argv[argi] );
					break;
				case 'H':	// Harris�̃e�X�g���[�h
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
