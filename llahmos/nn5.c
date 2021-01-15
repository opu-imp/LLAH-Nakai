#include "def_general.h"
#include <stdio.h>
#include <time.h>	// 乱数用
#ifdef	WIN
#include <windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#else
#include <glob.h>
#endif
#include <cv.h>
#include <highgui.h>

#include "disc.h"
#include "hash.h"
#include "dirs.h"
#include "hist.h"
#include "nn5.h"
#include "corres.h"
#include "gencomb.h"
#include "proj4p.h"
#include "projrecov.h"
#include "f_points.h"
#include "auto_connect.h"
#include "cr.h"
#include "proctime.h"
#include "score.h"
#include "nstr.h"
#include "extern.h"
#include "nears.h"
#include "fpath.h"
#include "hlist2.h"

strVoteList *vl = NULL;
char pvote[kMaxDocNum][kMaxPointNum];
int pcor[kMaxPointNum][2];
int pcor2[kMaxDocNum][kMaxPointNum];
int corres[kMaxPointNum][kMaxDocNum];
char flag1[kMaxDocNum];
char flag2[kMaxDocNum][kMaxPointNum];

int **com1 = NULL;
int **com2 = NULL;

extern long allocated_hash;

void VoteByHL( strHList *hl );
void ClearPVote( void );
void CalcPointCor( int i );
int VoteDirectlyByHL( strHList *hl, int *score, char *idx, char *idx_area, int p);
int VoteDirectlyByHL2( strHList2 *hl, int *score, char *idx, char *idx_area, int p);

#ifndef	max
#define	max(a, b)	((a > b) ? a : b);
#endif

/*
int cr2hindex(double cr)
// 複比の値をハッシュに使う最大値kHashNumの値に変換する
{
	unsigned int crint;
	crint = (unsigned int)((cr/(kErrorNN5CR*2)) + 0.5);
	if ( crint >= kHashNum) crint = kHashNum;
	return crint;
}
*/

int errfunc( const char *epath, int eerrno )
{
	printf("%d : %s\n", eerrno, epath );
	return 1;
}

unsigned int RealIndex(int *hindex, int num)
{
	int i;
	unsigned int ret = 0;

	for ( i = 0; i < eNumCom2; i++ ) {
		ret *= num;
		ret += hindex[i];
	}
	return ret;
}

unsigned int HIndexCnv(int *hindex, int num)
// 配列の形のハッシュインデックスを変換する
{
	int i;
	unsigned int ret = 0;

	for ( i = 0; i < eNumCom2; i++ ) {
		ret *= num;
		ret += hindex[i];
		ret %= kHashSize;
	}
	return ret;
}

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

int IsDat( char *str )
// strの末尾が.datか調べる
{
	int i, last_dot = -1;
	
	for ( i = 0; str[i] != '\0'; i++ ) {
		if ( str[i] == '.' )	last_dot = i;
	}
	if ( last_dot < 0 )	return 0;
	if ( !strcmp( str+last_dot+1, "dat" ) || !strcmp( str+last_dot+1, "DAT" ) )	return 1;
	else	return 0;
}

int IsTxt( char *str )
// strの末尾が.txtか調べる
{
	int i, last_dot = -1;
	
	for ( i = 0; str[i] != '\0'; i++ ) {
		if ( str[i] == '.' )	last_dot = i;
	}
	if ( last_dot < 0 )	return 0;
	if ( !strcmp( str+last_dot+1, "txt" ) || !strcmp( str+last_dot+1, "TXT" ) )	return 1;
	else	return 0;
}

