#include "def_general.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cv.h>
#include <highgui.h>
#include "dirs.h"
#include "proj4p.h"
#include "projrecov.h"

void GetNumRand( int num, int max, int *arr )
// 0～max-1の整数を重複なしでnum個生成し，arrに格納する
{
/*	int i, j, r;

	for ( i = 0; i < num; i++ ) {
gen:
		r = rand() % max;
		for ( j = 0; j < i; j++ ) {
			if ( r == arr[j] )	goto gen;
		}
		arr[i] = r;
	}
*/
	int i, j, r, *num_arr;

	num_arr = (int *)malloc( sizeof(int) * max );
	for ( i = 0; i < max; i++ )	num_arr[i] = i;
	for ( i = 0; i < num; i++ ) {
		r = rand() % (max - i);
		arr[i] = num_arr[r];
		for ( j = r; j < max - i - 1; j++ )	num_arr[j] = num_arr[j+1];
	}
	free(num_arr);
}

int CompareDouble( const double *a, const double *b )
// doubleの値を比較する
{
	if ( *a < *b )	return -1;
	else if ( *a > *b )	return 1;
	else	return 0;
}

double GetMedian( double *arr, int num )
// サイズnumの配列arr（非ソート）からメディアン値を得る
{
	int i;
	for ( i = 0; i < num; i++ )
		printf("%lf\n", arr[i]);
	printf("\n");
	qsort( arr, num, sizeof(double), (int (*)(const void*, const void*))CompareDouble );
	for ( i = 0; i < num; i++ )
		printf("%lf\n", arr[i]);
	printf("\n");
	return arr[(int)(num/2)];

}

int IsNearParam( strProjParam *param1, strProjParam *param2 )
// 近いパラメータか
{
	double diff = 0.0L;

	diff += (param1->a1 - param2->a1) * (param1->a1 - param2->a1);
	diff += (param1->a2 - param2->a2) * (param1->a2 - param2->a2);
	diff += (param1->a3 - param2->a3) * (param1->a3 - param2->a3);
	diff += (param1->b1 - param2->b1) * (param1->b1 - param2->b1);
	diff += (param1->b2 - param2->b2) * (param1->b2 - param2->b2);
	diff += (param1->b3 - param2->b3) * (param1->b3 - param2->b3);
	diff += (param1->c1 - param2->c1) * (param1->c1 - param2->c1);
	diff += (param1->c2 - param2->c2) * (param1->c2 - param2->c2);

	if ( diff < kNearParamThr )	return 1;
	else	return 0;
}

void CalcProjParamTop( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param, int mode, int method )
// ps->corpsへの射影変換パラメータを求める(mode==1なら逆）
{
	int i, j, randpi[4];
	strPoint strp1[4], strp2[4];
	strProjParam paramarr[kTryParamRand];

	if ( method == PARAM_IWAMURA ) {	// 岩村メソッドの場合
		srand(clock());

		for ( j = 0; j < kTryParamRand; j++ ) {
			// ランダムに4点非復元抽出する
			GetNumRand( 4, cor_num, randpi );
			for ( i = 0; i < 4; i++ ) {
				strp1[i].x = corps[cor[randpi[i]][1]].x;
				strp1[i].y = corps[cor[randpi[i]][1]].y;
				strp2[i].x = ps[cor[randpi[i]][0]].x;
				strp2[i].y = ps[cor[randpi[i]][0]].y;
			}
			if ( mode == PROJ_NORMAL )
				CalcProjParam( strp1, strp2, &(paramarr[j]) );
			else
				CalcProjParam( strp2, strp1, &(paramarr[j]) );
		}
		// 適切なパラメータを計算する
		GetAppropriateParam( paramarr, param );
	}
	else if ( method == PARAM_RANSAC ) {	// RANSACの場合
		GetAppropriateParamRANSAC( ps, corps, cor, cor_num, param, mode );
	}
}

