#include "def_general.h"

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include "disc.h"
#include "hash.h"

#ifdef	LLAHDOC_EXPORTS
#ifdef	WIN32
#include <windows.h>
#endif
#include "llahdoc_dll.h"
#else
// ��DLL�̏ꍇ�Ɉȉ��̊֐����g���ꍇ�̃w�b�_�t�@�C�������Ă���
#include "llahdoc_dll.h"
#endif

#include "dirs.h"
#include "auto_connect.h"
#include "extern.h"
#include "init.h"
#include "hist.h"
#include "nn5.h"
#include "inacr2.h"
#include "prop.h"
#include "nears.h"
#include "projrecov.h"

#ifdef	WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

int g_var = 1;

void Version( void )
// �o�[�W������\��
{
	printf( "%g\n", kLlahDocVersion );
}

char *MallocTest( int size )
// �������m�ۃe�X�g
{
	getchar();
	return malloc( size );
}

void MemChangeTest( char *mem, int size )
// ���������������E����e�X�g
{
	int i;

	for ( i = 0; i < size; i++ ) {
		mem[i] = i;
	}
	getchar();
	free( mem );
}

void GlobalSet( int var )
{
	g_var = var;
}

int GlobalGet( void )
{
	return g_var;
}

int LlahDocExtractFpRegFile( const char *fname, int *width, int *height, int **xs, int **ys )
// �����_���o
{
	int i, num;
	double *areas;
	cv::Mat img;
	cv::Point *ps;
	cv::Size size;

	eHashGaussMaskSize = kDfltHashGaussMaskSize;

	img = GetConnectedImage2( fname, CONST_HASH_MODE, eHashGaussMaskSize );	// �����摜���쐬
	if (img.empty())	return -1;
	num = MakeCentresFromImage( &ps, img, &size, &areas );	// �����_�𒊏o
	*width = size.width;
	*height = size.height;
	*xs = (int *)calloc( num, sizeof(int) );
	*ys = (int *)calloc( num, sizeof(int) );
	for ( i = 0; i < num; i++ ) {
		(*xs)[i] = ps[i].x;
		(*ys)[i] = ps[i].y;
	}

	return num;
}

