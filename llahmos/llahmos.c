#define	GLOBAL_DEFINE	/* extern変数にここで実体を与える */

#include "def_general.h"

#include <stdio.h>
#include <math.h>

#ifdef	WIN_TIME
#include <windows.h>
#include <mmsystem.h>
#endif

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
#include "cr.h"
#include "prop.h"
#include "extern.h"
#include "nears.h"
#include "daytime.h"
#include "inacr2.h"
#include "nstr.h"
#include "llahmos.h"
#include "nimg.h"

int main( int argc, char *argv[] )
{
	int i, argi, ret, num, *reg_nums, **nears = 0, *score, ret_time, res;
	int pcor[kMaxPointNum][2], pcornum = 0;
	CvPoint *ps = 0, **reg_pss;
	CvSize img_size, *reg_sizes;
	double *areas = NULL, **reg_areass;
	char **dbcors;
	strProjParam param, param_rev;
	IplImage *img, *img1, *img2, *mos;
	strDisc disc;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;
	int start, end;

	if ( ( argi = AnalyzeArgAndSetExtern2( argc, argv ) ) < 0 )	return 1;
	eDbDocs = 1;
	// メモリの確保
	reg_pss = (CvPoint **)calloc( eDbDocs, sizeof(CvPoint *) );
	reg_areass = (double **)calloc( eDbDocs, sizeof(double *) );
	reg_sizes = (CvSize *)calloc( eDbDocs, sizeof(CvSize) );
	reg_nums = (int *)calloc( eDbDocs, sizeof(int) );
	dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
	dbcors[0] = (char *)calloc( kMaxPathLen, sizeof(char) );

	// 画像1から特徴点を抽出
	img1 = cvLoadImage( argv[argi], 1 );
	fprintf( stderr, "%s\n", argv[argi] );
	strcpy( dbcors[0], argv[argi] );	// 元画像ファイル名を保存
	start = GetProcTimeMiliSec();
	if ( eIsJp )	img = GetConnectedImageJp2( argv[argi], RETRIEVE_MODE );	// 結像画像を作成
	else			img = GetConnectedImage2( argv[argi], RETRIEVE_MODE );	// 結像画像を作成
	reg_nums[0] = MakeCentresFromImage( &(reg_pss[0]), img, &(reg_sizes[0]), &(reg_areass[0]) );	// 特徴点を抽出
	end = GetProcTimeMiliSec();
	printf("Feature point extraction 1: %d ms\n", end - start);
	cvReleaseImage( &img );	// 画像を解放
	// 離散化ファイルの作成
	fprintf( stderr, "Making %s...\n", eDiscFileName );
	ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss, reg_nums, &disc );
	if ( ret <= 0 )	return 1;
	// ハッシュの構築
	start = GetProcTimeMiliSec();
	fprintf( stderr, "Constructing Hash Table...\n" );
	ret = ConstructHash2( eDbDocs, reg_pss, reg_areass, reg_nums, &disc, &hash, &hash2, &hash3 );
	end = GetProcTimeMiliSec();
	printf("Storage: %d ms\n", end - start);
	// 比例定数の計算
	eProp = 0.0L;

	// 画像2から特徴点を抽出
	argi++;
	img2 = cvLoadImage( argv[argi], 1 );
	start = GetProcTimeMiliSec();
	if ( eIsJp )	img = GetConnectedImageJp2( argv[argi], RETRIEVE_MODE );	// 結像画像を作成
	else			img = GetConnectedImage2( argv[argi], RETRIEVE_MODE );	// 結像画像を作成
	num = MakeCentresFromImage( &ps, img, &img_size, &areas );	// 特徴点を抽出
	end = GetProcTimeMiliSec();
	printf("Feature point extraction 2: %d ms\n", end - start);
	cvReleaseImage( &img );	// 画像を解放
	// 近傍構造を計算
	start = GetProcTimeMiliSec();
	MakeNearsFromCentres( ps, num, &nears );
	// 検索
	score = (int *)calloc( kMaxDocNum, sizeof(int) );
	eProp = 0.0L;