int GetAppropriateParamRANSAC( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param, int mode )
// RANSACによって射影変換パラメータを求める
{
	int i, j, randpi[4], score, max_score = -1, max_param = -1;
	strPoint strp1[4], strp2[4];
	strProjParam paramarr[kRANSACTry], zero_param;
	
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	for ( j = 0; j < kRANSACTry; j++ ) {	// kRANSACTry回繰り返す
		GetNumRand( 4, cor_num, randpi );	// ランダムに4点非復元抽出する
		for ( i = 0; i < 4; i++ ) {
			strp1[i].x = corps[cor[randpi[i]][1]].x;
			strp1[i].y = corps[cor[randpi[i]][1]].y;
			strp2[i].x = ps[cor[randpi[i]][0]].x;
			strp2[i].y = ps[cor[randpi[i]][0]].y;
		}
		// 4点を用いてパラメータを求める
		if ( mode == PROJ_NORMAL )
			CalcProjParam( strp1, strp2, &(paramarr[j]) );
		else
			CalcProjParam( strp2, strp1, &(paramarr[j]) );
		score = EvaluateParamRANSAC( ps, corps, cor, cor_num, &(paramarr[j]), mode );
//		printf("%d\n", score );
		if ( score > max_score ) {
			max_score = score;
			max_param = j;
		}
	}
	if ( max_score < kRANSACMinScore ) {	// 最大スコアが閾値以下
		*param = zero_param;
	}
	else {
		// 得られたパラメータをサポートする対応点からパラメータを求めなおす
		// かえって精度が低くなる．下手に平均したりしない方がいいのか
//		GetRefinedParamRANSAC( ps, corps, cor, cor_num, &(paramarr[max_param]), param, mode );
		// 最大のスコアを得たパラメータを返す
		*param = paramarr[max_param];
	}
//	*param = paramarr[max_param];
	return 1;
}

int EvaluateParamRANSAC( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param, int mode )
// RANSACでのパラメータの評価
{
	int i, score = 0;
	double dist;
	strPoint p1, p2, p2h;
	
	for ( i = 0; i < cor_num; i++ ) {
		if ( mode == PROJ_NORMAL ) {
			p1.x = corps[cor[i][1]].x;
			p1.y = corps[cor[i][1]].y;
			p2.x = ps[cor[i][0]].x;
			p2.y = ps[cor[i][0]].y;
		}
		else {
			p1.x = ps[cor[i][0]].x;
			p1.y = ps[cor[i][0]].y;
			p2.x = corps[cor[i][1]].x;
			p2.y = corps[cor[i][1]].y;
		}
		// パラメータから求められる対応点の座標との距離（の２乗）を求める
		ProjTrans( &p1, &p2h, param );
		dist = (p2.x-p2h.x)*(p2.x-p2h.x)+(p2.y-p2h.y)*(p2.y-p2h.y);
//		printf("%f\n", dist);
		if ( dist < kRANSACThr )	score++;	// 距離が閾値以下ならインクリメント
	}
	return score;
}

