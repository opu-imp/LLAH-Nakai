#include <stdio.h>
#include <stdlib.h>

#include "def_general.h"
#include "disc.h"
#include "extern.h"

int Con2DiscCR( double cr, strDisc *disc )
// �A���l�̕���𗣎U�l�ɕϊ�����
{
	if ( cr <= disc->min )	return 0;	// min��菬������΍ŏ��l�ł���0��
	else if ( cr >= disc->max )	return disc->num-1;	// max���傫����΍ő�l�ł���num-1��Ԃ�
	else return disc->dat[(int)(((cr - disc->min)/(disc->max-disc->min))*(double)disc->res)];	// �����łȂ���ΊY������dat�̒l��Ԃ�
}

int Con2DiscAngle( double cr )
// �A���l�𗣎U�l�ɕϊ�
{
	return (int)((cr - kMinAng)/(kMaxAng - kMinAng) * eDiscNum);
}

int Con2DiscCREq(double cr, strDisc *disc)
// �A���l�̕���𗣎U�l�ɕϊ�����i���Ԋu�j
{
	if ( cr <= disc->min )	return 0;	// min��菬������΍ŏ��l�ł���0��
	else if ( cr >= disc->max )	return disc->num-1;	// max���傫����΍ő�l�ł���num-1��Ԃ�
	else return (int)(((cr - disc->min)/(disc->max - disc->min))*disc->num);
//	else return disc->dat[(int)(((cr - disc->min)/(disc->max-disc->min))*(double)disc->res)];	// �����łȂ���ΊY������dat�̒l��Ԃ�
}

int LoadDisc( char *fname, strDisc *disc )
// fname���痣�U���\���̂�ǂݍ���
{
	int i;
	char line[kDiscLineLen];
	FILE *fp;

	if ( (fp = fopen( fname, "r" )) == NULL ) {	// fname���s��
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return 0;
	}

	fgets(line, kDiscLineLen, fp);
	sscanf( line, "%lf", &disc->min );
	fgets(line, kDiscLineLen, fp);
	sscanf( line, "%lf", &disc->max );
	fgets(line, kDiscLineLen, fp);
	sscanf( line, "%d", &disc->num );
	fgets(line, kDiscLineLen, fp);
	sscanf( line, "%d", &disc->res );
	disc->dat = (int *)calloc(disc->res, sizeof(int));
	for ( i = 0; i < disc->res; i++ ) {
		fgets(line, kDiscLineLen, fp);
		sscanf( line, "%d", &disc->dat[i] );
	}
	fclose( fp );
	return 1;
}
