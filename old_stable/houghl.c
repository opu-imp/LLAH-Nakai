#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <vector>

#include "dirs.h"
#include "hist.h"
#include "houghl.h"

void CalcHLHist( strHist *hist, strLineForCR *lines, int line_num, int ctype, int corder );

void CalcHoughLineCRHist(cv::Point *ps, int pt_num,  cv::Size size)
// �_�̏W����Hough�ϊ����Ē��������o���C����̓_���畡������߂ăq�X�g�O�������쐬����
{
	int i, line_num;
	cv::Mat cent;
	strLineForCR *lines = NULL;
	strHist hist;

	cent = cv::Mat::zeros(size, CV_8UC1);
	cent.setTo(cv::Scalar::all(0));

	for ( i = 0; i < pt_num; i++ ) {
		cv::circle( cent, ps[i], 0, cWhite, -1, 1, 0 );	// �_�̕`��
	}

	line_num = CalcHoughLinesImg(cent, &lines);	// �摜����Hough�ϊ��Œ��������߂�

//	for ( i = 0; i < line_num; i++ )	printf("%f %f\n", lines[i].rho, lines[i].theta);

	for ( i = 0; i < pt_num; i++ ) {
		ResisterPointToLines( ps[i], lines, line_num );
	}
/*
	for ( i = 0; i < line_num; i++ ) {
		printf("line%02d\n", i);
		cur = lines[i].first;
		while ( cur != NULL) {
			printf("%d %d\n", cur->pt.x, cur->pt.y );
			cur = cur->next;
		}
	}
*/
	InitHist(&hist, 200, 0.0, 10.0);
	CalcHLHist(&hist, lines, line_num, kUseCR, kAllAssort);
//	CalcHLHist(&hist, lines, line_num, kUseCR, kSeqOrder);
	OutPutHist(&hist, 200, 100);
//	DisplayHistNum(&hist);

}

void CalcHLHist( strHist *hist, strLineForCR *lines, int line_num, int ctype, int corder )
// ������q�X�g�O�������쐬����i�q�X�g�O�����͏������ς�
{
	int i, j, pt_num, p1, p2, p3, p4;
	double cr = 0.0;
	cv::Point pts[kLinePoints];
	strPtList *cur;

	for ( i = 0; i < line_num; i++ ) {
		cur = lines[i].first;
		j = 0;
		while ( cur != NULL) {	// �_��pts�ɃR�s�[
			pts[j].x = cur->pt.x;
			pts[j].y = cur->pt.y;
			cur = cur->next;
			j++;
		}
		pt_num = j;
		switch (corder) {
			case kSeqOrder:	// �擪����4����
                for ( j = 0; j+4 <= pt_num; j++ ) {
					switch (ctype) {
						case kUseCR:
                            cr = CalcCR(pts[j], pts[j+1], pts[j+2], pts[j+3]);
							break;
						case kUseJI:
							cr = CalcJI(pts[j], pts[j+1], pts[j+2], pts[j+3]);
							break;
					}
                    printf("%f\n", cr);
                    AddDataHist(hist, cr);
                    }
				break;
			case kAllAssort:	// ���ׂĂ̑g�ݍ��킹
				for ( p1 = 0; p1 < pt_num; p1++ ) {
					for ( p2 = 0; p2 < pt_num; p2++ ) {
						if ( p2 == p1 ) continue;
						for ( p3 = 0; p3 < pt_num; p3++ ) {
							if ( p3 == p1 || p3 == p2 ) continue;
							for ( p4 = 0; p4 < pt_num; p4++ ) {
								if ( p4 == p1 || p4 == p2 || p4 == p3 ) continue;
								switch (ctype) {
									case kUseCR:
										cr = CalcCR(pts[p1], pts[p2], pts[p3], pts[p4]);
										break;
									case kUseJI:
										cr = CalcJI(pts[p1], pts[p2], pts[p3], pts[p4]);
										break;
								}
//								printf("%f\n", cr);
								AddDataHist(hist, cr);
							}
						}
					}
				}
				break;
		}
	}
}

void ResisterPointToLines( cv::Point pt, strLineForCR *lines, int line_num )
// �_���߂������ɓo�^����
{
	int i;

	for ( i = 0; i < line_num; i++ ) {
		if ( IsCloseToLine(pt, &lines[i]) ) {
			AddPoint(pt, &lines[i]);
		}
	}
}

