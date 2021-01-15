#include "def_general.h"

#include <stdio.h>

#include "cv.h"
#include "highgui.h"
#include "disc.h"
#include "hash.h"

#ifdef	LLAHDOC_EXPORTS
#ifdef	WIN32
#include <windows.h>
#endif
#include "llahdoc_dll.h"
#else
// 非DLLの場合に以下の関数を使う場合のヘッダファイルを入れておく
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
// バージョンを表示
{
	printf( "%g\n", kLlahDocVersion );
}

char *MallocTest( int size )
// メモリ確保テスト
{
	getchar();
	return malloc( size );
}

void MemChangeTest( char *mem, int size )
// メモリ書き換え・解放テスト
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
// 特徴点抽出
{
	int i, num;
	double *areas;
	IplImage *img;
	CvPoint *ps;
	CvSize size;

	eHashGaussMaskSize = kDfltHashGaussMaskSize;

	img = GetConnectedImage2( fname, CONST_HASH_MODE );	// 結像画像を作成
	if ( img == NULL )	return -1;
	num = MakeCentresFromImage( &ps, img, &size, &areas );	// 特徴点を抽出
	*width = size.width;
	*height = size.height;
	*xs = (int *)calloc( num, sizeof(int) );
	*ys = (int *)calloc( num, sizeof(int) );
	for ( i = 0; i < num; i++ ) {
		(*xs)[i] = ps[i].x;
		(*ys)[i] = ps[i].y;
	}

	cvReleaseImage( &img );

	return num;
}

int LlahDocConstructDb( char inv, int n, int m, int d, const char *reg_dir, const char *reg_suffix, const char *db_dir )
{
	strDisc disc;
	int i, ret;
	CvPoint **reg_pss = NULL;
	CvSize *reg_sizes = NULL;
	int *reg_nums = NULL;
	char **dbcors = NULL;
	double **reg_areass = NULL;
	// argv
	int argc = 0;
	char *argv[12];
	// ハッシュ
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

	// パラメータからargvの作成
	// ダミー
	argv[argc] = (char *)calloc( strlen( "llahdoc.dll" ) + 1, sizeof( char ) );
	strcpy( argv[argc], "llahdoc.dll" );
	argc++;
	// 不変量タイプ
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
	// 登録画像
	// -c
	argv[argc] = (char *)calloc( 3, sizeof(char) );
	strcpy( argv[argc], "-c" );
	argc++;
	// ディレクトリ
	if ( reg_dir != NULL && reg_dir[0] != '\0' ) {	// reg_dirが指定されている
		argv[argc] = (char *)calloc( strlen( reg_dir ) + 1, sizeof(char) );
		strcpy( argv[argc], reg_dir );
		argc++;
	} else {	// reg_dirが指定されていない
		argv[argc] = (char *)calloc( strlen( kDfltHashSrcDir ) + 1, sizeof(char) );
		strcpy( argv[argc], kDfltHashSrcDir );
		argc++;
	}
	// 拡張子
	if ( reg_suffix != NULL && reg_suffix[0] != '\0' ) {	// reg_suffixが指定されている
		argv[argc] = (char *)calloc( strlen( reg_suffix ) + 1, sizeof(char) );
		strcpy( argv[argc], reg_suffix );
		argc++;
	} else {	// reg_dirが指定されていない
		argv[argc] = (char *)calloc( strlen( kDfltHashSrcSuffix ) + 1, sizeof(char) );
		strcpy( argv[argc], kDfltHashSrcSuffix );
		argc++;
	}
	// 出力先ディレクトリ
	if ( db_dir != NULL && db_dir[0] != '\0' ) {	// hash_dirが指定されている
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		strcpy( argv[argc], "-h" );
		argc++;
		argv[argc] = (char *)calloc( strlen( db_dir ) + 1, sizeof(char) );
		strcpy( argv[argc], db_dir );
		argc++;
	}
	// argvの確認
//	for ( i = 0; i < argc; i++ ) {
//		printf("%s\n", argv[i]);
//	}
	// argvを使って設定
	AnalyzeArgAndSetExtern2( argc, argv );

	// ./dcams -c [画像のディレクトリ] [画像の拡張子] (-o [出力ディレクトリ])
	fprintf( stderr, "Hash Constraction\n" );
	// 特徴点の抽出
	fprintf( stderr, "Extracting Feature Points...\n" );
	ret = CreatePointFile4( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
	if ( ret <= 0 )	return 0;
	// 離散化ファイルの作成
	fprintf( stderr, "Making %s...\n", eDiscFileName );
	ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss, reg_nums, &disc );
	if ( ret <= 0 )	return 0;
	// ハッシュの構築
	fprintf( stderr, "Constructing Hash Table...\n" );
	ret = ConstructHash2( eDbDocs, reg_pss, reg_areass, reg_nums, &disc, &hash, &hash2, &hash3 );
	if ( ret <= 0 )	return 0;
	// 衝突の多い項目を除去
	if ( !eNoHashList && eRemoveCollision )	{
		if ( eCompressHash )	ret = RefineHash2( hash2 );
		else					ret = RefineHash( hash );
	}
	// ハッシュのチェック
	if ( eExperimentMode ) {
		if ( eNoHashList )			ChkHash3( hash3 );
		else if ( eCompressHash )	ChkHash2( hash2 );
		else						ChkHash( hash );
	}
	// ハッシュの保存
	fprintf( stderr, "Saving Hash Table...\n" );
	if ( eNoHashList )			ret = SaveHash3( hash3 );
	else if ( eCompressHash )	ret = SaveHash2( hash2 );
	else						ret = SaveHash( hash );
	if ( ret <= 0 )	return 0;
	// 比例定数の計算
	fprintf( stderr, "Calculating Prop...\n" );
//	eProp = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss, reg_areass, reg_nums, &disc, hash, hash2, hash3 );
	eProp = 0.0L;
	// 設定ファイルの保存
	fprintf( stderr, "Saving Config...\n" );
	ret = SaveConfig();
	if ( ret <= 0 )	return 0;

	return 1;
}

