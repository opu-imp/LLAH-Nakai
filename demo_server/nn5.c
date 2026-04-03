#include "def_general.h"
#include <stdio.h>
#include <time.h>	// �����p
#ifdef	WIN32
#include <windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#else
#include <glob.h>
#endif
#include <opencv2/opencv.hpp>

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
// ����̒l���n�b�V���Ɏg���ő�lkHashNum�̒l�ɕϊ�����
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
// �z��̌`�̃n�b�V���C���f�b�N�X��ϊ�����
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
// com1��i�sj���n������Dgetcomb�p
{
	com1[i][j] = n;
}

void SetCom2( int i, int j, int n )
// com2��i�sj���n������Dgetcomb�p
{
	com2[i][j] = n;
}

int IsDat( char *str )
// str�̖�����.dat�����ׂ�
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
// str�̖�����.txt�����ׂ�
{
	int i, last_dot = -1;
	
	for ( i = 0; str[i] != '\0'; i++ ) {
		if ( str[i] == '.' )	last_dot = i;
	}
	if ( last_dot < 0 )	return 0;
	if ( !strcmp( str+last_dot+1, "txt" ) || !strcmp( str+last_dot+1, "TXT" ) )	return 1;
	else	return 0;
}

int RetrieveNN52( cv::Point *ps, double *areas, int num, int *nears[], cv::Size size, int *score, strDisc *disc, int *reg_nums, int *ret_time, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// �n�b�V�����猟������
{
	int i, j, max_score = 0, max_doc = 0;
//	DWORD start, end;
	int start, end;
	char line[kMaxLineLen];
	int pn;

	// �g�ݍ��킹�̍쐬
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �_�Ή��e�[�u���̍쐬
	ClearCorres();
	// flag2�̏�����
	ClearFlag2();
//	start = timeGetTime();
	start = GetProcTimeMicroSec();
	if ( eRotateOnce )
		CalcScore4( ps, areas, num, nears, score, disc, hash, hash2, hash3 );	// ���� **CalcScore4�ɕύX** 07/05/29
	else
		CalcScore3( ps, areas, num, nears, score, disc, hash, hash2, hash3 );
//	CheckByPPVar( ps, num, score );	// �ˉe�ϊ��p�����[�^�̕��U�Ń`�F�b�N
//	end = timeGetTime();
	end = GetProcTimeMicroSec();
//	printf("retrieval time : %d\n", end - start);
	if ( !eExperimentMode )	printf("retrieval time : %d��s\n", end - start);
	*ret_time = end - start;
	for ( i = 0; i < eDbDocs; i++ ) {
		score[i] = (int)((double)score[i] - (double)reg_nums[i] * eProp);	// CBDAR�ł̕␳
	}

	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	return max_doc;
}

int RetrieveNN5Cor2(cv::Point *ps, double *areas, int num, int *nears[], cv::Size size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, double prop, int *reg_nums, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// �n�b�V�����猟������i�Ή��_��Ԃ��o�[�W�����j2
{
	int i, j, max_score = 0, max_doc = 0, second_score = 0, second_doc = 0;
	int start, end;
	char line[kMaxLineLen];
	int pn;
	char pattr[kMaxPointNum];

	start = GetProcTimeMiliSec();
	memset( (void *)corres, 0xff, sizeof(int) * num * kMaxDocNum );
	memset( (void *)flag2, 0x00, sizeof(char) * eDbDocs * kMaxPointNum );
	end = GetProcTimeMiliSec();
//	printf("ret_init : %dms\n", end - start);
	start = GetProcTimeMiliSec();
	if ( eRotateOnce )	CalcScore4(ps, areas, num, nears, score, disc, hash, hash2, hash3);	// ����
	else				CalcScore3(ps, areas, num, nears, score, disc, hash, hash2, hash3);
	end = GetProcTimeMiliSec();
//	printf("ret_main : %dms\n", end - start);
//	eProp = 0.0;	// �X�R�A�����[���ɂ���
	for ( i = 0; i < eDbDocs; i++ ) {
		score[i] = (int)((double)score[i] - (double)reg_nums[i] * prop);	// CBDAR�ł̕␳
	}
	// ���[���ő�̂��̂�T��
	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	// �Ή��_���i�[����
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

int RetrieveNN5CorAll(cv::Point *ps, double *areas, int num, int *nears[], cv::Size size, int *score, int pcors[][kMaxPointNum][2], int *pcornums, strDisc *disc, int *reg_nums, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// �n�b�V�����猟������i���ׂĂ̕����ւ̑Ή��_��Ԃ��o�[�W�����j
{
	int i, j, max_score = 0, max_doc = 0, second_score = 0, second_doc = 0;
	int start, end;
	char line[kMaxLineLen];
	int pn;
	char pattr[kMaxPointNum];

	start = GetProcTimeMiliSec();
	memset( (void *)corres, 0xff, sizeof(int) * num * kMaxDocNum );
	memset( (void *)flag2, 0x00, sizeof(char) * eDbDocs * kMaxPointNum );
	end = GetProcTimeMiliSec();
//	printf("ret_init : %dms\n", end - start);
	start = GetProcTimeMiliSec();
	if ( eRotateOnce )	CalcScore4(ps, areas, num, nears, score, disc, hash, hash2, hash3);	// ����
	else				CalcScore3(ps, areas, num, nears, score, disc, hash, hash2, hash3);
	end = GetProcTimeMiliSec();
//	printf("ret_main : %dms\n", end - start);
//	eProp = 0.0;	// �X�R�A�����[���ɂ���
	for ( i = 0; i < eDbDocs; i++ ) {
		score[i] = (int)((double)score[i] - (double)reg_nums[i] * eProp);	// CBDAR�ł̕␳
	}
	// ���[���ő�̂��̂�T��
	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	// �Ή��_���i�[����
	for ( i = 0; i < kMaxDocNum; i++ )	pcornums[i] = 0;
	for ( i = 0; i < num; i++ ) {
		for ( j = 0; j < eDbDocs; j++ ) {
			if ( corres[i][j] >= 0 ) {
				pcors[j][pcornums[j]][0] = i;
				pcors[j][pcornums[j]][1] = corres[i][j];
				pcornums[j]++;
			}
		}
	}

	return max_doc;
}

int RetrieveNN5(cv::Point *ps, int num, int *nears[], cv::Size size, int *score, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// �n�b�V�����猟������
{
	int i, j, max_score = 0, max_doc = 0;
	strDisc disc;
//	DWORD start, end;
	int start, end;
	FILE *fp, *fpp;
	char line[kMaxLineLen];
	int pn;

	// �g�ݍ��킹�̍쐬
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �_�Ή��e�[�u���̍쐬
	for ( i = 0; i < kMaxPointNum; i++ ) {
		for ( j = 0; j < kMaxDocNum; j++ ) {
			corres[i][j] = -1;
		}
	}
	// flag2�̏�����
	for ( i = 0; i < kMaxDocNum; i++ ) {
		for ( j = 0; j < kMaxPointNum; j++ ) {
			flag2[i][j] = 0;
		}
	}
	LoadDisc( eDiscFileName, &disc );	// ���U���e�[�u���̍쐬
//	start = timeGetTime();
	start = GetProcTimeMicroSec();
	if ( eRotateOnce )	CalcScore4( ps, NULL, num, nears, score, &disc, hash, hash2, hash3 );
	else				CalcScore3( ps, NULL, num, nears, score, &disc, hash, hash2, hash3 );	// ����
//	CheckByPPVar( ps, num, score );	// �ˉe�ϊ��p�����[�^�̕��U�Ń`�F�b�N
//	end = timeGetTime();
	end = GetProcTimeMicroSec();
//	printf("retrieval time : %d\n", end - start);
	printf("retrieval time : %d��s\n", end - start);
	fpp = fopen( "prob.txt", "w" );
	fp = fopen( ePNFileName, "r" );
	for ( i = 0; i < kMaxDocNum; i++ ) {
		fgets(line, kMaxLineLen, fp );
		sscanf(line, "%d", &pn);
		fprintf(fpp, "%d,%d\n", pn, score[i]);
		score[i] = (int)((double)score[i] - (double)pn * eProp);	// CBDAR�ł̕␳
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

int RetrieveNN5Cor(cv::Point *ps, int num, int *nears[], cv::Size size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// �n�b�V�����猟������i�Ή��_��Ԃ��o�[�W�����j
{
	int i, j, max_score = 0, max_doc = 0;
	int start, end;
//	strDisc disc;
	FILE *fp;
	char line[kMaxLineLen];
	int pn;

	start = GetProcTimeMiliSec();
	// �g�ݍ��킹�̍쐬
//	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �_�Ή��e�[�u���̍쐬
//	for ( i = 0; i < kMaxPointNum; i++ ) {
//		for ( j = 0; j < kMaxDocNum; j++ ) {
//			corres[i][j] = -1;
//		}
//	}
	memset( (void *)corres, 0xff, sizeof(int) * num * kMaxDocNum );
	// flag2�̏�����
//	for ( i = 0; i < kMaxDocNum; i++ ) {
//		for ( j = 0; j < kMaxPointNum; j++ ) {
//			flag2[i][j] = 0;
//		}
//	}
	memset( (void *)flag2, 0x00, sizeof(char) * eDbDocs * kMaxPointNum );
//	LoadDisc( eDiscFileName, &disc );	// ���U���e�[�u���̍쐬
	end = GetProcTimeMiliSec();
//	printf("ret_init : %dms\n", end - start);
	start = GetProcTimeMiliSec();
	if ( eRotateOnce )	CalcScore4(ps, NULL, num, nears, score, disc, hash, hash2, hash3);	// ����
	else				CalcScore3(ps, NULL, num, nears, score, disc, hash, hash2, hash3);	// ����
	end = GetProcTimeMiliSec();
//	printf("ret_main : %dms\n", end - start);
	fp = fopen( ePNFileName, "r" );
//	eProp = 0.0;	// �X�R�A�����[���ɂ���
	for ( i = 0; i < kMaxDocNum; i++ ) {
		fgets(line, kMaxLineLen, fp );
		sscanf(line, "%d", &pn);
		score[i] = (int)((double)score[i] - (double)pn * eProp);	// CBDAR�ł̕␳
	}
	fclose(fp);
	// ���[���ő�̂��̂�T��
	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	// �Ή��_���i�[����
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

int RetrieveAndRecover(cv::Point *ps, int num, int *nears[], cv::Size size, char *img_fname, int mode, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// �n�b�V�����猟������
{
	int score[kMaxDocNum], i, max_score = 0, max_doc = 0;
//	DWORD start, end;
	cv::Mat img, exp;
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
			img = cv::imread(img_fname, cv::IMREAD_GRAYSCALE);
            RecovPT(ps, num, max_doc, size, img);
			break;
		case RET_MP_MODE:
			img = cv::imread(img_fname, cv::IMREAD_GRAYSCALE);
			exp = cv::Mat::zeros(size, CV_8UC1);
			cv::resize(img, exp, exp.size(), 0, 0, cv::INTER_CUBIC);
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

	// flag2�̏�����
	for ( i = 0; i < kMaxDocNum; i++ ) {
		for ( j = 0; j < kMaxPointNum; j++ ) {
			flag2[i][j] = 0;
		}
	}
}

void ClearCorres( void )
{
	int i, j;

	// �_�Ή��e�[�u���̍쐬
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

void CalcScore3(cv::Point *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// �e�����̓��_���v�Z����3
// 07/07/23 ��]�������胂�[�h�݂̂ɕύX
{
	char *hindex, *hindex_area;
	int h, i=0, j, k, l, st, tmp, max_vote = 0;
	int *idx, *idxcom1, *idxcom2;
	double cr = 0.0;
	strHList *hl;
	strHList2 *hl2;
	strRandPs randps[kMaxPointNum];
	HENTRY *phe;

	// �ł��؂�֌W
	if ( eTerminate != TERM_NON ) {
		for ( i = 0; i < num; i++ ) {	// �_�̃C���f�b�N�X�Ɨ������i�[
			randps[i].pindex = i;
			randps[i].rand_value = rand();
		}
		// �\�[�g
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
			case TERM_NON:	// �ł��؂�Ȃ�
				i = h;
				break;
			case TERM_PNUM:	// �����_��
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
		for ( j = 0; j < eNumCom1; j++ ) {	// �en�_�̑g�ݍ��킹�ɂ���
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];

			for ( st = 0; st < eGroup2Num; st++ ) {	// �e�J�n�_�ɂ���
                for ( k = 0; k < eNumCom2; k++ ) {	// �em�_�̑g�ݍ��킹�ɂ���
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
//							cr = CalcAngleFromThreePoints(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);	// �p�x���v�Z
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
				if ( eTerminate == TERM_VNUM && max_vote >= eTermVNum )	goto end_calcscore3_loop;	// ���[���ɂ��ł��؂�
				// ��]
				tmp = idxcom1[0];
				for ( k = 0; k < eGroup2Num-1; k++ )	idxcom1[k] = idxcom1[k+1];
				idxcom1[eGroup2Num-1] = tmp;
			}
		}
//		CalcPointCor( i );
	}
end_calcscore3_loop:
//	for ( i = 0; i < kMaxDocNum; i++ ) {	// �X�R�A�̊m�F
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


void CalcScore4(cv::Point *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// �e�����̓��_���v�Z����4�i��]��񂾂����ǔŁj
{
	char *hindex, **hindex_array, *inv_array, *hindex_area;
	int h, i=0, j, k, l, st, tmp, max_vote = 0, src, dst;
	int *idx, *idxcom1, *idxcom2, *idxcom1bak;
	double cr = 0.0;
	strHList *hl;
	strHList2 *hl2;
	strRandPs randps[kMaxPointNum];
	HENTRY *phe;

	// �ł��؂�֌W
	if ( eTerminate != TERM_NON ) {
		for ( i = 0; i < num; i++ ) {	// �_�̃C���f�b�N�X�Ɨ������i�[
			randps[i].pindex = i;
			randps[i].rand_value = rand();
		}
		// �\�[�g
		qsort(randps, num, sizeof(strRandPs), (int(*)(const void*, const void*))randps_compare);
	}

	hindex = (char *)calloc( eNumCom2, sizeof(char) );
	hindex_area = (char *)calloc( eGroup2Num, sizeof(char) );
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
	// ��]�s�ρE���ǔ�
	inv_array = (char *)calloc( eGroup2Num, sizeof(char) );

	for ( i = 0; i < kMaxDocNum; i++ )	score[i] = 0;
	for ( h = 0; h < num; h++ ) {
		switch ( eTerminate ) {
			case TERM_NON:	// �ł��؂�Ȃ�
				i = h;
				break;
			case TERM_PNUM:	// �����_��
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
		for ( j = 0; j < eNumCom1; j++ ) {	// �en�_�̑g�ݍ��킹�ɂ���
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m�_�̒��o
			// �J�n�_�̌���
			for ( st = 0; st < eGroup2Num; st++ ) {	// �e�J�n�_�ɂ���
				switch ( eInvType ) {	// ����f�_����s�ϗʂ��v�Z
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
				inv_array[st] = Con2DiscCR( cr, disc );	// ���U�����ĕۑ�
			}
			st = FindStartPoint( inv_array );	// �s�ϗʂ���J�n�_��������
			// st���J�n�_�Ƃ��ĉ�]������
			for ( k = 0; k < eGroup2Num; k++ ) {
				idxcom1bak[k] = idxcom1[k];
			}
			for ( k = 0; k < eGroup2Num; k++ ) {
				idxcom1[k] = idxcom1bak[(k+st) % eGroup2Num];
			}
			// �ȏ�ŉ�]�s�Ϗ����͊���
			for ( k = 0; k < eNumCom2; k++ ) {	// �em�_�̑g�ݍ��킹�ɂ���
				// ���FeIncludeCentre�̒��S�_���܂ޏ����͊O����
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
			if ( eTerminate == TERM_VNUM && max_vote >= eTermVNum )	goto end_calcscore4_loop;	// ���[���ɂ��ł��؂�
		}
	}
end_calcscore4_loop:
//	for ( i = 0; i < kMaxDocNum; i++ ) {	// �X�R�A�̊m�F
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

int HaveEnoughSimilarVector( cv::Point start, cv::Point end, int idx )
// ����idx�ɂ�����start�̎��͂�start->end�x�N�g���Ƌ߂��x�N�g�������_���\������
{
	int count = 0;
	cv::Point v, p, vp;
	strCorMap *cm;

	v.x = end.x - start.x;
	v.y = end.y - start.y;
	for ( cm = GetCM(idx); cm != NULL; cm = cm->next ) {
		p.x = cm->x;
		p.y = cm->y;
		if ( cm->cor_num > 1 )	continue;	// �Ƃ肠����2�_�ȏ゠��Ƃ��̓p�X
//		printf("%f\n", GetPointsDistance( start, p ));
		if ( GetPointsDistance( start, p ) > kDistNeighbor ) {	// ���͂̓_�łȂ���΃p�X
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

/* DrawCor ���o�[�W�����i�g���ĂȂ��H�j
void DrawCor( void )
// �_�ƑΉ��_�̊֌W���摜�ɕ`�悷��
{
	int i;
	cv::Mat img = cv::Mat::zeros(cv::Size(3139, 4500), CV_8UC3);
	cv::Point p1, p2;
	strCorMap *cm;
	strCorList *cl;

	for ( i = 3; i < 4; i++ ) {
		img.setTo(cv::Scalar::all(0));
		for ( cm = GetCM(i); cm != NULL; cm = cm->next ) {
			p1.x = cm->x;
			p1.y = cm->y;
			cv::circle(img, p1, 4, cWhite, 1, 8, 0);
//			if ( cm->cor_num > 1 )	continue;
			for ( cl = cm->cor; cl != NULL; cl = cl->next) {
                p2.x = cl->x;
                p2.y = cl->y;
                cv::line(img, p1, p2, cWhite, 2, 8, 0);
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
#define	kDrawCorScale	(0.5)
#define	kDrawCorRectThick	(4)
#define	kDrawCorPtRad	(4)
#define	kDrawCorLineThick	(4)

void DrawCor( cv::Point *ps, int num, cv::Size img_size, int res, cv::Point *corps, int cornum, cv::Size corsize, int pcor[][2], int pcornum )
{
	int i, width_all, height_all;
	cv::Mat img;
	
	width_all = kDrawCorHMargin * 2 + kDrawCorSpace + img_size.width + (int)(corsize.width * kDrawCorScale);
	height_all = kDrawCorVMargin * 2 + max( img_size.height, (int)(corsize.height * kDrawCorScale) );
//	printf( "%d,%d\n", width_all, height_all );
	img = cv::Mat::zeros(cv::Size( width_all, height_all ), CV_8UC3);
	img.setTo(cWhite);	// ���œh��Ԃ�

	// ��������̘g��`��
	cv::rectangle( img, cv::Point( kDrawCorHMargin, kDrawCorVMargin ), \
		cv::Point( kDrawCorHMargin + img_size.width, kDrawCorVMargin + img_size.height ), \
		cBlack, kDrawCorRectThick, cv::LINE_AA, 0 );
	
	// ��������̓����_�̕`��
	for ( i = 0; i < num; i++ ) {
		cv::circle( img, cv::Point( kDrawCorHMargin + ps[i].x, kDrawCorVMargin + ps[i].y ), kDrawCorPtRad, cBlack, -1, cv::LINE_AA, 0 );
	}
	// �o�^�摜�̘g��`��
	cv::rectangle( img, cv::Point( kDrawCorHMargin + img_size.width + kDrawCorSpace, kDrawCorVMargin), \
		cv::Point( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corsize.width * kDrawCorScale), kDrawCorVMargin + (int)(corsize.height * kDrawCorScale) ), \
		cBlack, kDrawCorRectThick, cv::LINE_AA, 0 );
	// �o�^�摜�̓����_�̕`��
	for ( i = 0; i < cornum; i++ ) {
		cv::circle( img, cv::Point( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corps[i].x * kDrawCorScale), kDrawCorVMargin + (int)(corps[i].y * kDrawCorScale) ), \
		kDrawCorPtRad, cBlack, -1, cv::LINE_AA, 0 );
	}
	// �Ή��֌W�̕`��
	for ( i = 0; i < pcornum; i++ ) {
		cv::line( img, cv::Point( kDrawCorHMargin + ps[pcor[i][0]].x, kDrawCorVMargin + ps[pcor[i][0]].y ), \
		cv::Point( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corps[pcor[i][1]].x * kDrawCorScale), kDrawCorVMargin + (int)(corps[pcor[i][1]].y * kDrawCorScale) ), \
		cRed, kDrawCorLineThick, cv::LINE_AA, 0 );
	}
	OutPutImage( img );
}

void DrawPointAttribute(  cv::Point *ps, int num, cv::Size img_size, char *pattr )
// �_�̐����E���E���Ή���`��
{
	int i;
	cv::Mat img;

	img = cv::Mat::zeros(img_size, CV_8UC3);
	cv::bitwise_not( img, img );
	for ( i = 0; i < num; i++ ) {
		switch ( pattr[i] ) {
			case 1:	// ����
				cv::circle( img, ps[i], 10, cRed, -1, cv::LINE_AA, 0 );
				break;
			case -1: // �s����
				cv::circle( img, ps[i], 10, cBlue, -1, cv::LINE_AA, 0 );
				break;
			default: // ���Ή�
				cv::circle( img, ps[i], 10, cBlack, -1, cv::LINE_AA, 0 );
				break;
		}
	}
	OutPutImage( img );
}
	
void CalcCRDiscriptorNN5(cv::Point *ps, int num, int *nears[])
// �ߖT5�_�ŕ�����v�Z����
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

void CalcOrderCW( int pt, cv::Point *ps, int *nears[], int idx[] )
// ���v���ɋߖT�_��z��ɓ����
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

void CalcOrderCWN( int pt, cv::Point *ps, int *nears[], int idx[], int num )
// ���v���ɋߖT�_��z��ɓ����i���_�o�[�W�����j
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

void CalcOrderCWFromNearest( int pt, cv::Point *ps, int *nears[], int idx_ret[] )
// ���v���ɋߖT�_��z��ɓ����i�ŋߖT�_���J�n�_�Ƃ���j
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

double CalcCR5( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 )
// ���ʏ��5�_�ŕ�����v�Z����
{
	cv::Point p1325, p1425;

	CalcInterPoint( p1, p3, p2, p5, &p1325 );
	CalcInterPoint( p1, p4, p2, p5, &p1425 );

	return CalcCR(p2, p1325, p1425, p5);
}

double CalcJI5( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 )
// ���ʏ��5�_�ŕ�����v�Z����
{
	cv::Point p1325, p1425;

	CalcInterPoint( p1, p3, p2, p5, &p1325 );
	CalcInterPoint( p1, p4, p2, p5, &p1425 );

	return CalcJI(p2, p1325, p1425, p5);
}

int CalcPerm(int n, int r, int i, int rslt[])
// n�ir���̏����i�Ԗڂ����߂�rslt�ɓ����Di�Ԗڂ��_���Ȃ�0��Ԃ��D
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

void CalcCRDiscriptorNN5Test(cv::Point *ps, int num, int *nears[])
// �ߖT5�_�ŕ�����v�Z����
{
	cv::Point p1, p2, p3, p4, p5, p1325, p1425;
	cv::Mat img = cv::Mat::zeros(cv::Size(150, 150), CV_8UC3);

	img.setTo(cv::Scalar::all(0));

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
	cv::circle( img, p1, 2, cWhite, -1, 1, 0 );	// �_�̕`��
	cv::circle( img, p2, 2, cWhite, -1, 1, 0 );	// �_�̕`��
	cv::circle( img, p3, 2, cWhite, -1, 1, 0 );	// �_�̕`��
	cv::circle( img, p4, 2, cWhite, -1, 1, 0 );	// �_�̕`��
	cv::circle( img, p5, 2, cWhite, -1, 1, 0 );	// �_�̕`��
	cv::circle( img, p1325, 2, cRed, -1, 1, 0 );	// �_�̕`��
	cv::circle( img, p1425, 2, cRed, -1, 1, 0 );	// �_�̕`��
	OutPutImage(img);
}

void CalcInterPoint( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point *p1234 )
// �_p1��p2�̒ʂ钼���C�_p3��p4�̒ʂ钼����2�����̌�_�����߂�
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
// �n�b�V�����\�z�i�ǉ��j����
{
	int n = 0;
#ifdef	WIN32
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

	fp = fopen(eDBCorFileName, "r");	// �t�@�C���Ɣԍ��̑Ή��t�@�C��
	for ( n = 0; fgets(line, kMaxLineLen, fp) != NULL; n++ );
	fclose(fp);
	LoadDisc( eDiscFileName, &disc );	// ���U���t�@�C���̓ǂݍ���
	LoadHash(disc.num);

	fp = fopen(eDBCorFileName, "a");	// �t�@�C���Ɣԍ��̑Ή��t�@�C��
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );

#ifdef	WIN32
	hSearch=FindFirstFile(eHashSrcPath, &fFind); /* ���摜�f�B���N�g���̒T���J�n */
	fprintf(fp, "%d %s\n", n, fFind.cFileName);
	printf("%s : %d\n", fFind.cFileName, n);
//	strcpy(fname, eHashSrcDir);
	GetDir( eHashSrcPath, kMaxPathLen, fname );	// �f�B���N�g���̎擾
	strcat(fname, fFind.cFileName);	// ���摜�t�@�C���̐�΃p�X�̍쐬
	ConstructHashSub( fname, n++, mode, &disc);	// ���摜�t�@�C���Ńn�b�V�����\�z
//	puts(fFind.cFileName);
    while (FindNextFile(hSearch,&fFind)) { /* �S�t�@�C�������� */
//		puts(fFind.cFileName);
		fprintf(fp, "%d %s\n", n, fFind.cFileName);
		printf("%s : %d\n", fFind.cFileName, n);
//		strcpy(fname, eHashSrcDir);
		GetDir( eHashSrcPath, kMaxPathLen, fname );	// �f�B���N�g���̎擾
		strcat(fname, fFind.cFileName);	// ���摜�t�@�C���̐�΃p�X�̍쐬
		ConstructHashSub( fname, n++, mode, &disc);	// ���摜�t�@�C���Ńn�b�V�����\�z
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

	SaveHash();	// �n�b�V���̕ۑ�
	fclose(fp);
}
#endif

int ConstructHash2( int doc_num, cv::Point **pss, double **areass, int *nums, strDisc *disc, strHList ***ptr_hash, strHList2 ***ptr_hash2, HENTRY **ptr_hash3 )
// �n�b�V�����\�z����2
{
	int i, **nears, ret = 1;
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

	if ( eNoHashList ) {
		hash3 = InitHash3();	// ���X�g�Ȃ����[�h
		if (hash3 == NULL)	ret = 0;
	} else if ( eCompressHash ) {
		hash2 = InitHash2();	// ���k���[�h
		if (hash2 == NULL)	ret = 0;
	} else {
		hash = InitHash();	// ���k�Ȃ����[�h�Ńn�b�V���̏�����
		if (hash == NULL)		ret = 0;
	}
	if ( ret == 0 ) {
		fprintf( stderr, "error: hash allocation error\n" );
		return 0;
	}

	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );	// �g�ݍ��킹�̍쐬

	for ( i = 0; i < doc_num; i++ ) {
		fprintf( stderr, "(%d/%d)[%ld]\n", i+1, doc_num, allocated_hash );
		MakeNearsFromCentres( pss[i], nums[i], &nears );	// �e�_�ɂ��ď��kNears�̋ߖT�_�̃C���f�b�N�X�����߁Cnears�Ɋi�[����
		if ( eRotateOnce )
			ConstructHashSubSub2( pss[i], areass[i], nums[i], nears, i, disc, hash, hash2, hash3 );	// �n�b�V�����\�z�̃T�u
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
// �n�b�V�����\�z����
{
	int n = 0, num;
#ifdef	WIN32
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
		fp = fopen(eDBCorFileName, "w");	// �t�@�C���Ɣԍ��̑Ή��t�@�C��
	InitHash();	// �n�b�V���̏�����
	LoadDisc( eDiscFileName, &disc );	// ���U���t�@�C���̓ǂݍ���
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
#ifdef	WIN32
	if ( mode == USEPF_MODE ) {
		sprintf(pf_path, "%s*%s", ePFPrefix, ePFSuffix);
		hSearch = FindFirstFile( pf_path, &fFind );	// �ŏ��̃t�@�C���������i�t�@�C�����͎g�킸�ɐ������g���j
		ret = TRUE;
		for ( i = 0; ret != FALSE; i++ ) {
			ConstructHashSub( NULL, i, mode, &disc );
			ret = FindNextFile( hSearch, &fFind );	// ���̃t�@�C��������
		}
		FindClose( hSearch );
		eDbDocs = i;
	}
	else {
		fprintf( stderr, "���̂Ƃ���Windows�ł��̃��[�h�͑Ή����Ă��܂���\n" );
	}
//	hSearch=FindFirstFile(eHashSrcPath, &fFind); /* ���摜�f�B���N�g���̒T���J�n */
//	fprintf(fp, "%d %s\n", n, fFind.cFileName);
//	printf("%s : %d\n", fFind.cFileName, n);
////	strcpy(fname, eHashSrcDir);
//	GetDir( eHashSrcPath, kMaxPathLen, fname );	// �f�B���N�g���̎擾
//	strcat(fname, fFind.cFileName);	// ���摜�t�@�C���̐�΃p�X�̍쐬
//	ConstructHashSub( fname, n++, mode, &disc);	// ���摜�t�@�C���Ńn�b�V�����\�z
////	puts(fFind.cFileName);
//    while (FindNextFile(hSearch,&fFind)) { /* �S�t�@�C�������� */
////		puts(fFind.cFileName);
//		fprintf(fp, "%d %s\n", n, fFind.cFileName);
//		printf("%s : %d\n", fFind.cFileName, n);
////		strcpy(fname, eHashSrcDir);
//		GetDir( eHashSrcPath, kMaxPathLen, fname );	// �f�B���N�g���̎擾
//		strcat(fname, fFind.cFileName);	// ���摜�t�@�C���̐�΃p�X�̍쐬
//		ConstructHashSub( fname, n++, mode, &disc);	// ���摜�t�@�C���Ńn�b�V�����\�z
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
		
//	SaveHash();	// �n�b�V���̕ۑ��@�ꎞ�I�ɏ���
	if ( mode != USEPF_MODE )	fclose(fp);
}

int ConstructHashSub( char *fname, int n, int mode, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// �n�b�V�����\�z����i�T�u�j
{
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	cv::Point *ps = 0;
	cv::Size img_size;
	cv::Mat img;
	FILE *fp;

	img_size.width = 0;
	img_size.height = 0;
	
	strcpy(pfname, ePFPrefix);
	sprintf(nstr, "%d", n);
	strcat(pfname, nstr);
	strcat(pfname, ePFSuffix);
	if ( mode == USEPF_MODE ) {	// �|�C���g�t�@�C���𗘗p
		if ( (fp = fopen(pfname, "r")) == NULL )	return 0;
		ps = (cv::Point *)calloc(kMaxPointNum, sizeof(cv::Point));
		num = 0;
		fgets(line, kMaxLineLen, fp);	// skip size of image
		sscanf( line, "%d,%d", &img_size.width, &img_size.height );
		while ( fgets(line, kMaxLineLen, fp) != NULL && num < kMaxPointNum ) {
			sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
			num++;
		}
		fclose(fp);
	} else {	// �|�C���g�t�@�C���łȂ��i�摜 or Dat or Txt�j
		if ( IsDat( fname ) || IsTxt( fname ) ) {	// dat�t�@�C���i�|�C���g�t�@�C���jor txt�t�@�C���i�T�C�Y�Ȃ��j
			if ( (fp = fopen(fname, "r")) == NULL )	return 0;
			ps = (cv::Point *)calloc(kMaxPointNum, sizeof(cv::Point));
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
		else {	// �摜�t�@�C��
			if ( mode == CONNECTED_MODE ) {	// �P��̏d�S
				img = GetConnectedImage( fname, CONST_HASH_MODE );
				num = MakeCentresFromImage( &ps, img, &img_size, NULL );
			} else if ( mode == ENCLOSED_MODE ) {	// �͂܂ꂽ�̈�
				img = cv::imread(fname, cv::IMREAD_GRAYSCALE);
				num = MakeFeaturePointsFromImage( &ps, img, &img_size );
			}
		}
		SavePointFile(pfname, ps, num, &img_size, NULL);
	}

	if ( num == 0 ) return 0;
//	printf("%d, %d\n", img_size.width, img_size.height );
//	DrawPoints( img_size, ps, num );
	MakeNearsFromCentres( ps, num, &nears );	// �e�_�ɂ��ď��kNears�̋ߖT�_�̃C���f�b�N�X�����߁Cnears�Ɋi�[����

	ConstructHashSubSub( ps, NULL, num, nears, n, disc );

	ReleaseCentres( ps );
	ReleaseNears( nears, num );
//	printf("%d\n", num );
	return num;
}
#endif

void ConstructHashSubSub(cv::Point *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// �n�b�V�����\�z����i�T�u�̃T�u�j
// 07/07/23 ��]��������݂̂ɕύX
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
		for ( j = 0; j < eNumCom1; j++ ) {	// �e8�_�̑g�ݍ��킹�ɂ���
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];

			for ( st = 0; st < eGroup2Num; st++ ) {	// �e�J�n�_�ɂ���
				for ( k = 0; k < eNumCom2; k++ ) {	// �e5�_�̑g�ݍ��킹�ɂ���
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
//							cr = CalcAngleFromThreePoints(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);	// �p�x���v�Z
							cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
						default:
							break;
					}
					hindex[k] = Con2DiscCR( cr, disc );
				}
				if ( !eRotateOnce ) {	// ��]�������胂�[�h�i�o�^�̎��͓K���ȂP�ʂ�̂݁j
					if ( eUseArea )	CalcHindexArea( idxcom1, areas, hindex_area );
					if ( eNoHashList )			AddHash3( hindex, hindex_area, disc->num, n, i, hash3 );	// ���X�g�Ȃ����[�h
					else if ( eCompressHash )	AddHash2( hindex, hindex_area, disc->num, n, i, hash2 );	// ���k���[�h�̏ꍇ
					else						AddHash( hindex, hindex_area, disc->num, n, i, hash );
					break;	// ��������͂��Ȃ�
				}
				// ��]
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

void ConstructHashSubSub2(cv::Point *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3)
// �n�b�V�����\�z����i�T�u�̃T�u�j����2�i��]��񂾂����ǔŁj
// 07/07/11 ��]��������i�o�^�͓K���Ȉ�ʂ�̂݁j�𓝍�
// ���ۂ͌������ɑ������肷��̂ŊJ�n�_���߂Ă��\��Ȃ����C
// ���ʂȏ����̏ȗ����Փ˂̉���̂��ߊJ�n�_�Ɋ�Â����בւ��������Ȃ�
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
	// ��]�s�ρE���ǔ�
	inv_array = (char *)calloc( eGroup2Num, sizeof(char) );

	for ( i = 0; i < num; i++ ) {
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// �en�_�̑g�ݍ��킹�ɂ���
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m�_�̒��o
			if ( eRotateOnce ) {	// ��]��ʂ胂�[�h�Ȃ�C�J�n�_�����߂ĕ��בւ���
				// �J�n�_�̌���
				for ( st = 0; st < eGroup2Num; st++ ) {	// �e�J�n�_�ɂ���
					switch ( eInvType ) {	// ����f�_����s�ϗʂ��v�Z
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
					inv_array[st] = Con2DiscCR( cr, disc );	// ���U�����ĕۑ�
				}
				st = FindStartPoint( inv_array );	// �s�ϗʂ���J�n�_��������
				// st���J�n�_�Ƃ��ĉ�]������
				for ( k = 0; k < eGroup2Num; k++ ) idxcom1bak[k] = idxcom1[k];
				for ( k = 0; k < eGroup2Num; k++ ) idxcom1[k] = idxcom1bak[(k+st) % eGroup2Num];
				// �ȏ�ŉ�]�s�Ϗ����͊���
			}
			for ( k = 0; k < eNumCom2; k++ ) {	// �em�_�̑g�ݍ��킹�ɂ���
				// ���FeIncludeCentre�̒��S�_���܂ޏ����͊O����
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
//						cr = CalcAngleFromThreePoints(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);	// �p�x���v�Z
						cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
					default:
						break;
				}
				hindex[k] = Con2DiscCR( cr, disc );
			}
			if ( eUseArea )	CalcHindexArea( idxcom1, areas, hindex_area );	// �ʐϓ����ʂ̌v�Z
			if ( eNoHashList )			AddHash3( hindex, hindex_area, disc->num, n, i, hash3 );	// ���X�g�Ȃ����[�h�̏ꍇ
			else if ( eCompressHash )	AddHash2( hindex, hindex_area, disc->num, n, i, hash2 );	// ���k���[�h�̏ꍇ
			else						AddHash( hindex, hindex_area, disc->num, n, i, hash );
			/*
			if ( eUseArea ) {
				CalcHindexArea( idxcom1, areas, hindex_area );
				AddHashArea( hindex, hindex_area, disc->num, n, i );
			}
			else {
				if ( eCompressHash )	AddHash2( hindex, disc->num, n, i );	// ���k���[�h�̏ꍇ
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

void SavePointFile( char *fname, cv::Point *ps, int num, cv::Size *size, double *areas )
// �_�f�[�^��ۑ�����
// �ʐς̕ۑ���ǉ� 06/01/13
{
	int i;
	char dir[kMaxPathLen], base[kMaxPathLen], area_fname[kMaxPathLen];
	FILE *fp;

	fp = fopen(fname, "w");
	fprintf(fp, "%d,%d\n", size->width, size->height);	// �擪�ɃT�C�Y��ۑ�����
	for ( i = 0; i < num; i++ )
		fprintf(fp, "%d,%d\n", ps[i].x, ps[i].y);
	fclose(fp);
	if (areas == NULL)	return;	// �ʐς��i�[����Ă��Ȃ�
	GetDir( fname, kMaxPathLen, dir );
	GetBasename( fname, kMaxPathLen, base );
	strcpy( area_fname, dir );
	strcat( area_fname, base );
	strcat( area_fname, ".are" );
	SaveAreaFile( area_fname, areas, num );
}

void SaveAreaFile( char *fname, double *areas, int num )
// �ʐς̔z��area����num�̎�����fname�ɏ����o��
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

int LoadPointFile( char *fname, cv::Point **ps0, cv::Size *size )
// �_�f�[�^��ǂݍ���
{
	int num;
	char line[kMaxLineLen];
	FILE *fp;
	cv::Point *ps;
	
	if ( (fp = fopen(fname, "r")) == NULL )	return 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size->width), &(size->height));
	ps = (cv::Point *)calloc(kMaxPointNum, sizeof(cv::Point));
	*ps0 = ps;
	num = 0;
	while ( fgets(line, kMaxLineLen, fp) != NULL && num < kMaxPointNum ) {
		sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
		num++;
	}
	fclose(fp);
	return num;
}

void LoadPointFileAll( cv::Point ***psall0, cv::Size **sizeall0, int **numall0 )
// �o�^�_�t�@�C�������ׂēǂݍ���
{
	cv::Point **psall;
	cv::Size *sizeall;
	int i, *numall;
	char fname[kMaxPathLen];
	
//	printf("%s\n%s\n%d\n", ePFPrefix, ePFSuffix, eDbDocs);
	psall = (cv::Point **)calloc( eDbDocs, sizeof(cv::Point *) );
	sizeall = (cv::Size *)calloc( eDbDocs, sizeof(cv::Size) );
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

void DrawNeighbor( cv::Size size, cv::Point *ps, int num, int *nears[] )
// �ߖT5�_��`�悷��
{
	int i, j, k, idx[1024]/*, hindex[10]*/, flag, fail[8];
	char txt[10];
//	double cr;
//	cv::Mat img = cv::Mat::zeros(size, CV_8UC3);
#ifdef USE073
	cv::Mat img = cv::imread("07s.bmp", cv::IMREAD_COLOR);
#else
	cv::Mat img = cv::imread("04s.bmp", cv::IMREAD_COLOR);
#endif
	cv::Scalar color;
	// CvFont removed in OpenCV 4.x
	cv::Point p[kPNum], mid;

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

//	img.setTo(cv::Scalar::all(0));
	for ( i = 0; i < num; i++ ) {
		color = cWhite;
//		cv::circle(img, ps[i], 2, color, 2, 8, 0);
		for ( flag = 0, j = 0; j < kPNum; j++ ) {
			if ( GetPointsDistance(ps[i], p[j]) < 10 ) {
				flag = 1;
				break;
			}
		}
		if ( !flag )	continue;
		
//		color = cWhite;
//		cv::circle(img, ps[i], 6, color, 2, 8, 0);
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
		for ( j = 0; j < eGroup1Num; j++ ) {	// �p�x���ɒ��S�����������
//			cv::line(img, ps[i], ps[idx[j]], color, 1, 8, 0);
			sprintf(txt, "%d", j+1);
			mid.x = (ps[idx[j]].x + ps[i].x)/2;
			mid.y = (ps[idx[j]].y + ps[i].y)/2;
			// cvInitFont removed - use cv::putText directly
//			cv::putText(img, txt, mid, cv::FONT_HERSHEY_PLAIN, 1.0, cGreen );
			cv::circle(img, ps[idx[j]], 8, cBlack, -1, 8, 0);
			cv::circle(img, ps[idx[j]], 8, cWhite, 2, 8, 0);
//			cv::circle(img, ps[idx[j]], 5, cWhite, 1, 8, 0);
		}

/*		for ( j = 0; j < eGroup1Num; j++ ) {
			cv::line(img, ps[i], ps[nears[i][j]], color, 1, 8, 0);
			sprintf(txt, "%d", j+1);
			mid.x = (ps[nears[i][j]].x + ps[i].x)/2;
			mid.y = (ps[nears[i][j]].y + ps[i].y)/2;
			// cvInitFont removed - use cv::putText directly
			cv::putText(img, txt, mid, cv::FONT_HERSHEY_PLAIN, 1.0, cGreen );
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
}

/*
void VoteByHL( strHList *hl )
// �n�b�V�����X�g��ǂ��vote����
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
		if (vp == NULL) {	// ������Ȃ������Ƃ�
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
// ���[���X�g��vote����
{
	strVoteList *vp;

	for ( vp = vl; vp != NULL; vp = vp->next ) {
		if ( vp->vote > kVoteThr )	score[vp->doc]++;
	}
}

void ClearVL( void )
// ���[���X�g���N���A����
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
// �n�b�V�����X�g���璼��vote����
// ����vote�������ōő�̂��̂�Ԃ�
{
	strHList *hp;
	static int max_vote = -1;
    
	for ( hp = hl; hp != NULL; hp = hp->next) {
//		if ( memcmp(hp->idx, idx, eNumCom2) == 0 && flag1[hp->doc] == 0 && flag2[hp->doc][hp->point] == 0) {
		// �Ή��֌W�`�F�b�N && ( �x�N�g���`�F�b�N�Ȃ� || �x�N�g����v )
		// 07/07/04 !eVectorCheck��eNoCompareInv�ɕύX
		if ( flag1[hp->doc] == 0 && /* �Ή��_�t���O�`�F�b�N */ \
//			 flag2[hp->doc][hp->point] == 0 ) {
			 flag2[hp->doc][hp->point] == 0 && \
			 ( eNoCompareInv || memcmp(hp->idx, idx, eNumCom2) == 0 ) /* �����ʂ̔�r */ ) {
//			 ( eNoCompareInv || !eUseArea || memcmp(hp->idx + eNumCom2, idx_area, eGroup2Num) ) /* �ʐϓ����ʂ̔�r */ \
//			) {
			score[hp->doc]++;	// ���[
			if ( score[hp->doc] > max_vote )	max_vote = score[hp->doc];	// �ő�Ȃ�X�V
			corres[p][hp->doc] = hp->point;
			flag1[hp->doc] = 1;	// hp->doc�͓��[�ς�
			flag2[hp->doc][hp->point] = 1;	// hp-doc��hp->point�͓��[�ς�
		}
	}
	return max_vote;
}

int VoteDirectlyByHL2( strHList2 *hl, int *score, char *idx, char *idx_area, int p )
// �n�b�V�����X�g���璼��vote����
// ����vote�������ōő�̂��̂�Ԃ�
// ���k��
{
	strHList2 *hp;
	static int max_vote = -1;
	unsigned long doc, point;
	char *r, *o;
    
	r = (char *)calloc( eNumCom2, sizeof(char) );
	o = (char *)calloc( eGroup2Num, sizeof(char) );
	for ( hp = hl; hp != NULL; hp = hp->next) {
//		if ( memcmp(hp->idx, idx, eNumCom2) == 0 && flag1[hp->doc] == 0 && flag2[hp->doc][hp->point] == 0) {
		// �Ή��֌W�`�F�b�N && ( �x�N�g���`�F�b�N�Ȃ� || �x�N�g����v )
		ReadHList2Dat( hp->dat, &doc, &point, r, o );
//		printf("%d, %d\n", doc, point );
		// 07/07/04 !eVectorChech�������CeUseArea�����̓����Ă��邩�͕s��
//		if ( flag1[doc] == 0 && flag2[doc][point] == 0 && ( eUseArea || eNoCompareInv  || memcmp(r, idx, eNumCom2) == 0 ) ) {
		// 07/07/20 �����𒲐�
		if ( flag1[doc] == 0 && /* �t���O�`�F�b�N */ \
			flag2[doc][point] == 0 && \
			( eNoCompareInv || memcmp(r, idx, eNumCom2) == 0 ) && /* �����ʂ̔�r */ \
			( eNoCompareInv || !eUseArea || memcmp(o, idx_area, eGroup2Num ) ) /* �ʐϓ����ʂ̔�r */ \
			) {
			score[doc]++;	// ���[
			if ( score[doc] > max_vote )	max_vote = score[doc];	// �ő�Ȃ�X�V
			corres[p][doc] = point;
			flag1[doc] = 1;	// hp->doc�͓��[�ς�
			flag2[doc][point] = 1;	// hp-doc��hp->point�͓��[�ς�
		}
	}
	free( r );
	free( o );
	return max_vote;
}

int VoteByHashEntry( HENTRY *phe, int *score, int p )
// �n�b�V���e�[�u���̃G���g������vote����
// ���X�g�Ȃ��o�[�W����
{
	static int max_vote = -1;
	int doc, point;

	if ( *phe == kFreeEntry || *phe == kInvalidEntry )	return max_vote;	// �󂫂܂��͖����ȃG���g��
	ReadHashEntry( phe, &doc, &point );
	if ( flag1[doc] == 0 && flag2[doc][point] == 0 ) {
		score[doc]++;	// ���[
		if ( score[doc] > max_vote )	max_vote = score[doc];	// �ő�Ȃ�X�V
		corres[p][doc] = point;
		flag1[doc] = 1;	// hp->doc�͓��[�ς�
		flag2[doc][point] = 1;	// hp-doc��hp->point�͓��[�ς�
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

void DrawCross( cv::Mat img, cv::Point center, int length, cv::Scalar color, int thickness )
// img��center�Ɂ~��`��
{
	cv::Point p1, p2;

	p1.x = center.x - length;
	p1.y = center.y - length;
	p2.x = center.x + length;
	p2.y = center.y + length;
	cv::line( img, p1, p2, color, thickness, 8, 0 );
	p1.x = center.x + length;
	p1.y = center.y - length;
	p2.x = center.x - length;
	p2.y = center.y + length;
	cv::line( img, p1, p2, color, thickness, 8, 0 );
}

void DrawP2P( cv::Point *ps, int num, int cor_doc, cv::Size size )
// �_����_�֐�������
{
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	int ori_num, i;
	FILE *fp;
	cv::Point corps[kMaxPointNum];
	cv::Mat img;
	cv::Size size2, size_all;

	// �f�[�^�x�[�X�̓_�t�@�C����������ĊJ��
	strcpy(pfname, ePFPrefix);
//	itoa(cor_doc, nstr, 10);
	sprintf(nstr, "%d", cor_doc);
	strcat(pfname, nstr);
	strcat(pfname, ePFSuffix);
	fp = fopen(pfname, "r");
	// �_�t�@�C���̃f�[�^��corps�Ɋi�[����
	ori_num = 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size2.width), &(size2.height));	// 1�s�ڂ̓T�C�Y
	while( fgets(line, kMaxLineLen, fp) != NULL ) {
		sscanf(line, "%d,%d", &(corps[ori_num].x), &(corps[ori_num].y));
		ori_num++;
	}
	fclose(fp);
	// �`�悷��摜�̍쐬�Ƃ�
	size_all.width = size.width + size2.width;
	size_all.height = max(size.height, size2.height);
	img = cv::Mat::zeros(size_all, CV_8UC3);
	img.setTo(cv::Scalar::all(0));

	cv::rectangle( img, cv::Point(1, 1), cv::Point(size.width-5, size.height-5), cWhite, 3, 8, 0 );
	// �S�_�̕`��i�N�G���j
	for ( i = 0; i < num; i++ )	cv::circle( img, ps[i], 8, cWhite, 3, 8, 0);
	cv::rectangle( img, cv::Point(size.width, 1), cv::Point(size.width + size2.width-5, size2.height-5), cWhite, 3, 8, 0);
	// �S�X�̕`��i�f�[�^�x�[�X�j
	for ( i = 0; i < ori_num; i++ )	DrawCross( img, cv::Point(size.width + corps[i].x, corps[i].y), 8, cWhite, 3 );
	for ( i = 0; i < num; i++ ) {
		if ( corres[i][cor_doc] >= 0 ) {
			cv::line(img, ps[i], cv::Point(size.width + corps[corres[i][cor_doc]].x, corps[corres[i][cor_doc]].y), cWhite, 3, 8, 0);
		}
//		if ( pcor[i][0] != cor_doc )	continue;
		// ��������
//		cv::line(img, ps[i], cv::Point(size.width + corps[pcor[i][1]].x, corps[pcor[i][1]].y), cWhite, 3, 8, 0);
	}
/*	for ( i = 0; i < ori_num; i++ )	DrawCross( img, corps[i], 8, cWhite, 3 );
	for ( i = 0; i < num; i++ ) {
		if ( pcor[i][0] != cor_doc )	continue;
		cv::line(img, ps[i], corps[pcor[i][1]], cWhite, 3, 8, 0);
	}*/
	OutPutImage(img);
}

void RecovPT( cv::Point *ps, int num, int cor_doc, cv::Size size0, cv::Mat img )
// �ˉe�ϊ��̕␳�g�b�v
{
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	int ori_num, i, cor[kMaxPointNum][2], cor_num = 0;
	FILE *fp;
	cv::Point corps[kMaxPointNum];
	cv::Size size;

	// �f�[�^�x�[�X�̓_�t�@�C����������ĊJ��
	strcpy(pfname, ePFPrefix);
//	itoa(cor_doc, nstr, 10);
	sprintf(nstr, "%d", cor_doc);
	strcat(pfname, nstr);
	strcat(pfname, ePFSuffix);
	fp = fopen(pfname, "r");
	// �_�t�@�C���̃f�[�^��corps�Ɋi�[����
	ori_num = 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size.width), &(size.height));	// 1�s�ڂ̓T�C�Y
	while( fgets(line, kMaxLineLen, fp) != NULL ) {
		sscanf(line, "%d,%d", &(corps[ori_num].x), &(corps[ori_num].y));
		ori_num++;
	}
	fclose(fp);
	// �L���ȁi����ID���������C�Ή��_�̂���j�_�����o���i���͂��������L���ȓ_�𑝂₷���@������j
	for ( i = 0; i < num; i++ ) {
		if ( pcor[i][0] == cor_doc ) {
			cor[cor_num][0] = i;
			cor[cor_num][1] = pcor[i][1];
			cor_num++;
		}
	}
#if 0
	fp = fopen( "temp.dat", "w" );
	fprintf(fp, "%d\n", num);	// �_�̐�
	for ( i = 0; i < num; i++ )	fprintf(fp, "%d,%d\n", ps[i].x, ps[i].y);	// �_�̍��W
	fprintf(fp, "%d\n", ori_num);	// �Ή������̓_�̐�
	for ( i = 0; i < ori_num; i++ )	fprintf(fp, "%d,%d\n", corps[i].x, corps[i].y);	// �Ή������̓_�̍��W
	fprintf(fp, "%d\n", cor_num);	// �Ή��֌W�̐�
	for ( i = 0; i < cor_num; i++ )	fprintf(fp, "%d,%d\n", cor[i][0], cor[i][1]);	// �Ή��֌W
	fprintf(fp, "%d,%d\n", size.width, size.height);	// �T�C�Y
	fprintf(fp, "%s\n", img_fname);	// �摜�t�@�C����
	fclose(fp);
#endif
	RecovPTSub( ps, corps, cor, cor_num, size, img );	// �T�u�ɔC����
}

void RecovPTFromTemp( void )
// �ˉe�ϊ��␳�̃e�X�g
{
	int num, ori_num, i, cor[kMaxPointNum][2], cor_num = 0;
	char line[kMaxLineLen], img_fname[kMaxPathLen];
	FILE *fp;
	cv::Size size;
	cv::Point *ps, *corps;

	fp = fopen("temp.dat", "r");
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d", &num);
	ps = (cv::Point *)calloc(num, sizeof(cv::Point));
	for ( i = 0; i < num; i++ ) {
		fgets(line, kMaxLineLen, fp);
		sscanf(line, "%d,%d", &(ps[i].x), &(ps[i].y));
	}
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d", &ori_num);
	corps = (cv::Point *)calloc(ori_num, sizeof(cv::Point));
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
//	RecovPTSub( ps, corps, cor, cor_num, size, img_fname );	// �T�u�ɔC����
}

void CheckByPPVar( cv::Point *ps, int num, int *score )
{
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	int ori_num, i, j, cor[kMaxPointNum][2], cor_doc, cor_num = 0;
	FILE *fp;
	cv::Point corps[kMaxPointNum];
	strScore *strscr;
	double ppvar[kCheckPPVarNum], var, v1;
	
	strscr = (strScore *)malloc(sizeof(strScore) * kMaxDocNum);	// strScore�\���̂̊m��
	SetAndSortScore( strscr, score, kMaxDocNum );	// strScore�ɂ���ă\�[�g
	for ( i = 0; i < kCheckPPVarNum; i++ ) {
		// �f�[�^�x�[�X�̓_�t�@�C����������ĊJ��
		cor_doc = strscr[i].n;	// �Ή�����ID
		strcpy(pfname, ePFPrefix);
		sprintf(nstr, "%d", cor_doc);
		strcat(pfname, nstr);
		strcat(pfname, ePFSuffix);
		fp = fopen(pfname, "r");
		// �_�t�@�C���̃f�[�^��corps�Ɋi�[����
		ori_num = 0;
		fgets(line, kMaxLineLen, fp);
//		sscanf(line, "%d,%d", &(size.width), &(size.height));	// 1�s�ڂ̓T�C�Y
		while( fgets(line, kMaxLineLen, fp) != NULL ) {
			sscanf(line, "%d,%d", &(corps[ori_num].x), &(corps[ori_num].y));
			ori_num++;
		}
		fclose(fp);
		// �L���ȁi����ID���������C�Ή��_�̂���j�_�����o��
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
// com1/com2������������
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

// �����s�������G���[���o��̂ŃR�����g�A�E�g�D���������[�N�ɂȂ�
//	if ( com1 != NULL)		free( com1 );
//	if ( com2 != NULL)		free( com2 );
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
// �����_���o���[�h2
{
	int n;
	FILE *fp, *fp_pn;
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	cv::Point *ps = 0;
	cv::Size img_size;
	cv::Mat img;
#ifdef	WIN32
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
	fp = fopen( eDBCorFileName, "w" );	// �t�@�C���Ɣԍ��̑Ή��t�@�C��
	fp_pn = fopen( ePNFileName, "w" );

#ifdef	WIN32
	hSearch = FindFirstFile( eHashSrcPath, &fFind );	// �ŏ��̃t�@�C��������
	for ( i = 0, ret = TRUE; ret != FALSE; i++, n++, ret = FindNextFile( hSearch, &fFind ) ) {
		GetDir( eHashSrcPath, kMaxPathLen, fname );	// �f�B���N�g���̎擾
		puts(fname);
		strcat(fname, fFind.cFileName);	// ���摜�t�@�C���̐�΃p�X�̍쐬
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
//	���[�h�͌��߂����ɂ���
//		if ( mode == CONNECTED_MODE ) {	// �P��̏d�S
			puts(fname);
			img = GetConnectedImage( fname, CONST_HASH_MODE );
			num = MakeCentresFromImage( &ps, img, &img_size, NULL );
			puts("end centre");
//		} else if ( mode == ENCLOSED_MODE ) {	// �͂܂ꂽ�̈�
//			img = cv::imread(gt.gl_pathv[i], cv::IMREAD_GRAYSCALE);
//			num = MakeFeaturePointsFromImage( &ps, img, &img_size );
//		}
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
//	���[�h�͌��߂����ɂ���
//		if ( mode == CONNECTED_MODE ) {	// �P��̏d�S
			img = GetConnectedImage( gt.gl_pathv[i], CONST_HASH_MODE );
			num = MakeCentresFromImage( &ps, img, &img_size, NULL );
//		} else if ( mode == ENCLOSED_MODE ) {	// �͂܂ꂽ�̈�
//			img = cv::imread(gt.gl_pathv[i], cv::IMREAD_GRAYSCALE);
//			num = MakeFeaturePointsFromImage( &ps, img, &img_size );
//		}
		SavePointFile( pfname, ps, num, &img_size, NULL );

		ReleaseCentres( ps );

		fprintf(fp_pn, "%d\n", num);
	}
	globfree(&gt);
#endif
	fclose(fp_pn);
	fclose(fp);
}

int CreatePointFile3( cv::Point ***p_reg_pss, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// �����_���o���[�h3�i���J�p�ɉ��ρj
{
	int i, j, n;
	FILE *fp, *fp_pn;
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	cv::Point *ps = 0;
	cv::Size img_size;
	cv::Mat img;
	char search_path[kMaxPathLen], pdat_fname[kMaxPathLen];
	cv::Point **reg_pss;
	cv::Size *reg_sizes;
	int *reg_nums;
	char **dbcors, **files;

	sprintf( search_path, "%s*.%s", eHashSrcDir, eHashSrcSuffix );	// �����p�X�̍쐬
	sprintf( pdat_fname, "%s%s", eDirsDir, ePointDatFileName );	// point.dat�̃p�X���쐬
	if ( ( fp = fopen( pdat_fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", pdat_fname );
		return 0;
	}
	eDbDocs = FindPath( search_path, &files );
	if ( eDbDocs <= 0 )	return 0;
	// �������̊m��
	reg_pss = (cv::Point **)calloc( eDbDocs, sizeof(cv::Point *) );
	reg_sizes = (cv::Size *)calloc( eDbDocs, sizeof(cv::Size) );
	reg_nums = (int *)calloc( eDbDocs, sizeof(int) );
	dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
	for ( i = 0; i < eDbDocs; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}
	// �t�@�C���o�́��i�[
	fprintf( fp, "%d\n", eDbDocs );	// �t�@�C�����̏o��
	for ( i = 0; i < eDbDocs; i++ ) {
		fprintf( stderr, "%s(%d/%d)\n", files[i], i+1, eDbDocs );
		fprintf( fp, "%s\n", files[i] );	// ���摜�t�@�C�������o��
		strcpy( dbcors[i], files[i] );	// ���摜�t�@�C������ۑ�
		if ( IsDat( files[i] ) ) {	// dat�t�@�C���̏ꍇ
			reg_nums[i] = LoadPointFile( files[i], &(reg_pss[i]), &(reg_sizes[i]) );
		}
		else {	// �摜�t�@�C���̏ꍇ
			img = GetConnectedImage2( files[i], CONST_HASH_MODE, eHashGaussMaskSize );	// �����摜���쐬
//			OutPutImage( img );
			reg_nums[i] = MakeCentresFromImage( &(reg_pss[i]), img, &(reg_sizes[i]), NULL );	// �����_�𒊏o
		}
//		SavePointFile( pfname, ps, num, &img_size, NULL );
		// �����_�����o��
		fprintf( fp, "%d,%d\n", reg_sizes[i].width, reg_sizes[i].height );	// �T�C�Y���o��
		fprintf( fp, "%d\n", reg_nums[i] );	// �����_�����o��
		for ( j = 0; j < reg_nums[i]; j++ ) {
			fprintf( fp, "%d,%d\n", reg_pss[i][j].x, reg_pss[i][j].y );	// �����_�̍��W���o��
		}
	}
	// �|�C���^�Ɋi�[
	*p_reg_pss = reg_pss;
	*p_reg_nums = reg_nums;
	*p_reg_sizes = reg_sizes;
	*p_dbcors = dbcors;
	// ���
	fclose( fp );
	for ( i = 0; i < eDbDocs; i++ ) {
		free( files[i] );
	}
	free( files );

	return i;
}

int CreatePointFile4( cv::Point ***p_reg_pss, double ***p_reg_areass, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors, char *hash_src_dir, char *hash_src_suffisx, int gaussian_parameter, char *dirs_dir, char *point_dat_file_name )
// �����_���o���[�h4�i�ʐςɊւ��鏈����ǉ��j
// 09/01/06 �f���A���n�b�V���ɑΉ����邽�߁C���낢��ƈ����Ŏw�肷��悤�ɂ���
{
	int i, j, n;
	FILE *fp, *fp_pn;
    int num = 0, **nears = 0;
	char pfname[kMaxPathLen], nstr[8], line[kMaxLineLen];
	cv::Point *ps = 0;
	cv::Size img_size;
	cv::Mat img, orig_img, fp_img, con_img;
	char search_path[kMaxPathLen], pdat_fname[kMaxPathLen];
	cv::Point **reg_pss;
	double **reg_areass;
	cv::Size *reg_sizes;
	int *reg_nums;
	char **dbcors, **files;

	sprintf( search_path, "%s*.%s", hash_src_dir, hash_src_suffisx );	// �����p�X�̍쐬
	sprintf( pdat_fname, "%s%s", dirs_dir, point_dat_file_name );	// point.dat�̃p�X���쐬
	if ( ( fp = fopen( pdat_fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", pdat_fname );
		return 0;
	}
	eDbDocs = FindPath( search_path, &files );
	if ( eDbDocs <= 0 )	return 0;
	if ( eDbDocs > kMaxDocNum ) {	// ��������kMaxDocNum�𒴂���
		fprintf( stderr, "error: # of images(%d) exceeds max(%d)\n", eDbDocs, kMaxDocNum );
		return 0;
	}
	// �������̊m��
	reg_pss = (cv::Point **)calloc( eDbDocs, sizeof(cv::Point *) );
	reg_areass = (double **)calloc( eDbDocs, sizeof(double *) );
	reg_sizes = (cv::Size *)calloc( eDbDocs, sizeof(cv::Size) );
	reg_nums = (int *)calloc( eDbDocs, sizeof(int) );
	dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
	for ( i = 0; i < eDbDocs; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}
	// �t�@�C���o�́��i�[
	fprintf( fp, "%d\n", eDbDocs );	// �t�@�C�����̏o��
	for ( i = 0; i < eDbDocs; i++ ) {
		fprintf( stderr, "%s(%d/%d)\n", files[i], i+1, eDbDocs );
		fprintf( fp, "%s\n", files[i] );	// ���摜�t�@�C�������o��
		strcpy( dbcors[i], files[i] );	// ���摜�t�@�C������ۑ�
		// dat�t�@�C���̏ꍇ�̏����͍폜�B�ʐς��Ȃ��̂�
		// �f���A���n�b�V���ł̕���or�P��̏ꍇ
		if ( gaussian_parameter == -1 )	gaussian_parameter = VariousLangMaskSizeDualChar( files[i] );
		else if ( gaussian_parameter == -2 )	gaussian_parameter = VariousLangMaskSizeDualWord( files[i] );
		if ( eIsJp )	img = GetConnectedImageJp2( files[i], CONST_HASH_MODE );	// �����摜���쐬
		else			img = GetConnectedImage2( files[i], CONST_HASH_MODE, gaussian_parameter );	// �����摜���쐬
#ifdef	DRAW_FP
		con_img = img.clone();
#endif
		reg_nums[i] = MakeCentresFromImage( &(reg_pss[i]), img, &(reg_sizes[i]), &(reg_areass[i]) );	// �����_�𒊏o
#ifdef	DRAW_FP
		// �����_�摜�̍쐬
		orig_img = cv::imread(files[i], cv::IMREAD_GRAYSCALE);
//		OutPutImage( con_img );
		fp_img = MakeFeaturePointImage( orig_img, con_img, reg_pss[i], reg_nums[i] );
		OutPutImage( fp_img );


#endif
		// �����_�����o��
		fprintf( fp, "%d,%d\n", reg_sizes[i].width, reg_sizes[i].height );	// �T�C�Y���o��
		fprintf( fp, "%d\n", reg_nums[i] );	// �����_�����o��
		for ( j = 0; j < reg_nums[i]; j++ ) {
			fprintf( fp, "%d,%d,%lf\n", reg_pss[i][j].x, reg_pss[i][j].y, reg_areass[i][j] );	// �����_�̍��W�Ɩʐς��o��
		}
	}
	// �|�C���^�Ɋi�[
	*p_reg_pss = reg_pss;
	*p_reg_areass = reg_areass;
	*p_reg_nums = reg_nums;
	*p_reg_sizes = reg_sizes;
	*p_dbcors = dbcors;
	// ���
	fclose( fp );
	for ( i = 0; i < eDbDocs; i++ ) {
		free( files[i] );
	}
	free( files );

	return i;
}

int LoadPointFile2( char *fname, cv::Point ***p_reg_pss, double ***p_reg_areass, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// �����_���o���[�h3�i���J�p�ɉ��ρj
{
	FILE *fp;
    int i, j, doc_num;
	char line[kMaxLineLen];
	cv::Point **reg_pss;
	double **reg_areass;
	cv::Size *reg_sizes;
	int *reg_nums;
	char **dbcors;

	if ( ( fp = fopen( fname, "r" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}
	fgets( line, kMaxLineLen, fp );
	sscanf( line, "%d", &doc_num );
	if ( doc_num != eDbDocs ) {	// �o�^����������v���Ȃ�
//		fprintf( stderr, "error: document number does not match\n" );
//		return 0;
		eDbDocs = doc_num;	// config.dat��ǂݍ��܂Ȃ��ꍇ�����邽��
	}
	// �������̊m��
	reg_pss = (cv::Point **)calloc( eDbDocs, sizeof(cv::Point *) );
	reg_areass = (double **)calloc( eDbDocs, sizeof(double *) );
	reg_sizes = (cv::Size *)calloc( eDbDocs, sizeof(cv::Size) );
	reg_nums = (int *)calloc( eDbDocs, sizeof(int) );
	dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
	for ( i = 0; i < eDbDocs; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}
	// �ǂݍ���
	for ( i = 0; i < eDbDocs && fgets( line, kMaxLineLen, fp ) != NULL; i++ ) {
		sscanf( line, "%s", dbcors[i] );	// ���摜�t�@�C���̃p�X
		fgets( line, kMaxLineLen, fp );
		sscanf( line, "%d,%d", &(reg_sizes[i].width), &(reg_sizes[i].height) );	// �T�C�Y
		fgets( line, kMaxLineLen, fp );
		sscanf( line, "%d", &(reg_nums[i]) );	// �����_��
		reg_pss[i] = (cv::Point *)calloc( reg_nums[i], sizeof(cv::Point) );	// �����_�f�[�^�̃������m��
		reg_areass[i] = (double *)calloc( reg_nums[i], sizeof(double) );	// �ʐσf�[�^�̃������m��
		for ( j = 0; j < reg_nums[i]; j++ ) {
			fgets( line, kMaxLineLen, fp );
			sscanf( line, "%d,%d,%lf", &(reg_pss[i][j].x), &(reg_pss[i][j].y), &(reg_areass[i][j]) );	// �����_�̍��W
		}
	}
	// �|�C���^�Ɋi�[
	*p_reg_pss = reg_pss;
	*p_reg_areass = reg_areass;
	*p_reg_nums = reg_nums;
	*p_reg_sizes = reg_sizes;
	*p_dbcors = dbcors;
	
	fclose( fp );
	return i;
}

void DrawPoints( cv::Size img_size, cv::Point *ps, int num )
// �����_��`��
{
	int i, width_all, height_all;
	cv::Mat pt;

	width_all = kDrawCorHMargin * 2 + img_size.width;
	height_all = kDrawCorVMargin * 2 + img_size.height;
//	printf( "%d,%d\n", width_all, height_all );
	pt = cv::Mat::zeros(cv::Size( width_all, height_all ), CV_8UC3);
	pt.setTo(cWhite);	// ���œh��Ԃ�

	// ��������̘g��`��
	cv::rectangle( pt, cv::Point( kDrawCorHMargin, kDrawCorVMargin ), \
		cv::Point( kDrawCorHMargin + img_size.width, kDrawCorVMargin + img_size.height ), \
		cBlack, kDrawCorRectThick, cv::LINE_AA, 0 );
	
	// ��������̓����_�̕`��
	for ( i = 0; i < num; i++ ) {
		cv::circle( pt, cv::Point( kDrawCorHMargin + ps[i].x, kDrawCorVMargin + ps[i].y ), kDrawCorPtRad, cBlack, -1, cv::LINE_AA, 0 );
	}
	OutPutImage( pt );
}

void CalcCRAndAddHist( cv::Point *ps, int num, int *nears[], strHist *hist )
// ������v�Z���ăq�X�g�O�����ɓ����
{
	int i=0, j, k, l, st, tmp, *idx;
	int *idxcom1, *idxcom2;
	double cr;

	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
	for ( i = 0; i < num; i++ ) {
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// �en�_�̑g�ݍ��킹�ɂ���
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];
			for ( st = 0; st < eGroup2Num; st++ ) {	// �e�J�n�_�ɂ���
                for ( k = 0; k < eNumCom2; k++ ) {	// �em�_�̑g�ݍ��킹�ɂ���
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
					AddDataHist( hist, cr );	// �q�X�g�O�����ɒǉ�
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
// �J�n�_��������
{
	int i, j, min;

	for ( i = 1, min = 0; i < eGroup2Num; i++ ) {
		// �܂�inv_array[min]��inv_array[i]���r���C
		// ����inv_array[min+1]��inv_array[i+1]�Ƃ����悤�ɏ��ɔ�r���ď������ق���min�Ƃ���D
		// �܂�Ԃ��̂���eGroup2Num��mod�����
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
// �ʐς���C���f�b�N�X���v�Z
// ���ʃo�[�W����
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
