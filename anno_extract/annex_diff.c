#include <stdio.h>
#include <opencv2/opencv.hpp>

#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "annex_diff.h"

void GetDiffImage0( cv::Mat diff, cv::Mat anno, cv::Mat orig, int mode )
// �����摜�𓾂�
{
	int i, j, k;
	signed int diffval;

	for ( j = 0; j < diff.rows; j++ ) {
		for ( i = 0; i < diff.cols; i++ ) {
			for ( k = 0; k < 3; k++ ) {
				diffval = (int)(255 - (unsigned char)(anno.data[j*(int)anno.step[0]+i*3+k])) - (255 - (unsigned char)(orig.data[j*(int)orig.step[0]+i*3+k]));
				if ( diffval >= 0 )	diff.data[j*(int)diff.step[0]+i*3+k] = (char)(0x00ff & diffval);
				else	diff.data[j*(int)diff.step[0]+i*3+k] = 0;
			}
		}
	}
}

void GetDiffImage( cv::Mat diff, cv::Mat anno, cv::Mat orig, int mode )
// �����摜�𓾂�
{
	unsigned char anno_pix[3], orig_pix[3];
	double diff_dbl, dist, min_dist;
	int i, j, k, nx, ny, min_nx, min_ny;
	signed int diffval;
	cv::Mat anno_lab, orig_lab;
	cv::Mat ori_img;
//	cv::Scalar color_tab[eDiffNear*2+1][eDiffNear*2+1];

	// �����摜�p�̐F�e�[�u��
//	for ( j = 0; j <= eDiffNear * 2; j++ ) {
//		for ( i = 0; i <= eDiffNear * 2; i++ ) {
//			color_tab[i][j] = cv::Scalar(0/*(int,0/*(int)(255.0L / (double)(eDiffNear*2) * (double)j)*/,(int)(255.0L / (double)(eDiffNear*2) * (double)i))(255.0L / (double)(eDiffNear*2) * ((double)(eDiffNear*4-i-j)/2.0L))*/ );
//		}
//	}
//	ori_img = cv::Mat::zeros(orig.size(), CV_8UC3);
	// CIE L*a*b*�̉摜���쐬
	anno_lab = cv::Mat::zeros(anno.size(), CV_8UC3);
	orig_lab = cv::Mat::zeros(orig.size(), CV_8UC3);
	cv::cvtColor( anno, anno_lab, cv::COLOR_BGR2Lab );
	cv::cvtColor( orig, orig_lab, cv::COLOR_BGR2Lab );
	
//	anno = anno_lab;
//	orig = orig_lab;

	for ( j = 0; j < diff.rows; j++ ) {
		for ( i = 0; i < diff.cols; i++ ) {
			for ( k = 0; k < 3; k++ ) {	// ��f�l�̎擾
				anno_pix[k] = (unsigned char)anno.data[j*(int)anno.step[0]+i*3+k];
			}
			min_dist = kMaxColorDist;
			min_nx = -eDiffNear;
			min_ny = -eDiffNear;
			// eDiffNear�͈̔͂ōł��߂����̂�T��
			for ( ny = -eDiffNear; ny <= eDiffNear; ny++ ) {
				if ( j + ny < 0 || j + ny >= orig.rows )	continue;
				for ( nx = -eDiffNear; nx <= eDiffNear; nx++ ) {
					if ( i + nx < 0 || i + nx >= orig.cols )	continue;	// �͈͂𒴂�����X�L�b�v
//					dist = CalcColorDist( anno_pix, (unsigned char *)&(orig.data[(j+ny)*(int)orig.step[0]+(i+nx)*3]) );
//					dist = 0;
					dist = CalcColorDist( (unsigned char *)&anno_lab.data[j*(int)anno_lab.step[0]+i*3], (unsigned char *)&orig_lab.data[j*(int)orig_lab.step[0]+i*3] );	// lab�ł̋��������߂�
					for ( k = 0; k < 3; k++ ) {	// RGB�ł̋�����������
						diff_dbl = anno_pix[k] - (unsigned char)orig.data[(j+ny)*(int)orig.step[0]+(i+nx)*3+k];
						dist += diff_dbl * diff_dbl;
					}
					if ( dist < min_dist /*|| dist == min_dist && nx*nx+ny*ny < min_nx*min_nx+min_ny*min_ny*/) {
						for ( k = 0; k < 3; k++ ) {
							orig_pix[k] = (unsigned char)orig.data[(j+ny)*(int)orig.step[0]+(i+nx)*3+k];
						}
						min_dist = dist;
						min_nx = nx;
						min_ny = ny;
						if ( min_dist < eDiffEraseThr )	goto end_search;
					}
				}
			}
			end_search:
			// ���������߂�
			if ( min_dist > eDiffLeaveThr ) {	// �����\���ɑ傫����΍�������炸��anno�����̂܂܎c��
				for ( k = 0; k < 3; k++ ) {
					diff.data[j*(int)diff.step[0]+i*3+k] = (unsigned char)(0x00ff & (255 - anno_pix[k]));
				}
			}
			else if ( min_dist < eDiffEraseThr ) {	// �����\���ɏ��������0�Ƃ���
				for ( k = 0; k < 3; k++ ) {
					diff.data[j*(int)diff.step[0]+i*3+k] = 0;
				}
			}
			else {	// �����łȂ���΍��������i���ɂȂ��Ă��܂��ꍇ��0�Ƃ���j
				for ( k = 0; k < 3; k++ ) {
					diffval = (int)(255 - anno_pix[k]) - (255 - orig_pix[k]);
					if ( diffval >= 0 )	diff.data[j*(int)diff.step[0]+i*3+k] = (unsigned char)(0x00ff & diffval);
					else	diff.data[j*(int)diff.step[0]+i*3+k] = 0;
				}
				// �����摜�̕`��
//				for ( k = 0; k < 3; k++ ) {
//					ori_img.data[(int)ori_img.step[0]*j+i*3+k] = (unsigned char)(color_tab[min_nx+eDiffNear][min_ny+eDiffNear].val[k]);
//				}
			}
		}
	}
//	cv::imwrite( "ori.bmp", ori_img );
//
}

double CalcColorDist( unsigned char *c1, unsigned char *c2 )
// �F��ԏ�̋������v�Z����
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
// RGB��CIE Lab�ɕϊ�����
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
