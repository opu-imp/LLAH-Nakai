/*****************************************************************
acr2: Create digitizing file "disc.txt" for cvtest by
analysing cross-ratios or affine invariants of document image.
*****************************************************************/

#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include "cvtest.h"
#include "hash.h"
#include "hist.h"
#include "disc.h"
#include "acr2.h"
#include "gencomb.h"
#include "cr.h"


int eNumCom1 = 8;	// nCm
int eNumCom2 = 7;	// mC(f or f-1)
int eGroup1Num;	// パラメータn
int eGroup2Num;	// パラメータm
int eGroup3Num;	// f or f-1
int eDiscNum;	// 離散化レベル
int	eInvType;	// 不変量のタイプ
int	eIncludeCentre = 0;	// 中心点を含むか
double eDiscMin;
double eDiscMax;

int com1[kMaxNumCom1][kMaxGroup2Num];
int com2[kMaxNumCom2][kMaxGroup3Num];

void SetCom1( int i, int j, int n )
// com1のi行j列にnを入れる．getcomb用
{
	com1[i][j] = n;
}

void SetCom2( int i, int j, int n )
// com2のi行j列にnを入れる．getcomb用
{
	com2[i][j] = n;
}

int CalcnCr( int n, int r )
{
	int i, num, den;
	for ( i = 0, num = 1, den = 1; i < r; i++ ) {
		num *= n - i;
		den *= i + 1;
	}
	return (int)(num / den);
}

void ReleaseCentres( CvPoint *ps )
{
	free( ps );
}

void ReleaseNears( int **nears, int num )
{
	int i;
	for ( i = 0; i < num; i++ ) {
		free(nears[i]);
	}
	free(nears);
}

int main( int argc, char *argv[] )
{
	int disc_num, num, **nears = 0, dnmin, dnmax, step, argi;
	char fname[1024];
	CvPoint *ps;
	CvSize img_size;
	strHist hist;
	strDisc disc;
	
	if ( argc < 7 ) {
//		fprintf(stderr, "argument defect\n");
//		printf("Usage: ./acr2 min_disc max_disc step_disc point_file\n\nCombination numbers(CR/Affine, n, m) are discribed in hash.h\n");
		printf("Usage: ./acr2 [r/i/a/s] [n] [m] [disc] [output] [point files...]\nEx. ./acr2 s 8 7 10 disc.txt a.dat b.dat c.dat\n");
		return 1;
	}
	switch ( argv[1][0] ) {
		case 'r':
			eInvType = CR_AREA;
			eGroup3Num = 5;
			eDiscMin = 0.0;
			eDiscMax = 4.0;
			break;
		case 'i':
			eInvType = CR_INTER;
			eGroup3Num = 5;
			eDiscMin = 0.0;
			eDiscMax = 4.0;
			break;
		case 'a':
			eInvType = AFFINE;
			eGroup3Num = 4;
			eDiscMin = 0.0;
			eDiscMax = 10.0;
			break;
		case 's':
			eInvType = SIMILAR;
			eGroup3Num = 3;
			eDiscMin = 0.0;
			eDiscMax = 4.0;
			break;
		default:
			return 1;
	}
	eGroup1Num = atoi(argv[2]);
	eGroup2Num = atoi(argv[3]);
	eDiscNum = atoi(argv[4]);
	strcpy( fname, argv[5] );
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	
	InitHist( &hist, kDiscRes, eDiscMin, eDiscMax );
	for ( argi = 6; argi < argc; argi++ ) {
		if ( (num = LoadPointFile( argv[argi], &ps, &img_size )) == 0 ) {
			fprintf(stderr, "no feature points extracted\n");
			return 1;
		}
		MakeNearsFromCentres( ps, num, &nears );	// 各点について上位kNears個の近傍点のインデックスを求め，nearsに格納する
		CalcCRAndAddHist( ps, num, nears, &hist );	// 複比orアフィン不変量を計算してヒストグラムへ
		ReleaseCentres( ps );
		ReleaseNears( nears, num );
	}
	Hist2Disc( &hist, &disc, eDiscNum );
	SaveDisc( fname, &disc );
//	OutPutHist(&hist, 1000, 1000);
}

void SaveDisc( char *fname, strDisc *disc )
// 離散化ファイルを保存
{
	int i;
	FILE *fp;
	
	if ( (fp = fopen(fname, "w")) == NULL )	return;
	fprintf(fp, "%f\n", disc->min);
	fprintf(fp, "%f\n", disc->max);
	fprintf(fp, "%d\n", disc->num);
	fprintf(fp, "%d\n", disc->res);
	for ( i = 0; i < disc->res; i++ )	fprintf(fp, "%d\n", disc->dat[i]);
	fclose(fp);
}

void Hist2Disc( strHist *hist, strDisc *disc, int disc_num )
// ヒストグラムから離散化データを作成
{
	int i, j, total, sum;
	
	disc->min = hist->min;
	disc->max = hist->max;
	disc->num = disc_num;
	disc->res = hist->size;
	disc->dat = (int *)calloc(disc->res, sizeof(int));
	for ( i = 0, total = 0; i < hist->size; i++ )	total += hist->bin[i];
	for ( i = 0, j = 0, sum = 0; i < hist->size; i++ ) {
		disc->dat[i] = j;
		sum += hist->bin[i];
		if ( sum > ((double)total/(double)disc_num) * (j+1) )	j++;
	}
}
	
