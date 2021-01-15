#include <stdio.h>
#include "cv.h"
#include "highgui.h"

#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "annex_diff.h"

void GetDiffImage0( IplImage *diff, IplImage *anno, IplImage *orig, int mode )
// 差分画像を得る
{
	int i, j, k;
	signed int diffval;

	for ( j = 0; j < diff->height; j++ ) {
		for ( i = 0; i < diff->width; i++ ) {
			for ( k = 0; k < 3; k++ ) {
				diffval = (int)(255 - (unsigned char)(anno->imageData[j*anno->widthStep+i*3+k])) - (255 - (unsigned char)(orig->imageData[j*orig->widthStep+i*3+k]));
				if ( diffval >= 0 )	diff->imageData[j*diff->widthStep+i*3+k] = (char)(0x00ff & diffval);
				else	diff->imageData[j*diff->widthStep+i*3+k] = 0;
			}
		}
	}
}

void GetDiffImage( IplImage *diff, IplImage *anno, IplImage *orig, int mode )
// 差分画像を得る
{
	unsigned char anno_pix[3], orig_pix[3];
	double diff_dbl, dist, min_dist;
	int i, j, k, nx, ny, min_nx, min_ny;
	signed int diffval;
	IplImage *anno_lab, *orig_lab;
	IplImage *ori_img;
//	CvScalar color_tab[eDiffNear*2+1][eDiffNear*2+1];

	// 方向画像用の色テーブル
//	for ( j = 0; j <= eDiffNear * 2; j++ ) {
//		for ( i = 0; i <= eDiffNear * 2; i++ ) {
//			color_tab[i][j] = CV_RGB( (int)(255.0L / (double)(eDiffNear*2) * (double)i), 0/*(int)(255.0L / (double)(eDiffNear*2) * (double)j)*/, 0/*(int)(255.0L / (double)(eDiffNear*2) * ((double)(eDiffNear*4-i-j)/2.0L))*/ );
//		}
//	}
//	ori_img = cvCreateImage( cvGetSize( orig ), IPL_DEPTH_8U, 3 );
	// CIE L*a*b*の画像を作成
	anno_lab = cvCreateImage( cvGetSize( anno ), IPL_DEPTH_8U, 3 );
	orig_lab = cvCreateImage( cvGetSize( orig ), IPL_DEPTH_8U, 3 );
	cvCvtColor( anno, anno_lab, CV_BGR2Lab );
	cvCvtColor( orig, orig_lab, CV_BGR2Lab );
	
//	anno = anno_lab;
//	orig = orig_lab;

	for ( j = 0; j < diff->height; j++ ) {
		for ( i = 0; i < diff->width; i++ ) {
			for ( k = 0; k < 3; k++ ) {	// 画素値の取得
				anno_pix[k] = (unsigned char)anno->imageData[j*anno->widthStep+i*3+k];
			}
			min_dist = kMaxColorDist;
			min_nx = -eDiffNear;
			min_ny = -eDiffNear;
			// eDiffNearの範囲で最も近いものを探す
			for ( ny = -eDiffNear; ny <= eDiffNear; ny++ ) {
				if ( j + ny < 0 || j + ny >= orig->height )	continue;
				for ( nx = -eDiffNear; nx <= eDiffNear; nx++ ) {
					if ( i + nx < 0 || i + nx >= orig->width )	continue;	// 範囲を超えたらスキップ
//					dist = CalcColorDist( anno_pix, (unsigned char *)&(orig->imageData[(j+ny)*orig->widthStep+(i+nx)*3]) );
//					dist = 0;
					dist = CalcColorDist( (unsigned char *)&anno_lab->imageData[j*anno_lab->widthStep+i*3], (unsigned char *)&orig_lab->imageData[j*orig_lab->widthStep+i*3] );	// labでの距離を求める
					for ( k = 0; k < 3; k++ ) {	// RGBでの距離を加える
						diff_dbl = anno_pix[k] - (unsigned char)orig->imageData[(j+ny)*orig->widthStep+(i+nx)*3+k];
						dist += diff_dbl * diff_dbl;
					}
					if ( dist < min_dist /*|| dist == min_dist && nx*nx+ny*ny < min_nx*min_nx+min_ny*min_ny*/) {
						for ( k = 0; k < 3; k++ ) {
							orig_pix[k] = (unsigned char)orig->imageData[(j+ny)*orig->widthStep+(i+nx)*3+k];
						}
						min_dist = dist;
						min_nx = nx;
						min_ny = ny;
						if ( min_dist < eDiffEraseThr )	goto end_search;
					}
				}
			}
			end_search:
			// 差分を求める
			if ( min_dist > eDiffLeaveThr ) {	// 差が十分に大きければ差分を取らずにannoをそのまま残す
				for ( k = 0; k < 3; k++ ) {
					diff->imageData[j*diff->widthStep+i*3+k] = (unsigned char)(0x00ff & (255 - anno_pix[k]));
				}
			}
			else if ( min_dist < eDiffEraseThr ) {	// 差が十分に小さければ0とする
				for ( k = 0; k < 3; k++ ) {
					diff->imageData[j*diff->widthStep+i*3+k] = 0;
				}
			}
			else {	// そうでなければ差分を取る（負になってしまう場合は0とする）
				for ( k = 0; k < 3; k++ ) {
					diffval = (int)(255 - anno_pix[k]) - (255 - orig_pix[k]);
					if ( diffval >= 0 )	diff->imageData[j*diff->widthStep+i*3+k] = (unsigned char)(0x00ff & diffval);
					else	diff->imageData[j*diff->widthStep+i*3+k] = 0;
				}
				// 方向画像の描画
//				for ( k = 0; k < 3; k++ ) {
//					ori_img->imageData[ori_img->widthStep*j+i*3+k] = (unsigned char)(color_tab[min_nx+eDiffNear][min_ny+eDiffNear].val[k]);
//				}
			}
		}
	}
//	cvSaveImage( "ori.bmp", ori_img );
//	cvReleaseImage( &ori_img );
}

