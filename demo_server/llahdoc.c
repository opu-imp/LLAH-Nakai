/* dcams.c */
#define	GLOBAL_DEFINE	/* extern変数にここで実体を与える */
// 変更テスト

#include "def_general.h"

#include <stdio.h>
#include <math.h>

// ネットワーク
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

#include "cv.h"
#include "highgui.h"

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
#define	kMaxNameLen	(20)	/* クライアント名の最大長さ */

#define	kTCP	(1)
#define	kUDP	(2)

#define	SEND_FLAG	0
#ifdef	WIN32
#define	RECV_FLAG	0
#else
#define	RECV_FLAG	MSG_WAITALL
#endif

//#define	kTopThr	3
//#define	kTopThr	0	/* 打ち切りモードの実験のために変更 */
//#define	kTopThr	0	/* 打ち切りモードの実験のために変更 */
//#define	kTopThr	20	/* 08/08/26 日本語モードでのfalse-positive除去のために変更 */
//#define	kTopThr	4	/* 08/09/02 アラビア語でrejectしすぎなので変更 */
#define	kTopThr	3	/* 08/09/14 ラーオ語でrejectしすぎなので変更 */
#define	kMinPoints	(20)	/* USBカメラモードでの最小特徴点数 */

#define	kMaxDocNameLen	(100)	/* 画像名の最大長 */

#define	kRecvBuffSize	(sizeof(CvSize) + sizeof(int) + sizeof(CvPoint) * kMaxPointNum)	/* 受信時のバッファのサイズ */
#define	kRecvBuffSizeAreas	(sizeof(CvSize) + sizeof(int) + (sizeof(CvPoint) + sizeof(unsigned short)) * kMaxPointNum)	/* 受信時のバッファのサイズ（面積も受信バージョン） */
#define	kSendBuffSize	(kMaxDocNameLen + sizeof(strProjParam) + sizeof(CvSize))

long allocated_hash = 0;

int RecvPoints( SOCKET sid, CvPoint **ps0, CvSize *size )
// 点データを受信
{
	int i, num = 0, ret, buff_cur;
	char buff[kRecvBuffSize];
	CvPoint *ps;
	
	// バッファに受信
	ret = recv( sid, buff, kRecvBuffSize, RECV_FLAG );
	if ( ret <= 0 )	return -1;	// 受信失敗
	// バッファから画像サイズを得る
	for ( i = 0, buff_cur = 0; i < sizeof(CvSize); i++, buff_cur++ ) {
		((char *)size)[i] = buff[buff_cur];
	}
	// バッファから特徴点数を得る
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		((char *)&num)[i] = buff[buff_cur];
	}
	// 特徴点数が1以上ならメモリを確保
	if ( num > 0 )	ps = (CvPoint *)calloc( num, sizeof(CvPoint) );
	else	ps = NULL;
	*ps0 = ps;
	// バッファから特徴点データを得る
	for ( i = 0; i < (int)(sizeof(CvPoint) * num); i++, buff_cur++ ) {
		((char *)ps)[i] = buff[buff_cur];
	}

	return num;
}

int RecvPointsAreas( SOCKET sid, CvPoint **ps0, double **areas0, CvSize *size )
// 点データを受信
{
	int i, num = 0, ret, buff_cur;
	char buff[kRecvBuffSizeAreas];
	CvPoint *ps;
	double *areas;
	unsigned short areas_us[kMaxPointNum];
	
	// バッファに受信
	ret = recv( sid, buff, kRecvBuffSizeAreas, RECV_FLAG );
	if ( ret <= 0 )	return -1;	// 受信失敗
	// バッファから画像サイズを得る
	for ( i = 0, buff_cur = 0; i < sizeof(CvSize); i++, buff_cur++ ) {
		((char *)size)[i] = buff[buff_cur];
	}
	// バッファから特徴点数を得る
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		((char *)&num)[i] = buff[buff_cur];
	}
	// 特徴点数が1以上ならメモリを確保
	if ( num > 0 )	{
		ps = (CvPoint *)calloc( num, sizeof(CvPoint) );
		areas = (double *)calloc( num, sizeof(double) );
	}
	else {
		ps = NULL;
		areas = NULL;
	}
	*ps0 = ps;
	*areas0 = areas;
	// バッファから特徴点データを得る
	for ( i = 0; i < (int)(sizeof(CvPoint) * num); i++, buff_cur++ ) {
		((char *)ps)[i] = buff[buff_cur];
	}
	// バッファから面積データを得る
	buff_cur = sizeof(CvSize) + sizeof(int) + sizeof(CvPoint) * kMaxPointNum;
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
// 文書名を送信
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

int SendResultCor( SOCKET sid, char *doc_name, int len, int doc, int pcor[][2], int pcornum, CvPoint *ps, CvPoint **psall, CvSize size, CvSize *sizeall )
// 文書名と対応点を送信
{
	int i, ret;
	CvPoint corps[kMaxPointNum][2];
	
	if ( send( sid, doc_name, len, SEND_FLAG ) < 0 )	return -1;	// 文書名を送信
	if ( send( sid, &size, sizeof(CvSize), SEND_FLAG ) < 0 )	return -1;	// クエリ画像のサイズを送信
	if ( send( sid, &(sizeall[doc]), sizeof(CvSize), SEND_FLAG ) < 0 )	return -1;	// 登録画像のサイズを送信
	if ( ret = send( sid, &pcornum, sizeof(int), SEND_FLAG ) < 0 )	return -1;	// 対応の数を送信
	for ( i = 0; i < pcornum; i++ ) {
//		printf("%d(%d,%d), %d(%d,%d)\n", pcor[i][0], ps[pcor[i][0]].x, ps[pcor[i][0]].y, pcor[i][1], psall[doc][pcor[i][1]].x, psall[doc][pcor[i][1]].y);
//		ret = send( sid, &(ps[pcor[i][0]]), sizeof(CvPoint), SEND_FLAG );	// クエリ側特徴点を送信
//		ret = send( sid, &(psall[doc][pcor[i][1]]), sizeof(CvPoint), SEND_FLAG );	// 登録側特徴点を送信
		corps[i][0] = ps[pcor[i][0]];
		corps[i][1] = psall[doc][pcor[i][1]];
	}
	
	if ( pcornum > 0 )	ret = send( sid, corps, sizeof(CvPoint)*2*pcornum, SEND_FLAG );
	return ret;
}

int SendResultParam( SOCKET sid, char *doc_name, int len, strProjParam *param, CvSize *img_size, struct sockaddr_in *addr, int ptc )
// 文書名と射影変換パラメータを送信
{
	int ret;
	char buff[kSendBuffSize];

	memcpy( buff, doc_name, kMaxDocNameLen );
	memcpy( buff + kMaxDocNameLen, param, sizeof(strProjParam) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam), img_size, sizeof(CvSize) );

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
// 通信設定を受信
{
	int ret;

	ret = recv( sid, (char *)ptc, sizeof(int), RECV_FLAG );	// プロトコル
	if ( ret <= 0 )	return -1;
	ret = recv( sid, (char *)pt_port, sizeof(int), RECV_FLAG );	// 特徴点ポート
	if ( ret <= 0 )	return -1;
	ret = recv( sid, (char *)res_port, sizeof(int), RECV_FLAG );	// 検索結果ポート
	if ( ret <= 0 )	return -1;
	ret = recv( sid, cl_name, kMaxNameLen, RECV_FLAG );	// クライアント名
	if ( ret <= 0 )	return -1;
	
	return 1;
}

extern int corres[kMaxPointNum][kMaxDocNum];

