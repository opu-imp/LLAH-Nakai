#include "def_general.h"

#include <stdio.h>
#include <math.h>
#include <cv.h>			// OpneCV
#include <highgui.h>	// OpenCV
#include <windows.h>
#include <mmsystem.h>
#include <dshow.h>
#include <conio.h>
#include <qedit.h>
#include "dirs.h"
#include "extern.h"
#include "hist.h"
#include "proj4p.h"
#include "dscap.h"
#include "dcamc.h"
#include "init.h"
#include "fptune.h"
#include "cFpTuneReg.h"
#include "cFpTuneCap.h"

cFpTuneReg *ptr_reg = NULL;
cFpTuneCap *ptr_cap = NULL;
int show_mode = SHOW_REG_IMG, cap_show_mode = SHOW_CAP_IMG, paused = -1;
CvPoint caporig, regorig;

void TrackCallbackReg( int param )
// �o�^�����̃g���b�N�o�[�̃R�[���o�b�N�֐�
{
	if ( ptr_reg != NULL ) {
		ptr_reg->Extract();
		switch ( show_mode ) {
			case SHOW_REG_IMG:
				cvShowImage( "Registered", ptr_reg->img );
				break;
			case SHOW_REG_SMT1:
				cvShowImage( "Registered", ptr_reg->smt1 );
				break;
			case SHOW_REG_BIN1:
				cvShowImage( "Registered", ptr_reg->bin1 );
				break;
			case SHOW_REG_SMT2:
				cvShowImage( "Registered", ptr_reg->smt2 );
				break;
			case SHOW_REG_BIN2:
				cvShowImage( "Registered", ptr_reg->bin2 );
				break;
			case SHOW_REG_PT:
				cvShowImage( "Registered", ptr_reg->pt );
				break;
			case SHOW_REG_OVER:
				cvShowImage( "Registered", ptr_reg->overlap );
				break;
		}
	}
}

void TrackCallbackCap( int param )
// �L���v�`���摜�̃g���b�N�o�[�̃R�[���o�b�N�֐�
{
	if ( ptr_cap != NULL && paused > 0 ) {
		ptr_cap->Extract();
		switch ( cap_show_mode ) {
			case SHOW_CAP_IMG:
				cvShowImage( "Capture", ptr_cap->img );
				break;
			case SHOW_CAP_BIN1:
				cvShowImage( "Capture", ptr_cap->bin1 );
				break;
			case SHOW_CAP_SMT:
				cvShowImage( "Capture", ptr_cap->smt );
				break;
			case SHOW_CAP_BIN2:
				cvShowImage( "Capture", ptr_cap->bin2 );
				break;
			case SHOW_CAP_PT:
				cvShowImage( "Capture", ptr_cap->pt );
				break;
			case SHOW_CAP_OVER:
				cvShowImage( "Capture", ptr_cap->overlap );
				break;
		}
	}
}

