/* dcams.c */
#define	GLOBAL_DEFINE	/* extern�ϐ��ɂ����Ŏ��̂�^���� */
// �ύX�e�X�g

#include "def_general.h"

#include <stdio.h>
#include <math.h>

// �l�b�g���[�N
#ifdef	WIN32
#include <winsock2.h>	// socket
#include "ws_cl.h"
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "sock_sv.h"
#endif

#ifdef	WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#include <opencv2/opencv.hpp>
#include <vector>

#include "disc.h"
#include "dirs.h"
#include "houghl.h"
#include "hist.h"
#include "hash.h"
#include "nn5.h"
#include "cluster.h"
//#include "perminv.h"
#include "proj4p.h"
#include "projrecov.h"
#include "f_points.h"
#include "auto_connect.h"
#include "proctime.h"
#include "score.h"
#include "init.h"
#include "cr.h"
#include "prop.h"
#include "extern.h"
#include "nears.h"
#include "daytime.h"
#include "inacr2.h"
#include "nstr.h"
#include "nimg.h"

//#define PORT_NO	12345
#define	kPort	12345
#define	kPointPort	65431
#define	kResultPort	65432
//#define	kPointPort	65433
//#define	kResultPort	65434
//#define	SERVER_NAME	"kana"
//#define	SERVER_NAME	"arena"
//#define	kServerName	"kana"
#define	kServerName	"mana"
#define	kClientName	"griffon"
//#define	kClientName	"raptor"
//#define	kClientname	"masamune"
#define	kMaxNameLen	(20)	/* �N���C�A���g���̍ő咷�� */

#define	kTCP	(1)
#define	kUDP	(2)

#define	SEND_FLAG	0
#ifdef	WIN32
#define	RECV_FLAG	0
#else
#define	RECV_FLAG	MSG_WAITALL
#endif

//#define	kTopThr	3
//#define	kTopThr	0	/* �ł��؂胂�[�h�̎����̂��߂ɕύX */
//#define	kTopThr	0	/* �ł��؂胂�[�h�̎����̂��߂ɕύX */
//#define	kTopThr	20	/* 08/08/26 ���{�ꃂ�[�h�ł�false-positive�����̂��߂ɕύX */
//#define	kTopThr	4	/* 08/09/02 �A���r�A���reject�������Ȃ̂ŕύX */
#define	kTopThr	3	/* 08/09/14 ���[�I���reject�������Ȃ̂ŕύX */
#define	kMinPoints	(20)	/* USB�J�������[�h�ł̍ŏ������_�� */

#define	kMaxDocNameLen	(100)	/* �摜���̍ő咷 */

#define	kRecvBuffSize	(sizeof(cv::Size) + sizeof(int) + sizeof(cv::Point) * kMaxPointNum)	/* ��M���̃o�b�t�@�̃T�C�Y */
#define	kRecvBuffSizeAreas	(sizeof(cv::Size) + sizeof(int) + (sizeof(cv::Point) + sizeof(unsigned short)) * kMaxPointNum)	/* ��M���̃o�b�t�@�̃T�C�Y�i�ʐς���M�o�[�W�����j */
#define	kSendBuffSize	(kMaxDocNameLen + sizeof(strProjParam) + sizeof(cv::Size))

long allocated_hash = 0;

int RecvPoints( SOCKET sid, cv::Point **ps0, cv::Size *size )
// �_�f�[�^����M
{
	int i, num = 0, ret, buff_cur;
	char buff[kRecvBuffSize];
	cv::Point *ps;
	
	// �o�b�t�@�Ɏ�M
	ret = recv( sid, buff, kRecvBuffSize, RECV_FLAG );
	if ( ret <= 0 )	return -1;	// ��M���s
	// �o�b�t�@����摜�T�C�Y�𓾂�
	for ( i = 0, buff_cur = 0; i < sizeof(cv::Size); i++, buff_cur++ ) {
		((char *)size)[i] = buff[buff_cur];
	}
	// �o�b�t�@��������_���𓾂�
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		((char *)&num)[i] = buff[buff_cur];
	}
	// �����_����1�ȏ�Ȃ烁�������m��
	if ( num > 0 )	ps = (cv::Point *)calloc( num, sizeof(cv::Point) );
	else	ps = NULL;
	*ps0 = ps;
	// �o�b�t�@��������_�f�[�^�𓾂�
	for ( i = 0; i < (int)(sizeof(cv::Point) * num); i++, buff_cur++ ) {
		((char *)ps)[i] = buff[buff_cur];
	}

	return num;
}

int RecvPointsAreas( SOCKET sid, cv::Point **ps0, double **areas0, cv::Size *size )
// �_�f�[�^����M
{
	int i, num = 0, ret, buff_cur;
	char buff[kRecvBuffSizeAreas];
	cv::Point *ps;
	double *areas;
	unsigned short areas_us[kMaxPointNum];
	
	// �o�b�t�@�Ɏ�M
	ret = recv( sid, buff, kRecvBuffSizeAreas, RECV_FLAG );
	if ( ret <= 0 )	return -1;	// ��M���s
	// �o�b�t�@����摜�T�C�Y�𓾂�
	for ( i = 0, buff_cur = 0; i < sizeof(cv::Size); i++, buff_cur++ ) {
		((char *)size)[i] = buff[buff_cur];
	}
	// �o�b�t�@��������_���𓾂�
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		((char *)&num)[i] = buff[buff_cur];
	}
	// �����_����1�ȏ�Ȃ烁�������m��
	if ( num > 0 )	{
		ps = (cv::Point *)calloc( num, sizeof(cv::Point) );
		areas = (double *)calloc( num, sizeof(double) );
	}
	else {
		ps = NULL;
		areas = NULL;
	}
	*ps0 = ps;
	*areas0 = areas;
	// �o�b�t�@��������_�f�[�^�𓾂�
	for ( i = 0; i < (int)(sizeof(cv::Point) * num); i++, buff_cur++ ) {
		((char *)ps)[i] = buff[buff_cur];
	}
	// �o�b�t�@����ʐσf�[�^�𓾂�
	buff_cur = sizeof(cv::Size) + sizeof(int) + sizeof(cv::Point) * kMaxPointNum;
	for ( i = 0; i < (int)(sizeof(unsigned short) * num); i++, buff_cur++ ) {
		((char *)areas_us)[i] = buff[buff_cur];
	}
	for ( i = 0; i < num; i++ ) {
		areas[i] = (double)areas_us[i];
		// printf("%lf\n", areas[i]);
	}

	return num;
}

int SendResult( SOCKET sid, char *doc_name, int len )
// �������𑗐M
{
	return send( sid, doc_name, len, SEND_FLAG );
}

int SendFileNameList( SOCKET sid, int num, char **fname_list, int len )
{
	int i, ret;

	ret = send( sid, (char *)&num, sizeof(int), SEND_FLAG );
	for ( i = 0; i < num; i++ ) {
		ret = send( sid, fname_list[i], len, SEND_FLAG );
	}
	return ret;
}

int SendResultCor( SOCKET sid, char *doc_name, int len, int doc, int pcor[][2], int pcornum, cv::Point *ps, cv::Point **psall, cv::Size size, cv::Size *sizeall )
// �������ƑΉ��_�𑗐M
{
	int i, ret;
	cv::Point corps[kMaxPointNum][2];
	
	if ( send( sid, doc_name, len, SEND_FLAG ) < 0 )	return -1;	// �������𑗐M
	if ( send( sid, &size, sizeof(cv::Size), SEND_FLAG ) < 0 )	return -1;	// �N�G���摜�̃T�C�Y�𑗐M
	if ( send( sid, &(sizeall[doc]), sizeof(cv::Size), SEND_FLAG ) < 0 )	return -1;	// �o�^�摜�̃T�C�Y�𑗐M
	if ( ret = send( sid, &pcornum, sizeof(int), SEND_FLAG ) < 0 )	return -1;	// �Ή��̐��𑗐M
	for ( i = 0; i < pcornum; i++ ) {
//		printf("%d(%d,%d), %d(%d,%d)\n", pcor[i][0], ps[pcor[i][0]].x, ps[pcor[i][0]].y, pcor[i][1], psall[doc][pcor[i][1]].x, psall[doc][pcor[i][1]].y);
//		ret = send( sid, &(ps[pcor[i][0]]), sizeof(cv::Point), SEND_FLAG );	// �N�G���������_�𑗐M
//		ret = send( sid, &(psall[doc][pcor[i][1]]), sizeof(cv::Point), SEND_FLAG );	// �o�^�������_�𑗐M
		corps[i][0] = ps[pcor[i][0]];
		corps[i][1] = psall[doc][pcor[i][1]];
	}
	
	if ( pcornum > 0 )	ret = send( sid, corps, sizeof(cv::Point)*2*pcornum, SEND_FLAG );
	return ret;
}

int SendResultParam( SOCKET sid, char *doc_name, int len, strProjParam *param, cv::Size *img_size, struct sockaddr_in *addr, int ptc )
// �������Ǝˉe�ϊ��p�����[�^�𑗐M
{
	int ret;
	char buff[kSendBuffSize];

	memcpy( buff, doc_name, kMaxDocNameLen );
	memcpy( buff + kMaxDocNameLen, param, sizeof(strProjParam) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam), img_size, sizeof(cv::Size) );

	if ( ptc == kTCP ) {
		ret = send( sid, buff, kSendBuffSize, SEND_FLAG );
	}
	else {
		ret = sendto( sid, buff, kSendBuffSize, SEND_FLAG, (struct sockaddr *)addr, sizeof(*addr) );
	}
	
	return ret;
#if 0	
	if ( ptc == kTCP ) {
		ret = send( sid, doc_name, len, SEND_FLAG );
		ret = send( sid, param, sizeof(strProjParam), SEND_FLAG );
	}
	else {
		ret = sendto( sid, doc_name, len, SEND_FLAG, (struct sockaddr *)addr, sizeof(*addr) );
		ret = sendto( sid, param, sizeof(strProjParam), SEND_FLAG, (struct sockaddr *)addr, sizeof(*addr) );
	}
	
	return ret;
#endif
}
int RecvComSetting( SOCKET sid, int *ptc, int *pt_port, int *res_port, char *cl_name )
// �ʐM�ݒ����M
{
	int ret;

	ret = recv( sid, (char *)ptc, sizeof(int), RECV_FLAG );	// �v���g�R��
	if ( ret <= 0 )	return -1;
	ret = recv( sid, (char *)pt_port, sizeof(int), RECV_FLAG );	// �����_�|�[�g
	if ( ret <= 0 )	return -1;
	ret = recv( sid, (char *)res_port, sizeof(int), RECV_FLAG );	// �������ʃ|�[�g
	if ( ret <= 0 )	return -1;
	ret = recv( sid, cl_name, kMaxNameLen, RECV_FLAG );	// �N���C�A���g��
	if ( ret <= 0 )	return -1;
	
	return 1;
}

extern int corres[kMaxPointNum][kMaxDocNum];

