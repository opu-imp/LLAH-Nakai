#include <stdio.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>
#include "cvtest.h"
#include "cr.h"

double CalcCR5ByArea( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 )
// �ʐςɊ�Â��ĕ��ʏ��5�_�ŕ�����v�Z����
{
	double s;
	if ( ( s = CalcArea(p1, p2, p4) * CalcArea(p1, p3, p5 )) < 0.0001 )	s = 0.0001;
	return (CalcArea(p1, p2, p3) * CalcArea(p1, p4, p5)) / s ;
}

double CalcArea( cv::Point p1, cv::Point p2, cv::Point p3 )
// �w�肳�ꂽ3�_�ň͂܂ꂽ�O�p�`�̖ʐς��v�Z����
{
	return fabs((p1.x-p3.x)*(p2.y-p3.y)-(p1.y-p3.y)*(p2.x-p3.x)) / (double)2.0L;
}

double CalcAffineInv( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 )
// ���ʏ��4�_�ŃA�t�B���ϊ��̕s�ϗʂ��v�Z����
{
//	���o�[�W����
//	if ( (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y) == 0 )	return 1000000;
//	else    return (double)((p3.x - p1.x) * (p4.y - p1.y) - (p4.x - p1.x) * (p3.y - p1.y)) / (double)((p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y));
//	�V�o�[�W�����iDAS�j
	double s;
	if ( (s = CalcArea(p1, p2, p3)) < 0.0001 )	s = 0.0001;
	return CalcArea(p1, p3, p4) / s ;
}

double CalcSimilarInv( cv::Point p1, cv::Point p2, cv::Point p3 )
// ���ʏ��3�_�ő����ϊ��̕s�ϗʂ��v�Z����
{
	double s;
	if ( (s = GetPointsDistance(p1, p2)) < 0.0001 )	s = 0.0001;
	return GetPointsDistance(p1, p3) / s ;
}