int RetrieveNN52( CvPoint *ps, double *areas, int num, int *nears[], CvSize size, int *score, strDisc *disc, int *reg_nums, int *ret_time, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// ハッシュから検索する
{
	int i, j, max_score = 0, max_doc = 0;
//	DWORD start, end;
	int start, end;
	char line[kMaxLineLen];
	int pn;

	// 組み合わせの作成
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// 点対応テーブルの作成
	ClearCorres();
	// flag2の初期化
	ClearFlag2();
//	start = timeGetTime();
	start = GetProcTimeMicroSec();
	if ( eRotateOnce )
		CalcScore4( ps, areas, num, nears, score, disc, hash, hash2, hash3 );	// 検索 **CalcScore4に変更** 07/05/29
	else
		CalcScore3( ps, areas, num, nears, score, disc, hash, hash2, hash3 );
//	CheckByPPVar( ps, num, score );	// 射影変換パラメータの分散でチェック
//	end = timeGetTime();
	end = GetProcTimeMicroSec();
//	printf("retrieval time : %d\n", end - start);
	if ( !eExperimentMode )	printf("retrieval time : %dμs\n", end - start);
	*ret_time = end - start;
	for ( i = 0; i < eDbDocs; i++ ) {
		score[i] = (int)((double)score[i] - (double)reg_nums[i] * eProp);	// CBDARでの補正
	}

	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	return max_doc;
}

int RetrieveNN5Cor2(CvPoint *ps, double *areas, int num, int *nears[], CvSize size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, int *reg_nums, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// ハッシュから検索する（対応点を返すバージョン）2
{
	int i, j, max_score = 0, max_doc = 0;
	int start, end;
	char line[kMaxLineLen];
	int pn;

	start = GetProcTimeMiliSec();
	memset( (void *)corres, 0xff, sizeof(int) * num * kMaxDocNum );
	memset( (void *)flag2, 0x00, sizeof(char) * eDbDocs * kMaxPointNum );
	end = GetProcTimeMiliSec();
//	printf("ret_init : %dms\n", end - start);
	start = GetProcTimeMiliSec();
	if ( eRotateOnce )	CalcScore4(ps, areas, num, nears, score, disc, hash, hash2, hash3);	// 検索
	else				CalcScore3(ps, areas, num, nears, score, disc, hash, hash2, hash3);
	end = GetProcTimeMiliSec();
//	printf("ret_main : %dms\n", end - start);
//	eProp = 0.0;	// スコア＝得票数にする
	for ( i = 0; i < eDbDocs; i++ ) {
		score[i] = (int)((double)score[i] - (double)reg_nums[i] * eProp);	// CBDARでの補正
	}
	// 得票数最大のものを探す
	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	// 対応点を格納する
	for ( i = 0, j = 0; i < num; i++ ) {
		if ( corres[i][max_doc] >= 0 )	{
			pcor[j][0] = i;
			pcor[j][1] = corres[i][max_doc];
			j++;
		}
	}
	*pcornum0 = j;
	return max_doc;
}

int RetrieveNN5(CvPoint *ps, int num, int *nears[], CvSize size, int *score, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// ハッシュから検索する
{
	int i, j, max_score = 0, max_doc = 0;
	strDisc disc;
//	DWORD start, end;
	int start, end;
	FILE *fp, *fpp;
	char line[kMaxLineLen];
	int pn;

	// 組み合わせの作成
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// 点対応テーブルの作成
	for ( i = 0; i < kMaxPointNum; i++ ) {
		for ( j = 0; j < kMaxDocNum; j++ ) {
			corres[i][j] = -1;
		}
	}
	// flag2の初期化
	for ( i = 0; i < kMaxDocNum; i++ ) {
		for ( j = 0; j < kMaxPointNum; j++ ) {
			flag2[i][j] = 0;
		}
	}
	LoadDisc( eDiscFileName, &disc );	// 離散化テーブルの作成
//	start = timeGetTime();
	start = GetProcTimeMicroSec();
	if ( eRotateOnce )	CalcScore4( ps, NULL, num, nears, score, &disc, hash, hash2, hash3 );
	else				CalcScore3( ps, NULL, num, nears, score, &disc, hash, hash2, hash3 );	// 検索
//	CheckByPPVar( ps, num, score );	// 射影変換パラメータの分散でチェック
//	end = timeGetTime();
	end = GetProcTimeMicroSec();
//	printf("retrieval time : %d\n", end - start);
	printf("retrieval time : %dμs\n", end - start);
	fpp = fopen( "prob.txt", "w" );
	fp = fopen( ePNFileName, "r" );
	for ( i = 0; i < kMaxDocNum; i++ ) {
		fgets(line, kMaxLineLen, fp );
		sscanf(line, "%d", &pn);
		fprintf(fpp, "%d,%d\n", pn, score[i]);
		score[i] = (int)((double)score[i] - (double)pn * eProp);	// CBDARでの補正
//		score[i] = (int)((double)score[i] * (double)kMaxPointNum / (double)(pn+800));
//		score[i] = (int)((double)score[i] * (double)kMaxPointNum / sqrt(pn));
	}
	fclose(fp);
	fclose(fpp);

	for ( i = 0; i < kMaxDocNum; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
//	DrawP2P(ps, num, max_doc, size);
	return max_doc;
}

int RetrieveNN5Cor(CvPoint *ps, int num, int *nears[], CvSize size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// ハッシュから検索する（対応点を返すバージョン）
{
	int i, j, max_score = 0, max_doc = 0;
	int start, end;
//	strDisc disc;
	FILE *fp;
	char line[kMaxLineLen];
	int pn;

	start = GetProcTimeMiliSec();
	// 組み合わせの作成
//	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// 点対応テーブルの作成
//	for ( i = 0; i < kMaxPointNum; i++ ) {
//		for ( j = 0; j < kMaxDocNum; j++ ) {
//			corres[i][j] = -1;
//		}
//	}
	memset( (void *)corres, 0xff, sizeof(int) * num * kMaxDocNum );
	// flag2の初期化
//	for ( i = 0; i < kMaxDocNum; i++ ) {
//		for ( j = 0; j < kMaxPointNum; j++ ) {
//			flag2[i][j] = 0;
//		}
//	}
	memset( (void *)flag2, 0x00, sizeof(char) * eDbDocs * kMaxPointNum );
//	LoadDisc( eDiscFileName, &disc );	// 離散化テーブルの作成
	end = GetProcTimeMiliSec();
//	printf("ret_init : %dms\n", end - start);
	start = GetProcTimeMiliSec();
	if ( eRotateOnce )	CalcScore4(ps, NULL, num, nears, score, disc, hash, hash2, hash3);	// 検索
	else				CalcScore3(ps, NULL, num, nears, score, disc, hash, hash2, hash3);	// 検索
	end = GetProcTimeMiliSec();
//	printf("ret_main : %dms\n", end - start);
	fp = fopen( ePNFileName, "r" );
//	eProp = 0.0;	// スコア＝得票数にする
	for ( i = 0; i < kMaxDocNum; i++ ) {
		fgets(line, kMaxLineLen, fp );
		sscanf(line, "%d", &pn);
		score[i] = (int)((double)score[i] - (double)pn * eProp);	// CBDARでの補正
	}
	fclose(fp);
	// 得票数最大のものを探す
	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	// 対応点を格納する
	for ( i = 0, j = 0; i < num; i++ ) {
		if ( corres[i][max_doc] >= 0 )	{
			pcor[j][0] = i;
			pcor[j][1] = corres[i][max_doc];
			j++;
		}
	}
	*pcornum0 = j;
	return max_doc;
}

int RetrieveAndRecover(CvPoint *ps, int num, int *nears[], CvSize size, char *img_fname, int mode, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// ハッシュから検索する
{
	int score[kMaxDocNum], i, max_score = 0, max_doc = 0;
//	DWORD start, end;
	IplImage *img, *exp;
	strDisc disc;

//	start = timeGetTime();
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	LoadDisc( eDiscFileName, &disc );
	if ( eRotateOnce )	CalcScore4(ps, NULL, num, nears, score, &disc, hash, hash2, hash3);
	else				CalcScore3(ps, NULL, num, nears, score, &disc, hash, hash2, hash3);
	for ( i = 0; i < kMaxDocNum; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
//	end = timeGetTime();
//	printf("Retrieving time : %dms\n", end - start);

	DrawP2P(ps, num, max_doc, size);
//	start = timeGetTime();
	switch ( mode ) {
		case RETRIEVE_MODE:
			img = cvLoadImage( img_fname, 0 );
            RecovPT(ps, num, max_doc, size, img);
			break;
		case RET_MP_MODE:
			img = cvLoadImage( img_fname, 0 );
			exp = cvCreateImage( size, 8, 1 );
			cvResize( img, exp, CV_INTER_CUBIC );
			OutPutImage( exp );
			RecovPT(ps, num, max_doc, size, exp);
			break;
	}

//	end = timeGetTime();
//	printf("PT Recovering time : %dms\n", end - start);
	return max_doc;
}

void ClearFlag1(void)
{
	int i;
	for ( i = 0; i < kMaxDocNum; i++ )	flag1[i] = 0;
}

void ClearFlag2( void )
{
	int i, j;

	// flag2の初期化
	for ( i = 0; i < kMaxDocNum; i++ ) {
		for ( j = 0; j < kMaxPointNum; j++ ) {
			flag2[i][j] = 0;
		}
	}
}

void ClearCorres( void )
{
	int i, j;

	// 点対応テーブルの作成
	for ( i = 0; i < kMaxPointNum; i++ ) {
		for ( j = 0; j < kMaxDocNum; j++ ) {
			corres[i][j] = -1;
		}
	}
}

int randps_compare( const strRandPs *a, const strRandPs *b )
{
	if ( a->rand_value > b->rand_value )	return -1;
	else if ( a->rand_value < b->rand_value )	return 1;
	else	return 0;
}

void CalcScore3(CvPoint *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// 各文書の得点を計算その3
// 07/07/23 回転総当たりモードのみに変更
{
	char *hindex, *hindex_area;
	int h, i=0, j, k, l, st, tmp, max_vote = 0;
	int *idx, *idxcom1, *idxcom2;
	double cr = 0.0;
	strHList *hl;
	strHList2 *hl2;
	strRandPs randps[kMaxPointNum];
	HENTRY *phe;

	// 打ち切り関係
	if ( eTerminate != TERM_NON ) {
		for ( i = 0; i < num; i++ ) {	// 点のインデックスと乱数を格納
			randps[i].pindex = i;
			randps[i].rand_value = rand();
		}
		// ソート
		qsort(randps, num, sizeof(strRandPs), (int(*)(const void*, const void*))randps_compare);
	}

	hindex = (char *)calloc( eNumCom2, sizeof(char) );
	hindex_area = (char *)calloc( eGroup2Num, sizeof(char) );
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );

	for ( i = 0; i < kMaxDocNum; i++ )	score[i] = 0;
	for ( h = 0; h < num; h++ ) {
		switch ( eTerminate ) {
			case TERM_NON:	// 打ち切りなし
				i = h;
				break;
			case TERM_PNUM:	// 特徴点数
				if ( h >= kTermPNum )	goto end_calcscore3_loop;
				i = randps[h].pindex;
				break;
			default:
				i = h;
				break;
		}
//		ClearPVote();
		ClearFlag1();
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// 各n点の組み合わせについて
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];

			for ( st = 0; st < eGroup2Num; st++ ) {	// 各開始点について
                for ( k = 0; k < eNumCom2; k++ ) {	// 各m点の組み合わせについて
//                	if ( eIncludeCentre || eInvType == SIMILAR ) {
                	if ( eIncludeCentre ) {
						idxcom2[0] = i;
						for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l+1] = idxcom1[com2[k][l]];
					} else {
						for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
					}
					switch ( eInvType ) {
						case CR_INTER:
							cr = CalcCR5(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
							break;
						case CR_AREA:
							cr = CalcCR5ByArea(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
							break;
						case AFFINE:
							cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
							break;
						case SIMILAR:
//							cr = CalcAngleFromThreePoints(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);	// 角度を計算
							cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
						default:
							break;
					}
					hindex[k] = Con2DiscCR( cr, disc );
				}
				if ( eUseArea )	CalcHindexArea( idxcom1, areas, hindex_area );
				if ( eNoHashList ) {
					phe = ReadHash3( hindex, hindex_area, disc->num, hash3 );
					max_vote = VoteByHashEntry( phe, score, i );
				}
				else if ( eCompressHash ) {
					hl2 = ReadHash2(hindex, hindex_area, disc->num, hash2);
					max_vote = VoteDirectlyByHL2( hl2, score, hindex, hindex_area, i );
				}
				else {
					hl = ReadHash(hindex, hindex_area, disc->num, hash);
					max_vote = VoteDirectlyByHL( hl, score, hindex, hindex_area, i );
				}
				if ( eTerminate == TERM_VNUM && max_vote >= eTermVNum )	goto end_calcscore3_loop;	// 得票数による打ち切り
				// 回転
				tmp = idxcom1[0];
				for ( k = 0; k < eGroup2Num-1; k++ )	idxcom1[k] = idxcom1[k+1];
				idxcom1[eGroup2Num-1] = tmp;
			}
		}
//		CalcPointCor( i );
	}
end_calcscore3_loop:
//	for ( i = 0; i < kMaxDocNum; i++ ) {	// スコアの確認
//		if ( score[i] > 0 ) {
//			printf("%03d : %d\n", i, score[i]);
//		}
//	}
	free( hindex );
	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
}


void CalcScore4(CvPoint *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// 各文書の得点を計算その4（回転一回だけ改良版）
{
	char *hindex, **hindex_array, *inv_array, *hindex_area;
	int h, i=0, j, k, l, st, tmp, max_vote = 0, src, dst;
	int *idx, *idxcom1, *idxcom2, *idxcom1bak;
	double cr = 0.0;
	strHList *hl;
	strHList2 *hl2;
	strRandPs randps[kMaxPointNum];
	HENTRY *phe;

	// 打ち切り関係
	if ( eTerminate != TERM_NON ) {
		for ( i = 0; i < num; i++ ) {	// 点のインデックスと乱数を格納
			randps[i].pindex = i;
			randps[i].rand_value = rand();
		}
		// ソート
		qsort(randps, num, sizeof(strRandPs), (int(*)(const void*, const void*))randps_compare);
	}

	hindex = (char *)calloc( eNumCom2, sizeof(char) );
	hindex_area = (char *)calloc( eGroup2Num, sizeof(char) );
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
	// 回転不変・改良版
	inv_array = (char *)calloc( eGroup2Num, sizeof(char) );

	for ( i = 0; i < kMaxDocNum; i++ )	score[i] = 0;
	for ( h = 0; h < num; h++ ) {
		switch ( eTerminate ) {
			case TERM_NON:	// 打ち切りなし
				i = h;
				break;
			case TERM_PNUM:	// 特徴点数
				if ( h >= kTermPNum )	goto end_calcscore4_loop;
				i = randps[h].pindex;
				break;
			default:
				i = h;
				break;
		}
//		ClearPVote();
		ClearFlag1();
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// 各n点の組み合わせについて
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出
			// 開始点の決定
			for ( st = 0; st < eGroup2Num; st++ ) {	// 各開始点について
				switch ( eInvType ) {	// 続くf点から不変量を計算
					case CR_INTER:
						cr = CalcCR5(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]], ps[idxcom1[(st+3)%eGroup2Num]], ps[idxcom1[(st+4)%eGroup2Num]]);
						break;
					case CR_AREA:
						cr = CalcCR5ByArea(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]], ps[idxcom1[(st+3)%eGroup2Num]], ps[idxcom1[(st+4)%eGroup2Num]]);
						break;
					case AFFINE:
						cr = CalcAffineInv(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]], ps[idxcom1[(st+3)%eGroup2Num]]);
						break;
					case SIMILAR:
						cr = CalcSimilarInv(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]]);
					default:
						break;
				}
				inv_array[st] = Con2DiscCR( cr, disc );	// 離散化して保存
			}
			st = FindStartPoint( inv_array );	// 不変量から開始点を見つける
			// stを開始点として回転させる
			for ( k = 0; k < eGroup2Num; k++ ) {
				idxcom1bak[k] = idxcom1[k];
			}
			for ( k = 0; k < eGroup2Num; k++ ) {
				idxcom1[k] = idxcom1bak[(k+st) % eGroup2Num];
			}
			// 以上で回転不変処理は完了
			for ( k = 0; k < eNumCom2; k++ ) {	// 各m点の組み合わせについて
				// 注：eIncludeCentreの中心点を含む処理は外した
				for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
				switch ( eInvType ) {
					case CR_INTER:
						cr = CalcCR5(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
						break;
					case CR_AREA:
						cr = CalcCR5ByArea(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
						break;
					case AFFINE:
						cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
						break;
					case SIMILAR:
						cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
					default:
						break;
				}
				hindex[k] = Con2DiscCR( cr, disc );
			}
			if ( eUseArea ) CalcHindexArea( idxcom1, areas, hindex_area );
			if ( eNoHashList ) {
				phe = ReadHash3( hindex, hindex_area, disc->num, hash3 );
				max_vote = VoteByHashEntry( phe, score, i );
			} else if ( eCompressHash ) {
				hl2 = ReadHash2(hindex, hindex_area, disc->num, hash2);
				max_vote = VoteDirectlyByHL2( hl2, score, hindex, hindex_area, i );
			} else {
				hl = ReadHash(hindex, hindex_area, disc->num, hash);
				max_vote = VoteDirectlyByHL( hl, score, hindex, hindex_area, i );
			}
			/*
			if ( eUseArea ) {
				CalcHindexArea( idxcom1, areas, hindex_area );
				hl2 = ReadHash2Area(hindex, hindex_area, disc->num);
				max_vote = VoteDirectlyByHL2( hl2, score, hindex, i );
			}
			else {
				if ( eCompressHash ) {
					hl2 = ReadHash2(hindex, disc->num);
					max_vote = VoteDirectlyByHL2( hl2, score, hindex, i );
				}
				else {
					hl = ReadHash(hindex, disc->num);
					max_vote = VoteDirectlyByHL( hl, score, hindex, i );
				}
			}
			*/
			if ( eTerminate == TERM_VNUM && max_vote >= eTermVNum )	goto end_calcscore4_loop;	// 得票数による打ち切り
		}
	}
end_calcscore4_loop:
//	for ( i = 0; i < kMaxDocNum; i++ ) {	// スコアの確認
//		if ( score[i] > 0 ) {
//			printf("%03d : %d\n", i, score[i]);
//		}
//	}
	free( hindex );
	free( idx );
	free( idxcom1 );
	free( idxcom1bak );
	free( idxcom2 );
//	for ( i = 0; i < eGroup2Num; i++ )	free( hindex_array[i] );
//	free( hindex_array );
}

int HaveEnoughSimilarVector( CvPoint start, CvPoint end, int idx )
// 文書idxにおいてstartの周囲にstart->endベクトルと近いベクトルを持つ点が十分ある
{
	int count = 0;
	CvPoint v, p, vp;
	strCorMap *cm;

	v.x = end.x - start.x;
	v.y = end.y - start.y;
	for ( cm = GetCM(idx); cm != NULL; cm = cm->next ) {
		p.x = cm->x;
		p.y = cm->y;
		if ( cm->cor_num > 1 )	continue;	// とりあえず2点以上あるときはパス
//		printf("%f\n", GetPointsDistance( start, p ));
		if ( GetPointsDistance( start, p ) > kDistNeighbor ) {	// 周囲の点でなければパス
//			if ( idx == 3)	printf("neighbor\n");
			continue;
		}
		vp.x = cm->cor->x - p.x;
		vp.y = cm->cor->y - p.y;
		if ( GetPointsDistance( v, vp ) < kNearVector )	count++;
//		else if (idx == 3)	printf("vector\n");
	}
	if ( count > kEnoughNum )	return 1;
//	else if ( idx == 3 )	printf("count\n");
	return 0;
}