int main( int argc, char** argv )
{
	int num, **nears = 0, argi = 1, res = 0, emode = RETRIEVE_MODE, rmode = NOT_RECOVER_MODE, \
	fmode = CONNECTED_MODE, pmode = NOT_PREPARED_MODE;
	int *score;
	cv::Point *ps = 0;
	cv::Size img_size;
	int start, end = 0, start_con, end_con, start_fp, end_fp, start_hl, end_hl, start_nears, end_nears;
	cv::Mat img;
	strDisc disc, disc1, disc2;
	char fname[kMaxPathLen];
	FILE *fp;
	double prop;
	// ���J�Ɍ����ĉ���
	// 09/01/06 �f���A���n�b�V���Ɍ����ĉ���
	int ret;
	cv::Point **reg_pss = NULL, **reg_pss1 = NULL, **reg_pss2 = NULL;
	cv::Size *reg_sizes = NULL, *reg_sizes1 = NULL, *reg_sizes2 = NULL;
	int *reg_nums = NULL, *reg_nums1 = NULL, *reg_nums2 = NULL;
	char **dbcors = NULL, **dbcors1 = NULL, **dbcors2 = NULL;
	// �������[�h�p
	int total_que = 0, suc_que = 0, total_time = 0, ret_time = 0;
	double total_diff = 0.0L, total_top = 0.0L;
	// �ʐς̗��p
	double *areas = NULL, **reg_areass = NULL, **reg_areass1 = NULL, **reg_areass2 = NULL;
	// DLL�����̉���
	strHList **hash = NULL, **hash_1 = NULL, **hash_2 = NULL;
	strHList2 **hash2 = NULL, **hash2_1 = NULL, **hash2_2 = NULL;
	HENTRY *hash3 = NULL, *hash3_1 = NULL, *hash3_2 = NULL;
	// �Ή��_
	int pcor[kMaxPointNum][2], pcornum = 0;
	int i, j, rank;
	// �f���A���n�b�V�������̉���
	double prop1, prop2;
	int gaussian_parameter;

#ifdef	WIN32
	timeBeginPeriod( 1 );//���x��1ms�ɐݒ�
#endif
	start = GetProcTimeMiliSec();

	ReadIniFile();

	if ( ( argi = AnalyzeArgAndSetExtern2( argc, argv ) ) < 0 )	return 1;
	switch ( eEntireMode ) {
		/************************ �ʏ팟�����[�h ************************/
		case RETRIEVE_MODE:
			// ./dcams
			fprintf( stderr, "Retrieval from Static Image\n" );
			// �ݒ�t�@�C���̓ǂݍ���
			ret = LoadConfig( eConfigFileName, &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &eProp, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
			// ���U���t�@�C���̓ǂݍ���
			sprintf( fname, "%s%s", eDirsDir, eDiscFileName );
			ret = LoadDisc( fname, &disc );
			// �n�b�V���̓ǂݍ���
			if ( eNoHashList )		hash3 = LoadHash3( disc.num, eHashDatFileName );
			if ( eCompressHash )	hash2 = LoadHash2( disc.num, eHashDatFileName );
			else					hash = LoadHash( disc.num, eHashDatFileName );
			// �_�t�@�C���̓ǂݍ���
			sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
			ret = LoadPointFile2( fname, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
			// �������[�h�Ȃ�n�b�V���̃T�C�Y���o��
			if ( eExperimentMode )	printf( "Allocated Hash Size : %d\n", allocated_hash );

			for ( ; argi < argc; argi++ ) {	// �I�v�V�����ȊO�̈����ɂ��Ď��s
				if ( !eExperimentMode ) {
					fprintf( stdout, "%s\n", argv[argi] );
					fprintf( stderr, "%s\n", argv[argi] );
				}
				// �����_�̒��o
				if ( IsDat( argv[argi] ) ) {	// dat�t�@�C���̏ꍇ
					num = LoadPointFile( argv[argi], &ps, &img_size );
					areas = NULL;	// �ʐς̂Ȃ�dat�t�@�C���Ȃ̂�
				}
				else {	// �����摜���쐬
#ifdef	WIN32	// OpenCV�̃o�[�W�������قȂ邽��
					img = GetConnectedImage( argv[argi], eEntireMode );
#else
					if ( eIsJp ) 	img = GetConnectedImageJp2( argv[argi], eEntireMode );
					else			img = GetConnectedImage2( argv[argi], eEntireMode, eHashGaussMaskSize );
#endif
					if (img.empty())	continue;
					// �����_�𒊏o
					num = MakeCentresFromImage( &ps, img, &img_size, &areas );
				}
				// �ߖT�\�����v�Z
				MakeNearsFromCentres( ps, num, &nears );
				// ����
				score = (int *)calloc( kMaxDocNum, sizeof(int) );
#ifdef	DRAW_COR
				GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
				res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, eProp, reg_nums, hash, hash2, hash3 );
				DrawCor( ps, num, img_size, res, reg_pss[res], reg_nums[res], reg_sizes[res], pcor, pcornum );
#else
#ifdef	DRAW_COR_ALL
				GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
				res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, eProp, reg_nums, hash, hash2, hash3 );
				res = 73;	// MIRU08�C�������Fcvpr011b040.jpg�ł̐����i�I���W�i���ł�2�ʁj
//				res = 9368;	// �I���W�i���ł�1�ʁi��Ď�@�ł�15�ʁj
				pcornum = 0;
				for ( j = 0; j < num; j++ ) {
					if ( corres[j][res] >= 0 ) {
						pcor[pcornum][0] = j;
						pcor[pcornum][1] = corres[j][res];
						pcornum++;
					}
				}
				rank = 1;
				for ( i = 0; i < eDbDocs; i++ ) {
					if ( score[i] > score[res] )	rank++;
				}
				printf("%d(rank %d)\n", res, rank);
//				DrawCor( ps, num, img_size, res, reg_pss[res], reg_nums[res], reg_sizes[res], pcor, pcornum );
/*
				for ( i = 0; i < eDbDocs; i++ ) {
					pcornum = 0;
					for ( j = 0; j < num; j++ ) {
						if ( corres[j][i] >= 0 ) {
							pcor[pcornum][0] = j;
							pcor[pcornum][1] = corres[j][i];
							pcornum++;
						}
					}
					DrawCor( ps, num, img_size, i, reg_pss[i], reg_nums[i], reg_sizes[i], pcor, pcornum );
				
				}
*/

#else
				res = RetrieveNN52( ps, areas, num, nears, img_size, score, &disc, reg_nums, &ret_time, hash, hash2, hash3 );
#endif
#endif
//				OutPutResult( score, 0, 5, argv[argi] );	// ���̂܂܂ł͎g���Ȃ�
				if ( !eExperimentMode )	printf( "%s : %d\n", dbcors[res], score[res] );
				if ( eExperimentMode ) {	// �������[�h�Ȃ琸�x�v��
					total_que++;
					if ( IsSucceed( argv[argi], dbcors[res] ) )	suc_que++;
					total_time += ret_time;
					total_diff += Calc12Diff( score );
					total_top += (double)score[res];
				}
				free( score );
			}
			if ( eExperimentMode ) {	// �������[�h�Ȃ猋�ʏo��
				printf("Accuracy : %d/%d(%.2f\%)\n", suc_que, total_que, ((double)suc_que)/((double)total_que)*100 );
				printf("Average Proc Time : %d micro sec\n", (int)(((double)total_time) / ((double)total_que)) );
				printf("Average Top Vote : %.2f\n", (total_top / (double)total_que) );
				printf("Average Diff : %.2f\n", (total_diff / (double)total_que) );
			}
			return 0;
		/************************ USB�J�������[�h ************************/
		case USBCAM_SERVER_MODE:
			ret = RetrieveUSBCamServer();
			return 0;
		/************************ USB�J�������[�h�i�f���A���n�b�V���j ************************/
		case USBCAM_SERVER_DUAL_HASH_MODE:
			ret = RetrieveUSBCamServerDualHash();
			return 0;
		/************************ �n�b�V���\�z���[�h ************************/
		case CONST_HASH_MODE:
			// ./llahdoc -c [�摜�̃f�B���N�g��] [�摜�̊g���q] (-h [�o�̓f�B���N�g��])
			fprintf( stderr, "Hash Constraction\n" );
			// �����_�̒��o
			fprintf( stderr, "Extracting Feature Points...\n" );
//			ret = CreatePointFile3( &reg_pss, &reg_sizes, &reg_nums, &dbcors );
			ret = CreatePointFile4( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors, eHashSrcDir, eHashSrcSuffix, eHashGaussMaskSize, eDirsDir, ePointDatFileName );
			if ( ret <= 0 )	return 1;
const_rest:
			// ���U���t�@�C���̍쐬
			fprintf( stderr, "Making %s...\n", eDiscFileName );
			ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss, reg_nums, &disc, eDiscFileName );
			if ( ret <= 0 )	return 1;
			// �n�b�V���̍\�z
			fprintf( stderr, "Constructing Hash Table...\n" );
			ret = ConstructHash2( eDbDocs, reg_pss, reg_areass, reg_nums, &disc, &hash, &hash2, &hash3 );
			if ( ret <= 0 )	return 1;
			// �Փ˂̑������ڂ�����
			if ( !eNoHashList && eRemoveCollision )	{
				if ( eCompressHash )	ret = RefineHash2( hash2 );
				else					ret = RefineHash( hash );
			}
			// �n�b�V���̃`�F�b�N
			if ( eExperimentMode ) {
				if ( eNoHashList )			ChkHash3( hash3 );
				else if ( eCompressHash )	ChkHash2( hash2 );
				else						ChkHash( hash );
			}
			// �n�b�V���̕ۑ�
			fprintf( stderr, "Saving Hash Table...\n" );
			if ( eNoHashList )			ret = SaveHash3( hash3, eHashDatFileName );
			else if ( eCompressHash )	ret = SaveHash2( hash2, eHashDatFileName );
			else						ret = SaveHash( hash, eHashDatFileName );
			if ( ret <= 0 )	return 1;
			// ���萔�̌v�Z
			fprintf( stderr, "Calculating Prop...\n" );
			eProp = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss, reg_areass, reg_nums, &disc, hash, hash2, hash3 );
			// �ݒ�t�@�C���̕ۑ�
			fprintf( stderr, "Saving Config...\n" );
			ret = SaveConfig( eInvType, eGroup1Num, eGroup2Num, eDiscNum, eProp, eCompressHash, eDocBit, ePointBit, eRBit, eOBit, eHList2DatByte, eRotateOnce, eUseArea, eIsJp, eNoHashList, eConfigFileName );

			if ( ret <= 0 )	return 1;

			return 0;
		/* point.dat��p�����n�b�V���\�z���[�h */
		case CONST_HASH_PF_MODE:
			fprintf( stderr, "Hash Construction using point.dat\n" );
			sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
			ret = LoadPointFile2( fname, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
			if ( ret <= 0 )	return 1;
			goto const_rest;

			return 0;
		/* �f���A���n�b�V���\�z���[�h */
		case CONST_DUAL_HASH_MODE:
			// ./llahdoc -D [�摜�̃f�B���N�g��] [�摜�̊g���q] (-h [�o�̓f�B���N�g��])
			fprintf( stderr, "Dual Hash Constraction\n" );
			// �����_�̒��o
			fprintf( stderr, "Extracting Feature Points 1...\n" );
			if ( eVariousAutoMaskSizeDual )	gaussian_parameter = -1;	// �����P�ʂ������t���O
			else	gaussian_parameter = 1;
			ret = CreatePointFile4( &reg_pss1, &reg_areass1, &reg_sizes1, &reg_nums1, &dbcors1, eHashSrcDir, eHashSrcSuffix, gaussian_parameter, eDirsDir, "point1.dat" );
			if ( ret <= 0 )	return 1;
			fprintf( stderr, "Extracting Feature Points 2...\n" );
			if ( eVariousAutoMaskSizeDual )	gaussian_parameter = -2;	// �P��P�ʂ������t���O
			else	gaussian_parameter = 23;
			ret = CreatePointFile4( &reg_pss2, &reg_areass2, &reg_sizes2, &reg_nums2, &dbcors2, eHashSrcDir, eHashSrcSuffix, gaussian_parameter, eDirsDir, "point2.dat" );
			if ( ret <= 0 )	return 1;
			// ���U���t�@�C���̍쐬
			fprintf( stderr, "Making %s...\n", eDiscFileName );
			ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss1, reg_nums1, &disc1, "disc1.txt" );
			if ( ret <= 0 )	return 1;
			ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss2, reg_nums2, &disc2, "disc2.txt" );
			if ( ret <= 0 )	return 1;
			// �n�b�V���̍\�z
			fprintf( stderr, "Constructing Hash Table 1...\n" );
			ret = ConstructHash2( eDbDocs, reg_pss1, reg_areass1, reg_nums1, &disc1, &hash_1, &hash2_1, &hash3_1 );
			if ( ret <= 0 )	return 1;
			fprintf( stderr, "Constructing Hash Table 2...\n" );
			ret = ConstructHash2( eDbDocs, reg_pss2, reg_areass2, reg_nums2, &disc2, &hash_2, &hash2_2, &hash3_2 );
			if ( ret <= 0 )	return 1;
			// �Փ˂̏�����n�b�V���̃`�F�b�N�͏Ȃ����D�K�v�Ȃ�CONSTRUCT_HASH���玝���Ă���
			// �n�b�V���̕ۑ�
			fprintf( stderr, "Saving Hash Table 1...\n" );
			if ( eNoHashList )			ret = SaveHash3( hash3_1, "hash1.dat" );
			else if ( eCompressHash )	ret = SaveHash2( hash2_1, "hash1.dat" );
			else						ret = SaveHash( hash_1, "hash1.dat" );
			if ( ret <= 0 )	return 1;
			fprintf( stderr, "Saving Hash Table 2...\n" );
			if ( eNoHashList )			ret = SaveHash3( hash3_2, "hash2.dat" );
			else if ( eCompressHash )	ret = SaveHash2( hash2_2, "hash2.dat" );
			else						ret = SaveHash( hash_2, "hash2.dat" );
			if ( ret <= 0 )	return 1;
			// ���萔�̌v�Z
			fprintf( stderr, "Calculating Prop 1...\n" );
			prop1 = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss1, reg_areass1, reg_nums1, &disc1, hash_1, hash2_1, hash3_1 );
			fprintf( stderr, "Calculating Prop 2...\n" );
			prop2 = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss2, reg_areass2, reg_nums2, &disc2, hash_2, hash2_2, hash3_2 );
			printf("%lf, %lf\n", prop1, prop2 );
			// �ݒ�t�@�C���̕ۑ�
			fprintf( stderr, "Saving Config 1...\n" );
			ret = SaveConfig( eInvType, eGroup1Num, eGroup2Num, eDiscNum, prop1, eCompressHash, eDocBit, ePointBit, eRBit, eOBit, eHList2DatByte, eRotateOnce, eUseArea, eIsJp, eNoHashList, "config1.dat" );
			fprintf( stderr, "Saving Config 2...\n" );
			ret = SaveConfig( eInvType, eGroup1Num, eGroup2Num, eDiscNum, prop2, eCompressHash, eDocBit, ePointBit, eRBit, eOBit, eHList2DatByte, eRotateOnce, eUseArea, eIsJp, eNoHashList, "config2.dat" );

			if ( ret <= 0 )	return 1;

			return 0;

		default:
			puts("def: under construction");
			return 0;
	}
	return 0;

