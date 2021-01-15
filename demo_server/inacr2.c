/*****************************************************************
acr2: Create digitizing file "disc.txt" for cvtest by
analysing cross-ratios or affine invariants of document image.
*****************************************************************/

#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include "def_general.h"
#include "dirs.h"
#include "hash.h"
#include "hist.h"
#include "disc.h"
#include "inacr2.h"
#include "gencomb.h"
#include "cr.h"
#include "extern.h"
#include "ncr.h"
#include "nears.h"
#include "nn5.h"


//int main( int argc, char *argv[] )
int MakeDiscFile( int doc_num, CvPoint **pss, int *nums, strDisc *disc, char *disc_file_name )
// ���U���t�@�C�����쐬����Dacr2�̑g�ݍ��ݔ�
{
//	int disc_num, num, **nears = 0, dnmin, dnmax, step, argi;
//	char fname[1024];
//	CvPoint *ps;
//	CvSize img_size;
	int i, ret, **nears = NULL;
	char disc_fname[kMaxPathLen];
	strHist hist;
//	strDisc disc;
	
//	if ( argc < 7 ) {
//		printf("Usage: ./acr2 [r/i/a/s] [n] [m] [disc] [output] [point files...]\nEx. ./acr2 s 8 7 10 disc.txt a.dat b.dat c.dat\n");
//		return 1;
//	}
	switch ( eInvType ) {
		case CR_AREA:
			eDiscMin = 0.0;
			eDiscMax = 4.0;
			break;
		case CR_INTER:
			eDiscMin = 0.0;
			eDiscMax = 4.0;
			break;
		case AFFINE:
			eDiscMin = 0.0;
			eDiscMax = 10.0;
			break;
		case SIMILAR:
			eDiscMin = 0.0;
			eDiscMax = 4.0;
			break;
		default:
			return 0;
	}
//	fprintf( stderr, "Init Hist\n" );
	InitHist( &hist, kDiscRes, eDiscMin, eDiscMax );	// �q�X�g�O�����̏�����
//	fprintf( stderr, "Generate Combination\n" );
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );	// �g�ݍ��킹�̍쐬

	for ( i = 0; i < doc_num; i++ ) {
		fprintf( stderr, "(%d/%d)\n", i+1, doc_num );
//		fprintf( stderr, "Make Nears\n" );
		MakeNearsFromCentres( pss[i], nums[i], &nears );
//		fprintf( stderr, "CalcCRAndAddHist\n" );
		CalcCRAndAddHist( pss[i], nums[i], nears, &hist );
//		fprintf( stderr, "ReleaseNears\n" );
		ReleaseNears( nears, nums[i] );
	}
//	fprintf( stderr, "Hist2Disc\n" );
	Hist2Disc( &hist, disc, eDiscNum );
	sprintf( disc_fname, "%s%s", eDirsDir, disc_file_name );
//	fprintf( stderr, "SaveDisc\n" );
	ret = SaveDisc( disc_fname, disc );

	return ret;
}

int SaveDisc( char *fname, strDisc *disc )
// ���U���t�@�C����ۑ�
{
	int i;
	FILE *fp;
	
	if ( (fp = fopen(fname, "w")) == NULL )	{
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}
	fprintf(fp, "%f\n", disc->min);
	fprintf(fp, "%f\n", disc->max);
	fprintf(fp, "%d\n", disc->num);
	fprintf(fp, "%d\n", disc->res);
	for ( i = 0; i < disc->res; i++ )	fprintf(fp, "%d\n", disc->dat[i]);
	fclose(fp);

	return 1;
}

void Hist2Disc( strHist *hist, strDisc *disc, int disc_num )
// �q�X�g�O�������痣�U���f�[�^���쐬
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

