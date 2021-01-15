#pragma once

class cFpTuneCap
{
private:
	IplImage *gray, *inv;
public:
	IplImage *img, *bin1, *smt, *bin2, *pt, *overlap, *cor;
	CvPoint2D32f *pfs;
	int adp_block, adp_sub, gparam, thr;

	cFpTuneCap( IplImage *cap, int def_adp_block, int def_adp_sub, int def_gparam, int def_thr );
	int Extract( void );
	int DrawCor( int pcor[][2], CvPoint2D32f *corpfs,  CvPoint cororig );
public:
	~cFpTuneCap( void );
};