int LlahDocConstructDb( char inv, int n, int m, int d, const char *reg_dir, const char *reg_suffix, const char *db_dir )
{
	strDisc disc;
	int i, ret;
	cv::Point **reg_pss = NULL;
	cv::Size *reg_sizes = NULL;
	int *reg_nums = NULL;
	char **dbcors = NULL;
	double **reg_areass = NULL;
	// argv
	int argc = 0;
	char *argv[12];
	// �n�b�V��
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

	// �p�����[�^����argv�̍쐬
	// �_�~�[
	argv[argc] = (char *)calloc( strlen( "llahdoc.dll" ) + 1, sizeof( char ) );
	strcpy( argv[argc], "llahdoc.dll" );
	argc++;
	// �s�ϗʃ^�C�v
	if ( inv == 'r' || inv == 'a' || inv == 's' ) {
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		sprintf( argv[argc], "-%c", inv );
		argc++;
	}
	if ( n > 0 ) {
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		strcpy( argv[argc], "-n" );
		argc++;
		argv[argc] = (char *)calloc( (int)(log10((double)n)+1.0L)+1, sizeof(char) );
		sprintf( argv[argc], "%d", n );
		argc++;
	}
	if ( m > 0 ) {
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		strcpy( argv[argc], "-m" );
		argc++;
		argv[argc] = (char *)calloc( (int)(log10((double)m)+1.0L)+1, sizeof(char) );
		sprintf( argv[argc], "%d", m );
		argc++;
	}
	if ( d > 0 ) {
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		strcpy( argv[argc], "-d" );
		argc++;
		argv[argc] = (char *)calloc( (int)(log10((double)d)+1.0L)+1, sizeof(char) );
		sprintf( argv[argc], "%d", d );
		argc++;
	}
	// �o�^�摜
	// -c
	argv[argc] = (char *)calloc( 3, sizeof(char) );
	strcpy( argv[argc], "-c" );
	argc++;
	// �f�B���N�g��
	if (reg_dir != NULL && reg_dir[0] != '\0' ) {	// reg_dir���w�肳��Ă���
		argv[argc] = (char *)calloc( strlen( reg_dir ) + 1, sizeof(char) );
		strcpy( argv[argc], reg_dir );
		argc++;
	} else {	// reg_dir���w�肳��Ă��Ȃ�
		argv[argc] = (char *)calloc( strlen( kDfltHashSrcDir ) + 1, sizeof(char) );
		strcpy( argv[argc], kDfltHashSrcDir );
		argc++;
	}
	// �g���q
	if (reg_suffix != NULL && reg_suffix[0] != '\0' ) {	// reg_suffix���w�肳��Ă���
		argv[argc] = (char *)calloc( strlen( reg_suffix ) + 1, sizeof(char) );
		strcpy( argv[argc], reg_suffix );
		argc++;
	} else {	// reg_dir���w�肳��Ă��Ȃ�
		argv[argc] = (char *)calloc( strlen( kDfltHashSrcSuffix ) + 1, sizeof(char) );
		strcpy( argv[argc], kDfltHashSrcSuffix );
		argc++;
	}
	// �o�͐�f�B���N�g��
	if (db_dir != NULL && db_dir[0] != '\0' ) {	// hash_dir���w�肳��Ă���
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		strcpy( argv[argc], "-h" );
		argc++;
		argv[argc] = (char *)calloc( strlen( db_dir ) + 1, sizeof(char) );
		strcpy( argv[argc], db_dir );
		argc++;
	}
	// argv�̊m�F
//	for ( i = 0; i < argc; i++ ) {
//		printf("%s\n", argv[i]);
//	}
	// argv���g���Đݒ�
	AnalyzeArgAndSetExtern2( argc, argv );

	// ./dcams -c [�摜�̃f�B���N�g��] [�摜�̊g���q] (-o [�o�̓f�B���N�g��])
	fprintf( stderr, "Hash Constraction\n" );
	// �����_�̒��o
	fprintf( stderr, "Extracting Feature Points...\n" );
	ret = CreatePointFile4( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors, eHashSrcDir, eHashSrcSuffix, eHashGaussMaskSize, eDirsDir, ePointDatFileName );
	if ( ret <= 0 )	return 0;
	// ���U���t�@�C���̍쐬
	fprintf( stderr, "Making %s...\n", eDiscFileName );
	ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss, reg_nums, &disc, eDiscFileName );
	if ( ret <= 0 )	return 0;
	// �n�b�V���̍\�z
	fprintf( stderr, "Constructing Hash Table...\n" );
	ret = ConstructHash2( eDbDocs, reg_pss, reg_areass, reg_nums, &disc, &hash, &hash2, &hash3 );
	if ( ret <= 0 )	return 0;
	// �Փ˂̑������ڂ�����
	if ( !eNoHashList && eRemoveCollision )	{
		if ( eCompressHash )	ret = RefineHash2( hash2 );
		else					ret = RefineHash( hash );
	}
	// �n�b�V���̃`�F�b�N
	if ( eExperimentMode ) {
		if ( eNoHashList )			ChkHash3( hash3 );
		else if ( eCompressHash )	ChkHash2( hash2 );
		else						ChkHash( hash );
	}
	// �n�b�V���̕ۑ�
	fprintf( stderr, "Saving Hash Table...\n" );
	if ( eNoHashList )			ret = SaveHash3( hash3, eHashDatFileName );
	else if ( eCompressHash )	ret = SaveHash2( hash2, eHashDatFileName );
	else						ret = SaveHash( hash, eHashDatFileName );
	if ( ret <= 0 )	return 0;
	// ���萔�̌v�Z
	fprintf( stderr, "Calculating Prop...\n" );
//	eProp = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss, reg_areass, reg_nums, &disc, hash, hash2, hash3 );
	eProp = 0.0L;
	// �ݒ�t�@�C���̕ۑ�
	fprintf( stderr, "Saving Config...\n" );
	ret = SaveConfig( eInvType, eGroup1Num, eGroup2Num, eDiscNum, eProp, eCompressHash, eDocBit, ePointBit, eRBit, eOBit, eHList2DatByte, eRotateOnce, eUseArea, eIsJp, eNoHashList, eConfigFileName );
	if ( ret <= 0 )	return 0;

	return 1;
}

