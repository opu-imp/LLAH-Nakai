#include <stdio.h>
#include "cv.h"
#include "highgui.h"

#include "annex_st.h"

void SimilarTransformation( IplImage *src, IplImage *dst, double *param )
// 相似変換する
{
	int i, j;
	CvPoint psrc, pdst;

	for ( j = 0; j < dst->height; j++ ) {
		for ( i = 0; i < dst->width; i++ ) {
			pdst.x = i;
			pdst.y = j;
			SimPoint( &pdst, &psrc, param );

			if ( psrc.x >= 0 && psrc.x < src->width && psrc.y >= 0 && psrc.y < src->height ) {
				dst->imageData[j*dst->widthStep+i*3+0] = src->imageData[psrc.y*src->widthStep+psrc.x*3+0];
				dst->imageData[j*dst->widthStep+i*3+1] = src->imageData[psrc.y*src->widthStep+psrc.x*3+1];
				dst->imageData[j*dst->widthStep+i*3+2] = src->imageData[psrc.y*src->widthStep+psrc.x*3+2];
			}
			else {
				dst->imageData[j*dst->widthStep+i*3+0] = kBgVal;
				dst->imageData[j*dst->widthStep+i*3+1] = kBgVal;
				dst->imageData[j*dst->widthStep+i*3+2] = kBgVal;
			}
//			dst->imageData[j*dst->widthStep+i*3+0] = src->imageData[j*src->widthStep+i*3+0];
//			dst->imageData[j*dst->widthStep+i*3+1] = src->imageData[j*src->widthStep+i*3+1];
//			dst->imageData[j*dst->widthStep+i*3+2] = src->imageData[j*src->widthStep+i*3+2];
		}
	}
}

void SimPoint( CvPoint *p1, CvPoint *p2, double *param )
// 点の相似変換後の座標を求める
{
	double theta, s, tx, ty;
	
	theta = param[0];
	s = param[1];
	tx = param[2];
	ty = param[3];
	
//	p2->x = (int)(param->s * cos(param->theta) * p1->x - param->s * sin(param->theta) * p1->y + param->tx);
//	p2->y = (int)(param->s * sin(param->theta) * p1->x + param->s * cos(param->theta) * p1->y + param->ty);
	p2->x = (int)(s * cos(theta) * p1->x - s * sin(theta) * p1->y + tx);
	p2->y = (int)(s * sin(theta) * p1->x + s * cos(theta) * p1->y + ty);
}

void CalcSimParam( CvPoint p11, CvPoint p12, CvPoint p21, CvPoint p22, double *param )
// p11,p12->p21,p22へと相似変換するパラメータを求める
{
	double theta, s, tx, ty;
	
	s = GetPointsDistance( p21, p22 ) / GetPointsDistance( p11, p12 );
//	param->tx = p21.x - p11.x;
//	param->ty = p21.y - p11.y;
	theta = atan2( (double)(p22.y - p21.y), (double)(p22.x - p21.x) ) - atan2( (double)(p12.y - p11.y), (double)(p12.x - p11.x) );
	tx = p21.x - (s * cos(theta) * p11.x - s * sin(theta) * p11.y);
	ty = p21.y - (s * sin(theta) * p11.x + s * cos(theta) * p11.y);
	param[0] = theta;
	param[1] = s;
	param[2] = tx;
	param[3] = ty;
}

void AffineTransformation( IplImage *src, IplImage *dst, double *param )
// 相似変換する
{
	int i, j;
	CvPoint psrc, pdst;

	for ( j = 0; j < dst->height; j++ ) {
		for ( i = 0; i < dst->width; i++ ) {
			pdst.x = i;
			pdst.y = j;
			AffPoint( &pdst, &psrc, param );

			if ( psrc.x >= 0 && psrc.x < src->width && psrc.y >= 0 && psrc.y < src->height ) {
				dst->imageData[j*dst->widthStep+i*3+0] = src->imageData[psrc.y*src->widthStep+psrc.x*3+0];
				dst->imageData[j*dst->widthStep+i*3+1] = src->imageData[psrc.y*src->widthStep+psrc.x*3+1];
				dst->imageData[j*dst->widthStep+i*3+2] = src->imageData[psrc.y*src->widthStep+psrc.x*3+2];
			}
			else {
				dst->imageData[j*dst->widthStep+i*3+0] = kBgVal;
				dst->imageData[j*dst->widthStep+i*3+1] = kBgVal;
				dst->imageData[j*dst->widthStep+i*3+2] = kBgVal;
			}
//			dst->imageData[j*dst->widthStep+i*3+0] = src->imageData[j*src->widthStep+i*3+0];
//			dst->imageData[j*dst->widthStep+i*3+1] = src->imageData[j*src->widthStep+i*3+1];
//			dst->imageData[j*dst->widthStep+i*3+2] = src->imageData[j*src->widthStep+i*3+2];
		}
	}
}

void AffPoint( CvPoint *p1, CvPoint *p2, double *param )
// 点のアフィン変換後の座標を求める
{
	double a1, b1, c1, a2, b2, c2;

	a1 = param[0];
	b1 = param[1];
	c1 = param[2];
	a2 = param[3];
	b2 = param[4];
	c2 = param[5];
	
	p2->x = (int)(a1 * p1->x + b1 * p1->y + c1);
	p2->y = (int)(a2 * p1->x + b2 * p1->y + c2);
//	printf("%d, %d\n", p2->x, p2->y);
}

void CalcAffParam( CvPoint p11, CvPoint p12, CvPoint p13, CvPoint p21, CvPoint p22, CvPoint p23, double *param )
// p11,p12,p13->p21,p22,p23へとアフィン変換するパラメータを求める
{
	double a1, b1, c1, a2, b2, c2;
	double x11, x12, x13, x21, x22, x23, y11, y12, y13, y21, y22, y23;
	double dena, denb;
	
	x11 = p11.x;
	x12 = p12.x;
	x13 = p13.x;
	x21 = p21.x;
	x22 = p22.x;
	x23 = p23.x;
	y11 = p11.y;
	y12 = p12.y;
	y13 = p13.y;
	y21 = p21.y;
	y22 = p22.y;
	y23 = p23.y;
	
	dena = (x11 - x12)*(y12 - y13) - (x12 - x13)*(y11 - y12);
	denb = (y11 - y12)*(x12 - x13) - (y12 - y13)*(x11 - x12);

	a1 = (double)((x21 - x22)*(y12 - y13) - (x22 - x23)*(y11 - y12)) / dena;
	b1 = (double)((x21 - x22)*(x12 - x13) - (x22 - x23)*(x11 - x12)) / denb;
	c1 = x21 - a1*x11 - b1*y11;
	
	a2 = (double)((y21 - y22)*(y12 - y13) - (y22 - y23)*(y11 - y12)) / dena;
	b2 = (double)((y21 - y22)*(x12 - x13) - (y22 - y23)*(x11 - x12)) / denb;
	c2 = y21 - a2*x11 - b2*y11;

	param[0] = a1;
	param[1] = b1;
	param[2] = c1;
	param[3] = a2;
	param[4] = b2;
	param[5] = c2;
}