#if 0
	if ( ( argi = AnalyzeArgAndSetExtern( argc, argv, &emode, &rmode, &fmode, &pmode ) ) < 0 )	return 1;
	
//	return 0;
	// �ȉ������C�n�b�V���\�z���ɕ���
	/************************ �ʏ팟�����[�h ************************/
	if ( emode == RETRIEVE_MODE || emode == RET_MP_MODE ) {
		LoadDisc( eDiscFileName, &disc );	// ���U���t�@�C���̓ǂݍ���
		start_hl = GetProcTimeMiliSec();
		LoadHash(disc.num);	// �n�b�V���̃��[�h
		end_hl = GetProcTimeMiliSec();
		printf("Hash loading time : %dms\n", end_hl - start_hl);
		
retrieve:		
		
		for ( ; argi < argc; argi++ ) {	// ���ׂĂ̈����ɂ���
			if ( strlen( argv[argi] ) >= kMaxPathLen ) {
				fprintf( stderr, "error: at main in dirs.c: too long path %s\n", argv[argi] );	// �����t�@�C���̃p�X�̕����񂪒�������
				continue;
			}
			strcpy( fname, argv[argi] );
			printf("%s\n", fname);
			// �����摜�̍쐬�i�p�ӂ���Ă��邩�͂ݗ̈�Ȃ烍�[�h�C�����łȂ���Όv�Z�j
			start_con = GetProcTimeMiliSec();
			if ( pmode == PREPARED_MODE || fmode == ENCLOSED_MODE ) {	// �����摜���p�ӂ���Ă��邩�C�͂܂ꂽ�̈������_�ɂ���Ȃ�
				img = cv::imread(fname, cv::IMREAD_GRAYSCALE);
	if (img.empty()) {
					fprintf(stderr, "�摜�t�@�C��%s���J���܂���\n", fname);
					return 1;
				}
				if ( pmode == PREPARED_MODE && rmode == RECOVER_MODE )	argi++;	// �����摜���e�Սς݂ŁC�ʓr���摜����ˉe�ϊ��̕␳���������Ȃ�
			}
			else if ( fmode != USEPF_MODE && !IsTxt(fname) && !IsDat(fname) ) {
				if ( (img = GetConnectedImage( fname, emode )).empty() ) {	// �����摜���쐬
					fprintf(stderr, "%s���猋���摜���쐬����̂Ɏ��s���܂���\n", fname);
					return 1;
				}
			}
			end_con = GetProcTimeMiliSec();
//			printf("Create word-connected image : %dms\n", end_con - start_con);
			// �����摜�i�͂ݗ̈�Ȃ猴�摜�j��������_�𒊏o
			start_fp = GetProcTimeMiliSec();
			if ( IsTxt( fname ) || IsDat( fname ) ) {
				num = LoadPointFile( fname, &ps, &img_size );
			} else if ( fmode == CONNECTED_MODE ) {	// �P��̏d�S������_�Ƃ���
				num = MakeCentresFromImage( &ps, img, &img_size, NULL );	// �摜����d�S���v�Z����
			} else if ( fmode == ENCLOSED_MODE ) {	// �͂܂ꂽ�̈�̏d�S������_�Ƃ���
				num = MakeFeaturePointsFromImage( &ps, img, &img_size );
			} else if ( fmode == USEPF_MODE ) {
				num = LoadPointFile( fname, &ps, &img_size );
			} else {
				fprintf(stderr, "�����_���o���[�h���s���ł�\n");
				return 1;
			}
			if ( num == 0 ) {
				fprintf(stderr, "�A���������݂���܂���ł���\n");
				return 1;
			}
//			return 1;	// �r���I��
			start_nears = GetProcTimeMiliSec();
            MakeNearsFromCentres( ps, num, &nears );	// �e�_�ɂ��ď��kNears�̋ߖT�_�̃C���f�b�N�X�����߁Cnears�Ɋi�[����
//			num = ScreenPointsWithNears( ps, num, nears );	// �ߖT�_�̋�����p���ē����_���ӂ邢�ɂ�����
            end_fp = GetProcTimeMiliSec();
//           printf("nears: %d\n", end_fp - start_nears );
//			printf("Extract feature points and analyse neighbors : %dms\n", end_fp - start_fp );
			score = (int *)calloc( kMaxDocNum, sizeof(int) );
			// ����
			if ( rmode == RECOVER_MODE ) {
                res = RetrieveAndRecover( ps, num, nears, img_size, fname, emode );	// ��������юˉe�ϊ��̕␳
			} else {
				res = RetrieveNN5( ps, num, nears, img_size, score );	// �����̂�
			}
			// ���낢��Ɖ��
			if (ps != NULL && num > 0 )
				ReleaseCentres( ps );	// ps�̉��
			ReleaseNears( nears, num );	// nears�̉��
			if ( fmode != USEPF_MODE && !IsDat( fname ) && !IsTxt( fname ))
			printf("%d\n", score[0]);
			// �v�Z���Ԃ̕\��
			OutPutResult( score, end-start, 5, fname );
			free(score);
		}
//		printf("%d\n", allocated_hash);
		end = GetProcTimeMiliSec();
		printf("Total processing time : %dms\n", end-start);
		return 0;
	}
	/************************ USB�J�������[�h ************************/
	else if ( emode == USBCAM_SERVER_MODE ) {
		SOCKET sid1, sid2, sidpt, sidres;
		int i, ret, start_com = 0, end_com, start_cam, end_cam;
		int start_dt = 0, end_dt, start_com_dt = 0, end_com_dt, start_cam_dt = 0, end_cam_dt, send_dt, recv_dt, start_ret_all, end_ret_all;
		int pcor[kMaxPointNum][2], pcornum = 0, *numall;
//		int ptc, pt_port, res_port;
		char doc_name[kMaxDocNameLen] = "";
		cv::Point **psall;
		cv::Size *sizeall;
		strProjParam param, zero_param;
		struct sockaddr_in addr;
		
		fprintf(stderr, "USB Camera Server Mode\n");
		fprintf(stderr, "%d\n%d\n%d\n%d\n%s\n%s\n", eTCPPort, eProtocol, ePointPort, eResultPort, eClientName, eServerName);
		LoadDisc( eDiscFileName, &disc );	// ���U���t�@�C���̓ǂݍ���
		start_hl = GetProcTimeMiliSec();
		LoadHash(disc.num);	// �n�b�V���̃��[�h
		end_hl = GetProcTimeMiliSec();
		fprintf(stderr, "Hash loading time : %dms\n", end_hl - start_hl);
		LoadPointFileAll( &psall, &sizeall, &numall );
		// �g�ݍ��킹�̍쐬
		GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
		// �l�b�g���[�N�֌W
		fprintf(stderr, "Ready\n");
		// �l�S�V�G�C�g�J�n
start_nego:
		fprintf(stderr, "Start negotiation\n");
#ifdef WIN32
		if ( ( sid1 = InitWinSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socket�̏�����
		if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#else
		if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socket�̏�����
		if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#endif
		ret = RecvComSetting( sid2, &eProtocol, &ePointPort, &eResultPort, eClientName );	// �ʐM�ݒ����M
		if ( ret < 0 )	return 1;
		fprintf(stderr, "%d, %d, %d, %s\n", eProtocol, ePointPort, eResultPort, eClientName );

		if ( eProtocol == kUDP ) {	// UDP�̏ꍇ�DTCP�̏ꍇ�͐ڑ��ς�
#ifdef	WIN32
			Sleep( 1000 );
			if ( ( sidpt = InitWinSockSvUDP( ePointPort ) ) < 0 )	return 1;
			if ( ( sidres = InitWinSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#else
			sleep(1);	// UDP�̏ꍇ�̓N���C�A���g���ɑ҂���Ԃɂ��Ȃ���΂Ȃ�Ȃ��̂�
			if ( ( sidpt = InitSockSvUDP( ePointPort ) ) < 0 )	return 1;
			if ( ( sidres = InitSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#endif
		}
			
		score = (int *)calloc( kMaxDocNum, sizeof(int) );
		zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
		param = zero_param;
		start_com = GetProcTimeMiliSec();
//		ret = SendResultCor( sid2, doc_name, 20, res, pcor, pcornum, ps, psall, img_size, sizeall );	// �_�~�[�𑗐M
//		puts("send start");
		if ( eProtocol == kTCP ) {
			ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, NULL, &addr, eProtocol );
		}
		else {
			ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, NULL, &addr, eProtocol );
		}
		
		for ( ; ; ) {
			recv_dt = GetDayTimeMicroSec();
//			puts("recv start");
			if ( eProtocol == kTCP ) {
				num = RecvPoints( sid2, &ps, &img_size );	// �����_�f�[�^����M
			}
			else {
				num = RecvPoints( sidpt, &ps, &img_size );	// �����_�f�[�^����M
			}
			num = ( num >= kMaxPointNum ) ? kMaxPointNum - 1 : num;
//			printf("%d points received.\n", num);
//			printf("recv(dt) : %d micro sec\n", GetDayTimeMicroSec() - recv_dt);
			end_com = GetProcTimeMiliSec();
			end_com_dt = GetDayTimeMicroSec();
//			printf("com : %dms\n", end_com - start_com);
//			printf("com : %dms\n", (int)(end_com_dt - start_com_dt)/1000);
			start_cam_dt = GetDayTimeMicroSec();
			if ( num < 0 )	break;	// �ʐM���s
			if ( num >= kMinPoints ) {	// �����_�����ŏ��l�ȏ�
//				puts("start nears");
				start_nears = GetProcTimeMiliSec();
				start_ret_all = GetProcTimeMicroSec();
//				MakeNearsFromCentres( ps, num, &nears );	// �e�_�ɂ��ď��kNears�̋ߖT�_�̃C���f�b�N�X�����߁Cnears�Ɋi�[����
				MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num, &nears );	// �ߖT�_�v�Z�i�����Łj
				end_nears = GetProcTimeMiliSec();
				// ����
				start = GetProcTimeMiliSec();
//				puts("start ret");
//				res = RetrieveNN5( ps, num, nears, img_size, score );	// �����̂�
				res = RetrieveNN5Cor( ps, num, nears, img_size, score, pcor, &pcornum, &disc );	// �����̂�
				end = GetProcTimeMiliSec();
				end_ret_all = GetProcTimeMicroSec();
//				printf("retrieval(micro sec) : %d\n", end_ret_all - start_ret_all);
				printf("%d\n", end_ret_all - start_ret_all);
//				printf("ret_all : %dms\n", end - start);
//				printf("nears : %dms\nret : %d\n", end_nears - start_nears, end - start );
//				DrawCor( ps, num, img_size, res, psall[res], numall[res], sizeall[res], pcor, pcornum );	// �Ή��֌W��`��
				// �ŏ��Ή��_���ȏ�Ȃ�A�ˉe�ϊ��p�����[�^���v�Z
				start = GetProcTimeMicroSec();
//				puts("start est");
//				if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_NORMAL, PARAM_IWAMURA );	// �L���v�`���摜�ɏ㏑��
//				if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_IWAMURA );	// �o�^�摜�ɏ㏑��
				if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// �o�^�摜�ɏ㏑��
				else	param = zero_param;
//				param = zero_param;
//				printf("%lf %lf %lf %lf %lf %lf %lf %lf\n", param.a1, param.a2, param.a3, param.b1, param.b2, param.b3, param.c1, param.c2 );			// ���낢��Ɖ��
				end = GetProcTimeMicroSec();
//				printf("param_est : %d micoro sec\n", end - start );
				if (ps != NULL && num > 0 )
					ReleaseCentres( ps );	// ps�̉��
				ReleaseNears( nears, num );	// nears�̉��
				// �v�Z���Ԃ̕\��
				OutPutResultSv( score, end-start, 0/*5*/, doc_name );
			}
			else {	// �����_�����ŏ��l�𖞂����Ȃ�
				puts("min");
				doc_name[0] = '\0';
				param = zero_param;
			}
			fprintf(stdout, "%s\n", doc_name);
			end_cam_dt = GetDayTimeMicroSec();
//			printf("Retrieval(Daytime) : %dms\n", (int)(end_cam_dt - start_cam_dt)/1000 );
//			sleep(1);
			start_com = GetProcTimeMiliSec();
			start_com_dt = GetDayTimeMicroSec();
			send_dt = GetDayTimeMicroSec();
//			ret = SendResultCor( sid2, doc_name, 20, res, pcor, pcornum, ps, psall, img_size, sizeall );	// �������ʂ𑗐M
			if ( eProtocol == kTCP ) {
				ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, NULL, &addr, eProtocol );
			}
			else {
				ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, NULL, &addr, eProtocol );
			}
//			printf("send(dt) : %d micro sec\n", GetDayTimeMicroSec() - send_dt);
			if ( ret < 0 )	break;
		}
		free(score);
		
		if ( eProtocol == kUDP ) {
#ifdef	WIN32
			CloseWinSock( sidpt );
			CloseWinSock( sidres );
#else
			CloseSock( sidpt );
			CloseSock( sidres );
#endif
		}
#ifdef	WIN32
		CloseWinSock( sid2 );
		CloseWinSock( sid1 );
#else
		ShutdownSockSvTCP( sid2 );	// shutdown
		CloseSock( sid1 );	// close
#endif
		
//		goto start_nego;	// �l�S�҂��܂Ŗ߂�
		return 0;
	}
	/************************ �n�b�V���\�z���[�h ************************/
	else if ( emode == CONST_HASH_MODE ) {
			ConstructHash(fmode);
			// ���萔�̌v�Z�ƕۑ�
			prop = CalcProp( ePropMakePath );
			eProp = prop;
			sprintf(fname, "%ssetting.txt", eDirsDir);
			fp = fopen(fname, "a");	// ���̂悤�Ȍ`�Œǉ�����̂͂悭�Ȃ��D�v���P
			fprintf(fp, "%lf\n%d\n", prop, eDbDocs);
			fclose(fp);
			
			end = GetProcTimeMiliSec();
//			printf("Total processing time : %dms\n", end-start);
			
//			goto check_hash;	// �������[�h�ֈڍs�@�ꎞ�I�ȑ[�u
			SaveHash();
			return 0;
	}
	/************************ �n�b�V���ǉ����[�h ************************/
	else if ( emode == ADD_HASH_MODE ) {	// �n�b�V���ǉ����[�h
			ConstructHashAdd(fmode);
			end = GetProcTimeMiliSec();
			printf("Total processing time : %dms\n", end-start);
			return 0;
	}
	/************************ �����_���o���[�h ************************/
	else if ( emode == CREATE_QPF_MODE || emode == CREATE_RPF_MODE ) {	// Mode for creating point files of query/registered image
		int connect_image_mode;
		char in_fname[kMaxPathLen], out_fname[kMaxPathLen];

		if ( argi + 1 >= argc ) {	// �����̕s��
			fprintf( stderr, "error: at main() in dirs.c: input file and/or output file is not specified\n" );
			return 1;
		}
		strncpy( in_fname, argv[argi], kMaxPathLen );
		strncpy( out_fname, argv[argi+1], kMaxPathLen );
		if ( emode == CREATE_QPF_MODE )	connect_image_mode = RETRIEVE_MODE;	// �����摜�̍쐬���̃��[�h��ݒ�
		else	connect_image_mode = CONST_HASH_MODE;
		CreatePointFile( in_fname, out_fname, connect_image_mode );
		return 0;
	}
	/************************ �����_���o���[�h�Q ************************/
	else if ( emode == CREATE_RPF_MODE2 ) {
		// ./dcams -G [�����p�X] [�����_�t�@�C���o�̓f�B���N�g��] [�Ή��֌W�t�@�C��] [�����_���t�@�C��] [�J�n�i���o�[�i�f�t�H���g��0�j
		printf("RPF2\n");
		CreatePointFile2();
		return 0;
	}
	/************************ �e�X�g���[�h ************************/
	else if ( emode == TEST_MODE ) {
//			AutoConnectTest( argv[2], &img_size );
//			RecovPTFromTemp();
//			printf("%d\n", sizeof(strHList));
//			LoadDisc( kDiscFileName, &disc );	// ���U���t�@�C���̓ǂݍ���
//			LoadHash(disc.num);
//			ChkHash();

		for ( ; argi < argc; argi++ ) {	// ���ׂĂ̈����ɂ���
			printf("%s\t", argv[argi]);
			if ( (img = GetConnectedImage( argv[argi], RETRIEVE_MODE )).empty() ) {	// �����摜���쐬
				fprintf(stderr, "%s���猋���摜���쐬����̂Ɏ��s���܂���\n", argv[argi]);
				return 1;
			}
			num = MakeCentresFromImage( &ps, img, &img_size, NULL );	// �摜����d�S���v�Z����
			printf("%d\n", num);
			// ���낢��Ɖ��
			ReleaseCentres( ps );	// ps�̉��
		}
		
		return 0;
	}
	/************************ �n�b�V���`�F�b�N���[�h ************************/
	else if ( emode == CHK_HASH_MODE ) {
		LoadDisc( eDiscFileName, &disc );	// ���U���t�@�C���̓ǂݍ���
		LoadHash(disc.num);	// �n�b�V���̃��[�h
check_hash:
		printf("%d\t", eDiscNum);	// �ꎞ�[�u
		ChkHash();
		goto retrieve;	// �������[�h�ւ̈ڍs�@�ꎞ�I�ȑ[�u
		return 0;
	}
#endif

}