strLlahDocDb *LlahDocLoadDb( const char *db_dir )
// ハッシュのロード
{
	int ret;
	// argv
	int argc = 0;
	char *argv[12];
	// Load
	char fname[kMaxPathLen], **dbcors = NULL;
	strDisc disc;
	int *numall = NULL;
	CvPoint **psall = NULL;
	double **areaall = NULL;
	CvSize *sizeall = NULL;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;
	// db
	strLlahDocDb *db = NULL;

		// パラメータからargvの作成
	// ダミー
	argv[argc] = (char *)calloc( strlen( "llahdoc.dll" ) + 1, sizeof( char ) );
	strcpy( argv[argc], "llahdoc.dll" );
	argc++;
	// ハッシュのディレクトリ
	if ( db_dir != NULL && db_dir[0] != '\0' ) {	// hash_dirが指定されている
		argv[argc] = (char *)calloc( 3, sizeof(char) );
		strcpy( argv[argc], "-h" );
		argc++;
		argv[argc] = (char *)calloc( strlen( db_dir ) + 1, sizeof(char) );
		strcpy( argv[argc], db_dir );
		argc++;
	}
	// argvを使って設定
	AnalyzeArgAndSetExtern2( argc, argv );

	db = (strLlahDocDb *)malloc( sizeof(strLlahDocDb) );	// Dbの確保
	ret = LoadConfig();	// 設定ファイルの読み込み
	if ( ret <= 0 )	return NULL;
	sprintf( fname, "%s%s", eDirsDir, eDiscFileName );
	ret = LoadDisc( fname, &disc );	// 離散化ファイルの読み込み
	if ( ret <= 0 )	return NULL;
	// ハッシュの読み込み
	if ( eNoHashList )			hash3 = LoadHash3( disc.num );
	else if ( eCompressHash )	hash2 = LoadHash2( disc.num );
	else						hash = LoadHash( disc.num );
	if ( ret <= 0 )	return NULL;
	sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
	ret = LoadPointFile2( fname, &psall, &areaall, &sizeall, &numall, &dbcors );	// 特徴点データの読み込み
	if ( ret <= 0 )	return NULL;
	// Dbへの代入
	db->disc = disc;
	db->hash = hash;
	db->hash2 = hash2;
	db->hash3 = hash3;
	db->reg_pss = psall;
	db->reg_areass = areaall;
	db->reg_sizes = sizeall;
	db->reg_nums = numall;
	db->dbcors = dbcors;
	// 組み合わせの作成
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );

	return db;
}

