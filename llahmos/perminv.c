#include "def_general.h"
#include <stdio.h>
#include <math.h>
#include "cv.h"
#include "highgui.h"

#include "disc.h"
#include "perminv.h"
#include "dirs.h"
#include "hist.h"
#include "hash.h"
#include "nn5.h"

#define USE073

int c6of8[kNum8C6][6];
int c4of6[kNum6C4][4];

void TestPerminv(CvSize size, CvPoint *ps, int num, int *nears[] )
{
	int i, j, k, l, idx[1024]/*, hindex[10]*/, flag, fail[8], points6[6], points4[4];
//	char txt[10];
	double /*rho, */fp, fd, i1[kNum6C4], i2[kNum6C4], sum1, sum2;
	double i1min, i1max, i2min, i2max;
	CvScalar color;
	CvPoint p[kPNum];

	for ( i = 0; i < kPNum; i++ ) {
		p[i].x = 0;
		p[i].y = 0;
	}
#ifdef USE073
	p[3].x = 200;	p[3].y = 1100;
#else
	p[3].x = 135;	p[3].y = 1187;
#endif

	MakeCom6of8();
	MakeCom4of6();
	for ( i = 0; i < num; i++ ) {
		color = cWhite;
		for ( flag = 0, j = 0; j < kPNum; j++ ) {
			if ( GetPointsDistance(ps[i], p[j]) < 10 ) {
				flag = 1;
				break;
			}
		}
		if ( !flag )	continue;
		
		CalcOrderCWN( i, ps, nears, idx, 10 );
#ifdef USE073
		fail[0] = 0;		fail[1] = 1;		fail[2] = 2;		fail[3] = 4;
		fail[4] = 6;		fail[5] = 7;		fail[6] = 8;		fail[7] = 9;
#else
		fail[0] = 0;		fail[1] = 1;		fail[2] = 2;		fail[3] = 3;
		fail[4] = 4;		fail[5] = 6;		fail[6] = 7;		fail[7] = 8;
//		fail[0] = 8;		fail[1] = 0;		fail[2] = 1;		fail[3] = 2;
//		fail[4] = 3;		fail[5] = 4;		fail[6] = 6;		fail[7] = 7;
#endif
		fp = CalcFdashp( ps[i], ps[idx[fail[1]]], ps[idx[fail[3]]], ps[idx[fail[0]]], ps[idx[fail[2]]] );
		fd = CalcFdashd( ps[i], ps[idx[fail[1]]], ps[idx[fail[3]]], ps[idx[fail[0]]], ps[idx[fail[2]]] );
		printf("F'+ : %f\nF'. : %f\n", fp, fd);

//		for ( j = 0; j < 8; j++ ) {
//			cr = CalcCR5(ps[idx[fail[j%8]]], ps[idx[fail[(j+1)%8]]], ps[idx[fail[(j+2)%8]]], ps[idx[fail[(j+3)%8]]], ps[idx[fail[(j+4)%8]]]);
//			printf("%f %d\n", cr, cr2hindex(cr));
//		}
//		for ( j = 0; j < kNum8C6; j++ ) printf("%d %d %d %d %d %d\n", c6of8[j][0], c6of8[j][1], c6of8[j][2], c6of8[j][3], c6of8[j][4], c6of8[j][5] );
//		for ( j = 0; j < kNum6C4; j++ ) printf("%d %d %d %d\n", c4of6[j][0], c4of6[j][1], c4of6[j][2], c4of6[j][3]);

		i1min = 10; i1max = 0; i2min = 10; i2max = 0;
		for ( j = 0; j < kNum8C6; j++ ) {
			for ( k = 0; k < 6; k++ )	points6[k] = idx[fail[c6of8[j][k]]];
			for ( k = 0; k < kNum6C4; k++ ) {
				for ( l = 0; l < 4; l++ )	points4[l] = points6[c4of6[k][l]];
				i1[k] = CalcFdashp( ps[i], ps[points4[0]], ps[points4[1]], ps[points4[2]], ps[points4[3]]);
				i2[k] = CalcFdashd( ps[i], ps[points4[0]], ps[points4[1]], ps[points4[2]], ps[points4[3]]);
				printf("%f %f\n", i1[k], i2[k]);
			}
			for ( k = 1; k <= 3; k++ ) {
				for ( l = 0, sum1 = 0, sum2 = 0; l < kNum6C4; l++ ) {
					sum1 += pow(i1[l], k);
					sum2 += pow(i2[l], k);
				}
				sum1 /= kNum6C4;
				sum1 = pow(sum1, (double)1.0L/(double)k);
				sum2 /= kNum6C4;
				sum2 = pow(sum2, (double)1.0L/(double)k);
//				printf("I(1,%d) = %f\n", k, sum1);
//				printf("I(2,%d) = %f\n", k, sum2);
				if ( sum1 < i1min )	i1min = sum1;
				if ( sum1 > i1max ) i1max = sum1;
				if ( sum2 < i2min ) i2min = sum2;
				if ( sum2 > i2max ) i2max = sum2;
			}
		}
//		printf("I1:%f-%f\nI2:%f-%f\n", i1min, i1max, i2min, i2max);


//		for ( j = 0; j < kNum8C5; j++ ) {
//			printf("%d %d %d %d %d\n", c5of8[j][0], c5of8[j][1], c5of8[j][2], c5of8[j][3], c5of8[j][4]);
//			cr = CalcCR5(ps[idx[fail[c5of8[j][0]]]], ps[idx[fail[c5of8[j][1]]]], ps[idx[fail[c5of8[j][2]]]], ps[idx[fail[c5of8[j][3]]]], ps[idx[fail[c5of8[j][4]]]]);
//			printf("%f %d\n", cr, cr2hindex(cr));
//			for ( k = 0; k < 5; k++ ) {
//                cr = CalcCR5(ps[idx[fail[c5of8[j][(0+k)%5]]]], ps[idx[fail[c5of8[j][(1+k)%5]]]], ps[idx[fail[c5of8[j][(2+k)%5]]]], ps[idx[fail[c5of8[j][(3+k)%5]]]], ps[idx[fail[c5of8[j][(4+k)%5]]]]);
//				printf("%d ", cr2hindex(cr));
//			}
//			printf("\n");
//		}
	}
}