/* DrawCor 旧バージョン（使ってない？）
void DrawCor( void )
// 点と対応点の関係を画像に描画する
{
	int i;
	IplImage *img = cvCreateImage( cvSize(3139, 4500), 8, 3);
	CvPoint p1, p2;
	strCorMap *cm;
	strCorList *cl;

	for ( i = 3; i < 4; i++ ) {
		cvZero(img);
		for ( cm = GetCM(i); cm != NULL; cm = cm->next ) {
			p1.x = cm->x;
			p1.y = cm->y;
			cvCircle(img, p1, 4, cWhite, 1, 8, 0);
//			if ( cm->cor_num > 1 )	continue;
			for ( cl = cm->cor; cl != NULL; cl = cl->next) {
                p2.x = cl->x;
                p2.y = cl->y;
                cvLine(img, p1, p2, cWhite, 2, 8, 0);
			}
		}
		OutPutImage(img);
	}
}
*/

//#define	max(A, B)	((A) > (B) ? (A) : (B))
#define	kDrawCorHMargin	(100)
#define	kDrawCorVMargin	(100)
#define	kDrawCorSpace	(400)
#define	kDrawCorScale	(1.0)
#define	kDrawCorRectThick	(4)
#define	kDrawCorPtRad	(4)
#define	kDrawCorLineThick	(4)

void DrawCor( CvPoint *ps, int num, CvSize img_size, int res, CvPoint *corps, int cornum, CvSize corsize, int pcor[][2], int pcornum )
{
	int i, width_all, height_all;
	IplImage *img;
	
	width_all = kDrawCorHMargin * 2 + kDrawCorSpace + img_size.width + (int)(corsize.width * kDrawCorScale);
	height_all = kDrawCorVMargin * 2 + max( img_size.height, (int)(corsize.height * kDrawCorScale) );
//	printf( "%d,%d\n", width_all, height_all );
	img = cvCreateImage( cvSize( width_all, height_all ), IPL_DEPTH_8U, 3 );
	cvSet( img, cWhite, NULL );	// 白で塗りつぶす

	// 検索質問の枠を描画
	cvRectangle( img, cvPoint( kDrawCorHMargin, kDrawCorVMargin ), \
		cvPoint( kDrawCorHMargin + img_size.width, kDrawCorVMargin + img_size.height ), \
		cBlack, kDrawCorRectThick, CV_AA, 0 );
	
	// 検索質問の特徴点の描画
	for ( i = 0; i < num; i++ ) {
		cvCircle( img, cvPoint( kDrawCorHMargin + ps[i].x, kDrawCorVMargin + ps[i].y ), kDrawCorPtRad, cBlack, -1, CV_AA, 0 );
	}
	// 登録画像の枠を描画
	cvRectangle( img, cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace, kDrawCorVMargin), \
		cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corsize.width * kDrawCorScale), kDrawCorVMargin + (int)(corsize.height * kDrawCorScale) ), \
		cBlack, kDrawCorRectThick, CV_AA, 0 );
	// 登録画像の特徴点の描画
	for ( i = 0; i < cornum; i++ ) {
		cvCircle( img, cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corps[i].x * kDrawCorScale), kDrawCorVMargin + (int)(corps[i].y * kDrawCorScale) ), \
		kDrawCorPtRad, cBlack, -1, CV_AA, 0 );
	}
	// 対応関係の描画
	for ( i = 0; i < pcornum; i++ ) {
		cvLine( img, cvPoint( kDrawCorHMargin + ps[pcor[i][0]].x, kDrawCorVMargin + ps[pcor[i][0]].y ), \
		cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corps[pcor[i][1]].x * kDrawCorScale), kDrawCorVMargin + (int)(corps[pcor[i][1]].y * kDrawCorScale) ), \
		cRed, kDrawCorLineThick, CV_AA, 0 );
	}
	OutPutImage( img );
	cvReleaseImage( &img );
}


void CalcCRDiscriptorNN5(CvPoint *ps, int num, int *nears[])
// 近傍5点で複比を計算する
{
	int i, j, idx[5]/*, hindex[5]*/;
//	unsigned int crint;
	double cr;

//	printf("%d\n%d", kHashDig, kHashSize);
	for ( i = 0; i < num; i++ ) {
        CalcOrderCWFromNearest( i, ps, nears, idx );
		for ( j = 0; j < 5; j++ ) {
			cr = CalcCR5(ps[idx[j%5]], ps[idx[(j+1)%5]], ps[idx[(j+2)%5]], ps[idx[(j+3)%5]], ps[idx[(j+4)%5]]);
//			hindex[j] = cr2hindex(cr);
		}
//		crint = HIndexCnv(hindex);
		
//		AddHash(crint, 11, ps[i].x, ps[i].y);
	}

//	PrintHash();
//	SaveHash();

}