strLlahDocDb *LlahDocLoadDb( const char *db_dir )
// �n�b�V���̃��[�h
{
	int ret;
	// argv
	int argc = 0;
	char *argv[12];
	// Load
	char fname[kMaxPathLen], **dbcors = NULL;
	strDisc disc;
	int *numall = NULL;
	cv::Point **psall = NULL;
	double **areaall = NULL;
	cv::Size *sizeall = NULL;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;
	// db
	strLlahDocDb *db = NULL;

		// �p�����[�^����argv�̍쐬
	// �_�~�[
	argv[argc] = (char *)calloc( strlen( "llahdoc.dll" ) + 1, sizeof( char ) );
	strcpy( argv[argc], "llahdoc.dll" );
	argc++;
	// �n�b�V���̃f�B���N�g��
	if (db_dir != NULL && db_dir[0] != '\0' ) {	// hash_dir���w�肳��Ă���
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		strcpy( argv[argc], "-h" );
		argc++;
		argv[argc] = (char *)calloc( strlen( db_dir ) + 1, sizeof(char) );
		strcpy( argv[argc], db_dir );
		argc++;
	}
	// argv���g���Đݒ�
	AnalyzeArgAndSetExtern2( argc, argv );

	db = (strLlahDocDb *)malloc( sizeof(strLlahDocDb) );	// Db�̊m��
//	ret = LoadConfig();	// �ݒ�t�@�C���̓ǂݍ���
	ret = LoadConfig( eConfigFileName, &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &eProp, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
	if ( ret <= 0 )	return NULL;
	sprintf( fname, "%s%s", eDirsDir, eDiscFileName );
	ret = LoadDisc( fname, &disc );	// ���U���t�@�C���̓ǂݍ���
	if ( ret <= 0 )	return NULL;
	// �n�b�V���̓ǂݍ���
	if ( eNoHashList )			hash3 = LoadHash3( disc.num, eHashDatFileName );
	else if ( eCompressHash )	hash2 = LoadHash2( disc.num, eHashDatFileName );
	else						hash = LoadHash( disc.num, eHashDatFileName );
	if ( ret <= 0 )	return NULL;
	sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
	ret = LoadPointFile2( fname, &psall, &areaall, &sizeall, &numall, &dbcors );	// �����_�f�[�^�̓ǂݍ���
	if ( ret <= 0 )	return NULL;
	// Db�ւ̑��
	db->disc = disc;
	db->hash = hash;
	db->hash2 = hash2;
	db->hash3 = hash3;
	db->reg_pss = psall;
	db->reg_areass = areaall;
	db->reg_sizes = sizeall;
	db->reg_nums = numall;
	db->dbcors = dbcors;
	// �g�ݍ��킹�̍쐬
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );

	return db;
}

void LlahDocReleaseDb( strLlahDocDb *db )
// �f�[�^�x�[�X�̉��
{
	int i;

	if (db == NULL)	return;
	ReleaseHash( db->hash );
	ReleaseHash2( db->hash2 );
	ReleaseHash3( db->hash3 );
	if ( db->reg_nums != NULL)	free( db->reg_nums );
	if ( db->reg_sizes != NULL)	free( db->reg_sizes );
	if ( db->reg_pss != NULL)
		for ( i = 0; i < eDbDocs; i++ )	free( db->reg_pss[i] );
	free( db->reg_pss );
	if ( db->reg_areass != NULL)
		for ( i = 0; i < eDbDocs; i++ )	free( db->reg_areass[i] );
	free( db->reg_areass );
	if ( db->dbcors != NULL)
		for ( i = 0; i < eDbDocs; i++ )	free( db->dbcors[i] );
	free( db->dbcors );
	if ( db->disc.dat != NULL)	free( db->disc.dat );

	free( db );
}

int LlahDocRetrieveImage( const char *img_fname, strLlahDocDb *db, char *result, int result_len )
// �Î~�摜����
{
	int i, num, **nears = 0, *score, res, ret_time, res_score;
	cv::Point *ps = 0;
	cv::Size img_size;
	cv::Mat img, img_cap;
	double *areas = NULL;
	// db
	char **dbcors = NULL;
	strDisc disc;
	int *reg_nums = NULL;
	cv::Point **reg_pss = NULL;
	double **reg_areass = NULL;
	cv::Size *reg_sizes = NULL;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

//	fprintf( stderr, "Retrieval from Static Image\n" );
	eEntireMode = RETRIEVE_MODE;
	eExperimentMode = 1;
	// db�����[�J���ϐ��ɑ��
	disc = db->disc;
	hash = db->hash;
	hash2 = db->hash2;
	hash3 = db->hash3;
	reg_pss = db->reg_pss;
	reg_areass = db->reg_areass;
	reg_sizes = db->reg_sizes;
	reg_nums = db->reg_nums;
	dbcors = db->dbcors;

	// �����摜���쐬
#ifdef	WIN32	// OpenCV�̃o�[�W�������قȂ邽��
//	img = GetConnectedImage2( img_fname, eEntireMode );
	img_cap = cv::imread(img_fname, cv::IMREAD_COLOR);
	if (img_cap.empty())	return 0;
	img = GetConnectedImageCam( img_cap );
#else
	if ( eIsJp ) 	img = GetConnectedImageJp2( query, eEntireMode );
	else			img = GetConnectedImage2( query, eEntireMode );
#endif
	if (img.empty())	return 0;
	// �����_�𒊏o
	num = MakeCentresFromImage( &ps, img, &img_size, &areas );
	// �ߖT�\�����v�Z
	MakeNearsFromCentres( ps, num, &nears );
	// ����
	score = (int *)calloc( kMaxDocNum, sizeof(int) );
	res = RetrieveNN52( ps, areas, num, nears, img_size, score, &disc, reg_nums, &ret_time, hash, hash2, hash3 );
//	OutPutResult( score, 0, 5, argv[argi] );	// ���̂܂܂ł͎g���Ȃ�
	if ( !eExperimentMode )	printf( "%s : %d\n", dbcors[res], score[res] );
	// ���ʂ̊i�[
	strncpy( result, dbcors[res], result_len );
	res_score = score[res];
	free( score );

	return res_score;
}