void GetRefinedParamRANSAC( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, strProjParam *param_src, strProjParam *param_dst, int mode )
// より精度の高いパラメータを求める
{
	int i, j, randpi[4], *support, sup_num = 0;
	double dist;
	strPoint p1, p2, p2h, strp1[4], strp2[4];
	strProjParam param_cur, zero_param;
	
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	support = (int *)calloc( cor_num, sizeof(int) );	// paramをサポートする対応点の初期化
	
	for ( i = 0; i < cor_num; i++ ) {	// paramをサポートする対応点を探す
		if ( mode == PROJ_NORMAL ) {
			p1.x = corps[cor[i][1]].x;
			p1.y = corps[cor[i][1]].y;
			p2.x = ps[cor[i][0]].x;
			p2.y = ps[cor[i][0]].y;
		}
		else {
			p1.x = ps[cor[i][0]].x;
			p1.y = ps[cor[i][0]].y;
			p2.x = corps[cor[i][1]].x;
			p2.y = corps[cor[i][1]].y;
		}
		// パラメータから求められる対応点の座標との距離（の２乗）を求める
		ProjTrans( &p1, &p2h, param_src );
		dist = (p2.x-p2h.x)*(p2.x-p2h.x)+(p2.y-p2h.y)*(p2.y-p2h.y);
		if ( dist < kRANSACThr ) {	// 距離が閾値以下ならサポートするものに追加
			support[sup_num++] = i;
		}
	}
	*param_dst = zero_param;	// すべてゼロで初期化
	for ( j = 0; j < kRANSACRefineNum; j++ ) {	// kRANSACRefineNum回繰り返す
		GetNumRand( 4, sup_num, randpi );	// ランダムに4点非復元抽出する
		for ( i = 0; i < 4; i++ ) {
			strp1[i].x = corps[cor[support[randpi[i]]][1]].x;
			strp1[i].y = corps[cor[support[randpi[i]]][1]].y;
			strp2[i].x = ps[cor[support[randpi[i]]][0]].x;
			strp2[i].y = ps[cor[support[randpi[i]]][0]].y;
		}
		// 4点を用いてパラメータを求める
		if ( mode == PROJ_NORMAL )
			CalcProjParam( strp1, strp2, &param_cur );
		else
			CalcProjParam( strp2, strp1, &param_cur );
		param_dst->a1 += param_cur.a1;
		param_dst->a2 += param_cur.a2;
		param_dst->a3 += param_cur.a3;
		param_dst->b1 += param_cur.b1;
		param_dst->b2 += param_cur.b2;
		param_dst->b3 += param_cur.b3;
		param_dst->c1 += param_cur.c1;
		param_dst->c2 += param_cur.c2;
	}
	param_dst->a1 /= (double)kRANSACRefineNum;
	param_dst->a2 /= (double)kRANSACRefineNum;
	param_dst->a3 /= (double)kRANSACRefineNum;
	param_dst->b1 /= (double)kRANSACRefineNum;
	param_dst->b2 /= (double)kRANSACRefineNum;
	param_dst->b3 /= (double)kRANSACRefineNum;
	param_dst->c1 /= (double)kRANSACRefineNum;
	param_dst->c2 /= (double)kRANSACRefineNum;

	free( support );
}

void RecovPTSub( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num, CvSize size, IplImage *src )
// 射影変換補正サブ
{
	int i, j, randpi[4];
//	double a1s[kTryParamRand], a2s[kTryParamRand], a3s[kTryParamRand], b1s[kTryParamRand], b2s[kTryParamRand], b3s[kTryParamRand], c1s[kTryParamRand], c2s[kTryParamRand];
	strPoint strp1[4], strp2[4], pp1, pp2;
	strProjParam param, paramarr[kTryParamRand];
	IplImage *img;

//	src = cvLoadImage( img_fname, 0 );
#if 0
	img = cvLoadImage( img_fname, 0 );
	src = cvCreateImage( cvSize(img->width*9, img->height*9), 8, 1 );
	cvResize( img, src, CV_INTER_CUBIC );
	cvReleaseImage( &img );
#endif
	srand(clock());

	for ( j = 0; j < kTryParamRand; j++ ) {
		// ランダムに4点非復元抽出する
		GetNumRand( 4, cor_num, randpi );

		for ( i = 0; i < 4; i++ ) {
			strp1[i].x = corps[cor[randpi[i]][1]].x;
			strp1[i].y = corps[cor[randpi[i]][1]].y;
			strp2[i].x = ps[cor[randpi[i]][0]].x;
			strp2[i].y = ps[cor[randpi[i]][0]].y;
		}
		CalcProjParam( strp1, strp2, &(paramarr[j]) );
	}

	// 適切なパラメータを計算する
	GetAppropriateParam( paramarr, &param );

	img = cvCreateImage( size, 8, 1 );
	for ( j = 0; j < size.height; j++ ) {
		for ( i = 0; i < size.width; i++ ) {
			pp1.x = i;
			pp1.y = j;
			ProjTrans( &pp1, &pp2, &param );
			if ( pp2.x >= src->width || pp2.y >= src->height || pp2.x < 0 || pp2.y < 0 )
				img->imageData[j*img->widthStep+i] = 0;
			else
				img->imageData[j*img->widthStep+i] = src->imageData[pp2.y*src->widthStep+pp2.x]; // imageDataの幅はwidthStepで定義されていることに注意
		}
	}
	OutPutImage(img);
}