void CalcCRAndAddHist( CvPoint *ps, int num, int *nears[], strHist *hist )
// 複比を計算してヒストグラムに入れる
{
	int i=0, j, k, l, st, tmp, idx[10];
	int idxcom1[kMaxGroup2Num], idxcom2[kMaxGroup3Num];
	double cr;

	// 組み合わせの作成
	gencomb(eGroup1Num, eGroup2Num, SetCom1);
	gencomb(eGroup2Num, eGroup3Num, SetCom2);

	for ( i = 0; i < num; i++ ) {
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// 各n点の組み合わせについて
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];
			for ( st = 0; st < eGroup2Num; st++ ) {	// 各開始点について
                for ( k = 0; k < eNumCom2; k++ ) {	// 各m点の組み合わせについて
                	if ( eIncludeCentre ) {
						idxcom2[0] = i;
						for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l+1] = idxcom1[com2[k][l]];	//
					} else {
						for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];	//
					}
					switch ( eInvType ) {
						case CR_INTER:
//							cr = CalcCR5(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
							break;
						case CR_AREA:
							cr = CalcCR5ByArea(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
							break;
						case AFFINE:
							cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
							break;
						case SIMILAR:
							cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
							break;
						default:
							break;
					}
					AddDataHist( hist, cr );	// ヒストグラムに追加
//					printf("%lf\n", cr);
				}

				tmp = idxcom1[0];
				for ( k = 0; k < eGroup2Num-1; k++ )	idxcom1[k] = idxcom1[k+1];
				idxcom1[eGroup2Num-1] = tmp;
			}
		}
	}
}

int LoadPointFile( char *fname, CvPoint **ps0, CvSize *size )
// 点データを読み込む
{
	int num;
	char line[1024];
	FILE *fp;
	CvPoint *ps;
	
	if ( (fp = fopen(fname, "r")) == NULL )	return 0;
	fgets(line, 1024, fp);
	sscanf(line, "%d,%d", &(size->width), &(size->height));
	ps = (CvPoint *)calloc(kMaxPointNum, sizeof(CvPoint));
	*ps0 = ps;
	num = 0;
	while ( fgets(line, 1024, fp) != NULL && num < kMaxPointNum ) {
		sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
		num++;
	}
	fclose(fp);
	return num;
}

void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 )
// 重心から近傍構造を計算する
{
	int i, **nears;

	nears = (int **)calloc(num, sizeof(int*));
	*nears0 = nears;

	for ( i = 0; i < num; i++ ) {
		nears[i] = (int *)calloc(kNears, sizeof(int));
	}
	for ( i = 0; i < num; i++ ) {
		NearestPoint( i, ps, num, nears[i] );
	}
}

void NearestPoint( int n, CvPoint *ps, int num, int nears[] )
//	サイズnumの点配列psのn番目の点と最も近いkNears個の点のインデックスをnearsに格納する
{
	int i, j, k, min_idxs[kNears];
	double dist, min_dists[kNears];
	CvPoint p;

	// 初期化
	for ( i = 0; i < kNears; i++ ) {
		min_idxs[i] = 0;
		min_dists[i] = 10000000;
	}
	p.x = ps[n].x; p.y = ps[n].y;
	for ( i = 0; i < num; i++ ) {
		if ( i == n )	continue;
		dist = sqrt((p.x - ps[i].x)*(p.x - ps[i].x)+(p.y - ps[i].y)*(p.y - ps[i].y));
		for ( j = 0; j < kNears; j++ ) {
			if ( dist < min_dists[j] ) {
				for ( k = kNears - 1; k > j; k-- ) {
					min_idxs[k] = min_idxs[k-1];
					min_dists[k] = min_dists[k-1];
				}
				min_idxs[j] = i;
				min_dists[j] = dist;
				break;
			}
		}
	}

	for ( i = 0; i < kNears; i++ ) {
		nears[i] = min_idxs[i];
	}
}

int OutPutImage(IplImage *img)
// 画像をファイルに保存する．ファイル名は自動的に連番になる
{
	static int n = 0;
	char filename[kFileNameLen];

	sprintf(filename, "output%02d.bmp", n++);
	return cvSaveImage(filename, img);
}

void CalcOrderCWN( int pt, CvPoint *ps, int *nears[], int idx[], int num )
// 時計回りに近傍点を配列に入れる（ｎ点バージョン）
{
	int i, j, tmp_i;
	double angs[1024], tmp_a;

	for ( i = 0; i < num; i++ ) {
		angs[i] = GetPointsAngle(ps[pt], ps[nears[pt][i]]);
		if ( angs[i] < 0.0 )	angs[i] += M_PI * 2.0;
		idx[i] = nears[pt][i];
	}
	for ( i = 0; i <= num-2; i++ ) {
		for ( j = num-1; j > i; j-- ) {
			if ( angs[j] < angs[i] ) {
				tmp_a = angs[i];
				angs[i] = angs[j];
				angs[j] = tmp_a;
				tmp_i = idx[i];
				idx[i] = idx[j];
				idx[j] = tmp_i;
			}
		}
	}
}

long double GetPointsAngle(CvPoint p1, CvPoint p2)
// p1から見たp2の角度を計算する（約-3.14から約3.14）
{
	return atan2(p2.y - p1.y, p2.x - p1.x);
}
