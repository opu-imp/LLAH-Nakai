#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "hist.h"
#include "cvtest.h"

int InitHist( strHist *hist, int size, double min, double max )
// �q�X�g�O����������������
{
	if ( size <= 0 || min >= max )	return 0;
	hist->size = size;
	hist->bin = (int *)calloc(size, sizeof(int));
	hist->min = min;
	hist->max = max;
	hist->max_freq = 0;
	return 1;
}

void ReleaseHist( strHist *hist )
// �q�X�g�O�������J������
{
	free(hist->bin);
}
int AddDataHist( strHist *hist, double dat )
// �q�X�g�O�����Ƀf�[�^��ǉ�����
{
	int idx;

	if ( dat < hist->min || dat > hist->max )
		return 0;
	idx = (int)((dat - hist->min) / ((hist->max - hist->min) / hist->size));
	hist->bin[idx]++;
	if ( hist->bin[idx] > hist->max_freq )
		hist->max_freq++;
	return 1;
}

double GetMaxBin( strHist *hist )
// �q�X�g�O�����̍ő�̂��̂�����bin�̉E�[��Ԃ�
{
	int i, max = 0, max_bin;
	
	for ( i = 0; i < hist->size; i++ ) {
		if ( hist->bin[i] > max ) {
			max = hist->bin[i];
			max_bin = i;
		}
	}
	return (((hist->max - hist->min) / (double)hist->size) * (double)(max_bin+1));
}

void OutPutHist( strHist *hist, int width, int height )
// �q�X�g�O������`�悵�ăt�@�C���ɏo�͂���
{
	int i;
	cv::Point p1, p2;

	cv::Mat img = cv::Mat::zeros(cv::Size(width, height), CV_8UC1);
	img.setTo(cv::Scalar::all(0));

	for ( i = 0; i < hist->size; i++ ) {
		p1.x = (int)((double)width / (double)hist->size)*(double)i;
		p1.y = (int)(double)height - ((double)height/(double)hist->max_freq)*(double)hist->bin[i];
		p2.x = (int)((double)width / (double)hist->size)*((double)i+1);
		p2.y = height;
		cv::rectangle( img, p1, p2, cWhite, cv::FILLED, 8, 0 );
	}
	OutPutImage( img );
}

void DisplayHistNum(strHist *hist)
// �q�X�g�O��������ʂɏo�͂���
{
	int i;

	for ( i = 0; i < hist->size; i++ ) {
		printf("%03d : %d\n", i, hist->bin[i]);
	}
	printf("max:%d\n", hist->max_freq);
}

void HistTest( void )
{
    cv::Mat src = cv::imread("img_0001.jpg", cv::IMREAD_COLOR);
    if( !src.empty() )
    {
        cv::Mat hsv;
        int h_bins = 30, s_bins = 32;
        int hist_size[] = {h_bins, s_bins};
        float h_ranges[] = { 0, 180 };
        float s_ranges[] = { 0, 255 };
        const float* ranges[] = { h_ranges, s_ranges };
        int channels[] = {0, 1};
        int scale = 10;
        cv::Mat hist_img = cv::Mat::zeros(cv::Size(h_bins*scale, s_bins*scale), CV_8UC3);
        cv::Mat hist;
        double max_value = 0;

        cv::cvtColor( src, hsv, cv::COLOR_BGR2HSV );
        cv::calcHist( &hsv, 1, channels, cv::Mat(), hist, 2, hist_size, ranges, true, false );
        cv::minMaxLoc( hist, 0, &max_value, 0, 0 );

        for( int h = 0; h < h_bins; h++ )
        {
            for( int s = 0; s < s_bins; s++ )
            {
                float bin_val = hist.at<float>(h, s);
                int intensity = cvRound(bin_val * 255 / max_value);
                cv::rectangle( hist_img, cv::Point( h*scale, s*scale ),
                             cv::Point( (h+1)*scale - 1, (s+1)*scale - 1),
                             cv::Scalar(intensity,intensity,intensity),
                             cv::FILLED, cv::LINE_8, 0 );
            }
        }

        cv::namedWindow( "Source", 1 );
        cv::imshow( "Source", src );

        cv::namedWindow( "H-S Histogram", 1 );
        cv::imshow( "H-S Histogram", hist_img );

        cv::waitKey(0);
    }
}
