#include <stdio.h>
#include <stdlib.h>

#include "def_general.h"
#include "disc.h"
#include "extern.h"

int Con2DiscCR( double cr, strDisc *disc )
// 連続値の複比を離散値に変換する
{
	if ( cr <= disc->min )	return 0;	// minより小さければ最小値である0を
	else if ( cr >= disc->max )	return disc->num-1;	// maxより大きければ最大値であるnum-1を返す
	else return disc->dat[(int)(((cr - disc->min)/(disc->max-disc->min))*(double)disc->res)];	// そうでなければ該当するdatの値を返す
}

int Con2DiscAngle( double cr )
// 連続値を離散値に変換
{
	return (int)((cr - kMinAng)/(kMaxAng - kMinAng) * eDiscNum);
}

int Con2DiscCREq(double cr, strDisc *disc)
// 連続値の複比を離散値に変換する（等間隔）
{
	if ( cr <= disc->min )	return 0;	// minより小さければ最小値である0を
	else if ( cr >= disc->max )	return disc->num-1;	// maxより大きければ最大値であるnum-1を返す
	else return (int)(((cr - disc->min)/(disc->max - disc->min))*disc->num);
//	else return disc->dat[(int)(((cr - disc->min)/(disc->max-disc->min))*(double)disc->res)];	// そうでなければ該当するdatの値を返す
}

int LoadDisc( char *fname, strDisc *disc )
// fnameから離散化構造体を読み込む
{
	int i;
	char line[kDiscLineLen];
	FILE *fp;

	if ( (fp = fopen( fname, "r" )) == NULL ) {	// fnameが不正
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