void CalcOrderCW( int pt, CvPoint *ps, int *nears[], int idx[] )
// 時計回りに近傍点を配列に入れる
{
	int i, j, tmp_i;
	double angs[5], tmp_a;

	for ( i = 0; i < 5; i++ ) {
		angs[i] = GetPointsAngle(ps[pt], ps[nears[pt][i]]);
		if ( angs[i] < 0.0 )	angs[i] += M_PI * 2.0;
		idx[i] = nears[pt][i];
	}
	for ( i = 0; i <= 3; i++ ) {
		for ( j = 4; j > i; j-- ) {
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

void CalcOrderCWN( int pt, CvPoint *ps, int *nears[], int idx[], int num )
// 時計回りに近傍点を配列に入れる（ｎ点バージョン）
{
	int i, j, tmp_i;
	double *angs, tmp_a;

	angs = (double *)calloc( num, sizeof(double) );
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
	free(angs);
}

void CalcOrderCWFromNearest( int pt, CvPoint *ps, int *nears[], int idx_ret[] )
// 時計回りに近傍点を配列に入れる（最近傍点を開始点とする）
{
	int i, j, tmp_i, idx[5];
	double angs[5], tmp_a;

	for ( i = 0; i < 5; i++ ) {
		angs[i] = GetPointsAngle(ps[pt], ps[nears[pt][i]]);
		if ( angs[i] < 0.0 )	angs[i] += M_PI * 2.0;
		idx[i] = nears[pt][i];
	}
	for ( i = 0; i <= 3; i++ ) {
		for ( j = 4; j > i; j-- ) {
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
	for ( i = 0; i < 5; i++ ) {
		if ( idx[i] == nears[pt][0] )
			break;
	}
	for ( j = 0; j < 5; j++ ) {
		idx_ret[j] = idx[(i+j)%5];
	}


}

double CalcCR5( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 )
// 平面上の5点で複比を計算する
{
	CvPoint p1325, p1425;

	CalcInterPoint( p1, p3, p2, p5, &p1325 );
	CalcInterPoint( p1, p4, p2, p5, &p1425 );

	return CalcCR(p2, p1325, p1425, p5);
}

double CalcJI5( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 )
// 平面上の5点で複比を計算する
{
	CvPoint p1325, p1425;

	CalcInterPoint( p1, p3, p2, p5, &p1325 );
	CalcInterPoint( p1, p4, p2, p5, &p1425 );

	return CalcJI(p2, p1325, p1425, p5);
}

int CalcPerm(int n, int r, int i, int rslt[])
// n進r桁の順列のi番目を求めてrsltに入れる．i番目がダメなら0を返す．
{
	int j, tmp, *fl;
	tmp = i;
	for ( j = 0; j < r; j++ ) {
		rslt[j] = tmp % n;
		tmp = tmp / n;
	}
	fl = (int *)calloc(n, sizeof(int));
	for ( j = 0; j < n; j++ ) {
		if ( fl[rslt[j]] )
			return 0;
		else 
			fl[rslt[j]] = 1;
	}
	return 1;
}

void CalcCRDiscriptorNN5Test(CvPoint *ps, int num, int *nears[])
// 近傍5点で複比を計算する
{
	CvPoint p1, p2, p3, p4, p5, p1325, p1425;
	IplImage *img = cvCreateImage( cvSize(150, 150), 8, 3 );

	cvZero(img);

	p1.x = 50;
	p1.y = 10;
	p2.x = 120;
	p2.y = 70;
	p3.x = 110;
	p3.y = 120;
	p4.x = 40;
	p4.y = 90;
	p5.x = 10;
	p5.y = 50;

	CalcInterPoint( p1, p3, p2, p5, &p1325 );
	CalcInterPoint( p1, p4, p2, p5, &p1425 );
	cvCircle( img, p1, 2, cWhite, -1, 1, 0 );	// 点の描画
	cvCircle( img, p2, 2, cWhite, -1, 1, 0 );	// 点の描画
	cvCircle( img, p3, 2, cWhite, -1, 1, 0 );	// 点の描画
	cvCircle( img, p4, 2, cWhite, -1, 1, 0 );	// 点の描画
	cvCircle( img, p5, 2, cWhite, -1, 1, 0 );	// 点の描画
	cvCircle( img, p1325, 2, cRed, -1, 1, 0 );	// 点の描画
	cvCircle( img, p1425, 2, cRed, -1, 1, 0 );	// 点の描画
	OutPutImage(img);
}

void CalcInterPoint( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint *p1234 )
// 点p1とp2の通る直線，点p3とp4の通る直線の2直線の交点を求める
{
	double x1, x2, x3, x4, y1, y2, y3, y4, a1, a2, b1, b2;

	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;
	x3 = p3.x;
	y3 = p3.y;
	x4 = p4.x;
	y4 = p4.y;

	a1 = (y1-y2) / (x1-x2);
	b1 = y1 - a1*x1;
	a2 = (y3-y4) / (x3-x4);
	b2 = y3 - a2*x3;

	p1234->x = (int)(-(b1-b2)/(a1-a2));
	p1234->y = (int)(-(a1*(b1-b2))/(a1-a2) + b1);
}

#if 0
void ConstructHashAdd(int mode)
// ハッシュを構築（追加）する
{
	int n = 0;
#ifdef	WIN
	WIN32_FIND_DATA fFind;
	HANDLE hSearch;
	char fname[kMaxPathLen];
#else
	glob_t gt;
	size_t i;
#endif
	char line[kMaxLineLen];
	FILE *fp;
	strDisc disc;

	fp = fopen(eDBCorFileName, "r");	// ファイルと番号の対応ファイル
	for ( n = 0; fgets(line, kMaxLineLen, fp) != NULL; n++ );
	fclose(fp);
	LoadDisc( eDiscFileName, &disc );	// 離散化ファイルの読み込み
	LoadHash(disc.num);

	fp = fopen(eDBCorFileName, "a");	// ファイルと番号の対応ファイル
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );

#ifdef	WIN
	hSearch=FindFirstFile(eHashSrcPath, &fFind); /* 元画像ディレクトリの探索開始 */
	fprintf(fp, "%d %s\n", n, fFind.cFileName);
	printf("%s : %d\n", fFind.cFileName, n);
//	strcpy(fname, eHashSrcDir);
	GetDir( eHashSrcPath, kMaxPathLen, fname );	// ディレクトリの取得
	strcat(fname, fFind.cFileName);	// 元画像ファイルの絶対パスの作成
	ConstructHashSub( fname, n++, mode, &disc);	// 元画像ファイルでハッシュを構築
//	puts(fFind.cFileName);
    while (FindNextFile(hSearch,&fFind)) { /* 全ファイルを処理 */
//		puts(fFind.cFileName);
		fprintf(fp, "%d %s\n", n, fFind.cFileName);
		printf("%s : %d\n", fFind.cFileName, n);
//		strcpy(fname, eHashSrcDir);
		GetDir( eHashSrcPath, kMaxPathLen, fname );	// ディレクトリの取得
		strcat(fname, fFind.cFileName);	// 元画像ファイルの絶対パスの作成
		ConstructHashSub( fname, n++, mode, &disc);	// 元画像ファイルでハッシュを構築
	}
#else
	glob(eHashSrcPath, GLOB_NOCHECK, errfunc, &gt);
	for ( i = 0; i < gt.gl_pathc; i++ ) {
		fprintf(fp, "%d %s\n", n, gt.gl_pathv[i]);
		printf("%s : %d\n", gt.gl_pathv[i], n);
		ConstructHashSub( gt.gl_pathv[i], n++, mode, &disc );
	}
	globfree(&gt);
#endif

	SaveHash();	// ハッシュの保存
	fclose(fp);
}
#endif

int ConstructHash2( int doc_num, CvPoint **pss, double **areass, int *nums, strDisc *disc, strHList ***ptr_hash, strHList2 ***ptr_hash2, HENTRY **ptr_hash3 )
// ハッシュを構築する2
{
	int i, **nears, ret = 1;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

	if ( eNoHashList ) {
		hash3 = InitHash3();	// リストなしモード
		if ( hash3 == NULL )	ret = 0;
	} else if ( eCompressHash ) {
		hash2 = InitHash2();	// 圧縮モード
		if ( hash2 == NULL )	ret = 0;
	} else {
		hash = InitHash();	// 圧縮なしモードでハッシュの初期化
		if ( hash == NULL )		ret = 0;
	}
	if ( ret == 0 ) {
		fprintf( stderr, "error: hash allocation error\n" );
		return 0;
	}

	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );	// 組み合わせの作成

	for ( i = 0; i < doc_num; i++ ) {
		fprintf( stderr, "(%d/%d)[%ld]\n", i+1, doc_num, allocated_hash );
		MakeNearsFromCentres( pss[i], nums[i], &nears );	// 各点について上位kNears個の近傍点のインデックスを求め，nearsに格納する
		if ( eRotateOnce )
			ConstructHashSubSub2( pss[i], areass[i], nums[i], nears, i, disc, hash, hash2, hash3 );	// ハッシュを構築のサブ
		else
			ConstructHashSubSub( pss[i], areass[i], nums[i], nears, i, disc, hash, hash2, hash3 );
		ReleaseNears( nears, nums[i] );
	}
	*ptr_hash = hash;
	*ptr_hash2 = hash2;
	*ptr_hash3 = hash3;

	return 1;
}

#if 0
void ConstructHash(int mode)
// ハッシュを構築する
{
	int n = 0, num;
#ifdef	WIN
	WIN32_FIND_DATA fFind;
	HANDLE hSearch;
	char fname[kMaxPathLen];
	BOOL ret;
	int i;
#else
	glob_t gt;
	size_t i;
#endif
	char pf_path[kMaxPathLen];
	FILE *fp = NULL, *fp_pn;
	strDisc disc;

//	printf("const\n");
	if ( mode != USEPF_MODE )
		fp = fopen(eDBCorFileName, "w");	// ファイルと番号の対応ファイル
	InitHash();	// ハッシュの初期化
	LoadDisc( eDiscFileName, &disc );	// 離散化ファイルの読み込み
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
#ifdef	WIN
	if ( mode == USEPF_MODE ) {
		sprintf(pf_path, "%s*%s", ePFPrefix, ePFSuffix);
		hSearch = FindFirstFile( pf_path, &fFind );	// 最初のファイルを検索（ファイル名は使わずに数だけ使う）
		ret = TRUE;
		for ( i = 0; ret != FALSE; i++ ) {
			ConstructHashSub( NULL, i, mode, &disc );
			ret = FindNextFile( hSearch, &fFind );	// 次のファイルを検索
		}
		FindClose( hSearch );
		eDbDocs = i;
	}
	else {
		fprintf( stderr, "今のところWindowsでこのモードは対応していません\n" );
	}
//	hSearch=FindFirstFile(eHashSrcPath, &fFind); /* 元画像ディレクトリの探索開始 */
//	fprintf(fp, "%d %s\n", n, fFind.cFileName);
//	printf("%s : %d\n", fFind.cFileName, n);
////	strcpy(fname, eHashSrcDir);
//	GetDir( eHashSrcPath, kMaxPathLen, fname );	// ディレクトリの取得
//	strcat(fname, fFind.cFileName);	// 元画像ファイルの絶対パスの作成
//	ConstructHashSub( fname, n++, mode, &disc);	// 元画像ファイルでハッシュを構築
////	puts(fFind.cFileName);
//    while (FindNextFile(hSearch,&fFind)) { /* 全ファイルを処理 */
////		puts(fFind.cFileName);
//		fprintf(fp, "%d %s\n", n, fFind.cFileName);
//		printf("%s : %d\n", fFind.cFileName, n);
////		strcpy(fname, eHashSrcDir);
//		GetDir( eHashSrcPath, kMaxPathLen, fname );	// ディレクトリの取得
//		strcat(fname, fFind.cFileName);	// 元画像ファイルの絶対パスの作成
//		ConstructHashSub( fname, n++, mode, &disc);	// 元画像ファイルでハッシュを構築
//	}
#else
	if ( mode == USEPF_MODE ) {
//		printf("stat\n");
		sprintf(pf_path, "%s*%s", ePFPrefix, ePFSuffix);
		glob(pf_path, GLOB_NOCHECK, errfunc, &gt);
		eDbDocs = gt.gl_pathc;
		for ( i = 0; i < gt.gl_pathc; i++ ) {
			ConstructHashSub( NULL, i, mode, &disc );
//			printf("%d\n", i);
		}
		globfree(&gt);
	}
	else {
		fp_pn = fopen( ePNFileName, "w" );
		glob(eHashSrcPath, GLOB_NOCHECK, errfunc, &gt);
		eDbDocs = gt.gl_pathc;
		for ( i = 0; i < gt.gl_pathc; i++ ) {
			fprintf(fp, "%d %s\n", n, gt.gl_pathv[i]);
			printf("%s : %d\n", gt.gl_pathv[i], n);
			num = ConstructHashSub( gt.gl_pathv[i], n++, mode, &disc );
			fprintf(fp_pn, "%d\n", num);
		}
		globfree(&gt);
		fclose(fp_pn);
	}
#endif
		
//	SaveHash();	// ハッシュの保存　一時的に除去
	if ( mode != USEPF_MODE )	fclose(fp);
}

int ConstructHashSub( char *fname, int n, int mode, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// ハッシュを構築する（サブ）
{
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	CvPoint *ps = 0;
	CvSize img_size;
	IplImage *img;
	FILE *fp;

	img_size.width = 0;
	img_size.height = 0;
	
	strcpy(pfname, ePFPrefix);
	sprintf(nstr, "%d", n);
	strcat(pfname, nstr);
	strcat(pfname, ePFSuffix);
	if ( mode == USEPF_MODE ) {	// ポイントファイルを利用
		if ( (fp = fopen(pfname, "r")) == NULL )	return 0;
		ps = (CvPoint *)calloc(kMaxPointNum, sizeof(CvPoint));
		num = 0;
		fgets(line, kMaxLineLen, fp);	// skip size of image
		sscanf( line, "%d,%d", &img_size.width, &img_size.height );
		while ( fgets(line, kMaxLineLen, fp) != NULL && num < kMaxPointNum ) {
			sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
			num++;
		}
		fclose(fp);
	} else {	// ポイントファイルでない（画像 or Dat or Txt）
		if ( IsDat( fname ) || IsTxt( fname ) ) {	// datファイル（ポイントファイル）or txtファイル（サイズなし）
			if ( (fp = fopen(fname, "r")) == NULL )	return 0;
			ps = (CvPoint *)calloc(kMaxPointNum, sizeof(CvPoint));
			num = 0;
			if ( IsDat( fname ) )	{
				fgets(line, kMaxLineLen, fp);	// skip size of image
				sscanf( line, "%d,%d", &img_size.width, &img_size.height );
			}
			while ( fgets(line, kMaxLineLen, fp) != NULL && num < kMaxPointNum ) {
				sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
				num++;
			}
			fclose(fp);
		}
		else {	// 画像ファイル
			if ( mode == CONNECTED_MODE ) {	// 単語の重心
				img = GetConnectedImage( fname, CONST_HASH_MODE );
				num = MakeCentresFromImage( &ps, img, &img_size, NULL );
			} else if ( mode == ENCLOSED_MODE ) {	// 囲まれた領域
				img = cvLoadImage( fname, 0 );
				num = MakeFeaturePointsFromImage( &ps, img, &img_size );
			}
			cvReleaseImage( &img );
		}
		SavePointFile(pfname, ps, num, &img_size, NULL);
	}

	if ( num == 0 ) return 0;
//	printf("%d, %d\n", img_size.width, img_size.height );
//	DrawPoints( img_size, ps, num );
	MakeNearsFromCentres( ps, num, &nears );	// 各点について上位kNears個の近傍点のインデックスを求め，nearsに格納する

	ConstructHashSubSub( ps, NULL, num, nears, n, disc );

	ReleaseCentres( ps );
	ReleaseNears( nears, num );
//	printf("%d\n", num );
	return num;
}
#endif

void ConstructHashSubSub(CvPoint *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// ハッシュを構築する（サブのサブ）
// 07/07/23 回転総当たりのみに変更
{
	char *hindex, *hindex_area;
	int i, j, k, l, st, tmp;
	int *idx, *idxcom1, *idxcom2;
	double cr = 0.0;

	hindex = (char *)calloc( eNumCom2, sizeof(char) );
	hindex_area = (char *)calloc( eGroup2Num, sizeof(char) );
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );

	for ( i = 0; i < num; i++ ) {
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// 各8点の組み合わせについて
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];

			for ( st = 0; st < eGroup2Num; st++ ) {	// 各開始点について
				for ( k = 0; k < eNumCom2; k++ ) {	// 各5点の組み合わせについて
//					if ( eIncludeCentre || eInvType == SIMILAR ) {
					if ( eIncludeCentre ) {
						idxcom2[0] = i;
						for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l+1] = idxcom1[com2[k][l]];
					} else {
						for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
					}
					switch ( eInvType ) {
						case CR_INTER:
							cr = CalcCR5(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
							break;
						case CR_AREA:
							cr = CalcCR5ByArea(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
							break;
						case AFFINE:
							cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
							break;
						case SIMILAR:
//							cr = CalcAngleFromThreePoints(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);	// 角度を計算
							cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
						default:
							break;
					}
					hindex[k] = Con2DiscCR( cr, disc );
				}
				if ( !eRotateOnce ) {	// 回転総当たりモード（登録の時は適当な１通りのみ）
					if ( eUseArea )	CalcHindexArea( idxcom1, areas, hindex_area );
					if ( eNoHashList )			AddHash3( hindex, hindex_area, disc->num, n, i, hash3 );	// リストなしモード
					else if ( eCompressHash )	AddHash2( hindex, hindex_area, disc->num, n, i, hash2 );	// 圧縮モードの場合
					else						AddHash( hindex, hindex_area, disc->num, n, i, hash );
					break;	// 総当たりはやらない
				}
				// 回転
				tmp = idxcom1[0];
				for ( k = 0; k < eGroup2Num-1; k++ )	idxcom1[k] = idxcom1[k+1];
				idxcom1[eGroup2Num-1] = tmp;
			}
		}
	}
	free( hindex );
	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
}

void ConstructHashSubSub2(CvPoint *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// ハッシュを構築する（サブのサブ）その2（回転一回だけ改良版）
// 07/07/11 回転総当たり（登録は適当な一通りのみ）を統合
// 実際は検索時に総当たりするので開始点を定めても構わないが，
// 無駄な処理の省略＆衝突の回避のため開始点に基づく並べ替え処理を省く
{
	char *hindex, **hindex_array, *inv_array, *hindex_area;
	int i, j, k, l, st, tmp;
	int *idx, *idxcom1, *idxcom2, *idxcom1bak;
	double cr = 0.0;

	hindex = (char *)calloc( eNumCom2, sizeof(char) );
	hindex_area = (char *)calloc( eGroup2Num, sizeof(char) );
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
	// 回転不変・改良版
	inv_array = (char *)calloc( eGroup2Num, sizeof(char) );

	for ( i = 0; i < num; i++ ) {
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// 各n点の組み合わせについて
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出
			if ( eRotateOnce ) {	// 回転一通りモードなら，開始点を決めて並べ替える
				// 開始点の決定
				for ( st = 0; st < eGroup2Num; st++ ) {	// 各開始点について
					switch ( eInvType ) {	// 続くf点から不変量を計算
						case CR_INTER:
							cr = CalcCR5(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]], ps[idxcom1[(st+3)%eGroup2Num]], ps[idxcom1[(st+4)%eGroup2Num]]);
							break;
						case CR_AREA:
							cr = CalcCR5ByArea(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]], ps[idxcom1[(st+3)%eGroup2Num]], ps[idxcom1[(st+4)%eGroup2Num]]);
							break;
						case AFFINE:
							cr = CalcAffineInv(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]], ps[idxcom1[(st+3)%eGroup2Num]]);
							break;
						case SIMILAR:
							cr = CalcSimilarInv(ps[idxcom1[st]], ps[idxcom1[(st+1)%eGroup2Num]], ps[idxcom1[(st+2)%eGroup2Num]]);
						default:
							break;
					}
					inv_array[st] = Con2DiscCR( cr, disc );	// 離散化して保存
				}
				st = FindStartPoint( inv_array );	// 不変量から開始点を見つける
				// stを開始点として回転させる
				for ( k = 0; k < eGroup2Num; k++ ) idxcom1bak[k] = idxcom1[k];
				for ( k = 0; k < eGroup2Num; k++ ) idxcom1[k] = idxcom1bak[(k+st) % eGroup2Num];
				// 以上で回転不変処理は完了
			}
			for ( k = 0; k < eNumCom2; k++ ) {	// 各m点の組み合わせについて
				// 注：eIncludeCentreの中心点を含む処理は外した
				for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
				switch ( eInvType ) {
					case CR_INTER:
						cr = CalcCR5(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
						break;
					case CR_AREA:
						cr = CalcCR5ByArea(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]], ps[idxcom2[4]]);
						break;
					case AFFINE:
						cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
						break;
					case SIMILAR:
//						cr = CalcAngleFromThreePoints(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);	// 角度を計算
						cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
					default:
						break;
				}
				hindex[k] = Con2DiscCR( cr, disc );
			}
			if ( eUseArea )	CalcHindexArea( idxcom1, areas, hindex_area );	// 面積特徴量の計算
			if ( eNoHashList )			AddHash3( hindex, hindex_area, disc->num, n, i, hash3 );	// リストなしモードの場合
			else if ( eCompressHash )	AddHash2( hindex, hindex_area, disc->num, n, i, hash2 );	// 圧縮モードの場合
			else						AddHash( hindex, hindex_area, disc->num, n, i, hash );
			/*
			if ( eUseArea ) {
				CalcHindexArea( idxcom1, areas, hindex_area );
				AddHashArea( hindex, hindex_area, disc->num, n, i );
			}
			else {
				if ( eCompressHash )	AddHash2( hindex, disc->num, n, i );	// 圧縮モードの場合
				else					AddHash( hindex, disc->num, n, i );
			}
			*/
		}
	}
	free( hindex );
	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom1bak );
	free( idxcom2 );
	free( inv_array );
}