void DrawPoint( cv::Mat img, cv::Point p, cv::Scalar color )
// img��p��color�œ_��ł�
{
#ifdef DRAW_BY_NUM
	static n = 0;
	char tmp[10];
	// CvFont removed in OpenCV 4.x
	sprintf(tmp, "%d", n++);
	// cvInitFont removed - use cv::putText directly
    cv::putText(img, tmp, p, cv::FONT_HERSHEY_PLAIN, 1.0, color);
#endif
#ifndef DRAW_BY_NUM
	cv::circle( img, p, 5, color, -1, 1, 0 );
#endif
}

void CalcCRDiscriptorLine(cv::Point *ps, int num, int *nears[])
// ����̕��@�Ńf�B�X�N���v�^���v�Z����
{
	int pt, pt0, ptd0, next/*, nlc, nl,*/, count = 0, cent[4];
	double ang, cr;

	pt = FirstPoint(ps, num);
#ifdef DRAW_PIC
	DrawPoint( out_img, ps[pt], cWhite);
#endif
	next = FirstFirstRightPointCR(pt, ps, nears);	// 1�s�ڂōŏ��̉E�̓_��T��
	while ( next < 0 ) {	// 1�s�ڂ̍ŏ��̉E�̓_��������Ȃ��ꍇ
		if ( (pt = NextLineCR(pt, ps, nears, 0.0)) < 0 )	// ����Ɏ��̍s���Ȃ��ꍇ
			return;
		next = FirstFirstRightPointCR(pt, ps, nears);	// ���̍s��1�s�ڂƂ��čŏ��̉E�̓_��T��
	}
	ang = GetPointsAngle(ps[pt], ps[next]);	// �ŏ��̓_�Ǝ��̓_�Ŋp�x���v�Z����

	cent[count++] = pt;	// �_��ۑ�
	
	ptd0 = pt;	// ���C���`��p
	pt0 = pt;
//	nlc = NextLineCR(pt, ps, nears, ang);	// �Ƃ肠�������̍s
	pt = next;
	cent[count++] = pt;
#ifdef DRAW_PIC
	DrawPoint( out_img, ps[pt], cWhite);
	cv::line( out_img, ps[ptd0], ps[pt], cWhite, 4, cv::LINE_AA, 0);
#endif
	ptd0 = pt;
	while ( (pt = RightPointCR(pt, pt0, ps, nears[pt], ang)) >= 0 ) {	//	�E�փg���[�X
		//	�����̏���
#ifdef DRAW_PIC
		DrawPoint( out_img, ps[pt], cWhite);
		cv::line( out_img, ps[ptd0], ps[pt], cWhite, 4, cv::LINE_AA, 0);
#endif
		ang = GetPointsAngle(ps[pt0], ps[pt]);	// �p�x���X�V
//		nl = NextLineCR(pt, ps, nears, ang);
//		if ( ps[nl].y < ps[nlc].y )	// ���̓_����T�������s�̍s���_�̕�����ɂ���Ȃ�
//			nlc = nl;

		cent[count++ % 4] = pt;	// �_��ۑ�
		if ( count % 4 == 0 ) {	// 4���܂�����
			cr = CalcCR( ps[cent[0]], ps[cent[1]], ps[cent[2]], ps[cent[3]] ); 
			printf("%lf ", cr);
		}
	}
	printf("%d\n", count);
	pt = NextLineCR(pt0, ps, nears, ang);	// ���̍s��
//	pt = nlc;

	while ( pt >= 0 ) {	// ���̍s���Ȃ���ΏI��
		ptd0 = pt;
		pt0 = pt;
//		nlc = NextLineCR(pt, ps, nears, ang);	// �Ƃ肠�������̍s
		count = 0;
		cent[count++] = pt;
#ifdef DRAW_PIC
		DrawPoint( out_img, ps[pt], cWhite);
#endif
		if ( (next = FirstRightPointCR(pt, ps, nears, ang)) >= 0 )	// �E�̓_�����������ꍇ
			ang = GetPointsAngle(ps[pt], ps[next]);	// �p�x���v�Z
		pt = next;

		while ( pt >= 0 ) {	// �E�̓_���Ȃ���ΏI��
			// �����̏���
			cent[count++ % 4] = pt;
			if ( count % 4 == 0 ) {	// 4���܂�����
				cr = CalcCR( ps[cent[0]], ps[cent[1]], ps[cent[2]], ps[cent[3]] ); 
				printf("%lf ", cr);
			}
#ifdef DRAW_PIC
			DrawPoint( out_img, ps[pt], cWhite);
			cv::line( out_img, ps[ptd0], ps[pt], cWhite, 4, cv::LINE_AA, 0);
#endif
			ang = GetPointsAngle(ps[pt0], ps[pt]);	//�@�p�x���X�V
//			nl = NextLineCR(pt, ps, nears, ang);
//			if ( ps[nl].y < ps[nlc].y )	nlc = nl;
			ptd0 = pt;
			pt = RightPointCR(pt, pt0, ps, nears[pt], ang);
		}
		pt = NextLineCR(pt0, ps, nears, ang);
//		pt = nlc;
		printf("& %d\\\\\n", count);
	}
}

