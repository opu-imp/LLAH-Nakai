#pragma once

class cFpTuneCap
{
private:
	cv::Mat gray, inv;
public:
	cv::Mat img, bin1, smt, bin2, pt, overlap, cor;
	cv::Point2f *pfs;
	int adp_block, adp_sub, gparam, thr;

	cFpTuneCap( cv::Mat cap, int def_adp_block, int def_adp_sub, int def_gparam, int def_thr );
	int Extract( void );
	int DrawCor( int pcor[][2], cv::Point2f *corpfs,  cv::Point cororig );
public:
	~cFpTuneCap( void );
};