void GetAppropriateParam( strProjParam *paramarr, strProjParam *param )
// パラメータ構造体配列paramarr[]から適切なパラメータparamを得る
{
#if 0
	int i, j, paramvote[kTryParamRand], maxvote = 0, maxparam;

	for ( i = 0; i < kTryParamRand; i++ ) {
		paramvote[i] = 0;
		for ( j = 0; j < kTryParamRand; j++ ) {
			if ( IsNearParam( &paramarr[i], &paramarr[j] ) )	paramvote[i]++;
		}
	}
	for ( i = 0; i < kTryParamRand; i++ ) {
//		printf("%d\n", paramvote[i]);
		if ( paramvote[i] > maxvote ) {
			maxparam = i;
			maxvote = paramvote[i];
		}
	}
	param->a1 = paramarr[maxparam].a1;
	param->a2 = paramarr[maxparam].a2;
	param->a3 = paramarr[maxparam].a3;
	param->b1 = paramarr[maxparam].b1;
	param->b2 = paramarr[maxparam].b2;
	param->b3 = paramarr[maxparam].b3;
	param->c1 = paramarr[maxparam].c1;
	param->c2 = paramarr[maxparam].c2;
#endif
#if 1
	int i;
	double pdouble[kTryParamRand][8], mean[8];

	for ( i = 0; i < kTryParamRand; i++ ) {
		pdouble[i][0] = paramarr[i].a1;
		pdouble[i][1] = paramarr[i].a2;
		pdouble[i][2] = paramarr[i].a3;
		pdouble[i][3] = paramarr[i].b1;
		pdouble[i][4] = paramarr[i].b2;
		pdouble[i][5] = paramarr[i].b3;
		pdouble[i][6] = paramarr[i].c1;
		pdouble[i][7] = paramarr[i].c2;
	}
	MeanParam( pdouble, mean, kTryParamRand, kErrorThrMeanParam, kNumVectorMeanParam );	// 個数固定
	param->a1 = mean[0];
	param->a2 = mean[1];
	param->a3 = mean[2];
	param->b1 = mean[3];
	param->b2 = mean[4];
	param->b3 = mean[5];
	param->c1 = mean[6];
	param->c2 = mean[7];
#endif
}

void RPTMain( char *fname1, char *fname2 )
// テストのメイン
{
	IplImage *img1, *img2;
	CvPoint p1[4], p2[4];

	if ( strcmp(fname1, "0104.bmp") || strcmp(fname2, "IMG_0214.bmp") ) {
		return;
	}
	img1 = cvLoadImage( fname1, 0 );
	img2 = cvLoadImage( fname2, 0 );

/*	p1[0].x = 748;
	p1[0].y = 1540;
	p1[1].x = 1990;
	p1[1].y = 2325;
	p1[2].x = 2466;
	p1[2].y = 1981;
	p1[3].x = 2405;
	p1[3].y = 3253;
	p2[0].x = 1200;
	p2[0].y = 1090;
	p2[1].x = 1633;
	p2[1].y = 693;
	p2[2].x = 1468;
	p2[2].y = 560;
	p2[3].x = 2084;
	p2[3].y = 580;
*/

	p1[0].x = 718;
	p1[0].y = 658;
	p1[1].x = 2508;
	p1[1].y = 860;
	p1[2].x = 710;
	p1[2].y = 3347;
	p1[3].x = 2513;
	p1[3].y = 3457;
	p2[0].x = 715;
	p2[0].y = 1097;
	p2[1].x = 933;
	p2[1].y = 549;
	p2[2].x = 2194;
	p2[2].y = 1105;
	p2[3].x = 2177;
	p2[3].y = 550;

	RecoverPTTest( img1, img2, p1, p2 );
}

