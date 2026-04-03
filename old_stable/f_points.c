#include "def_general.h"
#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <vector>

#include "f_points.h"
#include "dirs.h"

int MakeFeaturePointsFromImage(cv::Point **ps, cv::Mat img, cv::Size *size)
// �̈悩������_�𓾂�
{
	int i, num = 0;
    std::vector<std::vector<cv::Point>> contours;

    cv::Vec4i hierarchy_unused;  // for hierarchy if needed
    cv::Mat bin, p;

	size->width = img.cols;
	size->height = img.rows;

	bin = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);
//	cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 31, 10 );
	cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	OutPutImage( bin );
	cv::floodFill(bin, cv::Point(300,300), cBlack, nullptr, cv::Scalar::all(0), cv::Scalar::all(0), 4);
	OutPutImage( bin );
    cv::findContours(bin, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// �A�������𒊏o����

	num = CalcCentres(ps, contours, size, NULL);	// �e�A�������̏d�S���v�Z����

	p = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);
	for ( i = 0; i < num; i++ ) {
		cv::circle( p, (*ps)[i], 8, cWhite, -1, 8, 0 );
	}
//	printf("%d\n", num);
	OutPutImage( p );

	return num;
}