void SavePointFile( char *fname, CvPoint *ps, int num, CvSize *size, double *areas )
// 点データを保存する
// 面積の保存を追加 06/01/13
{
	int i;
	char dir[kMaxPathLen], base[kMaxPathLen], area_fname[kMaxPathLen];
	FILE *fp;

	fp = fopen(fname, "w");
	fprintf(fp, "%d,%d\n", size->width, size->height);	// 先頭にサイズを保存する
	for ( i = 0; i < num; i++ )
		fprintf(fp, "%d,%d\n", ps[i].x, ps[i].y);
	fclose(fp);
	if ( areas == NULL )	return;	// 面積が格納されていない
	GetDir( fname, kMaxPathLen, dir );
	GetBasename( fname, kMaxPathLen, base );
	strcpy( area_fname, dir );
	strcat( area_fname, base );
	strcat( area_fname, ".are" );
	SaveAreaFile( area_fname, areas, num );
}

void SaveAreaFile( char *fname, double *areas, int num )
// 面積の配列areaからnum個の実数をfnameに書き出す
{
	int i;
	FILE *fp;
	
	if ( ( fp = fopen( fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: at SaveAreaFile in nn5.c: cannot open %s\n", fname );
		return;
	}
	for ( i = 0; i < num; i++ ) {
		fprintf( fp, "%lf\n", areas[i] );
	}
	fclose( fp );
}

int LoadPointFile( char *fname, CvPoint **ps0, CvSize *size )
// 点データを読み込む
{
	int num;
	char line[kMaxLineLen];
	FILE *fp;
	CvPoint *ps;
	
	if ( (fp = fopen(fname, "r")) == NULL )	return 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size->width), &(size->height));
	ps = (CvPoint *)calloc(kMaxPointNum, sizeof(CvPoint));
	*ps0 = ps;
	num = 0;
	while ( fgets(line, kMaxLineLen, fp) != NULL && num < kMaxPointNum ) {
		sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
		num++;
	}
	fclose(fp);
	return num;
}

void LoadPointFileAll( CvPoint ***psall0, CvSize **sizeall0, int **numall0 )
// 登録点ファイルをすべて読み込む
{
	CvPoint **psall;
	CvSize *sizeall;
	int i, *numall;
	char fname[kMaxPathLen];
	
//	printf("%s\n%s\n%d\n", ePFPrefix, ePFSuffix, eDbDocs);
	psall = (CvPoint **)calloc( eDbDocs, sizeof(CvPoint *) );
	sizeall = (CvSize *)calloc( eDbDocs, sizeof(CvSize) );
	numall = (int *)calloc( eDbDocs, sizeof(int) );
	for ( i = 0; i < eDbDocs; i++ ) {
		sprintf(fname, "%s%d%s", ePFPrefix, i, ePFSuffix);
		fprintf(stderr, "%s\n", fname);
		numall[i] = LoadPointFile( fname, &(psall[i]), &(sizeall[i]) );
	}
	*psall0 = psall;
	*sizeall0 = sizeall;
	*numall0 = numall;
}

#define	USE073

void DrawNeighbor( CvSize size, CvPoint *ps, int num, int *nears[] )
// 近傍5点を描画する
{
	int i, j, k, idx[1024]/*, hindex[10]*/, flag, fail[8];
	char txt[10];
//	double cr;
//	IplImage *img = cvCreateImage( size, 8, 3);
#ifdef USE073
	IplImage *img = cvLoadImage("07s.bmp", 1);
#else
	IplImage *img = cvLoadImage("04s.bmp", 1);
#endif
	CvScalar color;
	CvFont font;
	CvPoint p[kPNum], mid;

	for ( i = 0; i < kPNum; i++ ) {
		p[i].x = 0;
		p[i].y = 0;
	}
#ifdef USE073
	p[0].x = 149;
	p[0].y = 175;
	p[1].x = 505;
	p[1].y = 172;
	p[2].x = 893;
	p[2].y = 1159;
	p[3].x = 200;
	p[3].y = 1100;
	p[4].x = 704;
	p[4].y = 293;
	p[5].x = 429;
	p[5].y = 234;
	p[6].x = 151;
	p[6].y = 792;
	p[7].x = 545;
	p[7].y = 729;
#else
	p[0].x = 108;
	p[0].y = 191;
	p[1].x = 331;
	p[1].y = 230;
	p[2].x = 552;
	p[2].y = 1226;
	p[3].x = 135;
	p[3].y = 1187;
	p[4].x = 451;
	p[4].y = 374;
	p[5].x = 288;
	p[5].y = 287;
	p[6].x = 106;
	p[6].y = 857;
	p[7].x = 354;
	p[7].y = 800;
#endif

//	cvZero(img);
	for ( i = 0; i < num; i++ ) {
		color = cWhite;
//		cvCircle(img, ps[i], 2, color, 2, 8, 0);
		for ( flag = 0, j = 0; j < kPNum; j++ ) {
			if ( GetPointsDistance(ps[i], p[j]) < 10 ) {
				flag = 1;
				break;
			}
		}
		if ( !flag )	continue;
		
//		color = cWhite;
//		cvCircle(img, ps[i], 6, color, 2, 8, 0);
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
/*		for ( j = 0; j < 10; j++ ) {
			cr = CalcCR5(ps[idx[j%10]], ps[idx[(j+1)%10]], ps[idx[(j+2)%10]], ps[idx[(j+3)%10]], ps[idx[(j+4)%10]]);
			hindex[j] = cr2hindex(cr);
			printf("%f %d\n", cr, hindex[j]);
		}
*/
#ifdef USE073
		fail[0] = 0;
		fail[1] = 1;
		fail[2] = 2;
		fail[3] = 4;
		fail[4] = 6;
		fail[5] = 7;
		fail[6] = 8;
		fail[7] = 9;
#else
		fail[0] = 0;
		fail[1] = 1;
		fail[2] = 2;
		fail[3] = 3;
		fail[4] = 4;
		fail[5] = 6;
		fail[6] = 7;
		fail[7] = 8;
#endif
//		for ( j = 0; j < 8; j++ ) {
//			cr = CalcCR5(ps[idx[fail[j%8]]], ps[idx[fail[(j+1)%8]]], ps[idx[fail[(j+2)%8]]], ps[idx[fail[(j+3)%8]]], ps[idx[fail[(j+4)%8]]]);
//			printf("%f %d\n", cr, cr2hindex(cr));
//		}
//		MakeCom8of10();
//		for ( j = 0; j < kNum10C8; j++ ) {
//			printf("%d %d %d %d %d %d %d %d\n", c8of10[j][0], c8of10[j][1], c8of10[j][2], c8of10[j][3], c8of10[j][4], c8of10[j][5], c8of10[j][6], c8of10[j][7]) ;
//		}
//		MakeCom5of8( );
//		for ( j = 0; j < kNum8C5; j++ ) {
//			printf("%d %d %d %d %d\n", c5of8[j][0], c5of8[j][1], c5of8[j][2], c5of8[j][3], c5of8[j][4]);
//			cr = CalcCR5(ps[idx[fail[c5of8[j][0]]]], ps[idx[fail[c5of8[j][1]]]], ps[idx[fail[c5of8[j][2]]]], ps[idx[fail[c5of8[j][3]]]], ps[idx[fail[c5of8[j][4]]]]);
//			printf("%f %d\n", cr, cr2hindex(cr));
//			for ( k = 0; k < 5; k++ ) {
  //              cr = CalcCR5(ps[idx[fail[c5of8[j][(0+k)%5]]]], ps[idx[fail[c5of8[j][(1+k)%5]]]], ps[idx[fail[c5of8[j][(2+k)%5]]]], ps[idx[fail[c5of8[j][(3+k)%5]]]], ps[idx[fail[c5of8[j][(4+k)%5]]]]);
	//			printf("%d ", cr2hindex(cr));
	//		}
	//		printf("\n");
	//	}
		for ( j = 0; j < eGroup1Num; j++ ) {	// 角度順に中心から線を引く
//			cvLine(img, ps[i], ps[idx[j]], color, 1, 8, 0);
			sprintf(txt, "%d", j+1);
			mid.x = (ps[idx[j]].x + ps[i].x)/2;
			mid.y = (ps[idx[j]].y + ps[i].y)/2;
			cvInitFont( &font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.1, 0, 1);
//			cvPutText( img, txt, mid, &font, cGreen );
			cvCircle(img, ps[idx[j]], 8, cBlack, -1, 8, 0);
			cvCircle(img, ps[idx[j]], 8, cWhite, 2, 8, 0);
//			cvCircle(img, ps[idx[j]], 5, cWhite, 1, 8, 0);
		}

/*		for ( j = 0; j < eGroup1Num; j++ ) {
			cvLine(img, ps[i], ps[nears[i][j]], color, 1, 8, 0);
			sprintf(txt, "%d", j+1);
			mid.x = (ps[nears[i][j]].x + ps[i].x)/2;
			mid.y = (ps[nears[i][j]].y + ps[i].y)/2;
			cvInitFont( &font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.1, 0, 1);
			cvPutText( img, txt, mid, &font, cGreen );
		}
*/	}
	for ( j = 0; j < eNumCom1; j++ ) {
		for ( k = 0; k < eGroup2Num; k++ )	printf("%d ", com1[j][k]);
		printf("\n");
	}
	gencomb(eGroup2Num, 5, SetCom2);
	for ( j = 0; j < eNumCom2; j++ ) {
		for ( k = 0; k < 5; k++ )	printf("%d ", com2[j][k]);
		printf("\n");
	}

	OutPutImage( img );
	cvReleaseImage( &img );
}