double CalcCR( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 )
// ������v�Z����
{
	double d13, d23, d14, d24;
	d13 = GetPointsDistance(p1, p3);
	d23 = GetPointsDistance(p2, p3);
	d14 = GetPointsDistance(p1, p4);
	d24 = GetPointsDistance(p2, p4);
	if ( d23 < kLittleVal ) d23 = kLittleVal;
	if ( d24 < kLittleVal ) d24 = kLittleVal;
	if ( d14 < kLittleVal ) d14 = kLittleVal;
	return ( (d13/ d23) / ( d14 / d24 ) );
}

double CalcJI( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 )
// j-invariant���v�Z����
{
	double tau;

	tau = CalcCR(p1, p2, p3, p4);
	return pow(tau*tau - tau + 1, 3) / (tau*tau*(tau-1)*(tau-1));
}

int FirstFirstRightPointCR(int pt, cv::Point *ps, int *nears[])
// CR�ɂ����āC1�s�ڂ̍ŏ��̓_pt���炾�������E�̓_��������
{
	int i, ptr;
	double ang;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[pt][i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang) < kFFRAngle && RightPointCR(ptr, pt, ps, nears[ptr], ang) >= 0)	// ���������E�ŁC�قƂ�Ǔ��������ɓ_�����Ȃ�
			return ptr;
	}
	return -1;
}

int FirstRightPointCR(int pt, cv::Point *ps, int *nears[], double ang0)
// CR�ɂ����āC2�s�ڈȍ~�̍ŏ��̓_pt���炾�������E�̓_��������
{
	int i, ptr;
	double ang;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[pt][i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang0-ang) < kRoughlyAngle && RightPointCR(ptr, pt, ps, nears[ptr], ang) >= 0)	// ang0�Ƃ�⓯���ŁC�قƂ�Ǔ��������ɓ_�����Ȃ�
			return ptr;
	}
	return -1;
}

int RightPointCR(int pt, int pt0, cv::Point *ps, int nears[], long double ang0 )
// CR�ɂ����āCpt����ang�ƂقƂ�Ǔ����p�x�̓_��������
{
	int i, ptr;
	long double ang, ang1;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		ang1 = GetPointsAngle(ps[pt0], ps[ptr]);
		if ( fabs(ang0-ang) < kNearlyAngle || (fabs(ang0-ang) < kAlmostAngle && fabs(ang0-ang1) < kCloseAngle) )	// �قƂ�Ǔ����p�x�Ȃ�@�������͉E���ɂ���C���s���Ƃ����߂��p�x�Ȃ�
			return ptr;
	}
	return -1;
}

int LeftPointCR(int pt, cv::Point *ps, int nears[], double ang0 )
// CR�ɂ����āCpt����ang0��180�x���΂̂��̂Ƃ�⓯���p�x�̓_��������
{
	int i, ptr;
	double ang;

	if ( ang0 < 0 )	ang0 += M_PI;	// ang0��180�x���]������
	else			ang0 -= M_PI;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang0-ang) < kRoughlyAngle )	// ��⓯���p�x�Ȃ�
			return ptr;
	}
	return -1;
}

int NextLineCR(int pt, cv::Point *ps, int *nears[], double ang0)
// CR�ɂ����āCpt���玟�̍s�̍ŏ��̓_��������D�i��̉��̓_�j
{
	int i, ptr;
	double ang;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[pt][i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang - M_PI/2) < kNLAngle ) {	// ����������
			while ( (pt = LeftPointCR(ptr, ps, nears[ptr], ang0)) >= 0 ) {
				ptr = pt;
			}
			return ptr;
		}
	}
	return -1;
}

//long double GetPointsAngle(cv::Point p1, cv::Point p2)
// p1���猩��p2�̊p�x���v�Z����i��-3.14�����3.14�j
//{
//	return atan2(p2.y - p1.y, p2.x - p1.x);
//}

void Calc1NNDiscriptor(cv::Point *ps, int num, int *nears[], char disc[])
// 1NN�Ńf�B�X�N���v�^���v�Z���Cdisc�ɏ�������
{
	int dp = 0, pt, pt0, rpt, *visited, connected, pt00;
#ifdef	CON_WORD
	double dist;
#endif

	// �K��L�^�i�������[�v�h�~�̂��߁j
	visited = (int *)calloc(num, sizeof(int));

	pt = FirstPoint(ps, num);


	while ( pt >= 0 ) {
		pt00 = -1;
		pt0 = pt;
		connected = 1;
#ifdef	CON_WORD
		dist = -1.0;
#endif
		while ( pt >= 0 ) {	// �s�̃��[�v
			visited[pt] = 1;
#ifdef DRAW_PIC
			cv::circle( out_img, ps[pt], 2, cWhite, -1, 1, 0 );
//			if ( pt00 >= 0 )	cv::line( out_img, ps[pt00], ps[pt], cWhite, 1, cv::LINE_AA, 0);
			if ( connected > 1 )	cv::line( out_img, ps[pt00], ps[pt], cWhite, 4, cv::LINE_AA, 0);
#endif
			rpt = RightPoint(pt, ps, num, nears[pt]);
			if ( rpt < 0 ) {	// 
				printf("%d", connected);
				disc[dp++] = '0' + connected;
				break;
			}
#ifdef	CON_1NN
			if ( IsConnected1NN(pt, rpt, nears) ) {
				connected++;
#endif
#ifdef	CON_WORD
			if ( IsConnectedWord(ps[pt], ps[rpt], &dist) ) {
				connected++;
#endif
			}
			else {
				printf("%d ", connected);
				disc[dp++] = '0' + connected;
				connected = 1;
			}
			pt00 = pt;

			pt = rpt;
		}
		printf("\\\\\n");
		disc[dp++] = '\n';
		pt = NextLine(pt0, ps, num, nears, visited);
	}


}

int IsConnected1NN(int p1, int p2, int *nears[])
// p1��p2��1NN�Őڑ�����Ă��邩���ׂ�
{
	if ( nears[p1][0] == p2 || nears[p2][0] == p1 )
		return 1;
	else
		return 0;
}

int IsConnectedWord(cv::Point p1, cv::Point p2, double *dist)
// p1��p2���ڑ�����Ă��邩�C�P��̋�؂�Œ��ׂ�
{
	double d0 = *dist;

	*dist = GetPointsDistance(p1, p2);
	if ( *dist < d0*kWordSep || d0 < 0 )
		return 1;
	else
		return 0;
}

//double GetPointsDistance(cv::Point p1, cv::Point p2)
// �_�̃��[�N���b�h���������߂�
//{
//		return sqrt((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y));
//}

int FirstPoint(cv::Point *ps, int num)
// �ŏ��̓_�i����̓_�j�𓾂�
{
	int i, min_idx = -1;
	double val, min_val = 1000000000;

	for ( i = 0; i < num; i++ ) {
		val = (ps[i].x)*(ps[i].x) + (ps[i].y)*(ps[i].y);
		if ( val < min_val ) {
			min_idx = i;
			min_val = val;
		}
	}
	return min_idx;
}

int NextLine(int pt, cv::Point *ps, int num, int *nears[], int visited[])
// pt���玟�̍s�̍ŏ��̓_��������ipt�̉��ɂȂ���ΉE�ɐi��ł����Ē��ׂ�D���̍s�Ɍ������獶�ɖ߂�邾���߂�j
{
	int next, pt0;

	pt0 = pt;
	while ( (next = DownPoint(pt, ps, num, nears[pt])) < 0 ) {
		if ( (pt = RightPoint(pt, ps, num, nears[pt])) < 0 )	// �E�̓_�ɐi��
			return -1;	// �Ȃ���Ύ��̍s�͂Ȃ�
	}
	while ( (pt = LeftPoint(next, ps, num, nears[next], visited)) >= 0 ) {
		// if ( visited[pt] )	break;	//���łɍs�������Ƃ�����ꍇ�i�������[�v�h�~�j
		next = pt;
	}
//	if ( pt0 == next ) return -1;
	return next;
}

int RightPoint(int pt, cv::Point *ps, int num, int nears[])
// pt�̉E�̓_��������
{
	int i;

	for ( i = 0; i < kNears; i++ ) {
		if ( IsOnRight(&ps[pt], &ps[nears[i]]) )
			return nears[i];
	}
	return -1;
}

int LeftPoint(int pt, cv::Point *ps, int num, int nears[], int visited[])
// pt�̍��̓_��������
{
	int i;

	for ( i = 0; i < kNears; i++ ) {
		if ( IsOnLeft(&ps[pt], &ps[nears[i]]) && !visited[nears[i]])
			return nears[i];
	}
	return -1;
}

int DownPoint(int pt, cv::Point *ps, int num, int nears[])
// pt�̉��̓_��������
{
	int i;

	for ( i = 0; i < kNears; i++ ) {
		if ( IsOnDown(&ps[pt], &ps[nears[i]]) )
			return nears[i];
	}
	return -1;
}

int IsOnRight( cv::Point *p1, cv::Point *p2 )
// p1�̉E��p2�����邩���ׂ�
{
	double deg = atan2(p2->y - p1->y, p2->x - p1->x);
	if ( fabs(deg) < kRightRange )
		return 1;
	else
		return 0;
}

int IsOnLeft( cv::Point *p1, cv::Point *p2 )
// p1�̍���p2�����邩���ׂ�
{
	double deg = atan2(p2->y - p1->y, p2->x - p1->x);
	if ( fabs(fabs(deg) - M_PI) < kLeftRange )
		return 1;
	else
		return 0;
}

int IsOnDown( cv::Point *p1, cv::Point *p2 )
// p1�̉���p2�����邩���ׂ�i���W�n���ʏ�ƈႤ���Ƃɒ��ӁI�j
{
	double deg = atan2(p2->y - p1->y, p2->x - p1->x);
	if ( fabs(deg - M_PI/2) < kDownRange )
		return 1;
	else
		return 0;
}

void JInvariantTest(void)
// j-invariant�̃e�X�g
{
	int i, j, k, l;
	cv::Point p[4];

	p[0].x = 0;
	p[0].y = 0;
	p[1].x = 19;
	p[1].y = 9;
	p[2].x = 29;
	p[2].y = 14;
	p[3].x = 45;
	p[3].y = 22;

	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			if ( j == i )	continue;
			for ( k = 0; k < 4; k++ ) {
				if ( k == i || k == j )	continue;
				for ( l = 0; l < 4; l++ ) {
					if ( l == i || l == j || l == k )	continue;
					printf("%d %d %d %d : %f %f\n", i, j, k, l, CalcCR(p[i], p[j], p[k], p[l]), CalcJI(p[i], p[j], p[k], p[l]));
				}
			}
		}
	}
}