double CalcFp( double rho )
// F+‚ðŒvŽZ‚·‚éH
{
//	return (double)(rho*rho*(1.0L-rho)*(1.0L-rho))/(pow((long double)rho*rho-rho+1.0L, 3));
	return (double)((double)8/(double)5) * ((rho*rho*(1.0L-rho)*(1-rho))/((rho*rho-rho+1.0L)*(rho*rho-rho+1.0L)*(rho*rho-rho+1.0L)));
}

double CalcFd( double rho )
// F.‚ðŒvŽZ‚·‚éH
{
	return (((double)3.0)*rho*rho*(1.0L-rho)*(1.0L-rho)) / (2.0L*pow(rho, 6) - 6.0L*(pow(rho, 5)) + 11.0L*(pow(rho, 4)) - 12.0L*(pow(rho,3)) + 11.0L*rho*rho - 6.0L*rho + 2.0L);
}

double CalcFdashp( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 )
// F'+‚ðŒvŽZ‚·‚é
{
	return CalcPsi(CalcRho(p1, p2, p3, p4, p5)) + CalcPsi(CalcRho(p1, p2, p3, p5, p4)) + CalcPsi(CalcRho(p1, p2, p4, p5, p3));
}

double CalcFdashd( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 )
// F'.‚ðŒvŽZ‚·‚é
{
	return CalcPsi(CalcRho(p1, p2, p3, p4, p5)) * CalcPsi(CalcRho(p1, p2, p3, p5, p4)) * CalcPsi(CalcRho(p1, p2, p4, p5, p3));
}

double CalcPsi( double rho )
// psi‚ðŒvŽZ‚·‚é
{
	return (2*rho)/(rho*rho+1);
}

double CalcRho( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 )
// rho‚ðŒvŽZ‚·‚é
{
	return (CalcP(p1, p2, p3) * CalcP(p1, p4, p5)) / (CalcP(p1, p2, p4) * CalcP(p1, p3, p5));
}

double CalcP( CvPoint p1, CvPoint p2, CvPoint p3 )
// P‚ðŒvŽZ‚·‚é
{
	double ab, ac, abac;

	ab = GetPointsDistance( p1, p2 );
	ac = GetPointsDistance( p1, p3 );
	abac = (p2.x - p1.x) * (p3.x - p1.x) + (p2.y - p1.y) * (p3.y - p1.y);

	return sqrt( (ab*ab)*(ac*ac) - abac*abac ) / 2;
}

void MakeCom6of8( void )
// 8ŒÂ‚©‚ç6ŒÂŽæ‚èo‚·‘g‚Ý‡‚í‚¹
{
	int i, j, k, l, m, n, num;

	for ( i = 0, num = 0; i < 3; i++ ) {
		for ( j = i+1; j < 4; j++ ) {
			for ( k = j+1; k < 5; k++ ) {
				for ( l = k+1; l < 6; l++ ) {
					for ( m = l+1; m < 7; m++ ) {
						for ( n = m+1; n < 8; n++ ) {
							c6of8[num][0] = i;
							c6of8[num][1] = j;
							c6of8[num][2] = k;
							c6of8[num][3] = l;
							c6of8[num][4] = m;
							c6of8[num][5] = n;
							num++;
						}
					}
				}
			}
		}
	}
}

void MakeCom4of6( void )
// 6ŒÂ‚©‚ç4ŒÂŽæ‚èo‚·‘g‚Ý‡‚í‚¹
{
	int i, j, k, l, num;

	for ( i = 0, num = 0; i < 3; i++ ) {
		for ( j = i+1; j < 4; j++ ) {
			for ( k = j+1; k < 5; k++ ) {
				for ( l = k+1; l < 6; l++ ) {
					c4of6[num][0] = i;
					c4of6[num][1] = j;
					c4of6[num][2] = k;
					c4of6[num][3] = l;
					num++;
				}
			}
		}
	}
}