/*
void VoteByHL( strHList *hl )
// ハッシュリストを読んでvoteする
{
	strHList *hp;
	strVoteList *vp;
    
	for ( hp = hl; hp != NULL; hp = hp->next) {
		for ( vp = vl; vp != NULL; vp = vp->next ) {
			if ( vp->doc == hp->doc && vp->point == hp->point && vp->pat == hp->pat ) {
				vp->vote++;
				break;
			}
		}
		if ( vp == NULL ) {	// 見つからなかったとき
			vp = (strVoteList *)malloc(sizeof(strVoteList));
			vp->doc = hp->doc;
			vp->point = hp->point;
			vp->pat = hp->pat;
			vp->vote = 1;
			vp->next = vl;
			vl = vp;
		}
	}
}
*/
void VoteByVL( int *score )
// 投票リストでvoteする
{
	strVoteList *vp;

	for ( vp = vl; vp != NULL; vp = vp->next ) {
		if ( vp->vote > kVoteThr )	score[vp->doc]++;
	}
}

void ClearVL( void )
// 投票リストをクリアする
{
	strVoteList *vp, *vp_n;
	for ( vp = vl; vp != NULL; ) {
		vp_n = vp->next;
		free(vp);
		vp = vp_n;
	}
	vl = NULL;
}

int VoteDirectlyByHL( strHList *hl, int *score, char *idx, char *idx_area, int p )
// ハッシュリストから直にvoteする
// 今回voteした中で最大のものを返す
{
	strHList *hp;
	static int max_vote = -1;
    
	for ( hp = hl; hp != NULL; hp = hp->next) {
//		if ( memcmp(hp->idx, idx, eNumCom2) == 0 && flag1[hp->doc] == 0 && flag2[hp->doc][hp->point] == 0) {
		// 対応関係チェック && ( ベクトルチェックなし || ベクトル一致 )
		// 07/07/04 !eVectorCheckをeNoCompareInvに変更
		if ( flag1[hp->doc] == 0 && /* 対応点フラグチェック */ \
//			 flag2[hp->doc][hp->point] == 0 ) {
			 flag2[hp->doc][hp->point] == 0 && \
			 ( eNoCompareInv || memcmp(hp->idx, idx, eNumCom2) == 0 ) /* 特徴量の比較 */ ) {
//			 ( eNoCompareInv || !eUseArea || memcmp(hp->idx + eNumCom2, idx_area, eGroup2Num) ) /* 面積特徴量の比較 */ \
//			) {
			score[hp->doc]++;	// 投票
			if ( score[hp->doc] > max_vote )	max_vote = score[hp->doc];	// 最大なら更新
			corres[p][hp->doc] = hp->point;
			flag1[hp->doc] = 1;	// hp->docは投票済み
			flag2[hp->doc][hp->point] = 1;	// hp-docのhp->pointは投票済み
		}
	}
	return max_vote;
}

int VoteDirectlyByHL2( strHList2 *hl, int *score, char *idx, char *idx_area, int p )
// ハッシュリストから直にvoteする
// 今回voteした中で最大のものを返す
// 圧縮版
{
	strHList2 *hp;
	static int max_vote = -1;
	unsigned long doc, point;
	char *r, *o;
    
	r = (char *)calloc( eNumCom2, sizeof(char) );
	o = (char *)calloc( eGroup2Num, sizeof(char) );
	for ( hp = hl; hp != NULL; hp = hp->next) {
//		if ( memcmp(hp->idx, idx, eNumCom2) == 0 && flag1[hp->doc] == 0 && flag2[hp->doc][hp->point] == 0) {
		// 対応関係チェック && ( ベクトルチェックなし || ベクトル一致 )
		ReadHList2Dat( hp->dat, &doc, &point, r, o );
//		printf("%d, %d\n", doc, point );
		// 07/07/04 !eVectorChechを除去，eUseAreaが何故入っているかは不明
//		if ( flag1[doc] == 0 && flag2[doc][point] == 0 && ( eUseArea || eNoCompareInv  || memcmp(r, idx, eNumCom2) == 0 ) ) {
		// 07/07/20 条件を調整
		if ( flag1[doc] == 0 && /* フラグチェック */ \
			flag2[doc][point] == 0 && \
			( eNoCompareInv || memcmp(r, idx, eNumCom2) == 0 ) && /* 特徴量の比較 */ \
			( eNoCompareInv || !eUseArea || memcmp(o, idx_area, eGroup2Num ) ) /* 面積特徴量の比較 */ \
			) {
			score[doc]++;	// 投票
			if ( score[doc] > max_vote )	max_vote = score[doc];	// 最大なら更新
			corres[p][doc] = point;
			flag1[doc] = 1;	// hp->docは投票済み
			flag2[doc][point] = 1;	// hp-docのhp->pointは投票済み
		}
	}
	free( r );
	free( o );
	return max_vote;
}

int VoteByHashEntry( HENTRY *phe, int *score, int p )
// ハッシュテーブルのエントリからvoteする
// リストなしバージョン
{
	static int max_vote = -1;
	int doc, point;

	if ( *phe == kFreeEntry || *phe == kInvalidEntry )	return max_vote;	// 空きまたは無効なエントリ
	ReadHashEntry( phe, &doc, &point );
	if ( flag1[doc] == 0 && flag2[doc][point] == 0 ) {
		score[doc]++;	// 投票
		if ( score[doc] > max_vote )	max_vote = score[doc];	// 最大なら更新
		corres[p][doc] = point;
		flag1[doc] = 1;	// hp->docは投票済み
		flag2[doc][point] = 1;	// hp-docのhp->pointは投票済み
	}
	return max_vote;
}

void ClearPVote( void )
{
	int i, j;

	for ( i = 0; i < kMaxDocNum; i++ )
		for ( j = 0; j < kMaxPointNum; j++ )
			pvote[i][j] = 0;
}

void CalcPointCor( int n )
{
	int i, j, max_vote = 0, max_doc = -1, max_point = -1;

	for ( i = 0; i < kMaxDocNum; i++ ) {
		for ( j = 0; j < kMaxPointNum; j++ ) {
			if ( pvote[i][j] > max_vote ) {
				max_doc = i;
				max_point = j;
				max_vote = pvote[i][j];
			}
		}
	}
	pcor[n][0] = max_doc;
	pcor[n][1] = max_point;
}

void DrawCross( IplImage *img, CvPoint center, int length, CvScalar color, int thickness )
// imgのcenterに×を描く
{
	CvPoint p1, p2;

	p1.x = center.x - length;
	p1.y = center.y - length;
	p2.x = center.x + length;
	p2.y = center.y + length;
	cvLine( img, p1, p2, color, thickness, 8, 0 );
	p1.x = center.x + length;
	p1.y = center.y - length;
	p2.x = center.x - length;
	p2.y = center.y + length;
	cvLine( img, p1, p2, color, thickness, 8, 0 );
}

void DrawP2P( CvPoint *ps, int num, int cor_doc, CvSize size )
// 点から点へ線を引く
{
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	int ori_num, i;
	FILE *fp;
	CvPoint corps[kMaxPointNum];
	IplImage *img;
	CvSize size2, size_all;

	// データベースの点ファイル名を作って開く
	strcpy(pfname, ePFPrefix);
//	itoa(cor_doc, nstr, 10);
	sprintf(nstr, "%d", cor_doc);
	strcat(pfname, nstr);
	strcat(pfname, ePFSuffix);
	fp = fopen(pfname, "r");
	// 点ファイルのデータをcorpsに格納する
	ori_num = 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size2.width), &(size2.height));	// 1行目はサイズ
	while( fgets(line, kMaxLineLen, fp) != NULL ) {
		sscanf(line, "%d,%d", &(corps[ori_num].x), &(corps[ori_num].y));
		ori_num++;
	}
	fclose(fp);
	// 描画する画像の作成とか
	size_all.width = size.width + size2.width;
	size_all.height = max(size.height, size2.height);
	img = cvCreateImage(size_all, 8, 3);
	cvZero(img);

	cvRectangle( img, cvPoint(1, 1), cvPoint(size.width-5, size.height-5), cWhite, 3, 8, 0 );
	// 全点の描画（クエリ）
	for ( i = 0; i < num; i++ )	cvCircle( img, ps[i], 8, cWhite, 3, 8, 0);
	cvRectangle( img, cvPoint(size.width, 1), cvPoint(size.width + size2.width-5, size2.height-5), cWhite, 3, 8, 0);
	// 全店の描画（データベース）
	for ( i = 0; i < ori_num; i++ )	DrawCross( img, cvPoint(size.width + corps[i].x, corps[i].y), 8, cWhite, 3 );
	for ( i = 0; i < num; i++ ) {
		if ( corres[i][cor_doc] >= 0 ) {
			cvLine(img, ps[i], cvPoint(size.width + corps[corres[i][cor_doc]].x, corps[corres[i][cor_doc]].y), cWhite, 3, 8, 0);
		}
//		if ( pcor[i][0] != cor_doc )	continue;
		// 線を引く
//		cvLine(img, ps[i], cvPoint(size.width + corps[pcor[i][1]].x, corps[pcor[i][1]].y), cWhite, 3, 8, 0);
	}
/*	for ( i = 0; i < ori_num; i++ )	DrawCross( img, corps[i], 8, cWhite, 3 );
	for ( i = 0; i < num; i++ ) {
		if ( pcor[i][0] != cor_doc )	continue;
		cvLine(img, ps[i], corps[pcor[i][1]], cWhite, 3, 8, 0);
	}*/
	OutPutImage(img);
}

