#include "def_general.h"
#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <vector>

#include "f_points.h"
#include "dirs.h"

int MakeFeaturePointsFromImage(cv::Point **ps, cv::Mat img, cv::Size *size)
// 閉領域から特徴点を得る
{
	int i, num = 0;
    std::vector<std::vector<cv::Point>> contours;

    std::vector<cv::Vec4i> hierarchy;
    cv::Mat bin, p;

	size->width = img.cols;
	size->height = img.rows;

	bin = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);
//	cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 31, 10 );
	cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	OutPutImage( bin );
	cv::floodFill(bin, cv::Point(300,300), cBlack, nullptr, cv::Scalar::all(0), cv::Scalar::all(0), 4);
	OutPutImage( bin );
    cv::findContours(bin, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// 連結成分を抽出する

	num = CalcCentres(ps, contours, hierarchy, size, NULL);	// 各連結成分の重心を計算する

	p = cv::Mat::zeros(cv::Size(size->width, size->height), CV_8UC1);
	for ( i = 0; i < num; i++ ) {
		cv::circle( p, (*ps)[i], 8, cWhite, -1, 8, 0 );
	}
//	printf("%d\n", num);
	OutPutImage( p );

	return num;
}
