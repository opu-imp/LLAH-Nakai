#include "def_general.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <opencv2/opencv.hpp>

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
// ���R�ΐ�����2�̑ΐ������߂�
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
// ������͂̊֐�2
{
	int argi;

	// ������
#ifdef	WIN32
	eEntireMode = USBCAM_SERVER_MODE;	// �����Ȃ��ł̓��A���^�C�������ɂ���
#else
	eEntireMode = RETRIEVE_MODE;	// LINUX�łł͐Î~�挟�����[�h���f�t�H���g�ɂ���
#endif
//	strcpy( eDirsDir, kDfltDirsDir );
	strcpy( eHashSrcDir, kDfltHashSrcDir );
	strcpy( eHashSrcSuffix, kDfltHashSrcSuffix );
	strcpy( ePointDatFileName, kDfltPointDatFileName );
	eHashGaussMaskSize = kDfltHashGaussMaskSize;
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

	for ( argi = 1; argi < argc && *(argv[argi]) == '-'; argi++ ) {
		switch ( *(argv[argi]+1) ) {	// '-'�̎��̕����ŕ���
			case 'r':	// ������g�p
				eInvType = CR_AREA;
				break;
			case 'a':	// �A�t�B���s�ϗʂ��g�p
				eInvType = AFFINE;
				break;
			case 's':	// �����s�ϗʂ��g�p
				eInvType = SIMILAR;
				break;
			case 'n':	// n���w��
				if ( ++argi < argc )	eGroup1Num = atoi( argv[argi] );
				break;
			case 'm':	// m���w��
				if ( ++argi < argc )	eGroup2Num = atoi( argv[argi] );
				break;
			case 'd':	// ���U�����x�������w��
				if ( ++argi < argc )	eDiscNum = atoi( argv[argi] );
				break;
			case 'c':	// �n�b�V���̍\�z
				eEntireMode = CONST_HASH_MODE;
				if ( ++argi < argc )	strcpy( eHashSrcDir, argv[argi] );	// argi��i�߂ĕ�������R�s�[�Dargc-1�𒴂����ꍇ�͉������Ȃ�
				if ( ++argi < argc )	strcpy( eHashSrcSuffix, argv[argi] );	// �g���q�������Ɏw��
				break;
			case 'h':	// �n�b�V�����̃f�B���N�g�����w��
				if ( ++argi < argc )	strcpy( eDirsDir, argv[argi] );
				break;
//			case 'f':	// �n�b�V���̉摜�t�@�C���̊g���q���w��
//				if ( ++argi < argc )	strcpy( eHashSrcSuffix, argv[argi] );
//				break;
			case 'S':	// USB�J�����T�[�o���[�h
				eEntireMode = USBCAM_SERVER_MODE;
				break;
			case 'x':	// point.dat���g���ăn�b�V�����\�z���郂�[�h
				eEntireMode = CONST_HASH_PF_MODE;
				break;
			case 'e':	// �������[�h
				eExperimentMode = 1;
				break;
			case 'z':	// �n�b�V�����k���[�h
				eCompressHash = 1;
				break;
			case 'y':	// ��]�̑���������s��Ȃ�
				eRotateOnce = 1;
				break;
			case 'w':	// �Փ˂̑������ڂ�����
				eRemoveCollision = 1;
				if ( ++argi < argc )	eMaxHashCollision = atoi( argv[argi] );
				break;
			case 'v':	// �����ʂ̔�r�Ȃ� -w 1 �𐄏�
				eNoCompareInv = 1;
				break;
			case 'u':	// �����ʌv�Z�ɖʐς𗘗p
				eUseArea = 1;
				break;
			case 'J':	// ���{�ꃂ�[�h
				eIsJp = 1;
				break;
			case 't':	// ���X�g�Ȃ�
				eNoHashList = 1;
				break;
			default:	// ���m�̈���
				fprintf( stderr, "warning: %c is an unknown argument\n",  *(argv[argi]+1) );
				break;
		}
	}
	// �f�B���N�g���̖����Ɂi�Ȃ���΁j�X���b�V����ǉ�
	AddSlash( eHashSrcDir, kMaxPathLen );
	AddSlash( eDirsDir, kMaxPathLen );
	// ���S�ȃp�X���쐬
	sprintf( eHashFileName, "%s%s", eDirsDir, eHashDatFileName );
	// f���w��
	switch ( eInvType ) {
		case CR_AREA:
			eGroup3Num = 5;
			break;
		case CR_INTER:	// �g���Ă��Ȃ��͂������C�ꉞ
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
	// �g�ݍ��킹�����v�Z
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	// �n�b�V�����k���[�h�ł̕K�v�o�C�g���Ȃǂ��v�Z
	eDocBit = (int)ceil( log2(kMaxDocNum) );	// doc�̃r�b�g��
	ePointBit = (int)ceil( log2(kMaxPointNum) );	// point�̃r�b�g��
	eRBit = (int)ceil( log2(eDiscNum) );	// �s�ϗ�1�̃r�b�g��
	eOBit = (int)ceil( log2(eGroup2Num) );	// �ʐϓ����ʁi�����j1�̃r�b�g��
	if ( eNoCompareInv ) {
		eHList2DatByte = (int)ceil( ((double)( eDocBit + ePointBit )) / 8.0L );	// ���X�g�̃f�[�^���̃o�C�g��
	} else {
		if ( eUseArea )	eHList2DatByte = (int)ceil( ((double)( eDocBit + ePointBit + eRBit*eNumCom2 + eOBit*eGroup2Num )) / 8.0L );	// ���X�g�̃f�[�^���̃o�C�g��
		else			eHList2DatByte = (int)ceil( ((double)( eDocBit + ePointBit + eRBit*eNumCom2 )) / 8.0L );	// ���X�g�̃f�[�^���̃o�C�g��
	}
	// ���X�g�Ȃ����[�h�Ŋ���o�C�g��������Ȃ����`�F�b�N
	if ( eNoHashList && (int)ceil( ((double)( eDocBit + ePointBit )) / 8.0L ) > sizeof( HENTRY ) ) {
		fprintf( stderr, "error: kMaxDocNum and/or kMaxPointNum is too large to store in HENTRY(%d bytes).\n", sizeof( HENTRY ) );
		return 0;
	}

	return argi;
}

int AnalyzeArgAndSetExtern( int argc, char *argv[], int *p_emode, int *p_rmode, int *p_fmode, int *p_pmode )
// ��������͂��A�p�����[�^�iextern�ϐ��j��ݒ肷��
// p_emode: �S�̂̃��[�h�i�o�^�A�����Ȃǁj
// p_rmode: �ˉe�ϊ��̕␳�����邩�ۂ�
// p_fmode: �����_�͉����g�����i�P��̏d�S�A�͂܂ꂽ�̈�̏d�S�A�|�C���g�t�@�C���j
// p_pmode: �����摜���p�ӂ���Ă��邩�ǂ����i����06/01/12�ł͌`�[���j
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
	// ���{�ꃂ�[�h�֌W
	eJpGParam1 = 1;
	eJpAdpBlock = 50;
	eJpAdpSub = 10;
	eJpGParam2 = 1;
	eJpThr = 200;
	// �ł��؂�֌W
	eTerminate = TERM_NON;
	eTermVNum = kDefaultTermVNum;

	srand( (unsigned int)time(NULL) );	// �����̏�����
/****** �����ꗗ ******
-n [���l]:	n�̒l
-m [���l]:	m�̒l
-d [���l]:	���U�����x����
-[r/i/a/s]:	�ʐςɂ�镡��/��_�ɂ�镡��/�A�t�B���s�ϗ�/�����s�ϗ�
-j:	���S�_���܂�
-E:	�ʐς�p����
-M:	�g�ѓd�b���[�h
-c:	�n�b�V���\�z���[�h
-C [�n�b�V���̃p�X]:	�n�b�V���\�z���[�h�i�p�X�w��j
-P [���萔�v�Z�p�̃p�X]:	���萔�v�Z�p�X�w��
-t:	�e�X�g���[�h
-R:	�ˉe�ϊ��̕␳���s��
-e:	�͂܂ꂽ�̈������_�ɗp����
-p:	�P��̘A�����ꂽ�摜�𗘗p����
-l:	�P��̘A�����ꂽ�摜���c��
-A:	�n�b�V���ǉ����[�h
-f [�_�t�@�C���̃f�B���N�g��] [�Ή��֌W�t�@�C��] [�����_���t�@�C��]:	�n�b�V���̍\�z�ɓ_�t�@�C����p����
-q:	��������̓����_�t�@�C���쐬���[�h
-g:	�o�^�摜�̓����_�t�@�C���쐬���[�h
-G [�o�^�摜�̌����p�X] [�����_�t�@�C���̃f�B���N�g��] [�Ή��֌W�t�@�C��] [�����_���t�@�C��] [�J�n�ԍ�]:	�o�^�摜�̓����_�t�@�C���쐬���[�h2
-D [���U���t�@�C���쐬�ɗp����p�X]:	���U���t�@�C���̍쐬�ɗp����p�X���w��
-I [DIRS�f�B���N�g��]:	�e��ݒ�t�@�C���̃f�B���N�g�����w��
-N:	�����_�x�N�g���̃`�F�b�N�Ȃ�
-h:	�n�b�V���̃`�F�b�N���[�h
-S:	USB�J�����T�[�o���[�h
-T [TCP�|�[�g�ԍ�]:	TCP�|�[�g�̎w��
-J:	���{�ꃂ�[�h
-v [���l]:	�ł��؂胂�[�h�ƃp�����[�^
****** �����ꗗ ******/
	for ( argi = 1; argi < argc && *(argv[argi]) == '-'; argi++ ) {
		for ( pi = 1; *(argv[argi]+pi) != '\0'; pi++ ) {
			switch ( *(argv[argi]+pi) ) {
				case 'n' :	// n�̐ݒ�
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					n = atoi(argv[argi]+pi);
					if ( n == 0 )	return -1;
					goto next_arg;
					break;
				case 'm' :	// m�̐ݒ�
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					m = atoi(argv[argi]+pi);
					if ( m == 0 )	return -1;
					goto next_arg;
					break;
				case 'd' :	// ���U�����x���̐ݒ�
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
				case 'j' :	// ���S�_���܂ނ�
					inc_centre = 1;
					break;
				case 'E' :	// �ʐς�p���邩
					use_area = 1;
					break;
				case 'M' :	// �g�ѓd�b���[�h
					*p_emode = RET_MP_MODE;
					break;
				case 'c' :	// �n�b�V���\�z���[�h
					*p_emode = CONST_HASH_MODE;
					break;
				case 'C' :	// �n�b�V���\�z���[�h�i�p�X�w��j
					*p_emode = CONST_HASH_MODE;
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( hash_path, argv[argi] );
					goto next_arg;
					break;
				case 'P' :	// ���萔�v�Z�p�X�w��
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					strcpy( prop_path, argv[argi] );
					goto next_arg;
					break;
				case 't' :	// �e�X�g���[�h
					*p_emode = TEST_MODE;
					break;
				case 'R' :	// �ˉe�ϊ��̕␳���s��
					*p_rmode = RECOVER_MODE;
					break;
				case 'e' :	// �͂܂ꂽ�̈������_�ɗp����
					*p_fmode = ENCLOSED_MODE;
					break;
				case 'p' :	// �P��̘A�����ꂽ�摜�𗘗p����
					*p_pmode = PREPARED_MODE;
					break;
				case 'l' :	// �P��̘A�����ꂽ�摜���c��
					*p_pmode = LEAVE_MODE;
					break;
				case 'A' :	// �n�b�V���ǉ����[�h
					*p_emode = ADD_HASH_MODE;
					break;
				case 'f' :	// �_�t�@�C����p����i�_�t�@�C���̃f�B���N�g���ƑΉ��t�@�C�������w��j
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
				case 'G' :	// �����_���o���[�h�Q
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
				case 'J' :	// ���{�ꃂ�[�h
					eIsJp = 1;
					break;
				case 'v' :	// �ł��؂胂�[�h
					eTerminate = TERM_VNUM;	// �Ƃ肠�������̃��[�h�ŌŒ�
					pi++;
					if ( *(argv[argi]+pi) == '\0' ) {
						argi++;
						pi = 0;
					}
					eTermVNum = atoi(argv[argi]+pi);	// �p�����[�^���w��
					goto next_arg;
					break;
				default :
					fprintf(stderr, "%c : �����ȃp�����[�^�ł�\n", *(argv[argi]+pi));
					break;
			}
		}
		next_arg:
			;
	}
	AddSlash( dirs_dir, 1024 );	// �X���b�V�����Ȃ���Βǉ�
	if ( *point_dir == '\0' ) {	// �_�t�@�C���̃f�B���N�g�����w�肳��Ă��Ȃ����
		sprintf( point_dir, "%spoint/", dirs_dir );	// dirs_dir����point�f�B���N�g�����쐬
	} else {
		AddSlash( point_dir, 1024 );	// �X���b�V�����Ȃ���Βǉ�
	}
	if ( *p_emode == CONST_HASH_MODE ) {	// �n�b�V���\�z���[�h�̂Ƃ�
#ifndef	WIN32	// DOS��mkdir�̓p�X�̃f���~�^��'\\'�łȂ��ƃG���[�ɂȂ�̂ŁCWindows�ł͖����ɂ���
		sprintf(cmd, "mkdir %s %s", kMkdirNoMesOpt, dirs_dir);	// �f�B���N�g�����Ȃ���΍쐬
		system(cmd);
		sprintf(cmd, "mkdir %s %s", kMkdirNoMesOpt, point_dir);
		system(cmd);
#endif
	}
	if ( *p_emode == RETRIEVE_MODE || *p_emode == USBCAM_SERVER_MODE )	LoadSetting( &inv_type, &n, &m, &disc_num, point_dir, dbcor_fn, pn_fn, &prop, &dbdocs );
	else	SaveSetting( inv_type, n, m, disc_num, point_dir, dbcor_fn, pn_fn );
	
	if ( *dbcor_fn == '\0' ) {	// �Ή��t�@�C�����w�肳��Ă��Ȃ����
		sprintf( dbcor_fn, "%sdbcor.dat", dirs_dir );
	}
	if ( *pn_fn == '\0' ) {	// �����_���t�@�C�����w�肳��Ă��Ȃ����
		sprintf( pn_fn, "%spnum.txt", dirs_dir );
	}
	sprintf( hash_fn, "%shash.dat", dirs_dir );
	if ( *p_emode == CREATE_QPF_MODE || *p_emode == CREATE_RPF_MODE ) {
		eUseArea = use_area;
		return argi;	// �����_����邾���Ȃ�SetExtern�͕s�v
	}
	if ( SetExtern( *p_emode, n, m, inv_type, disc_num, prop, inc_centre, use_area, vec_chk, hash_path, dirs_dir, point_dir, ".dat", \
		dbcor_fn, "disc/", dm_path, prop_path, pn_fn, hash_fn, dbdocs, start_num ) )	return argi;
	else	return -1;
}