int main( int argc, char** argv )
{
	int num, **nears = 0, argi = 1, res = 0, emode = RETRIEVE_MODE, rmode = NOT_RECOVER_MODE, \
	fmode = CONNECTED_MODE, pmode = NOT_PREPARED_MODE;
	int *score;
	CvPoint *ps = 0;
	CvSize img_size;
	int start, end = 0, start_con, end_con, start_fp, end_fp, start_hl, end_hl, start_nears, end_nears;
	IplImage *img;
	strDisc disc, disc1, disc2;
	char fname[kMaxPathLen];
	FILE *fp;
	double prop;
	// 公開に向けて改変
	// 09/01/06 デュアルハッシュに向けて改変
	int ret;
	CvPoint **reg_pss = NULL, **reg_pss1 = NULL, **reg_pss2 = NULL;
	CvSize *reg_sizes = NULL, *reg_sizes1 = NULL, *reg_sizes2 = NULL;
	int *reg_nums = NULL, *reg_nums1 = NULL, *reg_nums2 = NULL;
	char **dbcors = NULL, **dbcors1 = NULL, **dbcors2 = NULL;
	// 実験モード用
	int total_que = 0, suc_que = 0, total_time = 0, ret_time = 0;
	double total_diff = 0.0L, total_top = 0.0L;
	// 面積の利用
	double *areas = NULL, **reg_areass = NULL, **reg_areass1 = NULL, **reg_areass2 = NULL;
	// DLL向けの改変
	strHList **hash = NULL, **hash_1 = NULL, **hash_2 = NULL;
	strHList2 **hash2 = NULL, **hash2_1 = NULL, **hash2_2 = NULL;
	HENTRY *hash3 = NULL, *hash3_1 = NULL, *hash3_2 = NULL;
	// 対応点
	int pcor[kMaxPointNum][2], pcornum = 0;
	int i, j, rank;
	// デュアルハッシュ向けの改変
	double prop1, prop2;
	int gaussian_parameter;

#ifdef	WIN32
	timeBeginPeriod( 1 );//精度を1msに設定
#endif
	start = GetProcTimeMiliSec();

	ReadIniFile();

	if ( ( argi = AnalyzeArgAndSetExtern2( argc, argv ) ) < 0 )	return 1;
	switch ( eEntireMode ) {
		/************************ 通常検索モード ************************/
		case RETRIEVE_MODE:
			// ./dcams
			fprintf( stderr, "Retrieval from Static Image\n" );
			// 設定ファイルの読み込み
			ret = LoadConfig( eConfigFileName, &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &eProp, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
			// 離散化ファイルの読み込み
			sprintf( fname, "%s%s", eDirsDir, eDiscFileName );
			ret = LoadDisc( fname, &disc );
			// ハッシュの読み込み
			if ( eNoHashList )		hash3 = LoadHash3( disc.num, eHashDatFileName );
			if ( eCompressHash )	hash2 = LoadHash2( disc.num, eHashDatFileName );
			else					hash = LoadHash( disc.num, eHashDatFileName );
			// 点ファイルの読み込み
			sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
			ret = LoadPointFile2( fname, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
			// 実験モードならハッシュのサイズを出力
			if ( eExperimentMode )	printf( "Allocated Hash Size : %d\n", allocated_hash );

			for ( ; argi < argc; argi++ ) {	// オプション以外の引数について実行
				if ( !eExperimentMode ) {
					fprintf( stdout, "%s\n", argv[argi] );
					fprintf( stderr, "%s\n", argv[argi] );
				}
				// 特徴点の抽出
				if ( IsDat( argv[argi] ) ) {	// datファイルの場合
					num = LoadPointFile( argv[argi], &ps, &img_size );
					areas = NULL;	// 面積のないdatファイルなので
				}
				else {	// 結合画像を作成
#ifdef	WIN32	// OpenCVのバージョンが異なるため
					img = GetConnectedImage( argv[argi], eEntireMode );
#else
					if ( eIsJp ) 	img = GetConnectedImageJp2( argv[argi], eEntireMode );
					else			img = GetConnectedImage2( argv[argi], eEntireMode, eHashGaussMaskSize );
#endif
					if ( img == NULL )	continue;
					// 特徴点を抽出
					num = MakeCentresFromImage( &ps, img, &img_size, &areas );
					cvReleaseImage( &img );
				}
				// 近傍構造を計算
				MakeNearsFromCentres( ps, num, &nears );
				// 検索
				score = (int *)calloc( kMaxDocNum, sizeof(int) );
#ifdef	DRAW_COR
				GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
				res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, eProp, reg_nums, hash, hash2, hash3 );
				DrawCor( ps, num, img_size, res, reg_pss[res], reg_nums[res], reg_sizes[res], pcor, pcornum );
#else
#ifdef	DRAW_COR_ALL
				GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
				res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, eProp, reg_nums, hash, hash2, hash3 );
				res = 73;	// MIRU08修正実験：cvpr011b040.jpgでの正解（オリジナルでは2位）
//				res = 9368;	// オリジナルでの1位（提案手法では15位）
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
//				OutPutResult( score, 0, 5, argv[argi] );	// そのままでは使えない
				if ( !eExperimentMode )	printf( "%s : %d\n", dbcors[res], score[res] );
				if ( eExperimentMode ) {	// 実験モードなら精度計測
					total_que++;
					if ( IsSucceed( argv[argi], dbcors[res] ) )	suc_que++;
					total_time += ret_time;
					total_diff += Calc12Diff( score );
					total_top += (double)score[res];
				}
				free( score );
			}
			if ( eExperimentMode ) {	// 実験モードなら結果出力
				printf("Accuracy : %d/%d(%.2f\%)\n", suc_que, total_que, ((double)suc_que)/((double)total_que)*100 );
				printf("Average Proc Time : %d micro sec\n", (int)(((double)total_time) / ((double)total_que)) );
				printf("Average Top Vote : %.2f\n", (total_top / (double)total_que) );
				printf("Average Diff : %.2f\n", (total_diff / (double)total_que) );
			}
			return 0;
		/************************ USBカメラモード ************************/
		case USBCAM_SERVER_MODE:
			ret = RetrieveUSBCamServer();
			return 0;
		/************************ USBカメラモード（デュアルハッシュ） ************************/
		case USBCAM_SERVER_DUAL_HASH_MODE:
			ret = RetrieveUSBCamServerDualHash();
			return 0;
		/************************ ハッシュ構築モード ************************/
		case CONST_HASH_MODE:
			// ./llahdoc -c [画像のディレクトリ] [画像の拡張子] (-h [出力ディレクトリ])
			fprintf( stderr, "Hash Constraction\n" );
			// 特徴点の抽出
			fprintf( stderr, "Extracting Feature Points...\n" );
//			ret = CreatePointFile3( &reg_pss, &reg_sizes, &reg_nums, &dbcors );
			ret = CreatePointFile4( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors, eHashSrcDir, eHashSrcSuffix, eHashGaussMaskSize, eDirsDir, ePointDatFileName );
			if ( ret <= 0 )	return 1;
const_rest:
			// 離散化ファイルの作成
			fprintf( stderr, "Making %s...\n", eDiscFileName );
			ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss, reg_nums, &disc, eDiscFileName );
			if ( ret <= 0 )	return 1;
			// ハッシュの構築
			fprintf( stderr, "Constructing Hash Table...\n" );
			ret = ConstructHash2( eDbDocs, reg_pss, reg_areass, reg_nums, &disc, &hash, &hash2, &hash3 );
			if ( ret <= 0 )	return 1;
			// 衝突の多い項目を除去
			if ( !eNoHashList && eRemoveCollision )	{
				if ( eCompressHash )	ret = RefineHash2( hash2 );
				else					ret = RefineHash( hash );
			}
			// ハッシュのチェック
			if ( eExperimentMode ) {
				if ( eNoHashList )			ChkHash3( hash3 );
				else if ( eCompressHash )	ChkHash2( hash2 );
				else						ChkHash( hash );
			}
			// ハッシュの保存
			fprintf( stderr, "Saving Hash Table...\n" );
			if ( eNoHashList )			ret = SaveHash3( hash3, eHashDatFileName );
			else if ( eCompressHash )	ret = SaveHash2( hash2, eHashDatFileName );
			else						ret = SaveHash( hash, eHashDatFileName );
			if ( ret <= 0 )	return 1;
			// 比例定数の計算
			fprintf( stderr, "Calculating Prop...\n" );
			eProp = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss, reg_areass, reg_nums, &disc, hash, hash2, hash3 );
			// 設定ファイルの保存
			fprintf( stderr, "Saving Config...\n" );
			ret = SaveConfig( eInvType, eGroup1Num, eGroup2Num, eDiscNum, eProp, eCompressHash, eDocBit, ePointBit, eRBit, eOBit, eHList2DatByte, eRotateOnce, eUseArea, eIsJp, eNoHashList, eConfigFileName );

			if ( ret <= 0 )	return 1;

			return 0;
		/* point.datを用いたハッシュ構築モード */
		case CONST_HASH_PF_MODE:
			fprintf( stderr, "Hash Construction using point.dat\n" );
			sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
			ret = LoadPointFile2( fname, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
			if ( ret <= 0 )	return 1;
			goto const_rest;

			return 0;
		/* デュアルハッシュ構築モード */
		case CONST_DUAL_HASH_MODE:
			// ./llahdoc -D [画像のディレクトリ] [画像の拡張子] (-h [出力ディレクトリ])
			fprintf( stderr, "Dual Hash Constraction\n" );
			// 特徴点の抽出
			fprintf( stderr, "Extracting Feature Points 1...\n" );
			if ( eVariousAutoMaskSizeDual )	gaussian_parameter = -1;	// 文字単位を示すフラグ
			else	gaussian_parameter = 1;
			ret = CreatePointFile4( &reg_pss1, &reg_areass1, &reg_sizes1, &reg_nums1, &dbcors1, eHashSrcDir, eHashSrcSuffix, gaussian_parameter, eDirsDir, "point1.dat" );
			if ( ret <= 0 )	return 1;
			fprintf( stderr, "Extracting Feature Points 2...\n" );
			if ( eVariousAutoMaskSizeDual )	gaussian_parameter = -2;	// 単語単位を示すフラグ
			else	gaussian_parameter = 23;
			ret = CreatePointFile4( &reg_pss2, &reg_areass2, &reg_sizes2, &reg_nums2, &dbcors2, eHashSrcDir, eHashSrcSuffix, gaussian_parameter, eDirsDir, "point2.dat" );
			if ( ret <= 0 )	return 1;
			// 離散化ファイルの作成
			fprintf( stderr, "Making %s...\n", eDiscFileName );
			ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss1, reg_nums1, &disc1, "disc1.txt" );
			if ( ret <= 0 )	return 1;
			ret = MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss2, reg_nums2, &disc2, "disc2.txt" );
			if ( ret <= 0 )	return 1;
			// ハッシュの構築
			fprintf( stderr, "Constructing Hash Table 1...\n" );
			ret = ConstructHash2( eDbDocs, reg_pss1, reg_areass1, reg_nums1, &disc1, &hash_1, &hash2_1, &hash3_1 );
			if ( ret <= 0 )	return 1;
			fprintf( stderr, "Constructing Hash Table 2...\n" );
			ret = ConstructHash2( eDbDocs, reg_pss2, reg_areass2, reg_nums2, &disc2, &hash_2, &hash2_2, &hash3_2 );
			if ( ret <= 0 )	return 1;
			// 衝突の除去やハッシュのチェックは省いた．必要ならCONSTRUCT_HASHから持ってくる
			// ハッシュの保存
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
			// 比例定数の計算
			fprintf( stderr, "Calculating Prop 1...\n" );
			prop1 = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss1, reg_areass1, reg_nums1, &disc1, hash_1, hash2_1, hash3_1 );
			fprintf( stderr, "Calculating Prop 2...\n" );
			prop2 = CalcProp2( min( ePropMakeNum, eDbDocs ), reg_pss2, reg_areass2, reg_nums2, &disc2, hash_2, hash2_2, hash3_2 );
			printf("%lf, %lf\n", prop1, prop2 );
			// 設定ファイルの保存
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
	// 以下検索，ハッシュ構築等に分岐
	/************************ 通常検索モード ************************/
	if ( emode == RETRIEVE_MODE || emode == RET_MP_MODE ) {
		LoadDisc( eDiscFileName, &disc );	// 離散化ファイルの読み込み
		start_hl = GetProcTimeMiliSec();
		LoadHash(disc.num);	// ハッシュのロード
		end_hl = GetProcTimeMiliSec();
		printf("Hash loading time : %dms\n", end_hl - start_hl);
		
retrieve:		
		
		for ( ; argi < argc; argi++ ) {	// すべての引数について
			if ( strlen( argv[argi] ) >= kMaxPathLen ) {
				fprintf( stderr, "error: at main in dirs.c: too long path %s\n", argv[argi] );	// 検索ファイルのパスの文字列が長すぎる
				continue;
			}
			strcpy( fname, argv[argi] );
			printf("%s\n", fname);
			// 結合画像の作成（用意されているか囲み領域ならロード，そうでなければ計算）
			start_con = GetProcTimeMiliSec();
			if ( pmode == PREPARED_MODE || fmode == ENCLOSED_MODE ) {	// 結合画像が用意されているか，囲まれた領域を特徴点にするなら
				if ( (img = cvLoadImage(fname, 0)) == NULL ) {
					fprintf(stderr, "画像ファイル%sが開けません\n", fname);
					return 1;
				}
				if ( pmode == PREPARED_MODE && rmode == RECOVER_MODE )	argi++;	// 結合画像が容易済みで，別途元画像から射影変換の補正をしたいなら
			}
			else if ( fmode != USEPF_MODE && !IsTxt(fname) && !IsDat(fname) ) {
				if ( (img = GetConnectedImage( fname, emode )) == NULL ) {	// 結合画像を作成
					fprintf(stderr, "%sから結合画像を作成するのに失敗しました\n", fname);
					return 1;
				}
			}
			end_con = GetProcTimeMiliSec();
//			printf("Create word-connected image : %dms\n", end_con - start_con);
			// 結合画像（囲み領域なら原画像）から特徴点を抽出
			start_fp = GetProcTimeMiliSec();
			if ( IsTxt( fname ) || IsDat( fname ) ) {
				num = LoadPointFile( fname, &ps, &img_size );
			} else if ( fmode == CONNECTED_MODE ) {	// 単語の重心を特徴点とする
				num = MakeCentresFromImage( &ps, img, &img_size, NULL );	// 画像から重心を計算する
			} else if ( fmode == ENCLOSED_MODE ) {	// 囲まれた領域の重心を特徴点とする
				num = MakeFeaturePointsFromImage( &ps, img, &img_size );
			} else if ( fmode == USEPF_MODE ) {
				num = LoadPointFile( fname, &ps, &img_size );
			} else {
				fprintf(stderr, "特徴点抽出モードが不正です\n");
				return 1;
			}
			if ( num == 0 ) {
				fprintf(stderr, "連結成分がみつかりませんでした\n");
				return 1;
			}
//			return 1;	// 途中終了
			start_nears = GetProcTimeMiliSec();
            MakeNearsFromCentres( ps, num, &nears );	// 各点について上位kNears個の近傍点のインデックスを求め，nearsに格納する
//			num = ScreenPointsWithNears( ps, num, nears );	// 近傍点の距離を用いて特徴点をふるいにかける
            end_fp = GetProcTimeMiliSec();
//           printf("nears: %d\n", end_fp - start_nears );
//			printf("Extract feature points and analyse neighbors : %dms\n", end_fp - start_fp );
			score = (int *)calloc( kMaxDocNum, sizeof(int) );
			// 検索
			if ( rmode == RECOVER_MODE ) {
                res = RetrieveAndRecover( ps, num, nears, img_size, fname, emode );	// 検索および射影変換の補正
			} else {
				res = RetrieveNN5( ps, num, nears, img_size, score );	// 検索のみ
			}
			// いろいろと解放
			if ( ps != NULL && num > 0 )
				ReleaseCentres( ps );	// psの解放
			ReleaseNears( nears, num );	// nearsの解放
			if ( fmode != USEPF_MODE && !IsDat( fname ) && !IsTxt( fname ))	cvReleaseImage( &img );
			printf("%d\n", score[0]);
			// 計算時間の表示
			OutPutResult( score, end-start, 5, fname );
			free(score);
		}
//		printf("%d\n", allocated_hash);
		end = GetProcTimeMiliSec();
		printf("Total processing time : %dms\n", end-start);
		return 0;
	}
	/************************ USBカメラモード ************************/
	else if ( emode == USBCAM_SERVER_MODE ) {
		SOCKET sid1, sid2, sidpt, sidres;
		int i, ret, start_com = 0, end_com, start_cam, end_cam;
		int start_dt = 0, end_dt, start_com_dt = 0, end_com_dt, start_cam_dt = 0, end_cam_dt, send_dt, recv_dt, start_ret_all, end_ret_all;
		int pcor[kMaxPointNum][2], pcornum = 0, *numall;
//		int ptc, pt_port, res_port;
		char doc_name[kMaxDocNameLen] = "";
		CvPoint **psall;
		CvSize *sizeall;
		strProjParam param, zero_param;
		struct sockaddr_in addr;
		
		fprintf(stderr, "USB Camera Server Mode\n");
		fprintf(stderr, "%d\n%d\n%d\n%d\n%s\n%s\n", eTCPPort, eProtocol, ePointPort, eResultPort, eClientName, eServerName);
		LoadDisc( eDiscFileName, &disc );	// 離散化ファイルの読み込み
		start_hl = GetProcTimeMiliSec();
		LoadHash(disc.num);	// ハッシュのロード
		end_hl = GetProcTimeMiliSec();
		fprintf(stderr, "Hash loading time : %dms\n", end_hl - start_hl);
		LoadPointFileAll( &psall, &sizeall, &numall );
		// 組み合わせの作成
		GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
		// ネットワーク関係
		fprintf(stderr, "Ready\n");
		// ネゴシエイト開始
start_nego:
		fprintf(stderr, "Start negotiation\n");
#ifdef WIN32
		if ( ( sid1 = InitWinSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socketの初期化
		if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#else
		if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socketの初期化
		if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#endif
		ret = RecvComSetting( sid2, &eProtocol, &ePointPort, &eResultPort, eClientName );	// 通信設定を受信
		if ( ret < 0 )	return 1;
		fprintf(stderr, "%d, %d, %d, %s\n", eProtocol, ePointPort, eResultPort, eClientName );

		if ( eProtocol == kUDP ) {	// UDPの場合．TCPの場合は接続済み
#ifdef	WIN32
			Sleep( 1000 );
			if ( ( sidpt = InitWinSockSvUDP( ePointPort ) ) < 0 )	return 1;
			if ( ( sidres = InitWinSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#else
			sleep(1);	// UDPの場合はクライアントを先に待ち状態にしなければならないので
			if ( ( sidpt = InitSockSvUDP( ePointPort ) ) < 0 )	return 1;
			if ( ( sidres = InitSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#endif
		}
			
		score = (int *)calloc( kMaxDocNum, sizeof(int) );
		zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
		param = zero_param;
		start_com = GetProcTimeMiliSec();
//		ret = SendResultCor( sid2, doc_name, 20, res, pcor, pcornum, ps, psall, img_size, sizeall );	// ダミーを送信
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
				num = RecvPoints( sid2, &ps, &img_size );	// 特徴点データを受信
			}
			else {
				num = RecvPoints( sidpt, &ps, &img_size );	// 特徴点データを受信
			}
			num = ( num >= kMaxPointNum ) ? kMaxPointNum - 1 : num;
//			printf("%d points received.\n", num);
//			printf("recv(dt) : %d micro sec\n", GetDayTimeMicroSec() - recv_dt);
			end_com = GetProcTimeMiliSec();
			end_com_dt = GetDayTimeMicroSec();
//			printf("com : %dms\n", end_com - start_com);
//			printf("com : %dms\n", (int)(end_com_dt - start_com_dt)/1000);
			start_cam_dt = GetDayTimeMicroSec();
			if ( num < 0 )	break;	// 通信失敗
			if ( num >= kMinPoints ) {	// 特徴点数が最小値以上
//				puts("start nears");
				start_nears = GetProcTimeMiliSec();
				start_ret_all = GetProcTimeMicroSec();
//				MakeNearsFromCentres( ps, num, &nears );	// 各点について上位kNears個の近傍点のインデックスを求め，nearsに格納する
				MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num, &nears );	// 近傍点計算（分割版）
				end_nears = GetProcTimeMiliSec();
				// 検索
				start = GetProcTimeMiliSec();
//				puts("start ret");
//				res = RetrieveNN5( ps, num, nears, img_size, score );	// 検索のみ
				res = RetrieveNN5Cor( ps, num, nears, img_size, score, pcor, &pcornum, &disc );	// 検索のみ
				end = GetProcTimeMiliSec();
				end_ret_all = GetProcTimeMicroSec();
//				printf("retrieval(micro sec) : %d\n", end_ret_all - start_ret_all);
				printf("%d\n", end_ret_all - start_ret_all);
//				printf("ret_all : %dms\n", end - start);
//				printf("nears : %dms\nret : %d\n", end_nears - start_nears, end - start );
//				DrawCor( ps, num, img_size, res, psall[res], numall[res], sizeall[res], pcor, pcornum );	// 対応関係を描画
				// 最小対応点数以上なら、射影変換パラメータを計算
				start = GetProcTimeMicroSec();
//				puts("start est");
//				if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_NORMAL, PARAM_IWAMURA );	// キャプチャ画像に上書き
//				if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_IWAMURA );	// 登録画像に上書き
				if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// 登録画像に上書き
				else	param = zero_param;
//				param = zero_param;
//				printf("%lf %lf %lf %lf %lf %lf %lf %lf\n", param.a1, param.a2, param.a3, param.b1, param.b2, param.b3, param.c1, param.c2 );			// いろいろと解放
				end = GetProcTimeMicroSec();
//				printf("param_est : %d micoro sec\n", end - start );
				if ( ps != NULL && num > 0 )
					ReleaseCentres( ps );	// psの解放
				ReleaseNears( nears, num );	// nearsの解放
				// 計算時間の表示
				OutPutResultSv( score, end-start, 0/*5*/, doc_name );
			}
			else {	// 特徴点数が最小値を満たさない
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
//			ret = SendResultCor( sid2, doc_name, 20, res, pcor, pcornum, ps, psall, img_size, sizeall );	// 検索結果を送信
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
		
//		goto start_nego;	// ネゴ待ちまで戻る
		return 0;
	}
	/************************ ハッシュ構築モード ************************/
	else if ( emode == CONST_HASH_MODE ) {
			ConstructHash(fmode);
			// 比例定数の計算と保存
			prop = CalcProp( ePropMakePath );
			eProp = prop;
			sprintf(fname, "%ssetting.txt", eDirsDir);
			fp = fopen(fname, "a");	// このような形で追加するのはよくない．要改善
			fprintf(fp, "%lf\n%d\n", prop, eDbDocs);
			fclose(fp);
			
			end = GetProcTimeMiliSec();
//			printf("Total processing time : %dms\n", end-start);
			
//			goto check_hash;	// 検索モードへ移行　一時的な措置
			SaveHash();
			return 0;
	}
	/************************ ハッシュ追加モード ************************/
	else if ( emode == ADD_HASH_MODE ) {	// ハッシュ追加モード
			ConstructHashAdd(fmode);
			end = GetProcTimeMiliSec();
			printf("Total processing time : %dms\n", end-start);
			return 0;
	}
	/************************ 特徴点抽出モード ************************/
	else if ( emode == CREATE_QPF_MODE || emode == CREATE_RPF_MODE ) {	// Mode for creating point files of query/registered image
		int connect_image_mode;
		char in_fname[kMaxPathLen], out_fname[kMaxPathLen];

		if ( argi + 1 >= argc ) {	// 引数の不足
			fprintf( stderr, "error: at main() in dirs.c: input file and/or output file is not specified\n" );
			return 1;
		}
		strncpy( in_fname, argv[argi], kMaxPathLen );
		strncpy( out_fname, argv[argi+1], kMaxPathLen );
		if ( emode == CREATE_QPF_MODE )	connect_image_mode = RETRIEVE_MODE;	// 結合画像の作成時のモードを設定
		else	connect_image_mode = CONST_HASH_MODE;
		CreatePointFile( in_fname, out_fname, connect_image_mode );
		return 0;
	}
	/************************ 特徴点抽出モード２ ************************/
	else if ( emode == CREATE_RPF_MODE2 ) {
		// ./dcams -G [検索パス] [特徴点ファイル出力ディレクトリ] [対応関係ファイル] [特徴点数ファイル] [開始ナンバー（デフォルトは0）
		printf("RPF2\n");
		CreatePointFile2();
		return 0;
	}
	/************************ テストモード ************************/
	else if ( emode == TEST_MODE ) {
//			AutoConnectTest( argv[2], &img_size );
//			RecovPTFromTemp();
//			printf("%d\n", sizeof(strHList));
//			LoadDisc( kDiscFileName, &disc );	// 離散化ファイルの読み込み
//			LoadHash(disc.num);
//			ChkHash();

		for ( ; argi < argc; argi++ ) {	// すべての引数について
			printf("%s\t", argv[argi]);
			if ( (img = GetConnectedImage( argv[argi], RETRIEVE_MODE )) == NULL ) {	// 結合画像を作成
				fprintf(stderr, "%sから結合画像を作成するのに失敗しました\n", argv[argi]);
				return 1;
			}
			num = MakeCentresFromImage( &ps, img, &img_size, NULL );	// 画像から重心を計算する
			printf("%d\n", num);
			// いろいろと解放
			ReleaseCentres( ps );	// psの解放
			cvReleaseImage( &img );
		}
		
		return 0;
	}
	/************************ ハッシュチェックモード ************************/
	else if ( emode == CHK_HASH_MODE ) {
		LoadDisc( eDiscFileName, &disc );	// 離散化ファイルの読み込み
		LoadHash(disc.num);	// ハッシュのロード
check_hash:
		printf("%d\t", eDiscNum);	// 一時措置
		ChkHash();
		goto retrieve;	// 検索モードへの移行　一時的な措置
		return 0;
	}
#endif

}

void DrawPoint( IplImage *img, CvPoint p, CvScalar color )
// imgのpにcolorで点を打つ
{
#ifdef DRAW_BY_NUM
	static n = 0;
	char tmp[10];
	CvFont font;
	sprintf(tmp, "%d", n++);
	cvInitFont( &font, 7, 1.0, 1.0, 0.1, 0, 1);
    cvPutText( img, tmp, p, &font, color);
#endif
#ifndef DRAW_BY_NUM
	cvCircle( img, p, 5, color, -1, 1, 0 );
#endif
}

void CalcCRDiscriptorLine(CvPoint *ps, int num, int *nears[])
// 複比の方法でディスクリプタを計算する
{
	int pt, pt0, ptd0, next/*, nlc, nl,*/, count = 0, cent[4];
	double ang, cr;

	pt = FirstPoint(ps, num);
#ifdef DRAW_PIC
	DrawPoint( out_img, ps[pt], cWhite);
#endif
	next = FirstFirstRightPointCR(pt, ps, nears);	// 1行目で最初の右の点を探す
	while ( next < 0 ) {	// 1行目の最初の右の点が見つからない場合
		if ( (pt = NextLineCR(pt, ps, nears, 0.0)) < 0 )	// さらに次の行もない場合
			return;
		next = FirstFirstRightPointCR(pt, ps, nears);	// 次の行を1行目として最初の右の点を探す
	}
	ang = GetPointsAngle(ps[pt], ps[next]);	// 最初の点と次の点で角度を計算する

	cent[count++] = pt;	// 点を保存
	
	ptd0 = pt;	// ライン描画用
	pt0 = pt;
//	nlc = NextLineCR(pt, ps, nears, ang);	// とりあえず次の行
	pt = next;
	cent[count++] = pt;
#ifdef DRAW_PIC
	DrawPoint( out_img, ps[pt], cWhite);
	cvLine( out_img, ps[ptd0], ps[pt], cWhite, 4, CV_AA, 0);
#endif
	ptd0 = pt;
	while ( (pt = RightPointCR(pt, pt0, ps, nears[pt], ang)) >= 0 ) {	//	右へトレース
		//	何かの処理
#ifdef DRAW_PIC
		DrawPoint( out_img, ps[pt], cWhite);
		cvLine( out_img, ps[ptd0], ps[pt], cWhite, 4, CV_AA, 0);
#endif
		ang = GetPointsAngle(ps[pt0], ps[pt]);	// 角度を更新
//		nl = NextLineCR(pt, ps, nears, ang);
//		if ( ps[nl].y < ps[nlc].y )	// この点から探した次行の行頭点の方が上にあるなら
//			nlc = nl;

		cent[count++ % 4] = pt;	// 点を保存
		if ( count % 4 == 0 ) {	// 4つたまったら
			cr = CalcCR( ps[cent[0]], ps[cent[1]], ps[cent[2]], ps[cent[3]] ); 
			printf("%lf ", cr);
		}
	}
	printf("%d\n", count);
	pt = NextLineCR(pt0, ps, nears, ang);	// 次の行へ
//	pt = nlc;

	while ( pt >= 0 ) {	// 次の行がなければ終了
		ptd0 = pt;
		pt0 = pt;
//		nlc = NextLineCR(pt, ps, nears, ang);	// とりあえず次の行
		count = 0;
		cent[count++] = pt;
#ifdef DRAW_PIC
		DrawPoint( out_img, ps[pt], cWhite);
#endif
		if ( (next = FirstRightPointCR(pt, ps, nears, ang)) >= 0 )	// 右の点が見つかった場合
			ang = GetPointsAngle(ps[pt], ps[next]);	// 角度を計算
		pt = next;

		while ( pt >= 0 ) {	// 右の点がなければ終了
			// 何かの処理
			cent[count++ % 4] = pt;
			if ( count % 4 == 0 ) {	// 4つたまったら
				cr = CalcCR( ps[cent[0]], ps[cent[1]], ps[cent[2]], ps[cent[3]] ); 
				printf("%lf ", cr);
			}
#ifdef DRAW_PIC
			DrawPoint( out_img, ps[pt], cWhite);
			cvLine( out_img, ps[ptd0], ps[pt], cWhite, 4, CV_AA, 0);
#endif
			ang = GetPointsAngle(ps[pt0], ps[pt]);	//　角度を更新
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

double CalcCR( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 )
// 複比を計算する
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

double CalcJI( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 )
// j-invariantを計算する
{
	double tau;

	tau = CalcCR(p1, p2, p3, p4);
	return pow(tau*tau - tau + 1, 3) / (tau*tau*(tau-1)*(tau-1));
}

int FirstFirstRightPointCR(int pt, CvPoint *ps, int *nears[])
// CRにおいて，1行目の最初の点ptからだいたい右の点を見つける
{
	int i, ptr;
	double ang;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[pt][i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang) < kFFRAngle && RightPointCR(ptr, pt, ps, nears[ptr], ang) >= 0)	// だいたい右で，ほとんど同じ方向に点を持つなら
			return ptr;
	}
	return -1;
}

int FirstRightPointCR(int pt, CvPoint *ps, int *nears[], double ang0)
// CRにおいて，2行目以降の最初の点ptからだいたい右の点を見つける
{
	int i, ptr;
	double ang;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[pt][i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang0-ang) < kRoughlyAngle && RightPointCR(ptr, pt, ps, nears[ptr], ang) >= 0)	// ang0とやや同じで，ほとんど同じ方向に点を持つなら
			return ptr;
	}
	return -1;
}

int RightPointCR(int pt, int pt0, CvPoint *ps, int nears[], long double ang0 )
// CRにおいて，ptからangとほとんど同じ角度の点を見つける
{
	int i, ptr;
	long double ang, ang1;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		ang1 = GetPointsAngle(ps[pt0], ps[ptr]);
		if ( fabs(ang0-ang) < kNearlyAngle || (fabs(ang0-ang) < kAlmostAngle && fabs(ang0-ang1) < kCloseAngle) )	// ほとんど同じ角度なら　もしくは右側にあり，かつ行頭とごく近い角度なら
			return ptr;
	}
	return -1;
}

int LeftPointCR(int pt, CvPoint *ps, int nears[], double ang0 )
// CRにおいて，ptからang0と180度反対のものとやや同じ角度の点を見つける
{
	int i, ptr;
	double ang;

	if ( ang0 < 0 )	ang0 += M_PI;	// ang0を180度反転させる
	else			ang0 -= M_PI;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang0-ang) < kRoughlyAngle )	// やや同じ角度なら
			return ptr;
	}
	return -1;
}

