#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "dirs.h"
#include "annex.h"
//#include "annex_st.h"
//#include "transform.h"
//#include "clpoints.h"
#include "hash.h"
#include "disc.h"
#include "nn5j.h"
#include "gencomb.h"
#include "cr.h"
#include "proctime.h"

int corres[kMaxPointNum][kMaxDocNum];
char flag1[kMaxDocNum];
char flag2[kMaxDocNum][kMaxPointNum];
char flago[kMaxPointNum], flaga[kMaxPointNum];	// �_�Ή��t���O�iannex�p�j

int **com1 = NULL;
int **com2 = NULL;

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

void ConstructHashSub( cv::Point *ps, int num )
// �n�b�V�����\�z����i���O�̓T�u�����ǃ��C���j
{
	int **nears = NULL;
	strDisc disc;
	
	InitHash();	// �n�b�V���̏�����
	LoadDisc( eDiscFileName, &disc );	// ���U���t�@�C���̓ǂݍ���
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	MakeNearsFromCentres( ps, num, &nears );	// �e�_�ɂ��ď��kNears�̋ߖT�_�̃C���f�b�N�X�����߁Cnears�Ɋi�[����
	ConstructHashSubSub( ps, num, nears, 0, &disc );
}

void ConstructHashSubSub(cv::Point *ps, int num, int *nears[], int n, strDisc *disc)
// �n�b�V�����\�z����i���O�̓T�u�̃T�u�����ǃT�u�j
{
	char *hindex;
	int i, j, k, l;
	int *idx, *idxcom1, *idxcom2;
	double cr = 0.0;

	hindex = (char *)calloc( eNumCom2, sizeof(char) );
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
	for ( i = 0; i < num; i++ ) {
		CalcOrderCWN( i, ps, nears, idx, eGroup1Num );
		for ( j = 0; j < eNumCom1; j++ ) {	// �e8�_�̑g�ݍ��킹�ɂ���
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];
			for ( k = 0; k < eNumCom2; k++ ) {	// �e5�_�̑g�ݍ��킹�ɂ���
//				if ( eIncludeCentre || eInvType == SIMILAR ) {
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
//						cr = CalcAngleFromThreePoints(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);	// �p�x���v�Z
						cr = CalcSimilarInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]]);
					default:
						break;
				}
//				if ( eInvType == SIMILAR )	hindex[k] = Con2DiscAngle( cr );
//				else						hindex[k] = Con2DiscCR( cr, disc );
				hindex[k] = Con2DiscCR( cr, disc );
			}
			AddHash( hindex, disc->num, n, i );
		}
	}
	free( hindex );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
}

int RetrieveNN5(cv::Point *ps, int num, int *nears[], cv::Size size, int *score, int *pcor )
// �n�b�V�����猟������
{
	int i, j;
	strDisc disc;
//	DWORD start, end;
	int start, end;

	// �g�ݍ��킹�̍쐬
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �_�Ή��e�[�u���̍쐬
	for ( i = 0; i < kMaxPointNum; i++ ) {
		for ( j = 0; j < kMaxDocNum; j++ ) {
			corres[i][j] = -1;
		}
	}
	// flag2�̏�����
/*	for ( i = 0; i < kMaxDocNum; i++ ) {
		for ( j = 0; j < kMaxPointNum; j++ ) {
			flag2[i][j] = 0;
		}
	}*/
	ClearCorFlag();
	LoadDisc( eDiscFileName, &disc );	// ���U���e�[�u���̍쐬
//	start = timeGetTime();
	start = GetProcTimeMicroSec();
	CalcScore3(ps, num, nears, score, &disc);	// ����
//	CheckByPPVar( ps, num, score );	// �ˉe�ϊ��p�����[�^�̕��U�Ń`�F�b�N
//	end = timeGetTime();
	end = GetProcTimeMicroSec();
//	printf("retrieval time : %d\n", end - start);
//	printf("retrieval time : %d micro sec\n", end - start);

	// �Ή��_�̊i�[
	for ( i = 0; i < num; i++ ) {
		pcor[i] = corres[i][0];
	}
//	DrawP2P(ps, num, max_doc, size);
	return 0;
}

void CalcScore3(cv::Point *ps, int num, int *nears[], int *score, strDisc *disc)
// �e�����̓��_���v�Z����3
{
	char *hindex;
	int i=0, j, k, l, st, tmp;
	int *idx, *idxcom1, *idxcom2;
	double cr = 0.0;
	strHList *hl;

	hindex = (char *)calloc( eNumCom2, sizeof(char) );
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
	for ( i = 0; i < kMaxDocNum; i++ )	score[i] = 0;
	for ( i = 0; i < num; i++ ) {
//		ClearPVote();
//		ClearFlag1();
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
//					if ( eInvType == SIMILAR )	hindex[k] = Con2DiscAngle( cr );
//					else						hindex[k] = Con2DiscCR( cr, disc );
					hindex[k] = Con2DiscCR( cr, disc );
				}
//				idxx = RealIndex(hindex, disc->num);
//				crint = HIndexCnv(hindex, disc->num);
				hl = ReadHash(hindex, disc->num);
				VoteDirectlyByHL( hl, score, hindex, i );

				tmp = idxcom1[0];
				for ( k = 0; k < eGroup2Num-1; k++ )	idxcom1[k] = idxcom1[k+1];
				idxcom1[eGroup2Num-1] = tmp;
			}
		}