double CalcColorDist( unsigned char *c1, unsigned char *c2 )
// 色空間上の距離を計算する
{
	int i;
	double dist, diff, rgb1[3], lab1[3], rgb2[3], lab2[3];
	// CIE L*a*b*
//	for ( i = 0; i < 3; i++ ) {
//		rgb1[i] = (double)c1[i];
//		rgb2[i] = (double)c2[i];
//	}
//	RGB2Lab( rgb1, lab1 );
//	RGB2Lab( rgb2, lab2 );
	
	dist = 0.0L;
	for ( i = 1; i < 3; i++ ) {
//		diff = lab1[i] - lab2[i];
		diff = (double)c1[i] - c2[i];
		dist += diff * diff;
	}
	return dist;
}

double Lab_f( double t )
{
	if ( t > 0.008856 )	return pow(t, 1.0L / 3.0L);
	else	return 7.787 * t + 16 / 116;
}

void RGB2Lab( double *rgb, double *lab )
// RGBをCIE Labに変換する
{
	double x, y, z;
	
	x = 0.412453 * rgb[0] + 0.357580 * rgb[1] + 0.180423 * rgb[2];
	y = 0.212671 * rgb[0] + 0.715160 * rgb[1] + 0.072169 * rgb[2];
	z = 0.019334 * rgb[0] + 0.119193 * rgb[1] + 0.950227 * rgb[2];
	
	x = x / 0.950456;
	z = z / 1.088754;
	
	if ( y > 0.008856 )	lab[0] = 116 * pow(y, 1.0L/3.0L);
	else	lab[0] = 903.3 * y;
	
	lab[1] = 500 * (Lab_f(x) - Lab_f(y));
	lab[2] = 200 * (Lab_f(y) - Lab_f(z));
}