int NextLineCR(int pt, CvPoint *ps, int *nears[], double ang0)
// CRにおいて，ptから次の行の最初の点を見つける．（大体下の点）
{
	int i, ptr;
	double ang;

	for ( i = 0; i < kNears; i++ ) {
		ptr = nears[pt][i];
		ang = GetPointsAngle(ps[pt], ps[ptr]);
		if ( fabs(ang - M_PI/2) < kNLAngle ) {	// だいたい下
			while ( (pt = LeftPointCR(ptr, ps, nears[ptr], ang0)) >= 0 ) {
				ptr = pt;
			}
			return ptr;
		}
	}
	return -1;
}

//long double GetPointsAngle(CvPoint p1, CvPoint p2)
// p1から見たp2の角度を計算する（約-3.14から約3.14）
//{
//	return atan2(p2.y - p1.y, p2.x - p1.x);
//}

void Calc1NNDiscriptor(CvPoint *ps, int num, int *nears[], char disc[])
// 1NNでディスクリプタを計算し，discに書き込む
{
	int dp = 0, pt, pt0, rpt, *visited, connected, pt00;
#ifdef	CON_WORD
	double dist;
#endif

	// 訪問記録（無限ループ防止のため）
	visited = (int *)calloc(num, sizeof(int));

	pt = FirstPoint(ps, num);


	while ( pt >= 0 ) {
		pt00 = -1;
		pt0 = pt;
		connected = 1;
#ifdef	CON_WORD
		dist = -1.0;
#endif
		while ( pt >= 0 ) {	// 行のループ
			visited[pt] = 1;
#ifdef DRAW_PIC
			cvCircle( out_img, ps[pt], 2, cWhite, -1, 1, 0 );
//			if ( pt00 >= 0 )	cvLine( out_img, ps[pt00], ps[pt], cWhite, 1, CV_AA, 0);
			if ( connected > 1 )	cvLine( out_img, ps[pt00], ps[pt], cWhite, 4, CV_AA, 0);
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
// p1とp2が1NNで接続されているか調べる
{
	if ( nears[p1][0] == p2 || nears[p2][0] == p1 )
		return 1;
	else
		return 0;
}

int IsConnectedWord(CvPoint p1, CvPoint p2, double *dist)
// p1とp2が接続されているか，単語の区切りで調べる
{
	double d0 = *dist;

	*dist = GetPointsDistance(p1, p2);
	if ( *dist < d0*kWordSep || d0 < 0 )
		return 1;
	else
		return 0;
}

//double GetPointsDistance(CvPoint p1, CvPoint p2)
// 点のユークリッド距離を求める
//{
//		return sqrt((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y));
//}

int FirstPoint(CvPoint *ps, int num)
// 最初の点（左上の点）を得る
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

int NextLine(int pt, CvPoint *ps, int num, int *nears[], int visited[])
// ptから次の行の最初の点を見つける（ptの下になければ右に進んでいって調べる．次の行に言ったら左に戻れるだけ戻る）
{
	int next, pt0;

	pt0 = pt;
	while ( (next = DownPoint(pt, ps, num, nears[pt])) < 0 ) {
		if ( (pt = RightPoint(pt, ps, num, nears[pt])) < 0 )	// 右の点に進む
			return -1;	// なければ次の行はない
	}
	while ( (pt = LeftPoint(next, ps, num, nears[next], visited)) >= 0 ) {
		// if ( visited[pt] )	break;	//すでに行ったことがある場合（無限ループ防止）
		next = pt;
	}
//	if ( pt0 == next ) return -1;
	return next;
}

int RightPoint(int pt, CvPoint *ps, int num, int nears[])
// ptの右の点を見つける
{
	int i;

	for ( i = 0; i < kNears; i++ ) {
		if ( IsOnRight(&ps[pt], &ps[nears[i]]) )
			return nears[i];
	}
	return -1;
}

int LeftPoint(int pt, CvPoint *ps, int num, int nears[], int visited[])
// ptの左の点を見つける
{
	int i;

	for ( i = 0; i < kNears; i++ ) {
		if ( IsOnLeft(&ps[pt], &ps[nears[i]]) && !visited[nears[i]])
			return nears[i];
	}
	return -1;
}

int DownPoint(int pt, CvPoint *ps, int num, int nears[])
// ptの下の点を見つける
{
	int i;

	for ( i = 0; i < kNears; i++ ) {
		if ( IsOnDown(&ps[pt], &ps[nears[i]]) )
			return nears[i];
	}
	return -1;
}

int IsOnRight( CvPoint *p1, CvPoint *p2 )
// p1の右にp2があるか調べる
{
	double deg = atan2(p2->y - p1->y, p2->x - p1->x);
	if ( fabs(deg) < kRightRange )
		return 1;
	else
		return 0;
}

int IsOnLeft( CvPoint *p1, CvPoint *p2 )
// p1の左にp2があるか調べる
{
	double deg = atan2(p2->y - p1->y, p2->x - p1->x);
	if ( fabs(fabs(deg) - M_PI) < kLeftRange )
		return 1;
	else
		return 0;
}

int IsOnDown( CvPoint *p1, CvPoint *p2 )
// p1の下にp2があるか調べる（座標系が通常と違うことに注意！）
{
	double deg = atan2(p2->y - p1->y, p2->x - p1->x);
	if ( fabs(deg - M_PI/2) < kDownRange )
		return 1;
	else
		return 0;
}

void JInvariantTest(void)
// j-invariantのテスト
{
	int i, j, k, l;
	CvPoint p[4];

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

int MakeCentresFromImage(CvPoint **ps, IplImage *img, CvSize *size, double **areas)
// 画像のファイル名を与えて重心を計算し，重心の数を返す
// 06/01/12	面積の計算を追加
{
	int num;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0;

	size->width = img->width;
	size->height = img->height;

    cvFindContours( img, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する

	num = CalcCentres(ps, contours, size, areas);	// 各連結成分の重心を計算する

	if ( contours != NULL )	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	return num;
}

int CalcCentres(CvPoint **ps0, CvSeq *contours, CvSize *size, double **areas0)
// 重心を計算する
// 06/01/12	面積の計算を追加
{
	int i, num;
	double *areas = NULL;
	CvSeq *con0;
	CvMoments mom;
	CvPoint *ps;
	CvSize s;
	IplImage *img;

	s.width = size->width;
	s.height = size->height;
	img = cvCreateImage( s, 8, 3);

	// 点の数をカウント（暫定）
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
//	num = i;
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// ゴミ等で連結成分が多すぎるとき（本来は大きさを調べたりすべき）
	// 点を入れる配列を確保（numの値は正確ではないが，大きめということで）
	ps = (CvPoint *)calloc(num, sizeof(CvPoint));
	*ps0 = ps;
	// 面積を入れる配列を確保
	if ( areas0 ) {
		areas = (double *)calloc(num, sizeof(double));
		*areas0 = areas;
	}
	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 && i < num ; con0 = con0->h_next )
    {
		double d00;
#ifdef DRAW_PIC
		CvScalar color;
#endif

		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 0.00001 ) continue;	// 小さすぎる連結成分は除外

		ps[i].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		ps[i].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
		if ( areas ) {
			areas[i] = d00;
//			printf("%lf\n", areas[i]);
		}
//        cvDrawContours(img,con0,cWhite,cWhite,-1,1,8);	// replace CV_FILLED with 1 to see the outlines 
		cvCircle( img, ps[i], 5, cWhite, -1, 1, 0 );

#ifdef DRAW_PIC
		color = cRandom;
        cvDrawContours(dst,con0,color,color,-1,/*1*/CV_FILLED,8);	// replace CV_FILLED with 1 to see the outlines 
        cvDrawContours(out_img,con0,cWhite,cWhite,-1,1,8);	// replace CV_FILLED with 1 to see the outlines 
//        cvDrawContours(centre_img,con0,cWhite,cWhite,-1,1,8);	// replace CV_FILLED with 1 to see the outlines 
		cvCircle( centre_img, ps[i], 0/*5*/, cWhite, -1, 1, 0 );
#endif
		i++;
    }

	num = i;	// 連結成分数の更新

	cvNot( img, img );
//	OutPutImage( img );
	cvReleaseImage( &img );
#ifdef SHOW_PIC
	cvNamedWindow( "Components", 1 );
    cvShowImage( "Components", dst );
#endif

	return num;
}

void ReleaseCentres( CvPoint *ps )
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

int OutPutImage(IplImage *img)
// 画像をファイルに保存する．ファイル名は自動的に連番になる
{
	static int n = 0;
	char filename[kFileNameLen];

	sprintf(filename, "output%03d.jpg", n++);
	return cvSaveImage(filename, img);
}

void OutPutResult( int *s, int t, int disp, char *qname0 )
// 結果の表示
{
	int i, j, k, n, last_slash, last_period, doc_num;
	char line[kMaxPathLen], name0[kMaxPathLen], name[kMaxPathLen], qname[kMaxPathLen], **fname;
	FILE *fp;
	strScore *score;

	// クエリのベースネームの取得
	for ( j = 0, last_slash = 0, last_period = strlen(qname0); qname0[j] != '\0'; j++ ) {
		if ( qname0[j] == '/' )	last_slash = j;	// 最後のスラッシュの位置
		else if ( qname0[j] == '.' )	last_period = j;	// （最後の）ピリオドの位置
	}
	for ( j = last_slash + 1, k = 0; j < last_period; j++, k++ )	qname[k] = qname0[j];
	qname[k] = '\0';
//	puts(qname);
	// 画像ファイル名の読み込み
	fname = (char **)calloc(kMaxDocNum, sizeof(char *));
	fp = fopen(eDBCorFileName, "r");
	for ( i = 0; i < kMaxDocNum && fgets(line, kMaxPathLen, fp) != NULL; i++ ) {
		sscanf(line, "%d %s", &n, name0);	// フルパスの読み込み
		for ( j = 0, last_slash = 0, last_period = strlen(name0); name0[j] != '\0'; j++ ) {
			if ( name0[j] == '/' )	last_slash = j;	// 最後のスラッシュの位置
			else if ( name0[j] == '.' )	last_period = j;	// （最後の）ピリオドの位置
		}
		// ベースネームの取得
		for ( j = last_slash + 1, k = 0; j < last_period; j++, k++ )	name[k] = name0[j];
		name[k] = '\0';
		fname[n] = (char *)malloc( sizeof(char) * (strlen(name)+1) );
		strcpy( fname[n], name );
	}
	doc_num = i;
	fclose(fp);
	// スコアに入れる
	score = (strScore *)malloc(sizeof(strScore) * doc_num);
	SetAndSortScore( score, s, doc_num );
	// disp位まで表示
	for ( i = 0; i < disp; i++ ) {
		printf("%s\t%d\n", fname[score[i].n], score[i].vote);
	}
	// 正解が何位か
	for ( i = 0; i < doc_num; i++ ) {
		if ( !strncmp(qname, fname[score[i].n], strlen(fname[score[i].n])) )	break;
	}
	if ( i == 0 )	printf("Succeeded\n");
	else	printf("Failed\n");
	printf("Rank : %d\n", i+1);
	printf("Ratio : %f\n", (double)score[0].vote / (double)score[1].vote);
	// メモリの開放
	for ( i = 0; i < kMaxDocNum; i++ ) {
		if ( fname[i] != NULL )	free(fname[i]);
	}
	free(fname);
	free(score);
}

void OutPutResultSv( int *s, int t, int disp, char *result )
// 結果の表示
{
	int i, j, k, n, last_slash, last_period, doc_num;
	char line[kMaxPathLen], name0[kMaxPathLen], name[kMaxPathLen], qname[kMaxPathLen], **fname;
	FILE *fp;
	strScore *score;

	// 画像ファイル名の読み込み
	fname = (char **)calloc(kMaxDocNum, sizeof(char *));
	fp = fopen(eDBCorFileName, "r");
	for ( i = 0; i < kMaxDocNum && fgets(line, kMaxPathLen, fp) != NULL; i++ ) {
		sscanf(line, "%d %s", &n, name0);	// フルパスの読み込み
		for ( j = 0, last_slash = 0, last_period = strlen(name0); name0[j] != '\0'; j++ ) {
			if ( name0[j] == '/' )	last_slash = j;	// 最後のスラッシュの位置
			else if ( name0[j] == '.' )	last_period = j;	// （最後の）ピリオドの位置
		}
		// ベースネームの取得
		for ( j = last_slash + 1, k = 0; j < last_period; j++, k++ )	name[k] = name0[j];
		name[k] = '\0';
		fname[n] = (char *)malloc( sizeof(char) * (strlen(name)+1) );
		strcpy( fname[n], name );
	}
	doc_num = i;
	fclose(fp);
	// スコアに入れる
	score = (strScore *)malloc(sizeof(strScore) * doc_num);
	SetAndSortScore( score, s, doc_num );
	// disp位まで表示
	for ( i = 0; i < disp; i++ ) {
		printf("%s\t%d\n", fname[score[i].n], score[i].vote);
	}
	// 1位が閾値を超えたらresultにコピー
	if ( score[0].vote > kTopThr )	strcpy( result, fname[score[0].n] );
	else	result[0] = '\0';
	// メモリの開放
	for ( i = 0; i < kMaxDocNum; i++ ) {
		if ( fname[i] != NULL )	free(fname[i]);
	}
	free(fname);
	free(score);
}

#define	kScreenThr	(530)

int ScreenPointsWithNears( CvPoint *ps, int num, int **nears )
// 近傍点を用いて特徴点をふるいにかける
{
	int i, cnt;
	CvPoint *ps0;

	// psをps0にコピー
	ps0 = (CvPoint *)malloc( sizeof(CvPoint) * num );
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
// psで情報を表示
{
//	system("ps u | grep dirs | awk \'BEGIN { FS = \" \" }; { print $3\" \"$4\" \"$5\" \"$6}\'");
	system("ps opcpu,vsize,size,rss");
}

void CreatePointFile( char *in_fname, char *out_fname, int mode )
// 画像ファイルif_fnameからmodeで特徴点を抽出し、特徴点ファイルとしてout_fnameに出力する。
{
	IplImage *img;
	int num;
	double *areas;
	CvPoint *ps;
	CvSize img_size;
	
	if ( (img = GetConnectedImage( in_fname, mode )) == NULL ) {	// 結合画像を作成
		fprintf(stderr, "%sから結合画像を作成するのに失敗しました\n", in_fname);
		return;
	}
	num = MakeCentresFromImage( &ps, img, &img_size, &areas );	// 画像から重心を計算する
	SavePointFile( out_fname, ps, num, &img_size, areas );
	
	return;
}

int RetrieveUSBCamServer( void )
// USBカメラサーバモード
{
	int num, *numall = NULL, ret, res, *score = NULL, **nears = NULL;
	int pcor[kMaxPointNum][2], pcornum = 0;
	char fname[kMaxPathLen], doc_name[kMaxPathLen], **dbcors = NULL;
	CvPoint *ps = NULL, **psall = NULL;
	double *areas = NULL, **areaall = NULL;
	CvSize img_size, *sizeall = NULL, res_size;
	strDisc disc;
	strProjParam param, zero_param;
	SOCKET sid1, sid2, sidpt, sidres;
	struct sockaddr_in addr;
	// DLL用改変
	strHList **hash = NULL;
	strHList2 **hash2 = NULL;
	HENTRY *hash3 = NULL;

//	ret = LoadConfig();	// 設定ファイルの読み込み
	ret = LoadConfig( eConfigFileName, &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &eProp, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, eDiscFileName );
	ret = LoadDisc( fname, &disc );	// 離散化ファイルの読み込み
	if ( ret <= 0 )	return 0;
	// ハッシュの読み込み
	if ( eNoHashList )			hash3 = LoadHash3( disc.num, eHashDatFileName );
	else if ( eCompressHash )	hash2 = LoadHash2( disc.num, eHashDatFileName );
	else						hash = LoadHash( disc.num, eHashDatFileName );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, ePointDatFileName );
	ret = LoadPointFile2( fname, &psall, &areaall, &sizeall, &numall, &dbcors );	// 特徴点データの読み込み
	if ( ret <= 0 )	return 0;
	// 実験モードならハッシュのサイズを出力
	if ( eExperimentMode )	printf( "Allocated Hash Size : %d\n", allocated_hash );
	// 組み合わせの作成
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// ネットワーク関係
//start_nego2:
	printf("Ready\n");
#ifdef WIN32
	if ( ( sid1 = InitWinSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socketの初期化
	if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#else
	if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 0;	// socketの初期化
	if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	return 0;	// accept
#endif
	ret = RecvComSetting( sid2, &eProtocol, &ePointPort, &eResultPort, eClientName );	// 通信設定を受信
	if ( ret < 0 )	return 0;
//	send = SendFileNameList( sid2, 

//	fprintf(stderr, "%d, %d, %d, %s\n", eProtocol, ePointPort, eResultPort, eClientName );

	if ( eProtocol == kUDP ) {	// UDPの場合．TCPの場合は接続済み
#ifdef	WIN32
		Sleep( 1000 );
		if ( ( sidpt = InitWinSockSvUDP( ePointPort ) ) < 0 )	return 1;
		if ( ( sidres = InitWinSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#else
		sleep(1);	// UDPの場合はクライアントを先に待ち状態にしなければならないので
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

#ifdef	SHOW_SCORE_GRAPH	// スコアのグラフを表示
#ifdef	WIN32	// Linuxでのウィンドウ表示のやり方がわからないので
	cvNamedWindow( "Score", CV_WINDOW_AUTOSIZE );
#endif
#endif

	for ( ; ; ) {
		if ( eProtocol == kTCP ) num = RecvPointsAreas( sid2, &ps, &areas, &img_size );	// 特徴点データを受信
		else					num = RecvPointsAreas( sidpt, &ps, &areas, &img_size );	// 特徴点データを受信
		num = ( num >= kMaxPointNum ) ? kMaxPointNum - 1 : num;	// 特徴点数が最大値を超える場合は制限
//		printf("%d points received.\n", num);
		if ( num < 0 )	break;	// 通信失敗
		if ( num >= kMinPoints ) {	// 特徴点数が最小値以上
			MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num, &nears );	// 近傍点計算（分割版）
			// 検索
			res = RetrieveNN5Cor2( ps, areas, num, nears, img_size, score, pcor, &pcornum, &disc, eProp, numall, hash, hash2, hash3 );	// 検索のみ
#ifdef	SHOW_SCORE_GRAPH	// スコアのグラフを表示
#ifdef	WIN32	// Linuxでのウィンドウ表示のやり方がわからないので
			ShowScoreGraph( "Score", score, eDbDocs );
#endif
#endif
			// 最小対応点数以上なら、射影変換パラメータを計算
			if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, psall[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// 登録画像に上書き
			else	param = zero_param;
			if ( ps != NULL && num > 0 )	ReleaseCentres( ps );	// psの解放
			if ( areas != NULL && num > 0 )	free( areas );
			ReleaseNears( nears, num );	// nearsの解放
			// 計算時間の表示
//			OutPutResultSv( score, end-start, 0/*5*/, doc_name );
			if ( score[res] > kTopThr ) {	// トップの得票数が閾値以下（リジェクト）
				GetBasename2( dbcors[res], kMaxPathLen, doc_name );
			}
			else {
				doc_name[0] = '\0';
				param = zero_param;	// 検索結果がないのにパラメータだけあるってのも変なので
			}
			puts( doc_name );
		}
		else {	// 特徴点数が最小値を満たさない
//			puts("min");
			doc_name[0] = '\0';
			param = zero_param;
		}
		if ( doc_name[0] != '\0' )	res_size = sizeall[res];	// 検索成功ならそのサイズ
		else	res_size = sizeall[0];	// そうでなければデータベースの0番のサイズ

//		fprintf(stdout, "%s\n", doc_name);
		if ( eProtocol == kTCP )	ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		else						ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		if ( ret < 0 )	break;
	}
	// 終了処理
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
		
//		goto start_nego2;	// ネゴ待ちまで戻る
	return 1;
}

int RetrieveUSBCamServerDualHash( void )
// USBカメラサーバモード（デュアルハッシュ）
{
	int num, num1, num2, *numall = NULL, *numall1 = NULL, *numall2 = NULL, ret, res, *score = NULL, **nears = NULL, **nears1 = NULL, **nears2 = NULL;
	int res1, res2, *score1 = NULL, *score2 = NULL;
	int **pcor, pcornum = 0, pcor1[kMaxPointNum][2], pcornum1, pcor2[kMaxPointNum][2], pcornum2;
	char fname[kMaxPathLen], doc_name[kMaxPathLen], **dbcors = NULL, **dbcors1 = NULL, **dbcors2 = NULL;
	CvPoint *ps = NULL, **psall = NULL, *ps1 = NULL, *ps2 = NULL, **psall1 = NULL, **psall2 = NULL;
	double *areas = NULL, **areaall = NULL, *areas1 = NULL, *areas2 = NULL, **areaall1 = NULL, **areaall2 = NULL;
	CvSize img_size, *sizeall = NULL, res_size, *sizeall1 = NULL, *sizeall2 = NULL;
	strDisc disc, disc1, disc2;
	strProjParam param, zero_param;
	SOCKET sid1, sid2, sidpt, sidres;
	struct sockaddr_in addr;
	// DLL用改変
	strHList **hash = NULL, **hash_1 = NULL, **hash_2 = NULL;
	strHList2 **hash2 = NULL, **hash2_1 = NULL, **hash2_2 = NULL;
	HENTRY *hash3 = NULL, *hash3_1 = NULL, *hash3_2 = NULL;
	double prop1, prop2;

//	ret = LoadConfig();	// 設定ファイルの読み込み
	ret = LoadConfig( "config1.dat", &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &prop1, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
	if ( ret <= 0 )	return 0;
	ret = LoadConfig( "config2.dat", &eInvType, &eGroup1Num, &eGroup2Num, &eGroup3Num, &eNumCom1, &eNumCom2, &eDiscNum, &prop2, &eCompressHash, &eDocBit, &ePointBit, &eRBit, &eOBit, &eHList2DatByte, &eRotateOnce, &eUseArea, &eIsJp, &eNoHashList );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "disc1.txt" );
	ret = LoadDisc( fname, &disc1 );	// 離散化ファイルの読み込み
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "disc2.txt" );
	ret = LoadDisc( fname, &disc2 );	// 離散化ファイルの読み込み
	if ( ret <= 0 )	return 0;
	// ハッシュの読み込み
	if ( eNoHashList )			hash3_1 = LoadHash3( disc1.num, "hash1.dat" );
	else if ( eCompressHash )	hash2_1 = LoadHash2( disc1.num, "hash1.dat" );
	else						hash_1 = LoadHash( disc1.num, "hash1.dat" );
	if ( ret <= 0 )	return 0;
	if ( eNoHashList )			hash3_2 = LoadHash3( disc2.num, "hash2.dat" );
	else if ( eCompressHash )	hash2_2 = LoadHash2( disc2.num, "hash2.dat" );
	else						hash_2 = LoadHash( disc2.num, "hash2.dat" );
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "point1.dat" );
	ret = LoadPointFile2( fname, &psall1, &areaall1, &sizeall1, &numall1, &dbcors1 );	// 特徴点データの読み込み
	if ( ret <= 0 )	return 0;
	sprintf( fname, "%s%s", eDirsDir, "point2.dat" );
	ret = LoadPointFile2( fname, &psall2, &areaall2, &sizeall2, &numall2, &dbcors2 );	// 特徴点データの読み込み
	if ( ret <= 0 )	return 0;
	// 実験モードならハッシュのサイズを出力
//	if ( eExperimentMode )	printf( "Allocated Hash Size : %d\n", allocated_hash );
	// 組み合わせの作成
	GenerateCombination( eInvType, eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// ネットワーク関係
//start_nego2:
	printf("Ready\n");
#ifdef WIN32
	if ( ( sid1 = InitWinSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 1;	// socketの初期化
	if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	return 1;	// accept
#else
	if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	return 0;	// socketの初期化
	if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	return 0;	// accept
#endif
	ret = RecvComSetting( sid2, &eProtocol, &ePointPort, &eResultPort, eClientName );	// 通信設定を受信
	if ( ret < 0 )	return 0;
//	send = SendFileNameList( sid2, 

//	fprintf(stderr, "%d, %d, %d, %s\n", eProtocol, ePointPort, eResultPort, eClientName );

	if ( eProtocol == kUDP ) {	// UDPの場合．TCPの場合は接続済み
#ifdef	WIN32
		Sleep( 1000 );
		if ( ( sidpt = InitWinSockSvUDP( ePointPort ) ) < 0 )	return 1;
		if ( ( sidres = InitWinSockClUDP( eResultPort, eClientName, &addr ) ) < 0 )	return 1;
#else
		sleep(1);	// UDPの場合はクライアントを先に待ち状態にしなければならないので
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

#ifdef	SHOW_SCORE_GRAPH	// スコアのグラフを表示
#ifdef	WIN32	// Linuxでのウィンドウ表示のやり方がわからないので
	cvNamedWindow( "Score", CV_WINDOW_AUTOSIZE );
#endif
#endif

	for ( ; ; ) {
		if ( eProtocol == kTCP ) {
			num1 = RecvPointsAreas( sid2, &ps1, &areas1, &img_size );	// 特徴点データを受信
			num2 = RecvPointsAreas( sid2, &ps2, &areas2, &img_size );	// 特徴点データを受信
		} else {
			num1 = RecvPointsAreas( sidpt, &ps1, &areas1, &img_size );	// 特徴点データを受信
			num2 = RecvPointsAreas( sidpt, &ps2, &areas2, &img_size );	// 特徴点データを受信
		}
		num1 = ( num1 >= kMaxPointNum ) ? kMaxPointNum - 1 : num1;	// 特徴点数が最大値を超える場合は制限
		num2 = ( num2 >= kMaxPointNum ) ? kMaxPointNum - 1 : num2;	// 特徴点数が最大値を超える場合は制限
//		printf("%d points received.\n", num);
		if ( num1 < 0 || num2 < 0 )	break;	// 通信失敗
		if ( num1 >= kMinPoints && num2 >= kMinPoints ) {	// 特徴点数が最小値以上
			MakeNearsFromCentresDiv( ps1, num1, &img_size, kDivX, kDivY, eGroup1Num, &nears1 );	// 近傍点計算（分割版）
			res1 = RetrieveNN5Cor2( ps1, areas1, num1, nears1, img_size, score1, pcor1, &pcornum1, &disc1, prop1, numall1, hash_1, hash2_1, hash3_1 );	// 検索のみ
			MakeNearsFromCentresDiv( ps2, num2, &img_size, kDivX, kDivY, eGroup1Num, &nears2 );	// 近傍点計算（分割版）
			res2 = RetrieveNN5Cor2( ps2, areas2, num2, nears2, img_size, score2, pcor2, &pcornum2, &disc2, prop2, numall2, hash_2, hash2_2, hash3_2 );	// 検索のみ
			printf("res: %d, %d\n", res1, res2);
			printf("doc: %s, %s\n", dbcors1[res1], dbcors2[res2]);
#ifdef	SHOW_SCORE_GRAPH	// スコアのグラフを表示
#ifdef	WIN32	// Linuxでのウィンドウ表示のやり方がわからないので
			ShowScoreGraph( "Score", score, eDbDocs );
#endif
#endif
			// スコアを合算して検索結果を決定
			res = CalcTotalScore( eDbDocs, num1, num2, numall1, numall2, score1, score2 );
			printf("total_res: %d\n", res);
			// 結果の選択
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

			// 最小対応点数以上なら、射影変換パラメータを計算
			if ( pcornum >= kMinPointsToCalcParam ) {
				if ( score1[res1] > score2[res2] )
					 CalcProjParamTop( ps, psall[res], pcor1, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// 登録画像に上書き
				else
					 CalcProjParamTop( ps, psall[res], pcor2, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// 登録画像に上書き
			} else {
				param = zero_param;
			}
			if ( ps1 != NULL && num1 > 0 )	ReleaseCentres( ps1 );	// psの解放
			if ( areas1 != NULL && num1 > 0 )	free( areas1 );
			ReleaseNears( nears1, num1 );	// nearsの解放
			if ( ps2 != NULL && num2 > 0 )	ReleaseCentres( ps2 );	// psの解放
			if ( areas2 != NULL && num2 > 0 )	free( areas2 );
			ReleaseNears( nears2, num2 );	// nearsの解放
			// 計算時間の表示
//			OutPutResultSv( score, end-start, 0/*5*/, doc_name );
			if ( score[res] > kTopThr ) {	// トップの得票数が閾値以下（リジェクト）
				GetBasename2( dbcors[res], kMaxPathLen, doc_name );
				printf("%s, %s\n", dbcors[res], doc_name);
				res_size = sizeall[res];	// 検索成功ならそのサイズ
			}
			else {
				doc_name[0] = '\0';
				param = zero_param;	// 検索結果がないのにパラメータだけあるってのも変なので
				res_size = sizeall1[0];	// そうでなければデータベースの0番のサイズ
			}
			puts( doc_name );
		}
		else {	// 特徴点数が最小値を満たさない
//			puts("min");
			doc_name[0] = '\0';
			param = zero_param;
			res_size = sizeall1[0];	// データベースの0番のサイズ
		}

//		fprintf(stdout, "%s\n", doc_name);
		if ( eProtocol == kTCP )	ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		else						ret = SendResultParam( sidres, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		if ( ret < 0 )	break;
	}
	// 終了処理
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
		
//		goto start_nego2;	// ネゴ待ちまで戻る
	return 1;
}

int IsSucceed( char *str1, char *str2 )
// 実験モード用：成功判定
{
	char base1[kMaxPathLen], base2[kMaxPathLen];

	GetBasename( str1, kMaxPathLen, base1 );
	GetBasename( str2, kMaxPathLen, base2 );

//	printf( "%s, %s\n", base1, base2 );
	return !strcmp( base1, base2 );
}

double Calc12Diff( int *score )
// 1位と2位の差を計算
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

IplImage *MakeFeaturePointImage( IplImage *orig, IplImage *connected, CvPoint *ps, int num )
// 特徴点画像の作成（入力画像はグレイスケール）
{
	int i, j, p, con_flag = 0;
	IplImage *fp_img;
	unsigned char val[4];

	cvNot( connected, connected );
	fp_img = cvCreateImage( cvGetSize( orig ), IPL_DEPTH_8U, 3 );
	cvZero( fp_img );
	cvNot( fp_img, fp_img );
	if ( fp_img == NULL )	return NULL;
	for ( j = 0; j < orig->height; j++ ) {
		for ( i = 0; i < orig->width; i++ ) {
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
		cvCircle( fp_img, ps[p], 4, cRed, CV_FILLED, CV_AA, 0 );
	}

	return fp_img;
}

int compare_int( const int *a, const int *b )
// qsort()用の比較関数（降順）
{
	return *b - *a;
}

void ShowScoreGraph( char *win_name, int *score, int doc_num )
// スコアのグラフを表示
{
	static IplImage *img = NULL;
	int i, key, score_sorted[kMaxDocNum], max_score, bar_len;

	// 画像の作成
	if ( img == NULL )	img = cvCreateImage( cvSize( 100, 200 ), IPL_DEPTH_8U, 3 );

	// スコアのコピーとソート
	memcpy( score_sorted, score, sizeof(int) * doc_num );
	qsort( score_sorted, doc_num, sizeof(int), (int (*)(const void*, const void*))compare_int );

	// 表示
	cvZero( img );
	max_score = score_sorted[0];
	for ( i = 0; i < 200 && i < doc_num; i++ ) {
		bar_len = (int)((double)score_sorted[i] / (double)max_score * (double)100);
		cvLine( img, cvPoint( 0, i ), cvPoint( bar_len, i ), CV_RGB(255,0,0), 1, 8, 0 );
	}

	// 画像のウィンドウ表示
	cvShowImage( win_name, img );
	key = cvWaitKey( 1 );	// ないとウィンドウが更新されない
}

int compare_int_asc( const int *a, const int *b )
// qsort()用の比較関数（昇順）
{
	return *a - *b;
}

int CalcTotalScore( int doc_num, int num1, int num2, int *numall1, int *numall2, int *score1, int *score2 )
// デュアルハッシュモードで得られた二つの結果を合算し，最終的なスコアを計算して検索結果を決める
{
	int i, res, med1, med2, doc_max = -1;
	int *score_copy1, *score_copy2;
	double score_cur, score_max = DBL_MIN;

	score_copy1 = (int *)calloc( doc_num, sizeof(int) );
	score_copy2 = (int *)calloc( doc_num, sizeof(int) );
	// スコアのコピーとソート
	memcpy( score_copy1, score1, sizeof(int) * doc_num );
	memcpy( score_copy2, score2, sizeof(int) * doc_num );
	qsort( score_copy1, doc_num, sizeof(int), (int (*)(const void*, const void*))compare_int_asc );
	qsort( score_copy2, doc_num, sizeof(int), (int (*)(const void*, const void*))compare_int_asc );
	// 中央値の取得
	med1 = score_copy1[(int)(doc_num / 2)];
	med2 = score_copy2[(int)(doc_num / 2)];
	// 最終的なスコアをscore1-med1+score2-med2とする
	// score1/med1+score2/med2
	// 重み付け
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