//	res = RetrieveNN52( ps, areas, num, nears, img_size, score, &disc, reg_nums, &ret_time, hash, hash2, hash3 );
	res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, reg_nums, hash, hash2, hash3 );	// 検索のみ
	end = GetProcTimeMiliSec();
	printf("Num of points 1: %d\nNum of points 2: %d\nNum of cor points: %d\n", num, reg_nums[0], pcornum );
	printf("Retrieval: %d ms\n", end - start);

	DrawCor( ps, num, img_size, res, reg_pss[res], reg_nums[res], reg_sizes[res], pcor, pcornum );
	// 最小対応点数以上なら、射影変換パラメータを計算
	start = GetProcTimeMiliSec();
	CalcProjParamTop( ps, reg_pss[res], pcor, pcornum, &param, PROJ_NORMAL, PARAM_RANSAC );
	CalcProjParamTop( ps, reg_pss[res], pcor, pcornum, &param_rev, PROJ_REVERSE, PARAM_RANSAC );
	end = GetProcTimeMiliSec();
	printf("Estimate parameter: %d ms\n", end - start);

	printf("%d(%d)\n", res, score[res]);
	// 画像1と画像2を合成する
	fprintf( stderr, "Creating Mosaicing Image...\n" );
	start = GetProcTimeMiliSec();
	mos = MosaicImages( img1, img2, param, param_rev );
	end = GetProcTimeMiliSec();
	printf("Create mosaic image: %d\n", end - start);
	OutPutImage( mos );
	return 0;
}

IplImage *MosaicImages( IplImage *img1, IplImage *img2, strProjParam param, strProjParam param_rev )
// 2枚の画像を射影変換パラメータに基づいて合成する
{
	int i, j, k, x_min, y_min, x_max, y_max, x_exp, y_exp;
	int flag1, flag2;
	unsigned char val1[3], val2[3], val3[3];
	IplImage *mos;
	CvSize size_mos, size1, size2;
	strPoint img1_pt, img2_pt, src[4], dst[4];

	size1.width = img1->width;
	size1.height = img1->height;
	size2.width = img2->width;
	size2.height = img2->height;

//	mos = cvCreateImage( size1, IPL_DEPTH_8U, 3 );

/*	for ( j = 0; j < mos->height; j++ ) {
		for ( i = 0; i < mos->width; i++ ) {
			GetPix( img1, i, j, val );
			SetPix( mos, i, j, val );
		}
	}*/
	src[0].x = 0;
	src[0].y = 0;
	src[1].x = img2->width;
	src[1].y = 0;
	src[2].x = img2->width;
	src[2].y = img2->height;
	src[3].x = 0;
	src[3].y = img2->height;
	for ( i = 0; i < 4; i++ ) {
		ProjTrans( src + i, dst + i, &param_rev );
	}
	x_min = 0;
	y_min = 0;
	x_max = img1->width;
	y_max = img1->height;
	for ( i = 0; i < 4; i++ ) {
		if ( dst[i].x < x_min )	x_min = dst[i].x;
		if ( dst[i].y < y_min ) y_min = dst[i].y;
		if ( dst[i].x > x_max ) x_max = dst[i].x;
		if ( dst[i].y > y_max ) y_max = dst[i].y;
	}
	x_exp = 0;
	y_exp = 0;
	if ( x_min < 0 )	x_exp = abs( x_min );
	if ( y_min < 0 )	y_exp = abs( y_min );
	size_mos.width = x_exp + x_max;
	size_mos.height = y_exp + y_max;
	mos = cvCreateImage( size_mos, IPL_DEPTH_8U, 3 );
	cvZero( mos );
	for ( j = 0; j < size_mos.height; j++ ) {
		for ( i = 0; i < size_mos.width; i++ ) {
			img1_pt.x = i - x_exp;
			img1_pt.y = j - y_exp;
			ProjTrans( &img1_pt, &img2_pt, &param );
			flag1 = GetPixU( img1, i - x_exp, j - y_exp, val1 );
			flag2 = GetPixU( img2, img2_pt.x, img2_pt.y, val2 );
			for ( k = 0; k < 3; k++ )	val3[k] = val1[k] + val2[k] / 2;
			if ( flag1 && flag2 )	SetPixU( mos, i, j, val3 );
			else if ( flag1 )		SetPixU( mos, i, j, val1 );
			else if ( flag2 )		SetPixU( mos, i, j, val2 );
		}
	}
//	for ( i = 0; i < 4; i++ )	cvLine( mos, cvPoint( x_exp + dst[i].x, y_exp + dst[i].y ), cvPoint( x_exp + dst[(i+1)%4].x, y_exp + dst[(i+1)%4].y ), cRed, 4, CV_AA, 0 );

	return mos;
}