void RecoverPTTest( IplImage *img1, IplImage *img2, CvPoint *p1, CvPoint *p2 )
// 射影変換補正のテスト
{
	int i, j;
	strPoint strp1[4], strp2[4], pp1, pp2;
	strProjParam param;
	CvSize size1;
	IplImage *recov;

	for ( i = 0; i < 4; i++ ) {
		strp1[i].x = p1[i].x;
		strp1[i].y = p1[i].y;
		strp2[i].x = p2[i].x;
		strp2[i].y = p2[i].y;
	}

	CalcProjParam( strp1, strp2, &param );
	size1.width = img1->width;
	size1.height = img1->height;
	recov = cvCreateImage( size1, 8, 1 );
	for ( j = 0; j < size1.height; j++ ) {
		for ( i = 0; i < size1.width; i++ ) {
			pp1.x = i;
			pp1.y = j;
			ProjTrans( &pp1, &pp2, &param );
			recov->imageData[j*img1->widthStep+i] = img2->imageData[pp2.y*img2->widthStep+pp2.x]; // imageDataの幅はwidthStepで定義されていることに注意
		}
	}
	OutPutImage( recov );
}

// err0: １つ前の誤差と新たに計算した誤差がerr0未満なら終了
// threshold: 点を距離が近い順に並べたとき，threshold個までは次回の平均値の計算に使う(inside)
void MeanParam( double data[kTryParamRand][8], double *mean, int n, double err0, int threshold) {
	// variable for "for"
	int i,j;
	// counter
	int counter=0;
	// variance
	double *var;
	// distance
	double *dist;
	// distance #
//	int *dist_num;
	int *dist_n;
	// flag if the point is inside(1) or outside(0) of the circle
//	int inside_flag = 1;
	int *inside_flag;
	// size of data with inside_flag=1
	int size_flag1;
	// error
	double err = 0;
	// error (the last one) 
	double err_prev = 0;
	// sum of 1/var
	double sum_var;

	// malloc of variance
	var = (double *)malloc(sizeof(double)*8);
	// malloc of distance
	dist = (double *)malloc(sizeof(double)*n);
	// malloc of distance #
	dist_n = (int *)malloc(sizeof(int)*n);
	// malloc of inside_flag
	inside_flag = (int *)malloc(sizeof(int)*n);
	// initialization of inside_flag to be 1
	for (i=0; i<n; i++) {
		inside_flag[i] = 1;
	}
	// initialization of size_flag1
	size_flag1 = n;

	// loop (counterが0か1のときはerr0による評価はしない．つまり，２回は必ずループする)
	  while ( fabs(err-err_prev) > err0 || counter<=1) {
	
		// calculate mean
		for (j=0; j<8; j++) {
			mean[j] = 0;
			for (i=0; i<n; i++) {
				if (inside_flag[i]==1) {
					mean[j] += data[i][j];
				}
			}
			mean[j] /= (double)size_flag1;
		}
		// calculate variance
		for (j=0; j<8; j++) {
			var[j] = 0;
			for (i=0; i<n; i++) {
				if (inside_flag[i]==1) {
					var[j] += (data[i][j] - mean[j]) * (data[i][j] - mean[j]);
				}
			}
			var[j] /= (double)size_flag1;
		}

		// initialize params
		size_flag1 = 0;
		err_prev = err;
		err = 0;
		// sum of 1/var
		sum_var = 0;
		for (j=0; j<8; j++) {
			sum_var += 1/var[j];
		}
		// calculate distance between mean and data
		for (i=0; i<n; i++) {
			dist[i] = 0;
			for (j=0; j<8; j++) {
				dist[i] += (data[i][j]-mean[j]) * (data[i][j]-mean[j]) / var[j];
			}
			dist[i] /= sum_var;
			dist[i] = sqrt(dist[i]);
			// err is the sum of distance between mean and all data
			err += dist[i];
		}
		// initialization of pdist
		for (i=0; i<n; i++) {
			dist_n[i] = i;
		}
		// dist[dist_n[i]]が小さい順に並ぶようにdist_nをソート
//		quicksort(dist,dist_n,pdist,1,n);
//		quicksort(dist,dist_n,0,n-1);
		simplesort(dist,dist_n,0,n-1);
		// 距離が小さいものをinsideに，そうでないものをoutsideにする
		for (i=0; i<threshold; i++) {
			inside_flag[dist_n[i]] = 1;
			size_flag1++;
		}
		for (i=threshold; i<n; i++) {
			inside_flag[dist_n[i]] = 0;
		}

		counter++;

	} // while
}

