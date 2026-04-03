#define	GLOBAL_DEFINE

#include <stdio.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include "def_general.h"
#include "extern.h"
#include "image.h"
#include "annex.h"
#include "annex_st.h"
#include "transform.h"
#include "annex_diff.h"
#include "clpoints.h"
#include "nstr.h"
#include "proctime.h"

unsigned char max3( unsigned char d1, unsigned char d2, unsigned char d3 )
// 3�̍ő�l��Ԃ�
{
	if ( d1 > d2 ) {
		if ( d1 > d3 )	return d1;
		else	return d3;
	}
	else {
		if ( d2 > d3 )	return d2;
		else	return d3;
	}
}

void RGB2Gray( cv::Mat diff, cv::Mat gray )
// �J���[�摜���O���C�X�P�[���ɕϊ�����iRGB�̍ő�l���Z�b�g����j
{
	int i, j, w, h, wstep_diff, wstep_gray;
	unsigned char *diff_dat;
	
	w = diff.cols;
	h = diff.rows;
	wstep_diff = (int)diff.step[0];
	wstep_gray = (int)gray.step[0];
	
	for ( j = 0; j < h; j++ ) {
		for ( i = 0; i < w; i++ ) {
			diff_dat = &diff.data[j*wstep_diff+i*3];
			gray.data[j*wstep_gray+i] = max3( diff_dat[0], diff_dat[1], diff_dat[2] );
		}
	}
}


int FindFromDefaultDir( char *fname, char *init_fn, char *out_fn )
// �J�����g�f�B���N�g�������init_fn�ɋL�q����Ă���f�B���N�g������fname��T���A�p�X���쐬����out_fn�ɓ����
// ������Ȃ����0��Ԃ�
{
	char line[kMaxLineLen];
	FILE *fp, *fp_init;
	
	if ( ( fp = fopen( fname, "r" ) ) != NULL ) {	// ���̂܂܂ő���
		strcpy( out_fn, fname );
		fclose( fp );
		return 1;
	}
	if ( fname[0] == '/' ) {	// �擪��'/'�i�f�B���N�g�����w��ς݁j
		return 0;
	}
	if ( ( fp_init = fopen( init_fn, "r" ) ) == NULL ) {	// init_fn���J���Ȃ�
		return 0;
	}
	while ( fgets( line, kMaxLineLen, fp_init ) != NULL ) {	// ��s�ǂݍ���
		if ( line[strlen( line ) - 1] == '\n' ) {	// �����ɉ��s����������
			line[strlen( line ) - 1] = '\0';
		}
		strcat(line, fname);	// �p�X���쐬
		if ( ( fp = fopen( line, "r" ) ) != NULL ) {	// �쐬�����t�@�C�����ő���
			strcpy( out_fn, line );
			fclose( fp );
			fclose( fp_init );
			return 1;
		}
	}
	return 0;
}

