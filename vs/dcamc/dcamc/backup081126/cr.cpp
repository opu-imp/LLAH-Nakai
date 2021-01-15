#include <stdio.h>
#include <stdlib.h>

#include "cv.h"
#include "highgui.h"
#include "cr.h"
#include "dirs.h"
#include "def_general.h"

double CalcCR5ByArea( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 )
// 面積に基づいて平面上の5点で複比を計算する
{
	double s;
	if ( ( s = CalcArea(p1, p2, p4) * CalcArea(p1, p3, p5 )) < kMinValCr )	s = kMinValCr;
	return (CalcArea(p1, p2, p3) * CalcArea(p1, p4, p5)) / s ;
}

double CalcArea( CvPoint p1, CvPoint p2, CvPoint p3 )
// 指定された3点で囲まれた三角形の面積を計算する
{
	return fabs((double)((p1.x-p3.x)*(p2.y-p3.y)-(p1.y-p3.y)*(p2.x-p3.x))) / (double)2.0L;
}

double CalcAffineInv( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 )
// 平面上の4点でアフィン変換の不変量を計算する
{
//	旧バージョン
//	if ( (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y) == 0 )	return 1000000;
//	else    return (double)((p3.x - p1.x) * (p4.y - p1.y) - (p4.x - p1.x) * (p3.y - p1.y)) / (double)((p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y));
//	新バージョン（DAS）
	double s;
	if ( (s = CalcArea(p1, p2, p3)) < kMinValCr )	s = kMinValCr;
	return CalcArea(p1, p3, p4) / s ;
}

long double GetPointsAngle(CvPoint p1, CvPoint p2)
// p1から見たp2の角度を計算する（約-3.14から約3.14）
{
	return atan2((long double)(p2.y - p1.y), (long double)(p2.x - p1.x));
}

double CalcAngleFromThreePoints( CvPoint p1, CvPoint p2, CvPoint p3 )
// p1を頂点としたp2とp3の角度を求める
{
	double ang;

	ang = GetPointsAngle( p1, p3 ) - GetPointsAngle( p1, p2 );
	if ( ang < 0 ) ang += M_PI * 2.0;
	return ang;
}

double CalcSimilarInv( CvPoint p1, CvPoint p2, CvPoint p3 )
// 平面上の3点で相似変換の不変量を計算する
{
	double s;
	if ( (s = GetPointsDistance(p1, p2)) < kMinValCr )	s = kMinValCr;
	return GetPointsDistance(p1, p3) / s ;
}