//		CalcPointCor( i );
	}

//	for ( i = 0; i < kMaxDocNum; i++ ) {	// �X�R�A�̊m�F
//		if ( score[i] > 0 ) {
//			printf("%03d : %d\n", i, score[i]);
//		}
//	}
	free( hindex );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
}

void VoteDirectlyByHL( strHList *hl, int *score, char *idx, int p )
// �n�b�V�����X�g���璼��vote����
{
	strHList *hp;
    
	for ( hp = hl; hp != NULL; hp = hp->next) {
//		if ( memcmp(hp->idx, idx, eNumCom2) == 0 && flag1[hp->doc] == 0 && flag2[hp->doc][hp->point] == 0) {
		// �Ή��֌W�`�F�b�N && ( �x�N�g���`�F�b�N�Ȃ� || �x�N�g����v )
//		if ( flag1[hp->doc] == 0 && flag2[hp->doc][hp->point] == 0 && ( !eVectorCheck || memcmp(hp->idx, idx, eNumCom2) == 0 ) ) {
		if ( flaga[p] == 0 && flago[hp->point] == 0 && memcmp(hp->idx, idx, eNumCom2) == 0 ) {
			score[hp->doc]++;	// ���[
			corres[p][hp->doc] = hp->point;
			flaga[p] = 1;
			flago[hp->point] = 1;
//			flag1[hp->doc] = 1;	// hp->doc�͓��[�ς�
//			flag2[hp->doc][hp->point] = 1;	// hp-doc��hp->point�͓��[�ς�
		}
	}
}

void NearestPoint( int n, cv::Point *ps, int num, int nears[] )
//	�T�C�Ynum�̓_�z��ps��n�Ԗڂ̓_�ƍł��߂�kNears�̓_�̃C���f�b�N�X��nears�Ɋi�[����
{
	int i, j, k, min_idxs[kNears];
	double dist, min_dists[kNears];
	cv::Point p;

	// ������
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

void MakeNearsFromCentres( cv::Point *ps, int num, int ***nears0 )
// �d�S����ߖT�\�����v�Z����
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

double CalcCR5( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 )
// ���ʏ��5�_�ŕ�����v�Z����
{
	cv::Point p1325, p1425;

	CalcInterPoint( p1, p3, p2, p5, &p1325 );
	CalcInterPoint( p1, p4, p2, p5, &p1425 );

	return CalcCR(p2, p1325, p1425, p5);
}

double CalcCR( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 )
// ������v�Z����
{
	double d13, d23, d14, d24;
	d13 = GetPointsDistance(p1, p3);
	d23 = GetPointsDistance(p2, p3);
	d14 = GetPointsDistance(p1, p4);
	d24 = GetPointsDistance(p2, p4);
	if ( d23 < kLittleVal ) d23 = kLittleVal;
	if ( d24 < kLittleVal ) d24 = kLittleVal;
	if ( d14 < kLittleVal ) d14 = kLittleVal;
	return ( (d13/ d23) / ( d14 / d24 ) );
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

	if (com1 == NULL)	InitCom( &com1, eNumCom1, eGroup2Num );
	if (com2 == NULL)	InitCom( &com2, eNumCom2, eGroup3Num );
	
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

void ClearFlag1(void)
{
	int i;
	for ( i = 0; i < kMaxDocNum; i++ )	flag1[i] = 0;
}

void ClearCorFlag( void )
{
	int i;
	for ( i = 0; i < kMaxPointNum; i++ ) {
		flago[i] = 0;
		flaga[i] = 0;
	}
}