void LlahDocReleaseDb( strLlahDocDb *db )
// データベースの解放
{
	int i;

	if ( db == NULL )	return;
	ReleaseHash( db->hash );
	ReleaseHash2( db->hash2 );
	ReleaseHash3( db->hash3 );
	if ( db->reg_nums != NULL )	free( db->reg_nums );
	if ( db->reg_sizes != NULL )	free( db->reg_sizes );
	if ( db->reg_pss != NULL )
		for ( i = 0; i < eDbDocs; i++ )	free( db->reg_pss[i] );
	free( db->reg_pss );
	if ( db->reg_areass != NULL )
		for ( i = 0; i < eDbDocs; i++ )	free( db->reg_areass[i] );
	free( db->reg_areass );
	if ( db->dbcors != NULL )
		for ( i = 0; i < eDbDocs; i++ )	free( db->dbcors[i] );
	free( db->dbcors );
	if ( db->disc.dat != NULL )	free( db->disc.dat );

	free( db );
}

int LlahDocRetrieveImage( const char *img_fname, strLlahDocDb *db, char *result, int result_len )
// 静止画像検索
{
	int i, num, **nears = 0, *score, res, ret_time, res_score;
	CvPoint *ps = 0;
	CvSize img_size;
	IplImage *img, *img_cap;
	double *areas = NULL;
	// db
	char **dbcors = NULL;
	strDisc disc;
	int *reg_nums = NULL;
	CvPoint **reg_pss = NULL;
	double **reg_areass = NULL;
	CvSize *reg_sizes = NULL;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

//	fprintf( stderr, "Retrieval from Static Image\n" );
	eEntireMode = RETRIEVE_MODE;
	eExperimentMode = 1;
	// dbをローカル変数に代入
	disc = db->disc;
	hash = db->hash;
	hash2 = db->hash2;
	hash3 = db->hash3;
	reg_pss = db->reg_pss;
	reg_areass = db->reg_areass;
	reg_sizes = db->reg_sizes;
	reg_nums = db->reg_nums;
	dbcors = db->dbcors;

	// 結合画像を作成
#ifdef	WIN32	// OpenCVのバージョンが異なるため
//	img = GetConnectedImage2( img_fname, eEntireMode );
	img_cap = cvLoadImage( img_fname, 1 );
	if ( img_cap == NULL )	return 0;
	img = GetConnectedImageCam( img_cap );
#else
	if ( eIsJp ) 	img = GetConnectedImageJp2( query, eEntireMode );
	else			img = GetConnectedImage2( query, eEntireMode );
#endif
	if ( img == NULL )	return 0;
	// 特徴点を抽出
	num = MakeCentresFromImage( &ps, img, &img_size, &areas );
	cvReleaseImage( &img );
	// 近傍構造を計算
	MakeNearsFromCentres( ps, num, &nears );
	// 検索
	score = (int *)calloc( kMaxDocNum, sizeof(int) );
	res = RetrieveNN52( ps, areas, num, nears, img_size, score, &disc, reg_nums, &ret_time, hash, hash2, hash3 );
//	OutPutResult( score, 0, 5, argv[argi] );	// そのままでは使えない
	if ( !eExperimentMode )	printf( "%s : %d\n", dbcors[res], score[res] );
	// 結果の格納
	strncpy( result, dbcors[res], result_len );
	res_score = score[res];
	free( score );

	return res_score;
}