int TuneFeaturePointParam( char *fname )
// �����_���o�p�����[�^�̎蓮�`���[�j���O�c�[��
{
	int key;
	long buff_size;
	unsigned char *img_buff;
	IplImage *img_cap;
	CvSize img_size;
	strDirectShowCap dsc;
	// �o�^�摜�̓����_���o�N���X
	cFpTuneReg c_reg( fname, kDefaultTuneRegGParam1, kDefaultTuneRegAdpBlock, kDefaultTuneRegAdpSub, kDefaultTuneRegGParam2, kDefaultTuneRegThr );
	// �L���v�`���摜�̓����_���o�N���X
//	cFpTuneCap *ptr_cap;

	/********** �o�^�摜���̏��� **********/
	ptr_reg = &c_reg;
	cvNamedWindow( "Registered", CV_WINDOW_AUTOSIZE );
	cvCreateTrackbar( "GParam1", "Registered", &c_reg.gparam1, kTuneGParam1Max, TrackCallbackReg );
	cvCreateTrackbar( "AdpBlock", "Registered", &c_reg.adp_block, kTuneAdpBlockMax, TrackCallbackReg );
	cvCreateTrackbar( "AdpSub", "Registered", &c_reg.adp_sub, kTuneAdpSubMax, TrackCallbackReg );
	cvCreateTrackbar( "GParam2", "Registered", &c_reg.gparam2, kTuneGParam2Max, TrackCallbackReg );
	cvCreateTrackbar( "Thr", "Registered", &c_reg.thr, kTuneThrMax, TrackCallbackReg );
//	cvCreateTrackbar( "Mode", "Registered", &show_mode, SHOW_REG_MAX, TrackCallbackReg );
	cvShowImage( "Registered", c_reg.img );	// �摜��\��
	/********** �L���v�`���摜���̏��� **********/
	if ( ReadIniFile() == 0 ) {	// ini�t�@�C���̓ǂݍ��݂Ɏ��s
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 1;
	}
	InitDirectShowCap( eCamConfNum, &dsc, &(img_size.width), &(img_size.height) );	// USB�J������������
	buff_size = img_size.width * img_size.height * 3;
	img_buff = (unsigned char *)malloc( buff_size );	// �摜�̃o�b�t�@���m��
	img_cap = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// �摜���쐬
	// �����_���o�N���X���쐬
	ptr_cap = new cFpTuneCap( img_cap, kDefaultTuneCapAdpBlock, kDefaultTuneCapAdpSub, kDefaultTuneCapGParam, kDefaultTuneCapThr );
	StartDirectShowCap( &dsc );	// �L���v�`�����J�n
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// �E�B���h�E���쐬
	cvCreateTrackbar( "AdpBlock", "Capture", &ptr_cap->adp_block, kTuneCapAdpBlockMax, TrackCallbackCap );
	cvCreateTrackbar( "AdpSub", "Capture", &ptr_cap->adp_sub, kTuneCapAdpSubMax, TrackCallbackCap );
	cvCreateTrackbar( "GParam", "Capture", &ptr_cap->gparam, kTuneCapGParamMax, TrackCallbackCap );
	cvCreateTrackbar( "Thr", "Capture", &ptr_cap->thr, kTuneCapThrMax, TrackCallbackCap );
	// �E�B���h�E��z�u�i���̏ꏊ����ړ�����Ȃ����̂Ƃ���j
	cvMoveWindow( "Capture", 0, kTaskBarHeight );
	cvMoveWindow( "Registered", kCvWindowLeftMargin + kCvWindowRightMargin + ptr_cap->img->width, kTaskBarHeight );
	// ���_�̑��ΓI�ȍ��W
	regorig.x = ptr_cap->img->width + kCvWindowLeftMargin + kCvWindowRightMargin;
	regorig.y = 0;
	caporig.x = - kCvWindowLeftMargin - kCvWindowRightMargin - ptr_cap->img->width;
	caporig.y = 0;
	/********** ���[�v���� **********/
	for ( ; ; ) {
		if ( paused < 0 ) {	// �|�[�Y����Ă��Ȃ�
			// �L���v�`��
			CaptureDirectShowCap( &dsc, img_buff, buff_size );
			// �o�b�t�@���摜�ɕϊ�
			Buff2ImageData( img_buff, img_cap );
			ptr_cap->Extract();
			ptr_cap->DrawCor( NULL, ptr_reg->pfs, regorig );
			ptr_reg->DrawCor( NULL, ptr_cap->pfs, caporig );
			// �\��
			switch ( cap_show_mode ) {
				case SHOW_CAP_IMG:
					cvShowImage( "Capture", ptr_cap->img );
					break;
				case SHOW_CAP_BIN1:
					cvShowImage( "Capture", ptr_cap->bin1 );
					break;
				case SHOW_CAP_SMT:
					cvShowImage( "Capture", ptr_cap->smt );
					break;
				case SHOW_CAP_BIN2:
					cvShowImage( "Capture", ptr_cap->bin2 );
					break;
				case SHOW_CAP_PT:
					cvShowImage( "Capture", ptr_cap->pt );
					break;
				case SHOW_CAP_OVER:
					cvShowImage( "Capture", ptr_cap->overlap );
					break;
				case SHOW_CAP_COR:
					cvShowImage( "Capture", ptr_cap->cor );
					break;
			}
			if ( show_mode == SHOW_REG_COR )	cvShowImage( "Registered", ptr_reg->cor );
		}
		key = cvWaitKey( 1 );	// �Ȃ��ƃE�B���h�E���X�V����Ȃ�
		if ( key >= 0 ) {
			switch ( key ) {
				// �o�^�摜�̃L�[
				case 'a':	// ���摜
					show_mode = SHOW_REG_IMG;
					cvShowImage( "Registered", c_reg.img );
					break;
				case 's':	// �K�E�V�A���P
					show_mode = SHOW_REG_SMT1;
					cvShowImage( "Registered", c_reg.smt1 );
					break;
				case 'd':	// ��l���P
					show_mode = SHOW_REG_BIN1;
					cvShowImage( "Registered", c_reg.bin1 );
					break;
				case 'f':	// �K�E�V�A���Q
					show_mode = SHOW_REG_SMT2;
					cvShowImage( "Registered", c_reg.smt2 );
					break;
				case 'g':	// ��l���Q
					show_mode = SHOW_REG_BIN2;
					cvShowImage( "Registered", c_reg.bin2 );
					break;
				case 'h':	// �����_
					show_mode = SHOW_REG_PT;
					cvShowImage( "Registered", c_reg.pt );
					break;
				case 'j':	// �d�ˍ��킹
					show_mode = SHOW_REG_OVER;
					cvShowImage( "Registered", c_reg.overlap );
					break;
				case 'k':	// �Ή��֌W
					show_mode = SHOW_REG_COR;
					cvShowImage( "Registered", c_reg.cor );
					break;
				// �L���v�`���摜�̃L�[
				case 'z':	// ���摜
					cap_show_mode = SHOW_CAP_IMG;
					cvShowImage( "Capture", ptr_cap->img );
					break;
				case 'x':	// ��l���P
					cap_show_mode = SHOW_CAP_BIN1;
					cvShowImage( "Capture", ptr_cap->bin1 );
					break;
				case 'c':	// �K�E�V�A��
					cap_show_mode = SHOW_CAP_SMT;
					cvShowImage( "Capture", ptr_cap->smt );
					break;
				case 'v':	// ��l���Q
					cap_show_mode = SHOW_CAP_BIN2;
					cvShowImage( "Capture", ptr_cap->bin2 );
					break;
				case 'b':	// �����_
					cap_show_mode = SHOW_CAP_PT;
					cvShowImage( "Capture", ptr_cap->pt );
					break;
				case 'n':	// �d�ˍ��킹
					cap_show_mode = SHOW_CAP_OVER;
					cvShowImage( "Capture", ptr_cap->overlap );
					break;
				case 'm':	// �Ή��֌W
					cap_show_mode = SHOW_CAP_COR;
					cvShowImage( "Capture", ptr_cap->cor );
					break;
				// ���̑�
				case 'p':	// �ꎞ��~
					if ( paused < 0 ) // �|�[�Y����Ă��Ȃ�
						PauseDirectShowCap( &dsc );
					else
						ResumeDirectShowCap( &dsc );
					paused *= -1;
					break;
				case 'q':	// �I��
					goto end_tune;
			}
		}

	}
end_tune:
	delete ptr_cap;
	cvDestroyWindow( "Capture" );	// �E�B���h�E��j��
	cvDestroyWindow( "Registered" );	// �E�B���h�E��j��
	StopDirectShowCap( &dsc );	// �L���v�`�����I��
	ReleaseDirectShowCap( &dsc );	// DirectShow�̏��X�������[�X
	return 1;
}
