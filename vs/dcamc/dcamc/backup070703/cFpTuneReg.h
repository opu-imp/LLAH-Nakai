#pragma once

class cFpTuneReg
{
private:
	IplImage *inv;
public:
	IplImage *img, *smt1, *bin1, *smt2, *bin2, *pt, *overlap, *cor;
	CvPoint2D32f *pfs;
	int gparam1, adp_block, adp_sub, gparam2, thr;

	cFpTuneReg( char *fname, int def_gparam1, int def_adp_block, int def_adp_sub, int def_gparam2, int def_thr );
	int Extract( void );
	int DrawCor( int pcor[][2], CvPoint2D32f *corpfs,  CvPoint cororig );
public:
	~cFpTuneReg( void );
};