int MakeCentresFromImage(cv::Point **ps, cv::Mat img, cv::Size *size, double **areas)
// �摜�̃t�@�C������^���ďd�S���v�Z���C�d�S�̐���Ԃ�
// 06/01/12	�ʐς̌v�Z��ǉ�
{
	int num;
    std::vector<std::vector<cv::Point>> contours;

    cv::Vec4i hierarchy_unused;  // for hierarchy if needed

	size->width = img.cols;
	size->height = img.rows;

    cv::findContours(img, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);	// �A�������𒊏o����

	num = CalcCentres(ps, contours, size, areas);	// �e�A�������̏d�S���v�Z����

	return num;
}

int CalcCentres(cv::Point **ps0, std::vector<std::vector<cv::Point>> &contours, cv::Size *size, double **areas0)
// �d�S���v�Z����
// 06/01/12	�ʐς̌v�Z��ǉ�
{
	int i, num;
	double *areas = NULL;
	cv::Moments mom;
	cv::Point *ps;
	cv::Size s;
	cv::Mat img;

	s.width = size->width;
	s.height = size->height;
	img = cv::Mat::zeros(s, CV_8UC3);

	// �_�̐����J�E���g�i�b��j
	i = (int)contours.size();
//	num = i;
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// �S�~���ŘA����������������Ƃ��i�{���͑傫���𒲂ׂ��肷�ׂ��j
	// �_������z����m�ہinum�̒l�͐��m�ł͂Ȃ����C�傫�߂Ƃ������ƂŁj
	ps = (cv::Point *)calloc(num, sizeof(cv::Point));
	*ps0 = ps;
	// �ʐς�����z����m��
	if ( areas0 ) {
		areas = (double *)calloc(num, sizeof(double));
		*areas0 = areas;
	}
	// �A��������`��E�d�S���v�Z
    i = 0;
    for( int ci = 0; ci < (int)contours.size() && i < num; ci++ )
    {
		double d00;
#ifdef DRAW_PIC
		cv::Scalar color;
#endif

		mom = cv::moments(contours[ci]);
		d00 = mom.m00;
		if ( d00 < 0.00001 ) continue;	// ����������A�������͏��O

		ps[i].x = (int)(mom.m10 / d00);
		ps[i].y = (int)(mom.m01 / d00);
		if ( areas ) {
			areas[i] = d00;
//			printf("%lf\n", areas[i]);
		}
//        cv::drawContours(img, contours, ci, cWhite, 1, 8);	// replace cv::FILLED with 1 to see the outlines 
		cv::circle( img, ps[i], 5, cWhite, -1, 1, 0 );

#ifdef DRAW_PIC
		color = cRandom;
        cv::drawContours(dst, contours, ci, color, cv::FILLED, 8);	// replace cv::FILLED with 1 to see the outlines 
        cv::drawContours(out_img, contours, ci, cWhite, 1, 8);	// replace cv::FILLED with 1 to see the outlines 
//        cv::drawContours(centre_img, contours, ci, cWhite, 1, 8);	// replace cv::FILLED with 1 to see the outlines 
		cv::circle( centre_img, ps[i], 0/*5*/, cWhite, -1, 1, 0 );
#endif
		i++;
    }

	num = i;	// �A���������̍X�V

	cv::bitwise_not( img, img );
//	OutPutImage( img );
#ifdef SHOW_PIC
	cv::namedWindow( "Components", 1 );
    cv::imshow( "Components", dst );
#endif

	return num;
}

void ReleaseCentres( cv::Point *ps )
{
	free( ps );
}

void ReleaseNears( int **nears, int num )
{
	int i;
	for ( i = 0; i < num; i++ ) {
		free(nears[i]);
	}
	free(nears);
}

int OutPutImage(cv::Mat img)
// �摜���t�@�C���ɕۑ�����D�t�@�C�����͎����I�ɘA�ԂɂȂ�
{
	static int n = 0;
	char filename[kFileNameLen];

	sprintf(filename, "output%03d.jpg", n++);
	return cv::imwrite(filename, img);
}

void OutPutResult( int *s, int t, int disp, char *qname0 )
// ���ʂ̕\��
{
	int i, j, k, n, last_slash, last_period, doc_num;
	char line[kMaxPathLen], name0[kMaxPathLen], name[kMaxPathLen], qname[kMaxPathLen], **fname;
	FILE *fp;
	strScore *score;

	// �N�G���̃x�[�X�l�[���̎擾
	for ( j = 0, last_slash = 0, last_period = strlen(qname0); qname0[j] != '\0'; j++ ) {
		if ( qname0[j] == '/' )	last_slash = j;	// �Ō�̃X���b�V���̈ʒu
		else if ( qname0[j] == '.' )	last_period = j;	// �i�Ō�́j�s���I�h�̈ʒu
	}
	for ( j = last_slash + 1, k = 0; j < last_period; j++, k++ )	qname[k] = qname0[j];
	qname[k] = '\0';
//	puts(qname);
	// �摜�t�@�C�����̓ǂݍ���
	fname = (char **)calloc(kMaxDocNum, sizeof(char *));
	fp = fopen(eDBCorFileName, "r");
	for ( i = 0; i < kMaxDocNum && fgets(line, kMaxPathLen, fp) != NULL; i++ ) {
		sscanf(line, "%d %s", &n, name0);	// �t���p�X�̓ǂݍ���
		for ( j = 0, last_slash = 0, last_period = strlen(name0); name0[j] != '\0'; j++ ) {
			if ( name0[j] == '/' )	last_slash = j;	// �Ō�̃X���b�V���̈ʒu
			else if ( name0[j] == '.' )	last_period = j;	// �i�Ō�́j�s���I�h�̈ʒu
		}
		// �x�[�X�l�[���̎擾
		for ( j = last_slash + 1, k = 0; j < last_period; j++, k++ )	name[k] = name0[j];
		name[k] = '\0';
		fname[n] = (char *)malloc( sizeof(char) * (strlen(name)+1) );
		strcpy( fname[n], name );
	}
	doc_num = i;
	fclose(fp);
	// �X�R�A�ɓ����
	score = (strScore *)malloc(sizeof(strScore) * doc_num);
	SetAndSortScore( score, s, doc_num );
	// disp�ʂ܂ŕ\��
	for ( i = 0; i < disp; i++ ) {
		printf("%s\t%d\n", fname[score[i].n], score[i].vote);
	}
	// ���������ʂ�
	for ( i = 0; i < doc_num; i++ ) {
		if ( !strncmp(qname, fname[score[i].n], strlen(fname[score[i].n])) )	break;
	}
	if ( i == 0 )	printf("Succeeded\n");
	else	printf("Failed\n");
	printf("Rank : %d\n", i+1);
	printf("Ratio : %f\n", (double)score[0].vote / (double)score[1].vote);
	// �������̊J��
	for ( i = 0; i < kMaxDocNum; i++ ) {
		if ( fname[i] != NULL )	free(fname[i]);
	}
	free(fname);
	free(score);
}

void OutPutResultSv( int *s, int t, int disp, char *result )
// ���ʂ̕\��
{
	int i, j, k, n, last_slash, last_period, doc_num;
	char line[kMaxPathLen], name0[kMaxPathLen], name[kMaxPathLen], qname[kMaxPathLen], **fname;
	FILE *fp;
	strScore *score;

	// �摜�t�@�C�����̓ǂݍ���
	fname = (char **)calloc(kMaxDocNum, sizeof(char *));
	fp = fopen(eDBCorFileName, "r");
	for ( i = 0; i < kMaxDocNum && fgets(line, kMaxPathLen, fp) != NULL; i++ ) {
		sscanf(line, "%d %s", &n, name0);	// �t���p�X�̓ǂݍ���
		for ( j = 0, last_slash = 0, last_period = strlen(name0); name0[j] != '\0'; j++ ) {
			if ( name0[j] == '/' )	last_slash = j;	// �Ō�̃X���b�V���̈ʒu
			else if ( name0[j] == '.' )	last_period = j;	// �i�Ō�́j�s���I�h�̈ʒu
		}
		// �x�[�X�l�[���̎擾
		for ( j = last_slash + 1, k = 0; j < last_period; j++, k++ )	name[k] = name0[j];
		name[k] = '\0';
		fname[n] = (char *)malloc( sizeof(char) * (strlen(name)+1) );
		strcpy( fname[n], name );
	}
	doc_num = i;
	fclose(fp);
	// �X�R�A�ɓ����
	score = (strScore *)malloc(sizeof(strScore) * doc_num);
	SetAndSortScore( score, s, doc_num );
	// disp�ʂ܂ŕ\��
	for ( i = 0; i < disp; i++ ) {
		printf("%s\t%d\n", fname[score[i].n], score[i].vote);
	}
	// 1�ʂ�臒l�𒴂�����result�ɃR�s�[
	if ( score[0].vote > kTopThr )	strcpy( result, fname[score[0].n] );
	else	result[0] = '\0';
	// �������̊J��
	for ( i = 0; i < kMaxDocNum; i++ ) {
		if ( fname[i] != NULL )	free(fname[i]);
	}
	free(fname);
	free(score);
}

#define	kScreenThr	(530)

int ScreenPointsWithNears( cv::Point *ps, int num, int **nears )
// �ߖT�_��p���ē����_���ӂ邢�ɂ�����
{
	int i, cnt;
	cv::Point *ps0;

	// ps��ps0�ɃR�s�[
	ps0 = (cv::Point *)malloc( sizeof(cv::Point) * num );
	for ( i = 0; i < num; i++ ) {
		ps0[i].x = ps[i].x;
		ps0[i].y = ps[i].y;
	}
	for ( i = 0, cnt = 0; i < num; i++ ) {
		if ( GetPointsDistance(ps[i], ps[nears[i][6]]) < kScreenThr ) {
			ps[cnt].x = ps0[i].x;
			ps[cnt].y = ps0[i].y;
			cnt++;
		}
	}
	free(ps0);
	return cnt;
}

void DisplayPs( void )
// ps�ŏ���\��
{
//	system("ps u | grep dirs | awk \'BEGIN { FS = \" \" }; { print $3\" \"$4\" \"$5\" \"$6}\'");
	system("ps opcpu,vsize,size,rss");
}

void CreatePointFile( char *in_fname, char *out_fname, int mode )
// �摜�t�@�C��if_fname����mode�œ����_�𒊏o���A�����_�t�@�C���Ƃ���out_fname�ɏo�͂���B
{
	cv::Mat img;
	int num;
	double *areas;
	cv::Point *ps;
	cv::Size img_size;
	
	if ( (img = GetConnectedImage( in_fname, mode )).empty() ) {	// �����摜���쐬
		fprintf(stderr, "%s���猋���摜���쐬����̂Ɏ��s���܂���\n", in_fname);
		return;
	}
	num = MakeCentresFromImage( &ps, img, &img_size, &areas );	// �摜����d�S���v�Z����
	SavePointFile( out_fname, ps, num, &img_size, areas );
	
	return;
}