int LlahDocRetrieveIplImage( IplImage *img_cap, strLlahDocDb *db, char *result, int result_len )
// IplImage検索
// リアルタイム検索用
{
	int i, num, **nears = 0, *score, res, ret_time, res_score;
	CvPoint *ps = 0;
	CvSize img_size;
	IplImage *img;
	double *areas = NULL;
	// db
	char **dbcors = NULL;
	strDisc disc;
	int *reg_nums = NULL;
	CvPoint **reg_pss = NULL;
	double **reg_areass = NULL;
	CvSize *reg_sizes = NULL;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;
	// Webカメラ用
	strProjParam param, zero_param;
	CvSize res_size;
	int pcor[kMaxPointNum][2], pcornum = 0;

//	fprintf( stderr, "Retrieval from IplImage\n" );
	// dbをローカル変数に代入
	disc = db->disc;
	hash = db->hash;
	hash2 = db->hash2;
	hash3 = db->hash3;
	reg_pss = db->reg_pss;
	reg_areass = db->reg_areass;
	reg_sizes = db->reg_sizes;
	reg_nums = db->reg_nums;
	dbcors = db->dbcors;
	// Webカメラ用
	score = (int *)calloc( kMaxDocNum, sizeof(int) );
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	param = zero_param;
	res_size = reg_sizes[0];

#ifdef	WIN32	// OpenCVのバージョンが異なるため
//	img = GetConnectedImage( query, eEntireMode );
	// Webカメラモードでの特徴点抽出
	img = GetConnectedImageCam( img_cap );
#else
	fprintf( stderr, "This program currently support only Windows.\n" );
	return 0;
#endif
	if ( img == NULL ) {
		result[0] = '\0';
		param = zero_param;	// 検索結果がないのにパラメータだけあるってのも変なので
		return 0;
	}
	// 特徴点を抽出
	num = MakeCentresFromImage( &ps, img, &img_size, &areas );
	cvReleaseImage( &img );
	// 近傍構造を計算
//	MakeNearsFromCentres( ps, num, &nears );
	if ( num >= kMinPoints ) {	// 特徴点数が最小値以上
		MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num, &nears );	// 近傍点計算（分割版）
		// 検索
		res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, reg_nums, hash, hash2, hash3 );	// 検索のみ
		// 最小対応点数以上なら、射影変換パラメータを計算
		if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, reg_pss[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// 登録画像に上書き
		else	param = zero_param;
		if ( ps != NULL && num > 0 )	ReleaseCentres( ps );	// psの解放
		if ( areas != NULL && num > 0 )	free( areas );
		ReleaseNears( nears, num );	// nearsの解放
		// 計算時間の表示
//			OutPutResultSv( score, end-start, 0/*5*/, doc_name );
		if ( score[res] > kTopThr ) {	// トップの得票数が閾値以下（リジェクト）
//			GetBasename2( dbcors[res], kMaxPathLen, doc_name );
			strncpy( result, dbcors[res], result_len );
		}
		else {
			result[0] = '\0';
			param = zero_param;	// 検索結果がないのにパラメータだけあるってのも変なので
		}
//		puts( doc_name );
	}
	else {	// 特徴点数が最小値を満たさない
//		puts("min");
		res = 0;
		result[0] = '\0';
		param = zero_param;
	}
	if ( result[0] != '\0' )	res_size = reg_sizes[res];	// 検索成功ならそのサイズ
	else	res_size = reg_sizes[0];	// そうでなければデータベースの0番のサイズ

//	fprintf(stdout, "%s\n", doc_name);
//	if ( eProtocol == kTCP )	ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
//	else						ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
//	if ( ret < 0 )	break;
	// 終了処理
	res_score = score[res];
	free(score);
	// 結果の格納
//	strncpy( result, dbcors[res], result_len );

	return res_score;
}