double GetPPVar( CvPoint *ps, CvPoint *corps, int cor[][2], int cor_num )
// 射影変換パラメータの分散を求める
{
	int i, j, randpi[4];
	static int first_flag = 0, seed;
//	double a1s[kTryParamRand], a2s[kTryParamRand], a3s[kTryParamRand], b1s[kTryParamRand], b2s[kTryParamRand], b3s[kTryParamRand], c1s[kTryParamRand], c2s[kTryParamRand];
	strPoint strp1[4], strp2[4]/*, pp1, pp2*/;
	strProjParam /*param, */paramarr[kTryParamRand];

	if ( first_flag == 0 ) {
		seed = clock();
		first_flag = 1;
	}
	srand(seed++);

	for ( j = 0; j < kTryParamRand; j++ ) {
		// ランダムに4点非復元抽出する
		GetNumRand( 4, cor_num, randpi );

		for ( i = 0; i < 4; i++ ) {
			strp1[i].x = corps[cor[randpi[i]][1]].x;
			strp1[i].y = corps[cor[randpi[i]][1]].y;
			strp2[i].x = ps[cor[randpi[i]][0]].x;
			strp2[i].y = ps[cor[randpi[i]][0]].y;
		}
		CalcProjParam( strp1, strp2, &(paramarr[j]) );
	}

	// パラメータの分散の総和を計算し，返す
	return GetPPVarSub( paramarr );
}

double GetPPVarSub( strProjParam *paramarr )
// パラメータ構造体配列paramarr[]から適切なパラメータparamを得る
{
	int i;
	double pdouble[kTryParamRand][8]/*, mean[8]*/;

	for ( i = 0; i < kTryParamRand; i++ ) {
/*		pdouble[i][0] = (paramarr[i].a1 > 10.0) ? 10.0 : paramarr[i].a1;
		pdouble[i][1] = (paramarr[i].a2 > 10.0) ? 10.0 : paramarr[i].a2;
		pdouble[i][2] = (paramarr[i].a3 > 10.0) ? 10.0 : paramarr[i].a3;
		pdouble[i][3] = (paramarr[i].b1 > 10.0) ? 10.0 : paramarr[i].b1;
		pdouble[i][4] = (paramarr[i].b2 > 10.0) ? 10.0 : paramarr[i].b2;
		pdouble[i][5] = (paramarr[i].b3 > 10.0) ? 10.0 : paramarr[i].b3;
		pdouble[i][6] = (paramarr[i].c1 > 10.0) ? 10.0 : paramarr[i].c1;
		pdouble[i][7] = (paramarr[i].c2 > 10.0) ? 10.0 : paramarr[i].c2;
*/		pdouble[i][0] = paramarr[i].a1;
		pdouble[i][1] = paramarr[i].a2;
		pdouble[i][2] = paramarr[i].a3;
		pdouble[i][3] = paramarr[i].b1;
		pdouble[i][4] = paramarr[i].b2;
		pdouble[i][5] = paramarr[i].b3;
		pdouble[i][6] = paramarr[i].c1;
		pdouble[i][7] = paramarr[i].c2;
//		for ( j = 0; j < 8; j++ ) {
//			if ( fabs(pdouble[i][j]) > 10.0L )	pdouble[i][j] = 10.0L;
//		}
	}
	return GetPPVarSubSub( pdouble, kTryParamRand, 0.00001, 10 );
}