void RecovPT( CvPoint *ps, int num, int cor_doc, CvSize size0, IplImage *img )
// 射影変換の補正トップ
{
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	int ori_num, i, cor[kMaxPointNum][2], cor_num = 0;
	FILE *fp;
	CvPoint corps[kMaxPointNum];
	CvSize size;

	// データベースの点ファイル名を作って開く
	strcpy(pfname, ePFPrefix);
//	itoa(cor_doc, nstr, 10);
	sprintf(nstr, "%d", cor_doc);
	strcat(pfname, nstr);
	strcat(pfname, ePFSuffix);
	fp = fopen(pfname, "r");
	// 点ファイルのデータをcorpsに格納する
	ori_num = 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size.width), &(size.height));	// 1行目はサイズ
	while( fgets(line, kMaxLineLen, fp) != NULL ) {
		sscanf(line, "%d,%d", &(corps[ori_num].x), &(corps[ori_num].y));
		ori_num++;
	}
	fclose(fp);
	// 有効な（文書IDが正しく，対応点のある）点を取り出す（実はもう少し有効な点を増やす方法もある）
	for ( i = 0; i < num; i++ ) {
		if ( pcor[i][0] == cor_doc ) {
			cor[cor_num][0] = i;
			cor[cor_num][1] = pcor[i][1];
			cor_num++;
		}
	}
#if 0
	fp = fopen( "temp.dat", "w" );
	fprintf(fp, "%d\n", num);	// 点の数
	for ( i = 0; i < num; i++ )	fprintf(fp, "%d,%d\n", ps[i].x, ps[i].y);	// 点の座標
	fprintf(fp, "%d\n", ori_num);	// 対応文書の点の数
	for ( i = 0; i < ori_num; i++ )	fprintf(fp, "%d,%d\n", corps[i].x, corps[i].y);	// 対応文書の点の座標
	fprintf(fp, "%d\n", cor_num);	// 対応関係の数
	for ( i = 0; i < cor_num; i++ )	fprintf(fp, "%d,%d\n", cor[i][0], cor[i][1]);	// 対応関係
	fprintf(fp, "%d,%d\n", size.width, size.height);	// サイズ
	fprintf(fp, "%s\n", img_fname);	// 画像ファイル名
	fclose(fp);
#endif
	RecovPTSub( ps, corps, cor, cor_num, size, img );	// サブに任せる
}

void RecovPTFromTemp( void )
// 射影変換補正のテスト
{
	int num, ori_num, i, cor[kMaxPointNum][2], cor_num = 0;
	char line[kMaxLineLen], img_fname[kMaxPathLen];
	FILE *fp;
	CvSize size;
	CvPoint *ps, *corps;

	fp = fopen("temp.dat", "r");
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d", &num);
	ps = (CvPoint *)calloc(num, sizeof(CvPoint));
	for ( i = 0; i < num; i++ ) {
		fgets(line, kMaxLineLen, fp);
		sscanf(line, "%d,%d", &(ps[i].x), &(ps[i].y));
	}
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d", &ori_num);
	corps = (CvPoint *)calloc(ori_num, sizeof(CvPoint));
	for ( i = 0; i < ori_num; i++ ) {
		fgets(line, kMaxLineLen, fp);
		sscanf(line, "%d,%d", &(corps[i].x), &(corps[i].y));
	}
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d", &cor_num);
	for ( i = 0; i < cor_num; i++ ) {
		fgets(line, kMaxLineLen, fp);
		sscanf(line, "%d,%d", &(cor[i][0]), &(cor[i][1]));
	}
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size.width), &(size.height));
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%s", img_fname);

	fclose(fp);

	printf("%d, %d, %d\n", num, ori_num, cor_num);
//	RecovPTSub( ps, corps, cor, cor_num, size, img_fname );	// サブに任せる
}

void CheckByPPVar( CvPoint *ps, int num, int *score )
{
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	int ori_num, i, j, cor[kMaxPointNum][2], cor_doc, cor_num = 0;
	FILE *fp;
	CvPoint corps[kMaxPointNum];
	strScore *strscr;
	double ppvar[kCheckPPVarNum], var, v1;
	
	strscr = (strScore *)malloc(sizeof(strScore) * kMaxDocNum);	// strScore構造体の確保
	SetAndSortScore( strscr, score, kMaxDocNum );	// strScoreにいれてソート
	for ( i = 0; i < kCheckPPVarNum; i++ ) {
		// データベースの点ファイル名を作って開く
		cor_doc = strscr[i].n;	// 対応文書ID
		strcpy(pfname, ePFPrefix);
		sprintf(nstr, "%d", cor_doc);
		strcat(pfname, nstr);
		strcat(pfname, ePFSuffix);
		fp = fopen(pfname, "r");
		// 点ファイルのデータをcorpsに格納する
		ori_num = 0;
		fgets(line, kMaxLineLen, fp);
//		sscanf(line, "%d,%d", &(size.width), &(size.height));	// 1行目はサイズ
		while( fgets(line, kMaxLineLen, fp) != NULL ) {
			sscanf(line, "%d,%d", &(corps[ori_num].x), &(corps[ori_num].y));
			ori_num++;
		}
		fclose(fp);
		// 有効な（文書IDが正しく，対応点のある）点を取り出す
		for ( j = 0, cor_num=0; j < num; j++ ) {
			if ( corres[j][cor_doc] >= 0 ) {
				cor[cor_num][0] = j;
				cor[cor_num][1] = corres[j][cor_doc];
				cor_num++;
			}
		}
		if ( cor_num < 10 ) continue;
		for ( j = 0, var = 0.0; j < 10; j++ ) {
			v1 = GetPPVar( ps, corps, cor, cor_num );
//			printf("%f\n", v1);
			var += v1;
		}
//		ppvar[i] = GetPPVar( ps, corps, cor, cor_num );
		ppvar[i] = var / 10.0L;
		printf("%d : %d : %f\n", cor_doc, cor_num, ppvar[i]);
	}
	
	free(strscr);
}

void InitCom( int ***com0, int num1, int num2 )
// com1/com2を初期化する
{
	int i, **com;
	com = (int **)calloc( num1, sizeof(int *) );
	for ( i = 0; i < num1; i++ ) {
		com[i] = (int *)calloc( num2, sizeof(int) );
	}
	*com0 = com;
}

void GenerateCombination( int inv_type, int g1, int g2, int g3, void (*setcom1)(int, int, int), void (*setcom2)(int, int, int))
{
//	int i;

// 原因不明だがエラーが出るのでコメントアウト．メモリリークになる
//	if ( com1 != NULL )		free( com1 );
//	if ( com2 != NULL )		free( com2 );
	InitCom( &com1, eNumCom1, eGroup2Num );
	InitCom( &com2, eNumCom2, eGroup3Num );
	gencomb(g1, g2, setcom1);
//	if ( inv_type == SIMILAR ) {
//		for ( i = 0; i < g2-1; i++ ) {
//			com2[i][0] = i;
//			com2[i][1] = i+1;
//		}
//	} else {
		gencomb(g2, g3, setcom2);
//	}
}

void CreatePointFile2( void )
// 特徴点抽出モード2
{
	int n;
	FILE *fp, *fp_pn;
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	CvPoint *ps = 0;
	CvSize img_size;
	IplImage *img;
#ifdef	WIN
	WIN32_FIND_DATA fFind;
	HANDLE hSearch;
	char fname[kMaxPathLen];
	BOOL ret;
	int i;
#else
	glob_t gt;
	size_t i;
#endif

	n = eCPF2StartNum;
	fp = fopen( eDBCorFileName, "w" );	// ファイルと番号の対応ファイル
	fp_pn = fopen( ePNFileName, "w" );

#ifdef	WIN
	hSearch = FindFirstFile( eHashSrcPath, &fFind );	// 最初のファイルを検索
	for ( i = 0, ret = TRUE; ret != FALSE; i++, n++, ret = FindNextFile( hSearch, &fFind ) ) {
		GetDir( eHashSrcPath, kMaxPathLen, fname );	// ディレクトリの取得
		puts(fname);
		strcat(fname, fFind.cFileName);	// 元画像ファイルの絶対パスの作成
		puts(fname);

		fprintf( fp, "%d %s\n", n, fname );
		printf( "%s : %d\n", fFind.cFileName, n );
//		ConstructHashSub( NULL, i, mode, &disc );
		img_size.width = 0;
		img_size.height = 0;
		
		strcpy(pfname, ePFPrefix);
		sprintf(nstr, "%d", n);
		strcat(pfname, nstr);
		strcat(pfname, ePFSuffix);
//	モードは決めうちにした
//		if ( mode == CONNECTED_MODE ) {	// 単語の重心
			puts(fname);
			img = GetConnectedImage( fname, CONST_HASH_MODE );
			num = MakeCentresFromImage( &ps, img, &img_size, NULL );
			puts("end centre");
//		} else if ( mode == ENCLOSED_MODE ) {	// 囲まれた領域
//			img = cvLoadImage( gt.gl_pathv[i], 0 );
//			num = MakeFeaturePointsFromImage( &ps, img, &img_size );
//		}
		cvReleaseImage( &img );
		puts("end release");
		puts(pfname);
		SavePointFile( pfname, ps, num, &img_size, NULL );
		puts("saved");

		ReleaseCentres( ps );
		puts("end release centre");

		fprintf(fp_pn, "%d\n", num);
		printf("num : %d\n", num);
	}

	FindClose( hSearch );
	eDbDocs = i;
#else
	glob(eHashSrcPath, GLOB_NOCHECK, errfunc, &gt);
	eDbDocs = gt.gl_pathc;
	for ( i = 0; i < gt.gl_pathc; i++, n++ ) {
		fprintf(fp, "%d %s\n", n, gt.gl_pathv[i]);
		printf("%s : %d\n", gt.gl_pathv[i], n);
		
//		num = ConstructHashSub( gt.gl_pathv[i], n++, mode, &disc );

		img_size.width = 0;
		img_size.height = 0;
	
		strcpy(pfname, ePFPrefix);
		sprintf(nstr, "%d", n);
		strcat(pfname, nstr);
		strcat(pfname, ePFSuffix);
//	モードは決めうちにした
//		if ( mode == CONNECTED_MODE ) {	// 単語の重心
			img = GetConnectedImage( gt.gl_pathv[i], CONST_HASH_MODE );
			num = MakeCentresFromImage( &ps, img, &img_size, NULL );
//		} else if ( mode == ENCLOSED_MODE ) {	// 囲まれた領域
//			img = cvLoadImage( gt.gl_pathv[i], 0 );
//			num = MakeFeaturePointsFromImage( &ps, img, &img_size );
//		}
		cvReleaseImage( &img );
		SavePointFile( pfname, ps, num, &img_size, NULL );

		ReleaseCentres( ps );

		fprintf(fp_pn, "%d\n", num);
	}
	globfree(&gt);
#endif
	fclose(fp_pn);
	fclose(fp);
}