int RetrieveUSBCamServer( void )
// USB�J�����T�[�o���[�h
{
	int num, *numall = NULL, ret, res, *score = NULL, **nears = NULL;
	int pcor[kMaxPointNum][2], pcornum = 0;
	char fname[kMaxPathLen], doc_name[kMaxPathLen], **dbcors = NULL;
	cv::Point *ps = NULL, **psall = NULL;
	double *areas = NULL, **areaall = NULL;
	cv::Size img_size, *sizeall = NULL, res_size;
	strDisc disc;
	strProjParam param, zero_param;
	SOCKET sid1, sid2, sidpt, sidres;
	struct sockaddr_in addr;
	// DLL�p����
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

//	ret = LoadConfig();	// �ݒ�t�@�C���̓ǂݍ���
	ret = LoadConfig( eConfigFileName, &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &eProp, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, eDiscFileName );
	ret = LoadDisc( fname, &disc );	// ���U���t�@�C���̓ǂݍ���
	if ( ret <= 0 )	return 0;
	// �n�b�V���̓ǂݍ���
	if ( eNoHashList )			hash3 = LoadHash3( disc.num, eHashDatFileName );
	else if ( eCompressHash )	hash2 = LoadHash2( disc.num, eHashDatFileName );
	else						hash = LoadHash( disc.num, eHashDatFileName );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
	ret = LoadPointFile2( fname, &psall, &areaall, &sizeall, &numall, &dbcors );	// �����_�f�[�^�̓ǂݍ���
	if ( ret <= 0 )	return 0;
	// �������[�h�Ȃ�n�b�V���̃T�C�Y���o��
	if ( eExperimentMode )	printf( "Allocated Hash Size : %d\n", allocated_hash );
	// �g�ݍ��킹�̍쐬
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �l�b�g���[�N�֌W
//start_nego2:
	printf("Ready\n");
#ifdef WIN32
	if ( ( sid1 = InitWinSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socket�̏�����
	if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#else
	if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 0;	// socket�̏�����
	if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	return 0;	// accept
#endif
	ret = RecvComSetting( sid2, &eProtocol, &ePointPort, &eResultPort, eClientName );	// �ʐM�ݒ����M
	if ( ret < 0 )	return 0;
//	send = SendFileNameList( sid2, 

//	fprintf(stderr, "%d, %d, %d, %s\n", eProtocol, ePointPort, eResultPort, eClientName );

	if ( eProtocol == kUDP ) {	// UDP�̏ꍇ�DTCP�̏ꍇ�͐ڑ��ς�
#ifdef	WIN32
		Sleep( 1000 );
		if ( ( sidpt = InitWinSockSvUDP( ePointPort ) ) < 0 )	return 1;
		if ( ( sidres = InitWinSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#else
		sleep(1);	// UDP�̏ꍇ�̓N���C�A���g���ɑ҂���Ԃɂ��Ȃ���΂Ȃ�Ȃ��̂�
		if ( ( sidpt = InitSockSvUDP( ePointPort ) ) < 0 )	return 0;
		if ( ( sidres = InitSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 0;
#endif
	}
			
	score = (int *)calloc( kMaxDocNum, sizeof(int) );
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	param = zero_param;
	res_size = sizeall[0];
	puts("start send");
	if ( eProtocol == kTCP ) {
		ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
	}
	else {
		ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
	}
	puts("end send");

#ifdef	SHOW_SCORE_GRAPH	// �X�R�A�̃O���t��\��
#ifdef	WIN32	// Linux�ł̃E�B���h�E�\���̂������킩��Ȃ��̂�
	cv::namedWindow( "Score", CV_WINDOW_AUTOSIZE );
#endif
#endif

	for ( ; ; ) {
		if ( eProtocol == kTCP ) num = RecvPointsAreas( sid2, &ps, &areas, &img_size );	// �����_�f�[�^����M
		else					num = RecvPointsAreas( sidpt, &ps, &areas, &img_size );	// �����_�f�[�^����M
		num = ( num >= kMaxPointNum ) ? kMaxPointNum - 1 : num;	// �����_�����ő�l�𒴂���ꍇ�͐���
//		printf("%d points received.\n", num);
		if ( num < 0 )	break;	// �ʐM���s
		if ( num >= kMinPoints ) {	// �����_�����ŏ��l�ȏ�
			MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num, &nears );	// �ߖT�_�v�Z�i�����Łj
			// ����
			res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, eProp, numall, hash, hash2, hash3 );	// �����̂�
#ifdef	SHOW_SCORE_GRAPH	// �X�R�A�̃O���t��\��
#ifdef	WIN32	// Linux�ł̃E�B���h�E�\���̂������킩��Ȃ��̂�
			ShowScoreGraph( "Score", score, eDbDocs );
#endif
#endif
			// �ŏ��Ή��_���ȏ�Ȃ�A�ˉe�ϊ��p�����[�^���v�Z
			if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// �o�^�摜�ɏ㏑��
			else	param = zero_param;
			if (ps != NULL && num > 0 )	ReleaseCentres( ps );	// ps�̉��
			if (areas != NULL && num > 0 )	free( areas );
			ReleaseNears( nears, num );	// nears�̉��
			// �v�Z���Ԃ̕\��
//			OutPutResultSv( score, end-start, 0/*5*/, doc_name );
			if ( score[res] > kTopThr ) {	// �g�b�v�̓��[����臒l�ȉ��i���W�F�N�g�j
				GetBasename2( dbcors[res], kMaxPathLen, doc_name );
			}
			else {
				doc_name[0] = '\0';
				param = zero_param;	// �������ʂ��Ȃ��̂Ƀp�����[�^����������Ă̂��ςȂ̂�
			}
			puts( doc_name );
		}
		else {	// �����_�����ŏ��l�𖞂����Ȃ�
//			puts("min");
			doc_name[0] = '\0';
			param = zero_param;
		}
		if ( doc_name[0] != '\0' )	res_size = sizeall[res];	// ���������Ȃ炻�̃T�C�Y
		else	res_size = sizeall[0];	// �����łȂ���΃f�[�^�x�[�X��0�Ԃ̃T�C�Y

//		fprintf(stdout, "%s\n", doc_name);
		if ( eProtocol == kTCP )	ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		else						ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		if ( ret < 0 )	break;
	}
	// �I������
	free(score);
	if ( eProtocol == kUDP ) {
#ifdef	WIN32
		CloseWinSock( sidpt );
		CloseWinSock( sidres );
#else
		CloseSock( sidpt );
		CloseSock( sidres );
#endif
	}
#ifdef	WIN32
	CloseWinSock( sid2 );
	CloseWinSock( sid1 );
#else
	ShutdownSockSvTCP( sid2 );	// shutdown
	CloseSock( sid1 );	// close
#endif

#ifdef	SHOW_SCORE_GRAPH
#ifdef	WIN32
	cvDestroyWindow( "Score" );
#endif
#endif
		
//		goto start_nego2;	// �l�S�҂��܂Ŗ߂�
	return 1;
}

int RetrieveUSBCamServerDualHash( void )
// USB�J�����T�[�o���[�h�i�f���A���n�b�V���j
{
	int num, num1, num2, *numall = NULL, *numall1 = NULL, *numall2 = NULL, ret, res, *score = NULL, **nears = NULL, **nears1 = NULL, **nears2 = NULL;
	int res1, res2, *score1 = NULL, *score2 = NULL;
	int **pcor, pcornum = 0, pcor1[kMaxPointNum][2], pcornum1, pcor2[kMaxPointNum][2], pcornum2;
	char fname[kMaxPathLen], doc_name[kMaxPathLen], **dbcors = NULL, **dbcors1 = NULL, **dbcors2 = NULL;
	cv::Point *ps = NULL, **psall = NULL, *ps1 = NULL, *ps2 = NULL, **psall1 = NULL, **psall2 = NULL;
	double *areas = NULL, **areaall = NULL, *areas1 = NULL, *areas2 = NULL, **areaall1 = NULL, **areaall2 = NULL;
	cv::Size img_size, *sizeall = NULL, res_size, *sizeall1 = NULL, *sizeall2 = NULL;
	strDisc disc, disc1, disc2;
	strProjParam param, zero_param;
	SOCKET sid1, sid2, sidpt, sidres;
	struct sockaddr_in addr;
	// DLL�p����
	strHList **hash = NULL, **hash_1 = NULL, **hash_2 = NULL;
	strHList2 **hash2 = NULL, **hash2_1 = NULL, **hash2_2 = NULL;
	HENTRY *hash3 = NULL, *hash3_1 = NULL, *hash3_2 = NULL;
	double prop1, prop2;

//	ret = LoadConfig();	// �ݒ�t�@�C���̓ǂݍ���
	ret = LoadConfig( "config1.dat", &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &prop1, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
	if ( ret <= 0 )	return 0;
	ret = LoadConfig( "config2.dat", &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &prop2, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "disc1.txt" );
	ret = LoadDisc( fname, &disc1 );	// ���U���t�@�C���̓ǂݍ���
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "disc2.txt" );
	ret = LoadDisc( fname, &disc2 );	// ���U���t�@�C���̓ǂݍ���
	if ( ret <= 0 )	return 0;
	// �n�b�V���̓ǂݍ���
	if ( eNoHashList )			hash3_1 = LoadHash3( disc1.num, "hash1.dat" );
	else if ( eCompressHash )	hash2_1 = LoadHash2( disc1.num, "hash1.dat" );
	else						hash_1 = LoadHash( disc1.num, "hash1.dat" );
	if ( ret <= 0 )	return 0;
	if ( eNoHashList )			hash3_2 = LoadHash3( disc2.num, "hash2.dat" );
	else if ( eCompressHash )	hash2_2 = LoadHash2( disc2.num, "hash2.dat" );
	else						hash_2 = LoadHash( disc2.num, "hash2.dat" );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "point1.dat" );
	ret = LoadPointFile2( fname, &psall1, &areaall1, &sizeall1, &numall1, &dbcors1 );	// �����_�f�[�^�̓ǂݍ���
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "point2.dat" );
	ret = LoadPointFile2( fname, &psall2, &areaall2, &sizeall2, &numall2, &dbcors2 );	// �����_�f�[�^�̓ǂݍ���
	if ( ret <= 0 )	return 0;
	// �������[�h�Ȃ�n�b�V���̃T�C�Y���o��
//	if ( eExperimentMode )	printf( "Allocated Hash Size : %d\n", allocated_hash );
	// �g�ݍ��킹�̍쐬
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �l�b�g���[�N�֌W
//start_nego2:
	printf("Ready\n");
#ifdef WIN32
	if ( ( sid1 = InitWinSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socket�̏�����
	if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#else
	if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 0;	// socket�̏�����
	if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	return 0;	// accept
#endif
	ret = RecvComSetting( sid2, &eProtocol, &ePointPort, &eResultPort, eClientName );	// �ʐM�ݒ����M
	if ( ret < 0 )	return 0;
//	send = SendFileNameList( sid2, 

//	fprintf(stderr, "%d, %d, %d, %s\n", eProtocol, ePointPort, eResultPort, eClientName );

	if ( eProtocol == kUDP ) {	// UDP�̏ꍇ�DTCP�̏ꍇ�͐ڑ��ς�
#ifdef	WIN32
		Sleep( 1000 );
		if ( ( sidpt = InitWinSockSvUDP( ePointPort ) ) < 0 )	return 1;
		if ( ( sidres = InitWinSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#else
		sleep(1);	// UDP�̏ꍇ�̓N���C�A���g���ɑ҂���Ԃɂ��Ȃ���΂Ȃ�Ȃ��̂�
		if ( ( sidpt = InitSockSvUDP( ePointPort ) ) < 0 )	return 0;
		if ( ( sidres = InitSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 0;
#endif
	}
			
	score = (int *)calloc( kMaxDocNum, sizeof(int) );
	score1 = (int *)calloc( kMaxDocNum, sizeof(int) );
	score2 = (int *)calloc( kMaxDocNum, sizeof(int) );
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	param = zero_param;
	res_size = sizeall1[0];
	puts("start send");
	if ( eProtocol == kTCP ) {
		ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
	}
	else {
		ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
	}
	puts("end send");

#ifdef	SHOW_SCORE_GRAPH	// �X�R�A�̃O���t��\��
#ifdef	WIN32	// Linux�ł̃E�B���h�E�\���̂������킩��Ȃ��̂�
	cv::namedWindow( "Score", CV_WINDOW_AUTOSIZE );
#endif
#endif

	for ( ; ; ) {
		if ( eProtocol == kTCP ) {
			num1 = RecvPointsAreas( sid2, &ps1, &areas1, &img_size );	// �����_�f�[�^����M
			num2 = RecvPointsAreas( sid2, &ps2, &areas2, &img_size );	// �����_�f�[�^����M
		} else {
			num1 = RecvPointsAreas( sidpt, &ps1, &areas1, &img_size );	// �����_�f�[�^����M
			num2 = RecvPointsAreas( sidpt, &ps2, &areas2, &img_size );	// �����_�f�[�^����M
		}
		num1 = ( num1 >= kMaxPointNum ) ? kMaxPointNum - 1 : num1;	// �����_�����ő�l�𒴂���ꍇ�͐���
		num2 = ( num2 >= kMaxPointNum ) ? kMaxPointNum - 1 : num2;	// �����_�����ő�l�𒴂���ꍇ�͐���
//		printf("%d points received.\n", num);
		if ( num1 < 0 || num2 < 0 )	break;	// �ʐM���s
		if ( num1 >= kMinPoints && num2 >= kMinPoints ) {	// �����_�����ŏ��l�ȏ�
			MakeNearsFromCentresDiv( ps1, num1, &img_size, kDivX, kDivY, eGroup1Num, &nears1 );	// �ߖT�_�v�Z�i�����Łj
			res1 = RetrieveNN5Cor2( ps1, areas1, num1, nears1, img_size, score1, pcor1, &pcornum1, &disc1, prop1, numall1, hash_1, hash2_1, hash3_1 );	// �����̂�
			MakeNearsFromCentresDiv( ps2, num2, &img_size, kDivX, kDivY, eGroup1Num, &nears2 );	// �ߖT�_�v�Z�i�����Łj
			res2 = RetrieveNN5Cor2( ps2, areas2, num2, nears2, img_size, score2, pcor2, &pcornum2, &disc2, prop2, numall2, hash_2, hash2_2, hash3_2 );	// �����̂�
			printf("res: %d, %d\n", res1, res2);
			printf("doc: %s, %s\n", dbcors1[res1], dbcors2[res2]);
#ifdef	SHOW_SCORE_GRAPH	// �X�R�A�̃O���t��\��
#ifdef	WIN32	// Linux�ł̃E�B���h�E�\���̂������킩��Ȃ��̂�
			ShowScoreGraph( "Score", score, eDbDocs );
#endif
#endif
			// �X�R�A�����Z���Č������ʂ�����
			res = CalcTotalScore( eDbDocs, num1, num2, numall1, numall2, score1, score2 );
			printf("total_res: %d\n", res);
			// ���ʂ̑I��
			if ( score1[res1] > score2[res2] ) {
				ps = ps1;
				num = num1;
				dbcors = dbcors1;
				score = score1;
				res = res1;
				psall = psall1;
				sizeall = sizeall1;
				pcornum = pcornum1;
			} else {
				ps = ps2;
				num = num2;
				dbcors = dbcors2;
				score = score2;
				res = res2;
				psall = psall2;
				sizeall = sizeall2;
				pcornum = pcornum2;
			}

			// �ŏ��Ή��_���ȏ�Ȃ�A�ˉe�ϊ��p�����[�^���v�Z
			if ( pcornum >= kMinPointsToCalcParam ) {
				if ( score1[res1] > score2[res2] )
					 CalcProjParamTop( ps, psall[res], pcor1, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// �o�^�摜�ɏ㏑��
				else
					 CalcProjParamTop( ps, psall[res], pcor2, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// �o�^�摜�ɏ㏑��
			} else {
				param = zero_param;
			}
			if (ps1 != NULL && num1 > 0 )	ReleaseCentres( ps1 );	// ps�̉��
			if (areas1 != NULL && num1 > 0 )	free( areas1 );
			ReleaseNears( nears1, num1 );	// nears�̉��
			if (ps2 != NULL && num2 > 0 )	ReleaseCentres( ps2 );	// ps�̉��
			if (areas2 != NULL && num2 > 0 )	free( areas2 );
			ReleaseNears( nears2, num2 );	// nears�̉��
			// �v�Z���Ԃ̕\��
//			OutPutResultSv( score, end-start, 0/*5*/, doc_name );
			if ( score[res] > kTopThr ) {	// �g�b�v�̓��[����臒l�ȉ��i���W�F�N�g�j
				GetBasename2( dbcors[res], kMaxPathLen, doc_name );
				printf("%s, %s\n", dbcors[res], doc_name);
				res_size = sizeall[res];	// ���������Ȃ炻�̃T�C�Y
			}
			else {
				doc_name[0] = '\0';
				param = zero_param;	// �������ʂ��Ȃ��̂Ƀp�����[�^����������Ă̂��ςȂ̂�
				res_size = sizeall1[0];	// �����łȂ���΃f�[�^�x�[�X��0�Ԃ̃T�C�Y
			}
			puts( doc_name );
		}
		else {	// �����_�����ŏ��l�𖞂����Ȃ�
//			puts("min");
			doc_name[0] = '\0';
			param = zero_param;
			res_size = sizeall1[0];	// �f�[�^�x�[�X��0�Ԃ̃T�C�Y
		}

//		fprintf(stdout, "%s\n", doc_name);
		if ( eProtocol == kTCP )	ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		else						ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		if ( ret < 0 )	break;
	}
	// �I������
	free(score1);
	free(score2);
	if ( eProtocol == kUDP ) {
#ifdef	WIN32
		CloseWinSock( sidpt );
		CloseWinSock( sidres );
#else
		CloseSock( sidpt );
		CloseSock( sidres );
#endif
	}
#ifdef	WIN32
	CloseWinSock( sid2 );
	CloseWinSock( sid1 );
#else
	ShutdownSockSvTCP( sid2 );	// shutdown
	CloseSock( sid1 );	// close
#endif

#ifdef	SHOW_SCORE_GRAPH
#ifdef	WIN32
	cvDestroyWindow( "Score" );
#endif
#endif
		
//		goto start_nego2;	// �l�S�҂��܂Ŗ߂�
	return 1;
}

int IsSucceed( char *str1, char *str2 )
// �������[�h�p�F��������
{
	char base1[kMaxPathLen], base2[kMaxPathLen];

	GetBasename( str1, kMaxPathLen, base1 );
	GetBasename( str2, kMaxPathLen, base2 );

//	printf( "%s, %s\n", base1, base2 );
	return !strcmp( base1, base2 );
}

double Calc12Diff( int *score )
// 1�ʂ�2�ʂ̍����v�Z
{
	int i, score_1st = 1, score_2nd = 1;

	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > score_1st ) {
			score_2nd = score_1st;
			score_1st = score[i];
		} else if ( score[i] > score_2nd ) {
			score_2nd = score[i];
		}
	}
//	printf("%d\t%d\n", score_1st, score_2nd );
	return ((double) score_1st) / ((double) score_2nd);
}

cv::Mat MakeFeaturePointImage( cv::Mat orig, cv::Mat connected, cv::Point *ps, int num )
// �����_�摜�̍쐬�i���͉摜�̓O���C�X�P�[���j
{
	int i, j, p, con_flag = 0;
	cv::Mat fp_img;
	unsigned char val[4];

	cv::bitwise_not( connected, connected );
	fp_img = cv::Mat::zeros(orig.size(), CV_8UC3);
	fp_img.setTo(cv::Scalar::all(0));
	cv::bitwise_not( fp_img, fp_img );
	if (fp_img.empty())	return cv::Mat();
	for ( j = 0; j < orig.rows; j++ ) {
		for ( i = 0; i < orig.cols; i++ ) {
			con_flag = 0;
			GetPixU( connected, i, j, val );
			if ( val[0] != 255 ) {
				val[0] = 0;
				val[1] = 255;
				val[2] = 0;
				SetPixU( fp_img, i, j, val );
				con_flag = 1;
			}
			GetPixU( orig, i, j, val );
			if ( val[0] != 255 ) {
				if ( con_flag ) {
					val[1] = val[0];
					val[0] = 0;
					val[2] = 0;
					SetPixU( fp_img, i, j, val );
				}
				else {
					val[1] = val[0];
					val[2] = val[0];
					SetPixU( fp_img, i, j, val );
				}
			}
		}
	}
	for ( p = 0; p < num; p++ ) {
		cv::circle( fp_img, ps[p], 4, cRed, cv::FILLED, cv::LINE_AA, 0 );
	}

	return fp_img;
}

int compare_int( const int *a, const int *b )
// qsort()�p�̔�r�֐��i�~���j
{
	return *b - *a;
}

void ShowScoreGraph( char *win_name, int *score, int doc_num )
// �X�R�A�̃O���t��\��
{
	static cv::Mat img;
	int i, key, score_sorted[kMaxDocNum], max_score, bar_len;

	// �摜�̍쐬
	if (img.empty())	img = cv::Mat::zeros(cv::Size( 100, 200 ), CV_8UC3);

	// �X�R�A�̃R�s�[�ƃ\�[�g
	memcpy( score_sorted, score, sizeof(int) * doc_num );
	qsort( score_sorted, doc_num, sizeof(int), (int (*)(const void*, const void*))compare_int );

	// �\��
	img.setTo(cv::Scalar::all(0));
	max_score = score_sorted[0];
	for ( i = 0; i < 200 && i < doc_num; i++ ) {
		bar_len = (int)((double)score_sorted[i] / (double)max_score * (double)100);
		cv::line( img, cv::Point( 0, i ), cv::Point( bar_len, i ), cv::Scalar(0,0,255), 1, 8, 0 );
	}

	// �摜�̃E�B���h�E�\��
	cv::imshow( win_name, img );
	key = cv::waitKey( 1 );	// �Ȃ��ƃE�B���h�E���X�V����Ȃ�
}

int compare_int_asc( const int *a, const int *b )
// qsort()�p�̔�r�֐��i�����j
{
	return *a - *b;
}

int CalcTotalScore( int doc_num, int num1, int num2, int *numall1, int *numall2, int *score1, int *score2 )
// �f���A���n�b�V�����[�h�œ���ꂽ��̌��ʂ����Z���C�ŏI�I�ȃX�R�A���v�Z���Č������ʂ����߂�
{
	int i, res, med1, med2, doc_max = -1;
	int *score_copy1, *score_copy2;
	double score_cur, score_max = DBL_MIN;

	score_copy1 = (int *)calloc( doc_num, sizeof(int) );
	score_copy2 = (int *)calloc( doc_num, sizeof(int) );
	// �X�R�A�̃R�s�[�ƃ\�[�g
	memcpy( score_copy1, score1, sizeof(int) * doc_num );
	memcpy( score_copy2, score2, sizeof(int) * doc_num );
	qsort( score_copy1, doc_num, sizeof(int), (int (*)(const void*, const void*))compare_int_asc );
	qsort( score_copy2, doc_num, sizeof(int), (int (*)(const void*, const void*))compare_int_asc );
	// �����l�̎擾
	med1 = score_copy1[(int)(doc_num / 2)];
	med2 = score_copy2[(int)(doc_num / 2)];
	// �ŏI�I�ȃX�R�A��score1-med1+score2-med2�Ƃ���
	// score1/med1+score2/med2
	// �d�ݕt��
	for ( i = 0; i < doc_num; i++ ) {
//		score_cur = score1[i] - med1 + score2[i] - med2;
//		score_cur = (double)score1[i] / med1 + (double)score2[i] / med2;
//		score_cur = score1[i] + score2[i] * 5;
//		score_cur = (int)(score1[i] + score2[i] * (numall1[i] / numall2[i]));
		score_cur = score1[i] + score2[i] * (num1 / num2);
		if ( score_cur > score_max ) {
			score_max = score_cur;
			doc_max = i;
		}
	}
	res = doc_max;

	free( score_copy1 );
	free( score_copy2 );

	return res;
}
