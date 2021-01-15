#include <stdio.h>

#include "cv.h"
#include "highgui.h"
#include "annex_st.h"
#include "estimate.h"

// �s�ϗʃ^�C�v
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

#define	kMaxParamValNum	(8)	/* �ϊ��i����or�A�t�B���j�p�����[�^�̍ő吔 */

int EstimateParamRANSAC( CvPoint **clpsa, CvPoint **clpso, int cor[][4], int cor_num, double *param, int type )
// RANSAC�ɂ��Ή��_���瑊������уA�t�B���ϊ��p�����[�^�𐄒肷��iannex�p�j
{
	int i, j, num_p, trpnum, randpi[4], cori, cla, clo, pa, po;
	int score, max_score = -1, max_param = -1;
	double **paramarr;
	CvPoint strp1[4], strp2[4];
	
	if ( type == SIMILAR )		num_p = 2;
	else if ( type == AFFINE )	num_p = 3;
	else	return 0;
	trpnum = num_p * 2;
	// �p�����[�^�z��̃������m��
	paramarr = (double **)calloc( kRANSACTry, sizeof( double * ) );
	for ( i = 0; i < kRANSACTry; i++ )	paramarr[i] = (double *)calloc( trpnum, sizeof( double ) );
	// ����J�n
	for ( j = 0; j < kRANSACTry; j++ ) {	// kRANSACTry��J��Ԃ�
		GetNumRand( num_p, cor_num, randpi );	// �����_����num_p�_�񕜌����o����
		for ( i = 0; i < num_p; i++ ) {
			cori = randpi[i];	// �I�΂ꂽ�Ή��ԍ�
			cla = cor[cori][0];	// anno�̃N���X�^
			pa = cor[cori][1];	// anno�̓_
			clo = cor[cori][2];	// orig�̃N���X�^
			po = cor[cori][3];	// orig�̓_
			strp2[i].x = clpsa[cla][pa].x;
			strp2[i].y = clpsa[cla][pa].y;
			strp1[i].x = clpso[clo][po].x;
			strp1[i].y = clpso[clo][po].y;
		}
		// num_p�̑Ή��_����p�����[�^�����߂�
		if ( type == SIMILAR )		CalcSimParam( strp1[0], strp1[1], strp2[0], strp2[1], paramarr[j] );
		else if ( type == AFFINE )	CalcAffParam( strp1[0], strp1[1], strp1[2], strp2[0], strp2[1], strp2[2], paramarr[j] );
		else	return 0;
//		for ( i = 0; i < trpnum; i++ ) {
//			printf("%lf, ", paramarr[j][i]);
//		}
		score = EvaluateParamRANSAC( clpsa, clpso, cor, cor_num, paramarr[j], type );
//		printf("\nvote : %d\n", score );
		if ( score > max_score ) {
			max_score = score;
			max_param = j;
		}
	}
//	printf("max vote : %d\n", max_score);
	// 臒l�����Ȃǂ����ɍő�̃X�R�A�̂��̂��o��
	for ( i = 0; i < trpnum; i++ )	param[i] = paramarr[max_param][i];
	// ���������
	for ( i = 0; i < kRANSACTry; i++ )	free( paramarr[i] );
	free( paramarr );
	return 1;
}

int EvaluateParamRANSAC( CvPoint **clpsa, CvPoint **clpso, int cor[][4], int cor_num, double *param, int type )
// RANSAC�ł̃p�����[�^�̕]��
{
	int i, score = 0;
	int cla, clo, pa, po;
	double dist;
	CvPoint p1, p2, p2h;
	
//	printf("\ndist\n");
	for ( i = 0; i < cor_num; i++ ) {
		cla = cor[i][0];
		pa = cor[i][1];
		clo = cor[i][2];
		po = cor[i][3];
		p1.x = clpso[clo][po].x;
		p1.y = clpso[clo][po].y;
		p2.x = clpsa[cla][pa].x;
		p2.y = clpsa[cla][pa].y;
		// �p�����[�^���狁�߂���Ή��_�̍��W�Ƃ̋����i�̂Q��j�����߂�
		if ( type == SIMILAR )		SimPoint( &p1, &p2h, param );
		else if ( type == AFFINE )	AffPoint( &p1, &p2h, param );
		else	return -1;
//		printf("p1 : (%d,%d)\np2 : (%d,%d)\np2h : (%d,%d)\n", p1.x, p1.y, p2.x, p2.y, p2h.x, p2h.y);
		dist = (p2.x-p2h.x)*(p2.x-p2h.x)+(p2.y-p2h.y)*(p2.y-p2h.y);
//		printf( "%lf, ", dist);
//		printf("\n");
		if ( dist < kRANSACThr )	score++;	// ������臒l�ȉ��Ȃ�C���N�������g
	}
	return score;
}