int LlahDocRetrieveIplImage( cv::Mat img_cap, strLlahDocDb *db, char *result, int result_len )
// cv::Mat����
// ���A���^�C�������p
{
	int i, num, **nears = 0, *score, res, ret_time, res_score;
	cv::Point *ps = 0;
	cv::Size img_size;
	cv::Mat img;
	double *areas = NULL;
	// db
	char **dbcors = NULL;
	strDisc disc;
	int *reg_nums = NULL;
	cv::Point **reg_pss = NULL;
	double **reg_areass = NULL;
	cv::Size *reg_sizes = NULL;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;
	// Web�J�����p
	strProjParam param, zero_param;
	cv::Size res_size;
	int pcor[kMaxPointNum][2], pcornum = 0;

//	fprintf( stderr, "Retrieval from cv::Mat\n" );
	// db�����[�J���ϐ��ɑ��
	disc = db->disc;
	hash = db->hash;
	hash2 = db->hash2;
	hash3 = db->hash3;
	reg_pss = db->reg_pss;
	reg_areass = db->reg_areass;
	reg_sizes = db->reg_sizes;
	reg_nums = db->reg_nums;
	dbcors = db->dbcors;
	// Web�J�����p
	score = (int *)calloc( kMaxDocNum, sizeof(int) );
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	param = zero_param;
	res_size = reg_sizes[0];

#ifdef	WIN32	// OpenCV�̃o�[�W�������قȂ邽��
//	img = GetConnectedImage( query, eEntireMode );
	// Web�J�������[�h�ł̓����_���o
	img = GetConnectedImageCam( img_cap );
#else
	fprintf( stderr, "This program currently support only Windows.\n" );
	return 0;
#endif
	if (img.empty()) {
		result[0] = '\0';
		param = zero_param;	// �������ʂ��Ȃ��̂Ƀp�����[�^����������Ă̂��ςȂ̂�
		return 0;
	}
	// �����_�𒊏o
	num = MakeCentresFromImage( &ps, img, &img_size, &areas );
	// �ߖT�\�����v�Z
//	MakeNearsFromCentres( ps, num, &nears );
	if ( num >= kMinPoints ) {	// �����_�����ŏ��l�ȏ�
		MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num, &nears );	// �ߖT�_�v�Z�i�����Łj
		// ����
		res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, eProp, reg_nums, hash, hash2, hash3 );	// �����̂�
		// �ŏ��Ή��_���ȏ�Ȃ�A�ˉe�ϊ��p�����[�^���v�Z
		if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, reg_pss[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// �o�^�摜�ɏ㏑��
		else	param = zero_param;
		if (ps != NULL && num > 0 )	ReleaseCentres( ps );	// ps�̉��
		if (areas != NULL && num > 0 )	free( areas );
		ReleaseNears( nears, num );	// nears�̉��
		// �v�Z���Ԃ̕\��
//			OutPutResultSv( score, end-start, 0/*5*/, doc_name );
		if ( score[res] > kTopThr ) {	// �g�b�v�̓��[����臒l�ȉ��i���W�F�N�g�j
//			GetBasename2( dbcors[res], kMaxPathLen, doc_name );
			strncpy( result, dbcors[res], result_len );
		}
		else {
			result[0] = '\0';
			param = zero_param;	// �������ʂ��Ȃ��̂Ƀp�����[�^����������Ă̂��ςȂ̂�
		}
//		puts( doc_name );
	}
	else {	// �����_�����ŏ��l�𖞂����Ȃ�
//		puts("min");
		res = 0;
		result[0] = '\0';
		param = zero_param;
	}
	if ( result[0] != '\0' )	res_size = reg_sizes[res];	// ���������Ȃ炻�̃T�C�Y
	else	res_size = reg_sizes[0];	// �����łȂ���΃f�[�^�x�[�X��0�Ԃ̃T�C�Y

//	fprintf(stdout, "%s\n", doc_name);
//	if ( eProtocol == kTCP )	ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
//	else						ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
//	if ( ret < 0 )	break;
	// �I������
	res_score = score[res];
	free(score);
	// ���ʂ̊i�[
//	strncpy( result, dbcors[res], result_len );

	return res_score;
}
