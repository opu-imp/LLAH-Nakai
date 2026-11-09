#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include "hist.h"
#include "dirs.h"

int InitHist( strHist *hist, int size, double min, double max )
// ヒストグラムを初期化する
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
// ヒストグラムを開放する
{
	free(hist->bin);
}
int AddDataHist( strHist *hist, double dat )
// ヒストグラムにデータを追加する
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
// ヒストグラムの最大のものをもつbinの右端を返す
{
	int i, max = 0, max_bin = 0;
	
	for ( i = 0; i < hist->size; i++ ) {
		if ( hist->bin[i] > max ) {
			max = hist->bin[i];
			max_bin = i;
		}
	}
	return (((hist->max - hist->min) / (double)hist->size) * (double)(max_bin+1));
}

void OutPutHist( strHist *hist, int width, int height )
// ヒストグラムを描画してファイルに出力する
{
	int i;
	CvPoint p1, p2;

	IplImage *img = cvCreateImage( cvSize(width, height), 8, 1);
	cvZero(img);

	for ( i = 0; i < hist->size; i++ ) {
		p1.x = (int)(((double)width / (double)hist->size)*(double)i);
		p1.y = (int)((double)height - ((double)height/(double)hist->max_freq)*(double)hist->bin[i]);
		p2.x = (int)(((double)width / (double)hist->size)*((double)i+1));
		p2.y = height;
		cvRectangle( img, p1, p2, cWhite, CV_FILLED, 8, 0 );
	}
	OutPutImage( img );
}

void DisplayHistNum(strHist *hist)
// ヒストグラムを画面に出力する
{
	int i;

	for ( i = 0; i < hist->size; i++ ) {
		printf("%03d : %d\n", i, hist->bin[i]);
	}
	printf("max:%d\n", hist->max_freq);
}

void HistTest( void )
{
    IplImage* src;
    if( (src = cvLoadImage("img_0001.jpg", 1))!= 0)
    {
        IplImage* h_plane = cvCreateImage( cvGetSize(src), 8, 1 );
        IplImage* s_plane = cvCreateImage( cvGetSize(src), 8, 1 );
        IplImage* v_plane = cvCreateImage( cvGetSize(src), 8, 1 );
        IplImage* planes[] = { h_plane, s_plane };
        IplImage* hsv = cvCreateImage( cvGetSize(src), 8, 3 );
        int h_bins = 30, s_bins = 32;
        int hist_size[] = {h_bins, s_bins};
        float h_ranges[] = { 0, 180 }; /* hue varies from 0 (~0°red) to 180 (~360°red again) */
        float s_ranges[] = { 0, 255 }; /* saturation varies from 0 (black-gray-white) to 255 (pure spectrum color) */
        float* ranges[] = { h_ranges, s_ranges };
        int scale = 10;
        IplImage* hist_img = cvCreateImage( cvSize(h_bins*scale,s_bins*scale), 8, 3 );
        CvHistogram* hist;
        float max_value = 0;
        int h, s;

        cvCvtColor( src, hsv, CV_BGR2HSV );
        cvCvtPixToPlane( hsv, h_plane, s_plane, v_plane, 0 );
        hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY, ranges, 1 );
        cvCalcHist( planes, hist, 0, 0 );
        cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
        cvZero( hist_img );

        for( h = 0; h < h_bins; h++ )
        {
            for( s = 0; s < s_bins; s++ )
            {
                float bin_val = cvQueryHistValue_2D( hist, h, s );
                int intensity = cvRound(bin_val*255/max_value);
                cvRectangle( hist_img, cvPoint( h*scale, s*scale ),
                             cvPoint( (h+1)*scale - 1, (s+1)*scale - 1),
                             CV_RGB(intensity,intensity,intensity), /* graw a grayscale histogram.
                                                                       if you have idea how to do it
                                                                       nicer let us know */
                             1, CV_FILLED, 0 );
            }
        }

        cvNamedWindow( "Source", 1 );
        cvShowImage( "Source", src );

        cvNamedWindow( "H-S Histogram", 1 );
        cvShowImage( "H-S Histogram", hist_img );

        cvWaitKey(0);
    }
}