void EstimateParam( CvPoint **clpsa, CvPoint **clpso, int cor[][4], int cor_num, double *param, int type )
// �Ή��_���瑊������уA�t�B���ϊ��p�����[�^�𐄒肷��iannex�p�j
{
	int i, j, randpi[2], cori, cla, clo, pa, po, num_p, trpnum;
	double **paramarr;
//	strSimParam st_paramarr[kTryParamRand];
//	strAffParam af_paramarr[kTryParamRand];
	CvPoint strp1[3], strp2[3];
	
	if ( type == SIMILAR ) {
		num_p = 2;
	}
	else if ( type == AFFINE ) {
		num_p = 3;
	}
	trpnum = num_p * 2;
	paramarr = (double **)calloc( kTryParamRand, sizeof( double * ) );
	for ( i = 0; i < kTryParamRand; i++ ) {
		paramarr[i] = (double *)calloc( trpnum, sizeof( double ) );
	}
	for ( i = 0; i < kTryParamRand; i++ ) {
		// �����_����num_p�_�񕜌����o����
		GetNumRand( num_p, cor_num, randpi );
		
		for ( j = 0; j < num_p; j++ ) {
			cori = randpi[j];	// �I�΂ꂽ�Ή��ԍ�
			cla = cor[cori][0];	// anno�̃N���X�^
			pa = cor[cori][1];	// anno�̓_
			clo = cor[cori][2];	// orig�̃N���X�^
			po = cor[cori][3];	// orig�̓_
			strp2[j].x = clpsa[cla][pa].x;
			strp2[j].y = clpsa[cla][pa].y;
			strp1[j].x = clpso[clo][po].x;
			strp1[j].y = clpso[clo][po].y;
		}
		if ( type == SIMILAR ) {
			CalcSimParam( strp1[0], strp1[1], strp2[0], strp2[1], paramarr[i] );
		}
		else {
			CalcAffParam( strp1[0], strp1[1], strp1[2], strp2[0], strp2[1], strp2[2], paramarr[i] );
		}
	}
	
	// �K�؂ȃp�����[�^���v�Z����
//	GetAppropriateParam( paramarr, param, type );
	MeanParam( paramarr, param, kTryParamRand, trpnum, 0.00001, 100 );

	for ( i = 0; i < kTryParamRand; i++ ) {
		free( paramarr[i] );
	}
	free( paramarr );
}

void GetNumRand( int num, int max, int *arr )
// 0�`max-1�̐������d���Ȃ���num�������Carr�Ɋi�[����
{
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

void GetAppropriateParam( double **paramarr, double *param, int type )
// �p�����[�^�\���̔z��paramarr[]����K�؂ȃp�����[�^param�𓾂�
{
	int i, j, trpnum;
	double **pdouble, *mean;
	
	if ( type == SIMILAR ) {
		trpnum = 4;
	} else if ( type == AFFINE ) {
		trpnum = 6;
	}
	pdouble = (double **)calloc( kTryParamRand, sizeof( double * ) );
	for ( i = 0; i < kTryParamRand; i++ ) {
		pdouble[i] = (double *)calloc( trpnum, sizeof( double ) );
	}
	mean = (double *)calloc( trpnum, sizeof( double ) );

	for ( i = 0; i < kTryParamRand; i++ ) {
		for ( j = 0; j < trpnum; j++ ) {
			pdouble[i][j] = paramarr[i][j];
		}
	}
/*	if ( type == SIMILAR ) {
		for ( i = 0; i < kTryParamRand; i++ ) {
			pdouble[i][0] = st_paramarr[i].theta;
			pdouble[i][1] = st_paramarr[i].s;
			pdouble[i][2] = st_paramarr[i].tx;
			pdouble[i][3] = st_paramarr[i].ty;
		}
	}
	else if ( type == AFFINE ) {
		;
	}*/
	MeanParam( pdouble, mean, kTryParamRand, trpnum, 0.00001, 100 );
	for ( i = 0; i < trpnum; i++ ) {
		param[i] = mean[i];
	}
/*	if ( type == SIMILAR ) {
		st_param->theta = mean[0];
		st_param->s = mean[1];
		st_param->tx = mean[2];
		st_param->ty = mean[3];
	}
	else if ( type == AFFINE ) {
		;
	}*/
	
	for ( i = 0; i < kTryParamRand; i++ ) {
		free( pdouble[i] );
	}
	free( pdouble );
	free( mean );
}

// err0: �P�O�̌덷�ƐV���Ɍv�Z�����덷��err0�����Ȃ�I��
// threshold: �_���������߂����ɕ��ׂ��Ƃ��Cthreshold�܂ł͎���̕��ϒl�̌v�Z�Ɏg��(inside)
void MeanParam( double **data, double *mean, int n, int trpnum, double err0, int threshold) {
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
	var = (double *)malloc(sizeof(double)*trpnum);
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

	// loop (counter��0��1�̂Ƃ���err0�ɂ��]���͂��Ȃ��D�܂�C�Q��͕K�����[�v����)
	  while ( fabs(err-err_prev) > err0 || counter<=1) {
	
		// calculate mean
		for (j=0; j<trpnum; j++) {
			mean[j] = 0;
			for (i=0; i<n; i++) {
				if (inside_flag[i]==1) {
					mean[j] += data[i][j];
				}
			}
			mean[j] /= (double)size_flag1;
		}
		// calculate variance
		for (j=0; j<trpnum; j++) {
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
		for (j=0; j<trpnum; j++) {
			sum_var += 1/var[j];
		}
		// calculate distance between mean and data
		for (i=0; i<n; i++) {
			dist[i] = 0;
			for (j=0; j<trpnum; j++) {
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
		// dist[dist_n[i]]�����������ɕ��Ԃ悤��dist_n���\�[�g
//		quicksort(dist,dist_n,pdist,1,n);
//		quicksort(dist,dist_n,0,n-1);
		simplesort(dist,dist_n,0,n-1);
		// ���������������̂�inside�ɁC�����łȂ����̂�outside�ɂ���
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

void simplesort(double *d, keytype *a, int first, int last)
{
	int i, j, max_n, tmp_n;
	double max, tmp;

	for ( i = last; i > first; i-- ) {
		max = 0.0L;
		max_n = first;
		for ( j = first; j <= i; j++ ) {
			if ( d[j] >= max ) {
				max = d[j];
				max_n = j;
			}
		}
		tmp = d[max_n];
		tmp_n = a[max_n];
		d[max_n] = d[i];
		a[max_n] = a[i];
		d[i] = tmp;
		a[i] = tmp_n;
	}
}