double GetPPVarSubSub( double data[kTryParamRand][8], int n, double err0, int threshold) {
	// variable for "for"
	int i,j;
	// counter
	int counter=0;
	// variance
	double *var, *mean;
	// distance
	double *dist;
	// distance #
//	int *dist_num;
	int *dist_n;
	// flag if the point is inside(1) or outside(0) of the circle
//	int inside_flag = 1;
	int *inside_flag;
	// size of data with inside_flag=1
	int size_flag1;
	// error
	double err = 0;
	// error (the last one) 
	double err_prev = 0;
	// sum of 1/var
	double sum_var;

	// malloc of variance
	var = (double *)malloc(sizeof(double)*8);
	mean = (double *)malloc(sizeof(double)*8);
	// malloc of distance
	dist = (double *)malloc(sizeof(double)*n);
	// malloc of distance #
	dist_n = (int *)malloc(sizeof(int)*n);
	// malloc of inside_flag
	inside_flag = (int *)malloc(sizeof(int)*n);
	// initialization of inside_flag to be 1
	for (i=0; i<n; i++) {
		inside_flag[i] = 1;
	}
	// initialization of size_flag1
	size_flag1 = n;

	// loop (counterが0か1のときはerr0による評価はしない．つまり，２回は必ずループする)
	  while ( fabs(err-err_prev) > err0 || counter<=1) {
	
		// calculate mean
		for (j=0; j<8; j++) {
			mean[j] = 0;
			for (i=0; i<n; i++) {
				if (inside_flag[i]==1) {
					mean[j] += data[i][j];
				}
			}
			mean[j] /= (double)size_flag1;
		}
		// calculate variance
		for (j=0; j<8; j++) {
			var[j] = 0;
			for (i=0; i<n; i++) {
				if (inside_flag[i]==1) {
					var[j] += (data[i][j] - mean[j]) * (data[i][j] - mean[j]);
				}
			}
//			if ( var[j] < 0.0001L ) var[j] = 0.0001L;
			var[j] /= (double)size_flag1;
		}

		// initialize params
		size_flag1 = 0;
		err_prev = err;
		err = 0;
		// sum of 1/var
		sum_var = 0;
		for (j=0; j<8; j++) {
			sum_var += 1/var[j];
		}
		// calculate distance between mean and data
		for (i=0; i<n; i++) {
			dist[i] = 0;
			for (j=0; j<8; j++) {
				dist[i] += (data[i][j]-mean[j]) * (data[i][j]-mean[j]) / var[j];
			}
			dist[i] /= sum_var;
			dist[i] = sqrt(dist[i]);
			// err is the sum of distance between mean and all data
			err += dist[i];
		}
		// initialization of pdist
		for (i=0; i<n; i++) {
			dist_n[i] = i;
		}
		// dist[dist_n[i]]が小さい順に並ぶようにdist_nをソート
//		quicksort(dist,dist_n,pdist,1,n);
//		quicksort(dist,dist_n,0,n-1);
		simplesort(dist,dist_n,0,n-1);
		// 距離が小さいものをinsideに，そうでないものをoutsideにする
		for (i=0; i<threshold; i++) {
			inside_flag[dist_n[i]] = 1;
			size_flag1++;
		}
		for (i=threshold; i<n; i++) {
			inside_flag[dist_n[i]] = 0;
		}

		counter++;

	} // while
	return err;
}