int SetExtern( int emode, int n, int m, int inv_type, int disc_num, double prop, int inc_centre, int use_area, int vec_chk, \
 char *hs_path, char *dirs_dir, char *pf_pref, char *pf_suf, char *cor_fn, \
 char *disc_dir, char *dm_path, char *prop_path, char *pn_fn, char *hash_fn, int dbdocs, int start_num )
// �p�����[�^��ݒ肷��
{
	int i, num, den;
	char it, disc_fn[1024], cmd[1024];

	// �s�ϗʃ^�C�v�̐ݒ�
	eInvType = inv_type;
	// ���S�_���܂߂邩�ǂ���
	eIncludeCentre = inc_centre;
	// �ʐς�p���邩�ǂ���
	eUseArea = use_area;
	// �����ʂ̈�v������s����
	eVectorCheck = vec_chk;
	// n�����m�̐ݒ�
	eGroup1Num = n;
	eGroup2Num = m;
	// �s�ϗʃ^�C�v�ɉ�����eGroup3Num�̐ݒ�
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
//	if ( eIncludeCentre || inv_type == SIMILAR )	// ���S�_���܂�
	if ( eIncludeCentre )	// ���S�_���܂�
		eGroup3Num--;

	// nCm�̐ݒ�
	for ( i = 0, num = 1, den = 1; i < eGroup2Num; i++ ) {
		num *= eGroup1Num - i;
		den *= i + 1;
	}
	eNumCom1 = (int)(num / den);
//	if ( inv_type == SIMILAR ) {
//		eNumCom2 = eGroup2Num - 1;
//	} else {
	// mCf�̐ݒ�
		for ( i = 0, num = 1, den = 1; i < eGroup3Num; i++ ) {
			num *= eGroup2Num - i;
			den *= i + 1;
		}
		eNumCom2 = (int)(num / den);
//	}
	// �ʎq�����x��
	eDiscNum = disc_num;
	eProp = prop;

	// ���U���t�@�C���̐ݒ�
	sprintf( disc_fn, "%sdisc.txt", dirs_dir );
	if ( *dm_path != '\0' ) {	// dm_path���ݒ肳��Ă���Ȃ�Cacr2���N������disc.txt�𐶐�����
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
	// ���萔�̐ݒ�
	strcpy( ePropMakePath, prop_path );
//	if ( inv_type != SIMILAR && !IsExist( eDiscFileName ) ) {
	if ( !IsExist( eDiscFileName ) && emode != CREATE_RPF_MODE2 ) {
		fprintf(stderr, "���U���t�@�C��%s�͑��݂��܂���\n", eDiscFileName );
		return 0;
	}

	strcpy( eDirsDir, dirs_dir );
	strcpy( eHashSrcPath, hs_path );	// �n�b�V���̌��摜�t�@�C���̒T���p�X
//	strcpy( eHashSrcDir, hs_dir );	// �n�b�V���̌��摜�t�@�C���̃f�B���N�g��
	strcpy( ePFPrefix, pf_pref );	// �n�b�V���̓_�t�@�C���̃f�B���N�g��
	strcpy( ePFSuffix, pf_suf );	// �n�b�V���̓_�t�@�C���̊g���q
	strcpy( eDBCorFileName, cor_fn );	// �Ή��t�@�C��
	strcpy( ePNFileName, pn_fn );	// �_�̐��̃t�@�C��
	strcpy( eHashFileName, hash_fn ); // �n�b�V���̃t�@�C����
	eDbDocs = dbdocs;	// �f�[�^�x�[�X�̃T�C�Y
	eCPF2StartNum = start_num;	// �����_���o���[�h�Q�ł̊J�n�ԍ�

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
// ������Ō�̉��s���폜
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
// ini�t�@�C����ǂ�
{
	char line[kMaxLineLen], *tok;
	FILE *fp;

	// �f�t�H���g�l�̐ݒ�
	eTCPPort = kDefaultTCPPort;
	eProtocol = kDefaultProtocol;
	ePointPort = kDefaultPointPort;
	eResultPort = kDefaultResultPort;
	strcpy( eClientName, kDefaultClientName );
	strcpy( eServerName, kDefaultServerName );
	strcpy( eDirsDir, kDfltDirsDir );

	if ( ( fp = fopen( kIniFileName, "r" ) ) == NULL ) {	// ini�t�@�C�����Ȃ�
		fprintf( stderr, "warning: %s cannot be opened\n", kIniFileName );
		return 0;
	}
	for ( ; fgets( line, kMaxLineLen, fp ) != NULL;  ) {	// ini�t�@�C�����s���Ƃɏ���
//		puts( line );
		tok = strtok( line, " =\t\n" );	// strtok�ōs�𕪉�
		if (tok == NULL)	continue;	// �g�[�N���Ȃ�
		if ( *tok == '#' )	continue;	// �s����#�Ȃ�R�����g�Ƃ݂Ȃ�
		if ( strcmp( tok, "TCPPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL)	eTCPPort = atoi(tok);
		}
		else if ( strcmp( tok, "Protocol" ) == 0 ) {	// Protocol
			tok = strtok( NULL, " =\t\n" );
			if (tok == NULL)	continue;	// �g�[�N���Ȃ�
			if ( strcmp( tok, "TCP" ) == 0 )	eProtocol = 1;
			else if ( strcmp( tok, "UDP" ) == 0 )	eProtocol = 2;
		}
		else if ( strcmp( tok, "PointPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL)	ePointPort = atoi( tok );
		}
		else if ( strcmp( tok, "ResultPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL)	eResultPort = atoi( tok );
		}
		else if ( strcmp( tok, "ClientName" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL)	strcpy( eClientName, tok );
		}
		else if ( strcmp( tok, "ServerName" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL)	strcpy( eServerName, tok );
		}
		else if ( strcmp( tok, "DatabaseDir" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL)	strcpy( eDirsDir, tok );
		}
	}
	fclose( fp );

	return 1;
}

int SaveConfig( void )
// �ݒ�t�@�C����ۑ�
{
	char fname[kMaxPathLen], ch;
	FILE *fp;

	// config.dat�̃p�X���쐬
	sprintf( fname, "%s%s", eDirsDir, eConfigFileName );
	if ( ( fp = fopen( fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}
	// config.dat�̃o�[�W����
	fprintf( fp, "%s\n", kConfigVerStr );
	// �s�ϗʃ^�C�v
	switch ( eInvType ) {
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
	fprintf( fp, "%c %d\n", 'n', eGroup1Num );
	// m
	fprintf( fp, "%c %d\n", 'm', eGroup2Num );
	// d
	fprintf( fp, "%c %d\n", 'd', eDiscNum );
	// prop
	fprintf( fp, "%c %lf\n", 'p', eProp );
	// �n�b�V�����k���[�h�Ȃ�r�b�g���Ȃ�
	if ( eCompressHash )	fprintf( fp, "%c %d %d %d %d %d\n", 'z', eDocBit, ePointBit, eRBit, eOBit, eHList2DatByte );
	// ��]��ʂ胂�[�h
	if ( eRotateOnce )	fprintf( fp, "%c\n", 'y' );
	// �ʐς��g�p���邩�ǂ���
	if ( eUseArea )	fprintf( fp, "%c\n", 'u' );
	// ���{�ꃂ�[�h
	if ( eIsJp )	fprintf( fp, "%c\n", 'J' );
	// �n�b�V�����X�g�Ȃ����[�h
	if ( eNoHashList )	fprintf( fp, "%c\n", 't' );

	fclose( fp );
	return 1;
/* ���o�[�W����
	fprintf( fp, "%c %d %d %d\n", ch, eGroup1Num, eGroup2Num, eDiscNum );
	fprintf( fp, "%lf\n%d\n", eProp, eDbDocs );
	// �n�b�V�����k���[�h���ǂ���
	fprintf( fp, "%s\n", (eCompressHash) ? kCompressHashDiscroptor : kNormalHashDiscroptor );
	// �r�b�g��
	fprintf( fp, "%d %d %d %d\n", eDocBit, ePointBit, eRBit, eHList2DatByte );

	fclose( fp );
	return 1;
*/
}

int LoadConfig( void )
// �ݒ�t�@�C����ǂݍ���
{
	char fname[kMaxPathLen], line[kMaxLineLen], ch, *fgets_ret;
	FILE *fp;

	// config.dat�̃p�X���쐬���ĊJ��
	sprintf( fname, "%s%s", eDirsDir, eConfigFileName );
	if ( ( fp = fopen( fname, "r" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}
	fgets_ret = fgets( line, kMaxLineLen, fp );	// ��s�ǂݍ���
	// �ǂݍ��ݎ��s�������̓o�[�W����������ƈ�v����
	if (fgets_ret == NULL || !strcmp( line, kConfigVerStr ) ) {
		fprintf( stderr, "error: %s version error\n", eConfigFileName );
		return 0;
	}
	// �e�s����͂��Đݒ�
	// ��:	a
	//		n 7
	//		m 6
	//		d 15
	//		z 15 12 4 4
	//		y
	while ( fgets( line, kMaxLineLen, fp ) != NULL ) {
		switch ( line[0] ) {
			// �s�ϗʃ^�C�v
			case kInvCharCRArea:	// r
				eInvType = CR_AREA;
				eGroup3Num = 5;
				break;
			case kInvCharCRInter:	// i
				eInvType = CR_INTER;
				eGroup3Num = 5;
				break;
			case kInvCharAffine:	// a
				eInvType = AFFINE;
				eGroup3Num = 4;
				break;
			case kInvCharSimilar:	//s
				eInvType = SIMILAR;
				eGroup3Num = 3;
				break;
			case 'n':
				sscanf( line, "%c %d", &ch, &eGroup1Num );
				break;
			case 'm':
				sscanf( line, "%c %d", &ch, &eGroup2Num );
				break;
			case 'd':
				sscanf( line, "%c %d", &ch, &eDiscNum );
				break;
			case 'p':
				sscanf( line, "%c %lf", &ch, &eProp );
				break;
			case 'z':
				eCompressHash = 1;
				sscanf( line, "%c %d %d %d %d %d", &ch, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte );
				break;
			case 'y':
				eRotateOnce = 1;
				break;
			case 'u':
				eUseArea = 1;
				break;
			case 'J':
				eIsJp = 1;
				break;
			case 't':
				eNoHashList = 1;
				break;
			default:
				break;
		}
	}
	fclose( fp );
	// �g�ݍ��킹�����v�Z
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	return 1;

	/* ���o�[�W����
	sscanf( line, "%c %d %d %d", &ch, &eGroup1Num, &eGroup2Num, &eDiscNum );
	switch ( ch ) {	// �����ɂ���ĕ���
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
	fgets( line, kMaxLineLen, fp );	// ��s�ǂݍ���
	sscanf( line, "%lf", &eProp );
	fgets( line, kMaxLineLen, fp );	// ��s�ǂݍ���
	sscanf( line, "%d", &eDbDocs );
	// �n�b�V�����k���[�h�ǉ�
	fgets_ret = fgets( line, kMaxLineLen, fp );	// ��s�ǂݍ���
	if ( !strncmp( line, kCompressHashDiscroptor, strlen(kCompressHashDiscroptor) ) ) {
		puts("Compress Hash Mode");
		eCompressHash = 1;	// �n�b�V�����k���[�h
	}
	else {
		eCompressHash = 0;
	}
	// �r�b�g��
	fgets( line, kMaxLineLen, fp );	// ��s�ǂݍ���
	sscanf( line, "%d %d %d %d", &eDocBit, &ePointBit, &eRBit, &eHList2DatByte );

	fclose( fp );
	// �g�ݍ��킹�����v�Z
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );

	return 1;
	*/
}