int CreatePointFile3( CvPoint ***p_reg_pss, CvSize **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// 特徴点抽出モード3（公開用に改変）
{
	int i, j, n;
	FILE *fp, *fp_pn;
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	CvPoint *ps = 0;
	CvSize img_size;
	IplImage *img;
	char search_path[kMaxPathLen], pdat_fname[kMaxPathLen];
	CvPoint **reg_pss;
	CvSize *reg_sizes;
	int *reg_nums;
	char **dbcors, **files;

	sprintf( search_path, "%s*.%s", eHashSrcDir, eHashSrcSuffix );	// 検索パスの作成
	sprintf( pdat_fname, "%s%s", eDirsDir, ePointDatFileName );	// point.datのパスを作成
	if ( ( fp = fopen( pdat_fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", pdat_fname );
		return 0;
	}
	eDbDocs = FindPath( search_path, &files );
	if ( eDbDocs <= 0 )	return 0;
	// メモリの確保
	reg_pss = (CvPoint **)calloc( eDbDocs, sizeof(CvPoint *) );
	reg_sizes = (CvSize *)calloc( eDbDocs, sizeof(CvSize) );
	reg_nums = (int *)calloc( eDbDocs, sizeof(int) );
	dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
	for ( i = 0; i < eDbDocs; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}
	// ファイル出力＆格納
	fprintf( fp, "%d\n", eDbDocs );	// ファイル数の出力
	for ( i = 0; i < eDbDocs; i++ ) {
		fprintf( stderr, "%s(%d/%d)\n", files[i], i+1, eDbDocs );
		fprintf( fp, "%s\n", files[i] );	// 元画像ファイル名を出力
		strcpy( dbcors[i], files[i] );	// 元画像ファイル名を保存
		if ( IsDat( files[i] ) ) {	// datファイルの場合
			reg_nums[i] = LoadPointFile( files[i], &(reg_pss[i]), &(reg_sizes[i]) );
		}
		else {	// 画像ファイルの場合
			img = GetConnectedImage2( files[i], CONST_HASH_MODE );	// 結像画像を作成
//			OutPutImage( img );
			reg_nums[i] = MakeCentresFromImage( &(reg_pss[i]), img, &(reg_sizes[i]), NULL );	// 特徴点を抽出
			cvReleaseImage( &img );	// 画像を解放
		}
//		SavePointFile( pfname, ps, num, &img_size, NULL );
		// 特徴点情報を出力
		fprintf( fp, "%d,%d\n", reg_sizes[i].width, reg_sizes[i].height );	// サイズを出力
		fprintf( fp, "%d\n", reg_nums[i] );	// 特徴点数を出力
		for ( j = 0; j < reg_nums[i]; j++ ) {
			fprintf( fp, "%d,%d\n", reg_pss[i][j].x, reg_pss[i][j].y );	// 特徴点の座標を出力
		}
	}
	// ポインタに格納
	*p_reg_pss = reg_pss;
	*p_reg_nums = reg_nums;
	*p_reg_sizes = reg_sizes;
	*p_dbcors = dbcors;
	// 解放
	fclose( fp );
	for ( i = 0; i < eDbDocs; i++ ) {
		free( files[i] );
	}
	free( files );

	return i;
}

int CreatePointFile4( CvPoint ***p_reg_pss, double ***p_reg_areass, CvSize **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// 特徴点抽出モード4（面積に関する処理を追加）
{
	int i, j, n;
	FILE *fp, *fp_pn;
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	CvPoint *ps = 0;
	CvSize img_size;
	IplImage *img;
	char search_path[kMaxPathLen], pdat_fname[kMaxPathLen];
	CvPoint **reg_pss;
	double **reg_areass;
	CvSize *reg_sizes;
	int *reg_nums;
	char **dbcors, **files;

	sprintf( search_path, "%s*.%s", eHashSrcDir, eHashSrcSuffix );	// 検索パスの作成
	sprintf( pdat_fname, "%s%s", eDirsDir, ePointDatFileName );	// point.datのパスを作成
	if ( ( fp = fopen( pdat_fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", pdat_fname );
		return 0;
	}
	eDbDocs = FindPath( search_path, &files );
	if ( eDbDocs <= 0 )	return 0;
	if ( eDbDocs > kMaxDocNum ) {	// 文書数がkMaxDocNumを超えた
		fprintf( stderr, "error: # of images(%d) exceeds max(%d)\n", eDbDocs, kMaxDocNum );
		return 0;
	}
	// メモリの確保
	reg_pss = (CvPoint **)calloc( eDbDocs, sizeof(CvPoint *) );
	reg_areass = (double **)calloc( eDbDocs, sizeof(double *) );
	reg_sizes = (CvSize *)calloc( eDbDocs, sizeof(CvSize) );
	reg_nums = (int *)calloc( eDbDocs, sizeof(int) );
	dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
	for ( i = 0; i < eDbDocs; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}
	// ファイル出力＆格納
	fprintf( fp, "%d\n", eDbDocs );	// ファイル数の出力
	for ( i = 0; i < eDbDocs; i++ ) {
		fprintf( stderr, "%s(%d/%d)\n", files[i], i+1, eDbDocs );
		fprintf( fp, "%s\n", files[i] );	// 元画像ファイル名を出力
		strcpy( dbcors[i], files[i] );	// 元画像ファイル名を保存
		// datファイルの場合の処理は削除。面積がないので
		if ( eIsJp )	img = GetConnectedImageJp2( files[i], CONST_HASH_MODE );	// 結像画像を作成
		else			img = GetConnectedImage2( files[i], CONST_HASH_MODE );	// 結像画像を作成
		reg_nums[i] = MakeCentresFromImage( &(reg_pss[i]), img, &(reg_sizes[i]), &(reg_areass[i]) );	// 特徴点を抽出
		cvReleaseImage( &img );	// 画像を解放
		// 特徴点情報を出力
		fprintf( fp, "%d,%d\n", reg_sizes[i].width, reg_sizes[i].height );	// サイズを出力
		fprintf( fp, "%d\n", reg_nums[i] );	// 特徴点数を出力
		for ( j = 0; j < reg_nums[i]; j++ ) {
			fprintf( fp, "%d,%d,%lf\n", reg_pss[i][j].x, reg_pss[i][j].y, reg_areass[i][j] );	// 特徴点の座標と面積を出力
		}
	}
	// ポインタに格納
	*p_reg_pss = reg_pss;
	*p_reg_areass = reg_areass;
	*p_reg_nums = reg_nums;
	*p_reg_sizes = reg_sizes;
	*p_dbcors = dbcors;
	// 解放
	fclose( fp );
	for ( i = 0; i < eDbDocs; i++ ) {
		free( files[i] );
	}
	free( files );

	return i;
}

int LoadPointFile2( char *fname, CvPoint ***p_reg_pss, double ***p_reg_areass, CvSize **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// 特徴点抽出モード3（公開用に改変）
{
	FILE *fp;
    int i, j, doc_num;
	char line[kMaxLineLen];
	CvPoint **reg_pss;
	double **reg_areass;
	CvSize *reg_sizes;
	int *reg_nums;
	char **dbcors;

	if ( ( fp = fopen( fname, "r" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}
	fgets( line, kMaxLineLen, fp );
	sscanf( line, "%d", &doc_num );
	if ( doc_num != eDbDocs ) {	// 登録文書数が一致しない
//		fprintf( stderr, "error: document number does not match\n" );
//		return 0;
		eDbDocs = doc_num;	// config.datを読み込まない場合があるため
	}
	// メモリの確保
	reg_pss = (CvPoint **)calloc( eDbDocs, sizeof(CvPoint *) );
	reg_areass = (double **)calloc( eDbDocs, sizeof(double *) );
	reg_sizes = (CvSize *)calloc( eDbDocs, sizeof(CvSize) );
	reg_nums = (int *)calloc( eDbDocs, sizeof(int) );
	dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
	for ( i = 0; i < eDbDocs; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}
	// 読み込み
	for ( i = 0; i < eDbDocs && fgets( line, kMaxLineLen, fp ) != NULL; i++ ) {
		sscanf( line, "%s", dbcors[i] );	// 元画像ファイルのパス
		fgets( line, kMaxLineLen, fp );
		sscanf( line, "%d,%d", &(reg_sizes[i].width), &(reg_sizes[i].height) );	// サイズ
		fgets( line, kMaxLineLen, fp );
		sscanf( line, "%d", &(reg_nums[i]) );	// 特徴点数
		reg_pss[i] = (CvPoint *)calloc( reg_nums[i], sizeof(CvPoint) );	// 特徴点データのメモリ確保
		reg_areass[i] = (double *)calloc( reg_nums[i], sizeof(double) );	// 面積データのメモリ確保
		for ( j = 0; j < reg_nums[i]; j++ ) {
			fgets( line, kMaxLineLen, fp );
			sscanf( line, "%d,%d,%lf", &(reg_pss[i][j].x), &(reg_pss[i][j].y), &(reg_areass[i][j]) );	// 特徴点の座標
		}
	}
	// ポインタに格納
	*p_reg_pss = reg_pss;
	*p_reg_areass = reg_areass;
	*p_reg_nums = reg_nums;
	*p_reg_sizes = reg_sizes;
	*p_dbcors = dbcors;
	
	fclose( fp );
	return i;
}

void DrawPoints( CvSize img_size, CvPoint *ps, int num )
// 特徴点を描画
{
	int i, width_all, height_all;
	IplImage *pt;

	width_all = kDrawCorHMargin * 2 + img_size.width;
	height_all = kDrawCorVMargin * 2 + img_size.height;
//	printf( "%d,%d\n", width_all, height_all );
	pt = cvCreateImage( cvSize( width_all, height_all ), IPL_DEPTH_8U, 3 );
	cvSet( pt, cWhite, NULL );	// 白で塗りつぶす

	// 検索質問の枠を描画
	cvRectangle( pt, cvPoint( kDrawCorHMargin, kDrawCorVMargin ), \
		cvPoint( kDrawCorHMargin + img_size.width, kDrawCorVMargin + img_size.height ), \
		cBlack, kDrawCorRectThick, CV_AA, 0 );
	
	// 検索質問の特徴点の描画
	for ( i = 0; i < num; i++ ) {
		cvCircle( pt, cvPoint( kDrawCorHMargin + ps[i].x, kDrawCorVMargin + ps[i].y ), kDrawCorPtRad, cBlack, -1, CV_AA, 0 );
	}
	OutPutImage( pt );

	cvReleaseImage( &pt );
}

void CalcCRAndAddHist( CvPoint *ps, int num, int *nears[], strHist *hist )
// 複比を計算してヒストグラムに入れる
{
	int i=0, j, k, l, st, tmp, *idx;
	int *idxcom1, *idxcom2;
	double cr;

	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
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
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
}

int FindStartPoint( char *inv_array )
// 開始点を見つける
{
	int i, j, min;

	for ( i = 1, min = 0; i < eGroup2Num; i++ ) {
		// まずinv_array[min]とinv_array[i]を比較し，
		// 次にinv_array[min+1]とinv_array[i+1]というように順に比較して小さいほうをminとする．
		// 折り返しのためeGroup2Numでmodを取る
		for ( j = 0; j < eGroup2Num; j++ ) {
			if ( inv_array[(min+j)%eGroup2Num] < inv_array[(i+j)%eGroup2Num] ) {
				break;
			} else if ( inv_array[(min+j)%eGroup2Num] > inv_array[(i+j)%eGroup2Num] ) {
				min = i;
				break;
			}
		}
	}
	return min;
}

void CalcHindexArea( int *idxcom1, double *areas, char *hindex_area )
// 面積からインデックスを計算
// 順位バージョン
{
	int i, j;
	int *num_array, tmp_num;
	double *area_array, tmp_area;

	num_array = (int *)calloc( eGroup2Num, sizeof(int) );
	area_array = (double *)calloc( eGroup2Num, sizeof(double) );

	for ( i = 0; i < eGroup2Num; i++ ) {
		num_array[i] = i;
		area_array[i] = areas[idxcom1[i]];
	}
	for ( i = 0; i < eGroup2Num-1; i++ ) {
		for ( j = i+1; j < eGroup2Num; j++ ) {
			if ( area_array[i] < area_array[j] ) {
				tmp_num = num_array[i];
				num_array[i] = num_array[j];
				num_array[j] = tmp_num;
				tmp_area = area_array[i];
				area_array[i] = area_array[j];
				area_array[j] = tmp_area;
			}
		}
	}
	for ( i = 0; i < eGroup2Num; i++ )	hindex_area[i] = (char)num_array[i];

	free( num_array );
	free( area_array );
}