void AddPoint( cv::Point pt, strLineForCR *line )
// �_�𒼐��̓_���X�g�ɒǉ�����
{
	strPtList *newpl = (strPtList *)calloc(1, sizeof(strPtList));
	strPtList *cur;
	
	newpl->pt.x = pt.x;
	newpl->pt.y = pt.y;
	newpl->next = NULL;
	if ( line->first == NULL) {
		line->first = newpl;
		return;
	}
	cur = line->first;
	while ( cur->next != NULL)
		cur = cur->next;
	cur->next = newpl;
}



int IsCloseToLine( cv::Point pt, strLineForCR *line )
// �_�ƒ������߂������肷��
{
	if ( fabs((pt.x * cos(line->theta) + pt.y * sin(line->theta)) - line->rho) < kCloseToLine )
		return 1;
	else
		return 0;
}

int x2yrh(int x, float rho, float theta)
//	rho-theta�^�̒����̎���x����y�����߂�
{
	return (int)((rho - x * cos(theta))/sin(theta));
}

int y2xrh(int y, float rho, float theta)
{
	return (int)((rho - y * sin(theta))/cos(theta));
}

int CalcHoughLinesImg(cv::Mat dst, strLineForCR **line_cr)
{
//	cv::Mat  dst = cv::imread("htest.bmp", cv::IMREAD_GRAYSCALE);
    cv::Mat color_dst;
    std::vector<std::vector<cv::Point>> contours;

    cv::Vec4i hierarchy_unused;  // for hierarchy if needed
    // lines_vec declared at HoughLines call site
    // (CvSeq* lines removed)
    int i, w, h;

//	dst1 = cv::Mat::zeros(dst.size(), CV_8UC3);
	color_dst = cv::Mat::zeros(dst.size(), CV_8UC1);
	color_dst.setTo(cv::Scalar::all(0));
	color_dst = dst.clone();
	w = color_dst.cols;
	h = color_dst.rows;

//    cv::cvtColor( dst, color_dst, cv::COLOR_GRAY2BGR );

	cv::dilate(dst, color_dst, cv::Mat(), cv::Point(-1,-1), 5);
	OutPutImage(color_dst);

//    lines = cvHoughLines2( dst, storage, CV_HOUGH_STANDARD, 2, CV_PI/180, /*150*/4, 0, 0 );
//htest.bmp�p    lines = cvHoughLines2( dst, storage, CV_HOUGH_STANDARD, 1, CV_PI/90, /*150*/3, 0, 0 );
    std::vector<cv::Vec2f> lines_vec;
    cv::HoughLines(dst, lines_vec, 8, CV_PI/180, /*150*/10);

	*line_cr = (strLineForCR *)calloc((int)lines_vec.size(), sizeof(strLineForCR));

    for( i = 0; i < (int)lines_vec.size(); i++ )
    {
        float* line = (float*)&lines_vec[i];
        float rho = line[0];
        float theta = line[1];
		cv::Point pt[2];
		int xt, yt, count = 0;
//        double a = cos(theta), b = sin(theta);

		(*line_cr)[i].rho = rho;
		(*line_cr)[i].theta = theta;

//		printf("%f %f\n", rho, theta);
//		printf("(0, %d) (%d, %d) (%d, 0) (%d, %d)\n", x2yrh(0, rho, theta), w, x2yrh(w, rho, theta), y2xrh(0, rho, theta), h, y2xrh(h, rho, theta));
		// �ȉ��C�������������߂̋N�_�ƏI�_��������D�摜���o�͂��Ȃ��̂Ȃ�s�v�D
		yt = x2yrh(0, rho, theta);
		if ( yt >= 0 && yt < h && count < 2) {
			pt[count].x = 0;
			pt[count].y = yt;
			count++;
		}
		yt = x2yrh(w, rho, theta);
		if ( yt >= 0 && yt < h && count < 2) {
			pt[count].x = w;
			pt[count].y = yt;
			count++;
		}
		xt = y2xrh(0, rho, theta);
		if ( xt >= 0 && xt < w && count < 2) {
			pt[count].x = xt;
			pt[count].y = 0;
			count++;
		}
		xt = y2xrh(h, rho, theta);
		if ( xt >= 0 && xt < w && count < 2) {
			pt[count].x = xt;
			pt[count].y = h;
			count++;
		}
        cv::line( color_dst, pt[0], pt[1], cWhite, 4, 8, 0 );

		//�ȉ��ɂ����������������v���O�����������ɊԈ���Ă����D�T���v���Ƃ����ǐM�p�ł��Ȃ��ȁD
	}

//	OutPutImage(dst);
	OutPutImage(color_dst);

	return (int)lines_vec.size();
}