int InitAnnex( int argc, char *argv[], int *clcptr )
// �R�}���h���C������e��p�����[�^��ݒ肷��iannex�p�j
{
	int result, i, num, den;
	
	// �f�t�H���g�l�ɐݒ�
	eGroup1Num = kDefaultGroup1Num;
	eGroup2Num = kDefaultGroup2Num;
	eGroup3Num = kDefaultGroup3Num;
	eInvType = kDefaultInvType;
	eDiscNum = kDefaultDiscNum;
	*clcptr = kDefaultClusters;
	eResizeMode = kDefaultResizeMode;
	eOrigMode = kDefaultOrigMode;
	eTransMode = kDefaultTransMode;
	strcpy( eOutPutDir, kDefaultOutPutDir );
	eResizeMainOrig = kDefaultResizeMainOrig;
	eResizeMainAnno = kDefaultResizeMainAnno;
	eBinThr = -1;
	eClosingIter = kDefaultClosingIter;
	eMinAreaMain = kDefaultMinAreaMain;
	eDilMaskIter = kDefaultDilMaskIter;
	strcpy( eAcr2Path, kDefaultAcr2Path );
	eClScale = kDefaultClScale;
	eErodeClIter = kDefaultErodeClIter;
	eClCmpStep = kDefaultClCmpStep;
	eKMeansMaxIter = kDefaultKMeansMaxIter;
	eKMeansEpsilon = kDefaultKMeansEpsilon;
	eMinAreaCl = kDefaultMinAreaCl;
	eMaxAreaCl = kDefaultMaxAreaCl;
	eDiffNear = kDefaultDiffNear;
	eDiffLeaveThr = kDefaultDiffLeaveThr;
	eDiffEraseThr = kDefaultDiffEraseThr;
	eOrigErodeIterMain = kDefaultOrigErodeIterMain;
	eOrigGaussParamMain = kDefaultOrigGaussParamMain;
	eAnnoGaussPramMain = kDefaultAnnoGaussParamMain;

	// �p�����[�^���
	while ( ( result = getopt( argc, argv, "Ac:d:D:e:E:g:G:h:H:i:I:l:L:m:M:n:N:Oo:p:P:R:St:T:z:Z:" ) ) != -1 ) {
		switch ( result ) {
			case 'n':	// �p�����[�^n
				eGroup1Num = atoi( optarg );
				break;
			case 'm':	// �p�����[�^m
				eGroup2Num = atoi( optarg );
				break;
			case 'd':	// �p�����[�^k�i���U�����x�����j
				eDiscNum = atoi( optarg );
				break;
			case 'c':	// �N���X�^��
				*clcptr = atoi( optarg );
				break;
			case 'S':	// ���T�C�Y���[�h�i���݂͖����j
				eResizeMode = 1;
				break;
			case 'O':	// ��r�Ώۂ��I���W�i��PDF���瓾���摜�Ƃ��郂�[�h
				eOrigMode = 1;
				break;
			case 'A':	// TransMode���A�t�B���ϊ��ɂ���
				eTransMode = AFFINE;
				break;
			case 'o':	// �o�̓f�B���N�g���̐ݒ�
				strcpy( eOutPutDir, optarg );
				AddSlash( eOutPutDir, kMaxPathLen );
				break;
			case 'z':	// main�ŏk������ۂ̔{���i���摜�j
				eResizeMainOrig = atof( optarg );
				break;
			case 'Z':	// main�ŏk������ۂ̔{���i�������݉摜�j
				eResizeMainAnno = atof( optarg );
				break;
			case 't':	// ��l������ۂ�臒l
				eBinThr = atof( optarg );
				break;
			case 'h':	// �΂�΂�ɂȂ����A����������������ۂ̃����t�H���W���Z�̃p�����[�^
				eClosingIter = atoi( optarg );
				break;
			case 'M':	// main�Ńm�C�Y��������ۂ̍ŏ��ʐ�
				eMinAreaMain = atof( optarg );
				break;
			case 'D':	// �}�X�N�𑾂点��ۂ̃p�����[�^
				eDilMaskIter = atoi( optarg );
				break;
			case 'P':	// �s�ϗʂ̗��U���e�[�u�����쐬����R�}���hacr2�̃p�X
				strcpy( eAcr2Path, optarg );
				break;
			case 'L':	// �����_���o���ɏk������ۂ̔{��
				eClScale = atof( optarg );
				break;
			case 'I':	// �N���X�^�����O�̑O��Erode����ۂ̃p�����[�^
				eErodeClIter = atoi( optarg );
				break;
			case 'T':	// �N���X�^�����O�O�ɏk������ۂ̃X�e�b�v�i4�Ȃ�T�C�Y�͖�1/4�ɂȂ�j
				eClCmpStep = atoi( optarg );
				break;
			case 'i':	// �N���X�^�����O���̍ő�J��Ԃ���
				eKMeansMaxIter = atoi( optarg );
				break;
			case 'p':	// �N���X�^�����O���̐��x�p�����[�^
				eKMeansEpsilon = atof( optarg );
				break;
			case 'e':	// �����_���o�ł̃m�C�Y�������̍ŏ��ʐ�
				eMinAreaCl = atof( optarg );
				break;
			case 'E':	// �����_���o�ł̃m�C�Y�������̍ő�ʐ�
				eMaxAreaCl = atof( optarg );
				break;
			case 'N':	// �ł��߂���f��T���ۂ͈̔�
				eDiffNear = atoi( optarg );
				break;
			case 'l':	// ��������炸�ɂ��̂܂܎c���ۂ�臒l
				eDiffLeaveThr = atof( optarg );
				break;
			case 'H':	// �\���ɋ߂��Ƃ݂Ȃ��ۂ�臒l
				eDiffEraseThr = atof( optarg );
				break;
			case 'R':	// ���摜��PDF�̏ꍇ�A���摜��erode����p�����[�^
				eOrigErodeIterMain = atoi( optarg );
				break;
			case 'G':	// ���摜���X�L�����̏ꍇ�A���摜��Gaussian��������p�����[�^
				eOrigGaussParamMain = atoi( optarg );
				break;
			case 'g':	// �������݉摜��Gaussian��������p�����[�^
				eAnnoGaussPramMain = atoi( optarg );
				break;
			case ':':
				fprintf( stderr, "Error: %c needs value\n", result );
				return 0;
			case '?':
				fprintf( stderr, "Error: unknown parameter %c\n", result );
				return 0;
		}
	}
	if ( eBinThr < 0 ) {	// �ύX����Ă��Ȃ�
		if ( eOrigMode )	eBinThr = kDefaultBinThrOrig;
		else	eBinThr = kDefaultBinThrScan;
	}
	if ( optind + 1 >= argc ) {	// �t�@�C����������Ȃ�
		fprintf( stderr, "Usage: annex [option] [original image] [annotated image]\n" );
		return 0;
	}
	if ( !FindFromDefaultDir( argv[optind], kInitFileName, eOrigFileName ) ) {
		fprintf( stderr, "Error: %s cannot find\n", argv[optind] );
		return 0;
	}
	if ( !FindFromDefaultDir( argv[optind+1], kInitFileName, eAnnoFileName ) ) {
		fprintf( stderr, "Error: %s cannot find\n", argv[optind+1] );
		return 0;
	}
		
//	strcpy( eOrigFileName, argv[optind] );
//	strcpy( eAnnoFileName, argv[optind+1] );
	
	if ( eGroup1Num < eGroup2Num || eGroup2Num < eGroup3Num ) {	// n, m�̒l���`�F�b�N
		fprintf( stderr, "Error: illegal n or m\n" );
		return 0;
	}
	// nCm�̐ݒ�
	for ( i = 0, num = 1, den = 1; i < eGroup2Num; i++ ) {
		num *= eGroup1Num - i;
		den *= i + 1;
	}
	eNumCom1 = (int)(num / den);
	// mCf�̐ݒ�
	for ( i = 0, num = 1, den = 1; i < eGroup3Num; i++ ) {
		num *= eGroup2Num - i;
		den *= i + 1;
	}
	eNumCom2 = (int)(num / den);

	if ( *clcptr <= 1 || *clcptr > kMaxClusters ) {	// �N���X�^�����`�F�b�N
		fprintf( stderr, "Error: illegal number (2-%d)\n", kMaxClusters );
		return 1;
	}
	strcpy( eDiscFileName, kDefaultDiscFileName );

//	printf("%d %d %d %d %d %s %s\n", eGroup1Num, eGroup2Num, eDiscNum, *clcptr, eResizeMode, eOrigFileName, eAnnoFileName );
	
	return 1;
}

				
int main( int argc, char *argv[] )
{
	double d00;
	int cluster_count, start_all, start;
	char fname[kMaxPathLen], base[kFileNameLen];
	// �摜�f�[�^�ւ̃|�C���^�B���̂�o***, a***�Ŋm��
	cv::Mat orig, anno, orig_small, anno_small, anno_trans, orig_erode, anno_smth, diff, gray, bin, clean, small_mask, mask, dil_mask, anno_trans_big, nand, bin_erode, bin_dil;
	// �摜�f�[�^�̎��̂ŁA�������g���܂킷
	// o***: ���摜���Aa***: �������݉摜���A*6**: 600dpi�i���̃T�C�Y�j�A*3**: 300dpi�i�k���T�C�Y�j�A**3*: 3�`���l���i�J���[�j�A**1*: 1�`���l���i�O���[�X�P�[���j
	cv::Mat o631, o632, o633, o634, a631, o331, o332, o333, a331, o311, o312;
    std::vector<std::vector<cv::Point>> contours;

    cv::Vec4i hierarchy_unused;  // for hierarchy if needed
	cv::Moments mom;
	double param[kMaxParamValNum];	// ����/�A�t�B���ϊ��p�����[�^

	start_all = GetProcTimeMiliSec();
	// ������
	if ( InitAnnex( argc, argv, &cluster_count ) == 0 ) {
		fprintf( stderr, "Error: initialization\n" );
		return 1;
	}
	// �摜�̃��[�h
	start = GetProcTimeMiliSec();
	orig = cv::imread(eOrigFileName, cv::IMREAD_COLOR); if ( orig.empty() ) {
		fprintf( stderr, "Image file load error: %s\n", eOrigFileName );
		return 1;
	}
	anno = cv::imread(eAnnoFileName, cv::IMREAD_COLOR); if ( anno.empty() ) {
		fprintf( stderr, "Image file load error: %s\n", eAnnoFileName );
		return 1;
	}
	printf("Loading image: %d msec\n", GetProcTimeMiliSec() - start );
	// �摜�f�[�^�̎��̂��쐬
	o631 = orig;
	o632 = cv::Mat::zeros(o631.size(), CV_8UC3);
	o633 = cv::Mat::zeros(o631.size(), CV_8UC3);
	o634 = cv::Mat::zeros(o631.size(), CV_8UC3);
	a631 = anno;
	o331 = cv::Mat::zeros( cv::Size( (int)(o631.cols * eResizeMainOrig), (int)(o631.rows * eResizeMainOrig) ), CV_8UC3 );
	o332 = cv::Mat::zeros(o331.size(), CV_8UC3);
	o333 = cv::Mat::zeros(o331.size(), CV_8UC3);
	a331 = cv::Mat::zeros( cv::Size( (int)(a631.cols * eResizeMainAnno), (int)(a631.rows * eResizeMainAnno) ), CV_8UC3 );
	o311 = cv::Mat::zeros(o331.size(), CV_8UC1);
	o312 = cv::Mat::zeros(o331.size(), CV_8UC1);
	
#ifdef	COR_PRESET_MODE	// �Ή��_�蓮�ݒ胂�[�h�i���݂͖����j
	if ( TransformAnnoPreset( anno_small, orig_small, &param_small ) == 0 ) {
		fprintf( stderr, "�Ή����Ă��Ȃ��\��������܂��B\n" );
	}
#else
	// �ϊ��p�����[�^�̎擾
	start = GetProcTimeMiliSec();
	if ( TransformAnno( anno, orig, cluster_count, param ) == 0 ) {
		fprintf( stderr, "Warning: TransformAnno returned 0\n" );
	}
#ifdef	CHK_COR
	fprintf( stderr, "exit(CHK_COR defined)\n" );
	return 1;
#endif
	// ����ꂽ�p�����[�^��p���ď������݉摜�����摜�ɍ��킹��
	anno_trans_big = o634;
	if ( eTransMode == SIMILAR ) {
		SimilarTransformation( anno, anno_trans_big, param );
	}
	else if ( eTransMode == AFFINE ) {
		AffineTransformation( anno, anno_trans_big, param );
	}
	// �ȍ~�̏����̌y���̂��ߏk��
	orig_small = o331;
//	anno_small = a331;
	cv::resize( orig, orig_small, orig_small.size(), 0, 0, cv::INTER_NEAREST );
//	cv::resize( anno, anno_small, anno_small.size(), 0, 0, cv::INTER_NEAREST );
	anno_trans = o332;
	cv::resize( anno_trans_big, anno_trans, anno_trans.size(), 0, 0, cv::INTER_NEAREST );

//	SimilarTransformation( anno_small, anno_trans, &param_small );
	printf("Transforming image: %d msec\n", GetProcTimeMiliSec() - start );
#endif
	GetBasename( eAnnoFileName, kMaxPathLen, base );	// �����ߒ��̉摜�o�͂̂��߂Ƀt�@�C�������擾
#ifdef	ANNEX_OUTPUT_IMAGE	// �����ߒ��̉摜���o��
	sprintf( fname, "%s%s06trans.jpg", eOutPutDir, base );
	cv::imwrite( fname, anno_trans );
#endif
	// ���摜��erode���A�������݉摜���X���[�W���O����
	start = GetProcTimeMiliSec();
	orig_erode = o333;
//	cv::erode(orig_small, orig_erode, cv::Mat(), cv::Point(-1,-1), 15);
	if ( eOrigMode ) {	// ���摜���I���W�i���̏ꍇ�A�ɂ��݂��Č����邽�ߑ��点��K�v������
		cv::erode( orig_small, orig_erode, cv::Mat(), cv::Point(-1,-1), eOrigErodeIterMain );
	}
	else {	// ���摜���X�L�������ꂽ���̂̏ꍇ�ł��A�K�E�V�A���t�B���^�Ńh�b�g�p�^�[�����ڂ���
//		cv::erode(orig_small, orig_erode, cv::Mat(), cv::Point(-1,-1), 10);
		cv::GaussianBlur(orig_small, orig_erode, cv::Size(eOrigGaussParamMain, eOrigGaussParamMain), 0);
	}
//		orig_small.copyTo(orig_erode);
//	cv::imwrite( "orig_erode.jpg", orig_erode );
	anno_smth = o331;
	cv::GaussianBlur(anno_trans, anno_smth, cv::Size(eAnnoGaussPramMain, eAnnoGaussPramMain), 0);
//	anno_trans.copyTo(anno_smth);
//	cv::imwrite( "anno_smth.jpg", anno_smth );
	printf("Eroding image: %d msec\n", GetProcTimeMiliSec() - start);
	// ���������
	start = GetProcTimeMiliSec();
	diff = o332;
	GetDiffImage( diff, anno_smth, orig_erode, 1 );
	printf("Making diff image: %d msec\n", GetProcTimeMiliSec() - start );
#ifdef	ANNEX_OUTPUT_IMAGE
	sprintf( fname, "%s%s07diff.jpg", eOutPutDir, base );
	cv::imwrite( fname, diff );
#endif
	// �O���C�X�P�[���ɕϊ�����i��l���̂��߁j
	start = GetProcTimeMiliSec();
	gray = o311;
//	cv::cvtColor( diff, gray, CV_RGB2GRAY );
	RGB2Gray( diff, gray );
	// 2�l������
	bin = o312;
	
	cv::threshold( gray, bin, eBinThr, kBinMaxVal, cv::THRESH_BINARY );
//	if ( eOrigMode ) {
//		cv::threshold( gray, bin, kDefaultBinThrOrig, kBinMaxVal, cv::THRESH_BINARY );
//	}
//	else {
//		cv::threshold( gray, bin, kDefaultBinThrScan, kBinMaxVal, cv::THRESH_BINARY );
//	}

	printf("Binarizing image: %d msec\n", GetProcTimeMiliSec() - start);
#ifdef	ANNEX_OUTPUT_IMAGE
	sprintf( fname, "%s%s08bin.jpg", eOutPutDir, base );
	cv::imwrite( fname, bin );
#endif
	// �����t�H���W���Z�iClosing�j�ɂ��΂�΂�ɂȂ����������݂���������
	bin_dil = o311;
	cv::dilate( bin, bin_dil, cv::Mat(), cv::Point(-1,-1), eClosingIter );
//	cv::imwrite( "dil.jpg", bin_dil );
	bin_erode = o312;
	cv::erode( bin_dil, bin_erode, cv::Mat(), cv::Point(-1,-1), eClosingIter );
//	cv::imwrite( "erode.jpg", bin_erode );
	bin = bin_erode;
	// �A�������𒊏o���A�ʐςɊ�Â��ăm�C�Y����������
	start = GetProcTimeMiliSec();
	clean = o311;
	clean.setTo(0);
	cv::findContours(bin, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// �A�������𒊏o����
	for( int _ci = 0; _ci < (int)contours.size(); _ci++ ) {		// �A��������`��
		mom = cv::moments(contours[_ci]);
		d00 = mom.m00;	// �ʐς����߂�
		if ( d00 < eMinAreaMain ) continue;	// ����������A�������͏��O
		cv::drawContours(clean, contours, _ci, cWhite, cv::FILLED, kDrawContourLineType);
	}
	printf("Cleaning image: %d msec\n", GetProcTimeMiliSec() - start);
#ifdef	ANNEX_OUTPUT_IMAGE
	sprintf( fname, "%s%s09clean.jpg", eOutPutDir, base );
	cv::imwrite( fname, clean );
#endif
	// �}�X�N���쐬����
	start = GetProcTimeMiliSec();
	small_mask = o331;
	cv::cvtColor( clean, small_mask, cv::COLOR_GRAY2BGR );	// 3�`���l���ɕϊ�����
	mask = o631;
	cv::resize( small_mask, mask, mask.size(), 0, 0, cv::INTER_NEAREST );
	dil_mask = o632;
	cv::dilate( mask, dil_mask, cv::Mat(), cv::Point(-1,-1), eDilMaskIter );	// �}�X�N��dilate�ő��点��
	printf("Making mask: %d msec\n", GetProcTimeMiliSec() - start);
//	cv::imwrite( "dil_mask.jpg", dil_mask );
	// 600dpi��anno���쐬����
	start = GetProcTimeMiliSec();
//	anno_trans_big = o631;
//	SimilarTransformation( anno, anno_trans_big, &param );
//	cv::imwrite( "anno_trans_big.jpg", anno_trans_big );
	printf("Transforming: %d msec\n", GetProcTimeMiliSec() - start);
	// �}�X�N��p���ď������ݒ��o����
	start = GetProcTimeMiliSec();
	nand = o633;
//	MaskImage( anno_trans_big, dil_mask, nand );
	cv::bitwise_not( anno_trans_big, anno_trans_big );
	cv::bitwise_and( anno_trans_big, dil_mask, nand );
	cv::bitwise_not( nand, nand );
	printf("And image: %d msec\n", GetProcTimeMiliSec() - start );
	sprintf( fname, "%s%s10and.jpg", eOutPutDir, base );
	cv::imwrite( fname, nand );
	printf("Total: %d msec\n", GetProcTimeMiliSec() - start_all );
	
	return 0;
}

void MaskImage( cv::Mat img, cv::Mat mask, cv::Mat dst )
// �}�X�N�������s���i���삪�����������ߎg�p�����j
{
	int i;
	
	for ( i = 0; i < (img.total() * img.elemSize()); i++ ) {
		dst.data[i] = !(unsigned char) ( !((unsigned char)img.data[i]) & (unsigned char)mask.data[i] );
		if ( dst.data[i] != 0 ) {
			printf("%02x ", dst.data[i]);
		}
	}
}
