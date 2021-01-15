#define	GLOBAL_DEFINE	/* extern変数にここで実体を与える */

#include "def_general.h"	// proctime用
#include <stdio.h>
#include <winsock2.h>	// socket
#include <cv.h>			// OpneCV
#include <highgui.h>	// OpenCV
#include <windows.h>
#include <mmsystem.h>
#include <dshow.h>
#include <conio.h>
#include <qedit.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS		//for use of string.h wihout error

#include "extern.h"
#include "ws_cl.h"
#include "dirs.h"
#include "proctime.h"
#include "dscap.h"
#include "auto_connect.h"
#include "proj4p.h"
#include "dcamc.h"
#include "nimg.h"
#include "ar.h"
#include "init.h"
#include "fptune.h"
#include "camharris.h"

// マウスのコールバック関数用のグローバル変数
IplImage *g_thumb = NULL, *g_thumb_draw;
double g_ratio = 0.0;
int g_paste_complete = 0;
CvPoint g_p1, g_p2;

int LoadPointFile( char *fname, CvPoint **ps0, CvSize *size );

int SendPoints( SOCKET sock, CvPoint *ps, int num, CvSize *size, struct sockaddr_in *addr, int ptc )
// 点データを送信
{
	int i, num_send, buff_cur;
	char buff[kSendBuffSize];

	// 画像サイズをバッファに格納
	int j=0;
	for ( i = 0, buff_cur = 0; i < sizeof(CvSize); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)size)[i];
	}
	// 特徴点数をバッファに格納
	num_send = (num > kMaxPointNumServer) ? kMaxPointNumServer : num;	// 特徴点数がサーバ側の最大値を超える場合は制限
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)&num_send)[i];
	}
	// 特徴点データをバッファに格納
	for ( i = 0; i < (int)sizeof(CvPoint) * num_send; i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)ps)[i];
	}
	// バッファをまとめて送信（一回の通信で済む）
	if ( ptc == kTCP ) {
		send( sock, buff, kSendBuffSize, SEND_FLAG );
	}
	else {
		sendto( sock, buff, kSendBuffSize, SEND_FLAG, (struct sockaddr *)addr, sizeof(*addr) );
	}

	return 1;
}

int SendPointsAreas( SOCKET sock, CvPoint *ps, double *areas, int num, CvSize *size, struct sockaddr_in *addr, int ptc )
// 点データを送信
// 面積も送信
{
	int i, num_send, buff_cur;
	char buff[kSendBuffSizeAreas];
	unsigned short areas_us[kMaxPointNumServer];

	// 画像サイズをバッファに格納
	int j=0;
	for ( i = 0, buff_cur = 0; i < sizeof(CvSize); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)size)[i];
	}
	// 特徴点数をバッファに格納
	num_send = (num > kMaxPointNumServer) ? kMaxPointNumServer : num;	// 特徴点数がサーバ側の最大値を超える場合は制限
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)&num_send)[i];
	}
	// 特徴点データをバッファに格納
	for ( i = 0; i < (int)sizeof(CvPoint) * num_send; i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)ps)[i];
	}
	// 面積をunsigned shortに直して格納
	for ( i = 0; i < num_send; i++ ) {
		areas_us[i] = (unsigned short)areas[i];
		// printf("%lf\n", areas[i]);
	}
	buff_cur = sizeof(CvSize) + sizeof(int) + sizeof(CvPoint) * kMaxPointNumServer;
	for ( i = 0; i < (int)sizeof(unsigned short) * num_send; i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)areas_us)[i];
	}
	// バッファをまとめて送信（一回の通信で済む）
	if ( ptc == kTCP ) {
		send( sock, buff, kSendBuffSizeAreas, SEND_FLAG );
	}
	else {
		sendto( sock, buff, kSendBuffSizeAreas, SEND_FLAG, (struct sockaddr *)addr, sizeof(*addr) );
	}

	return 1;
}

int SendImage( SOCKET sock, IplImage *img )
// 画像を送信
{
	int ret;
	CvSize size;
	
	size.width = img->width;
	size.height = img->height;
	send( sock, (const char *)&size, sizeof(CvSize), SEND_FLAG );	// サイズを送信
	send( sock, (const char *)&(img->nChannels), sizeof(int), SEND_FLAG );	// チャネルを送信
	send( sock, (const char *)&(img->depth), sizeof(int), SEND_FLAG );	// ビット数を送信
	ret = send( sock, (const char *)(img->imageData), img->imageSize, SEND_FLAG );	// 画像本体を送信
	
	return 1;
}

int SendComSetting( SOCKET sock, int ptc, int pt_port, int res_port, char *cl_name )
// 通信設定を送信
{
	send( sock, (const char *)&ptc, sizeof(int), SEND_FLAG );
	send( sock, (const char *)&pt_port, sizeof(int), SEND_FLAG );
	send( sock, (const char *)&res_port, sizeof(int), SEND_FLAG );
	send( sock, (const char *)cl_name, kMaxNameLen, SEND_FLAG );

	return 1;
}

int RecvResult( SOCKET sock, char *doc_name, int len )
// 文書名を受信
{
	return recv( sock, doc_name, len, RECV_FLAG );
}

int RecvFileNameList( SOCKET sock, int *doc_num, char ***fname_list0, int len )
{
	int i, ret;
	char **fname_list = NULL;

	ret = recv( sock, (char *)doc_num, sizeof(int), RECV_FLAG );
	fname_list = (char **)calloc( *doc_num, sizeof(char *) );
	for ( i = 0; i < *doc_num; i++ ) {
		fname_list[i] = (char *)calloc( len, sizeof(char) );
		ret = recv( sock, fname_list[i], len, RECV_FLAG );
	}
	*fname_list0 = fname_list;

	return ret;
}

int RecvResultCor( SOCKET sock, char *doc_name, int len, CvPoint corps[][2], int *corpsnum, CvSize *query_size, CvSize *reg_size )
// 文書名と対応点を受信
{
	int i, ret;
	
	ret = recv( sock, doc_name, len, RECV_FLAG );	// 文書名を受信
	puts(doc_name);
	ret = recv( sock, (char *)query_size, sizeof(CvSize), RECV_FLAG );	// クエリ画像のサイズを受信
//	printf("%d,%d\n", query_size->width, query_size->height);
	ret = recv( sock, (char *)reg_size, sizeof(CvSize), RECV_FLAG );	// 登録画像のサイズを受信
	ret = recv( sock, (char *)corpsnum, sizeof(int), RECV_FLAG );	// 対応の数を受信
//	printf("%d\n", *corpsnum );
	if ( *corpsnum > 0 )	ret = recv( sock, (char *)corps, sizeof(CvPoint)*2*(*corpsnum), SEND_FLAG );
	for ( i = 0; i < *corpsnum; i++ ) {
//		ret = recv( sock, (char *)&(corps[i][0]), sizeof(CvPoint), RECV_FLAG );
//		ret = recv( sock, (char *)&(corps[i][1]), sizeof(CvPoint), RECV_FLAG );
//		printf("%d : (%d, %d), (%d, %d)\n", i, corps[i][0].x, corps[i][0].y, corps[i][1].x, corps[i][1].y);
	}
	return ret;
}

int RecvResultParam( SOCKET sock, char *doc_name, int len, strProjParam *param, CvSize *img_size )
// 文書名と射影変換パラメータを受信
{
	int ret;
	char buff[kRecvBuffSize];

	ret = recv( sock, buff, kRecvBuffSize, RECV_FLAG );
	memcpy( doc_name, buff, kMaxDocNameLen );
	memcpy( param, buff + kMaxDocNameLen, sizeof(strProjParam) );
	memcpy( img_size, buff + kMaxDocNameLen + sizeof(strProjParam), sizeof(CvSize) );

	return ret;

#if 0

	ret = recv( sock, doc_name, len, RECV_FLAG );
	ret = recv( sock, (char *)param, sizeof(strProjParam), RECV_FLAG );
	
	return ret;
#endif
}

void Buff2ImageData( unsigned char *buff, IplImage *img )
{
	int i;

	for ( i = 0; i < img->height; i++ ) {
		memcpy( &(img->imageData[img->widthStep*(img->height - i - 1)]), &(buff[img->width*3*i]), img->width*3 );
	}
}

void ReleaseCentres( CvPoint *ps )
{
	free( ps );
}

#define	CENTROID
//#define	SECOND_ORDER_MOMENT
//#define	kMaxNoiseArea	(10)
#define	kMaxNoiseArea	(20)	// 裏写り対策

int CalcCentres(CvPoint **ps0, CvSeq *contours, CvSize *size, double **areas0)
// 重心を計算する
// 06/01/12	面積の計算を追加
{
	int i, num;
	double *areas = NULL;
//	double max_area = 0.0L;	// 連結成分の面積の最大値を調べる用
	CvSeq *con0;
	CvMoments mom;
	CvPoint *ps;

	// 点の数をカウント（暫定）
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// ゴミ等で連結成分が多すぎるとき（本来は大きさを調べたりすべき）
	// 点を入れる配列を確保（numの値は正確ではないが，大きめということで）
	ps = (CvPoint *)calloc(num, sizeof(CvPoint));
	*ps0 = ps;
	// 面積を入れる配列を確保
	if ( areas0 ) {	// 面積配列のポインタが与えられている
		areas = (double *)calloc(num, sizeof(double));
		*areas0 = areas;
	}
	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 && i < num ; con0 = con0->h_next )
    {
		double d00;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < kMaxNoiseArea ) continue;	// 小さすぎる連結成分は除外

#ifdef	CENTROID
		ps[i].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		ps[i].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
#endif
#ifdef	SECOND_ORDER_MOMENT
		// 07/05/30
		// なぜこっちにしていたのか分からない。直し忘れか？
		// どちらにせよあまり影響は与えていなかった（2次の項は1ピクセルにも満たない）。
//		printf( "%f + %f, %f + %f\n", cvGetSpatialMoment( &mom, 1, 0 ) / d00, cvGetNormalizedCentralMoment( &mom, 2, 0 ), cvGetSpatialMoment( &mom, 0, 1 ) / d00, cvGetNormalizedCentralMoment( &mom, 0, 2 ) );
		ps[i].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00 + cvGetNormalizedCentralMoment( &mom, 2, 0 ) );
		ps[i].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00 + cvGetNormalizedCentralMoment( &mom, 0, 2 ) );
#endif
		if ( areas ) areas[i] = d00;
//		if ( d00 > max_area ) max_area = d00;
		// 130万画素で単語の重心を得る場合にはunsigned shortの最大値（65535）を超えることはなさそうだ。
		// 但し、解像度が上がったり対象が変わったりした場合にはその限りではない。
		i++;
    }
	num = i;	// 連結成分数の更新
//	printf("max area: %lf\n", max_area );

	return num;
}

int CalcFloatCentres(CvPoint2D32f **pfs0, CvSeq *contours, CvSize *size, double **areas0)
// 重心を計算する（float版）
// 06/01/12	面積の計算を追加
{
	int i, num;
	CvSeq *con0;
	CvMoments mom;
	CvPoint2D32f *pfs;

	// 点の数をカウント（暫定）
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// ゴミ等で連結成分が多すぎるとき（本来は大きさを調べたりすべき）
	// 点を入れる配列を確保（numの値は正確ではないが，大きめということで）
	pfs = (CvPoint2D32f *)calloc(num, sizeof(CvPoint2D32f));
	*pfs0 = pfs;
	// 連結成分を描画・重心を計算
    for( i = 0, con0 = contours; con0 != 0 && i < num ; con0 = con0->h_next )
    {
		double d00;
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < kMaxNoiseArea ) continue;	// 小さすぎる連結成分は除外

#ifdef	CENTROID
		pfs[i].x = (float)(cvGetSpatialMoment( &mom, 1, 0 ) / d00);
		pfs[i].y = (float)(cvGetSpatialMoment( &mom, 0, 1 ) / d00);
#endif
#ifdef	SECOND_ORDER_MOMENT
		pfs[i].x = (float)(cvGetSpatialMoment( &mom, 1, 0 ) / d00 + cvGetNormalizedCentralMoment( &mom, 2, 0 ) );
		pfs[i].y = (float)(cvGetSpatialMoment( &mom, 0, 1 ) / d00 + cvGetNormalizedCentralMoment( &mom, 0, 2 ) );
#endif
		i++;
    }
	num = i;	// 連結成分数の更新

	return num;
}

int MakeCentresFromImage(CvPoint **ps, IplImage *img, CvSize *size, double **areas)
// 画像のファイル名を与えて重心を計算し，重心の数を返す
// 06/01/12	面積の計算を追加
{
	int num, ret;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0;

	size->width = img->width;
	size->height = img->height;
	
    ret = cvFindContours( img, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
	if ( ret <= 0 ) {	// 連結成分が見つからなかった
		cvReleaseMemStorage( &storage );
		return 0;
	}
	num = CalcCentres(ps, contours, size, areas);	// 各連結成分の重心を計算する

	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	return num;
}

int MakeFloatCentresFromImage(CvPoint2D32f **pfs, IplImage *img, CvSize *size, double **areas)
// 画像のファイル名を与えて重心を計算し，重心の数を返す
// 06/01/12	面積の計算を追加
{
	int num, ret;
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *contours = 0;

	size->width = img->width;
	size->height = img->height;
	
    ret = cvFindContours( img, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
	if ( ret <= 0 ) {	// 連結成分が見つからなかった
		cvReleaseMemStorage( &storage );
		return 0;
	}
	num = CalcFloatCentres(pfs, contours, size, areas);	// 各連結成分の重心を計算する

	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	return num;
}

#define	kDrawPointsHMargin	(100)
#define	kDrawPointsVMargin	(100)
#define	kDrawPointsRectThick	(4)
#define	kDrawPointsPtRad	(4)

void DrawPoints2( CvPoint *ps, int num, CvSize img_size )
// 特徴点を描画し，保存する
{
	int i;
	IplImage *img;

	img = cvCreateImage( cvSize( kDrawPointsHMargin*2+img_size.width, kDrawPointsVMargin*2+img_size.height ), IPL_DEPTH_8U, 3 );
	cvSet( img, cWhite, NULL );	// 白で塗りつぶす
	// 枠を描画
	cvRectangle( img, cvPoint( kDrawPointsHMargin, kDrawPointsVMargin ), \
		cvPoint( kDrawPointsHMargin + img_size.width, kDrawPointsVMargin + img_size.height ), \
		cBlack, kDrawPointsRectThick, CV_AA, 0 );
	// 特徴点を描画
	for ( i = 0; i < num; i++ ) {
		cvCircle( img, cvPoint( kDrawPointsHMargin + ps[i].x, kDrawPointsVMargin + ps[i].y ), kDrawPointsPtRad, cBlack, -1, CV_AA, 0 );
	}
	OutPutImage( img );
	cvReleaseImage( &img );
}

IplImage *DrawPoints( CvPoint *ps, int num, CvSize *img_size )
// 特徴点を描画する
{
	int i;
	IplImage *img_pt;
	
	img_pt = cvCreateImage( *img_size, IPL_DEPTH_8U, 1 );
	cvZero( img_pt );
	for ( i = 0; i < num; i++ ) {
		cvCircle( img_pt, ps[i], 3, cWhite, -1, CV_AA, 0 );
	}
	
	return img_pt;
}

IplImage *DrawPointsOverlap( CvPoint *ps, int num, CvSize *img_size, IplImage *img )
// 特徴点を描画する
{
	int i;
	IplImage *img_pt;
	
	img_pt = cvCloneImage( img );
	for ( i = 0; i < num; i++ ) {
		cvCircle( img_pt, ps[i], 3, cRed, -1, CV_AA, 0 );
	}
	
	return img_pt;
}

int OutPutImage(IplImage *img)
// 画像をファイルに保存する．ファイル名は自動的に連番になる
{
	static int n = 0;
	char filename[kFileNameLen];

	sprintf(filename, "output%03d.jpg", n++);
	return cvSaveImage(filename, img);
}

void DrawCor( CvPoint corps[][2], int corpsnum, IplImage *img_cap, char *doc_name )
// 対応点の描画
{
	int i;
	char thumb_fname[kMaxPathLen];
	IplImage *query_small, *reg_small, *cor_img;
	CvSize qs, rs, cor_size;
	CvPoint p1, p2;
	
	sprintf( thumb_fname, "\\\\leo\\nakai\\didb\\thumb\\%s.bmp", doc_name );
	printf("%s\n", thumb_fname );
	if ( doc_name[0] == '\0')	return;
	reg_small = cvLoadImage( thumb_fname, 1 );
	
	qs.width = (int)(img_cap->width / 2);
	qs.height = (int)(img_cap->height / 2);
	rs.width = reg_small->width;
	rs.height = reg_small->height;
	
	cor_size.width = qs.width + rs.width;
	cor_size.height = (qs.height > rs.height) ? qs.height : rs.height;
	
	cor_img = cvCreateImage( cor_size, IPL_DEPTH_8U, 3 );
	for ( i = 0; i < corpsnum; i++ ) {
		p1.x = (int)(corps[i][0].x / 2);
		p1.y = (int)(corps[i][0].y / 2);
		p2.x = (int)(corps[i][1].x / 4) + qs.width;
		p2.y = (int)(corps[i][1].y / 4);
		cvLine( cor_img, p1, p2, cWhite, 1, CV_AA, 0 );
	}
	cvShowImage( "Corres", cor_img );
	cvReleaseImage( &reg_small );
	cvReleaseImage( &cor_img );
}

void DrawSash( IplImage *img, strProjParam *param )
// 枠を描画
{
	int i;
	strPoint p1[5], p2[5];


	p1[0].x = 0;
	p1[0].y = 0;
	p1[1].x = 1700;
	p1[1].y = 0;
	p1[2].x = 1700;
	p1[2].y = 2200;
	p1[3].x = 0;
	p1[3].y = 2200;
	p1[4].x = 0;
	p1[4].y = 0;
/*	
	p1[0].x = 1000;
	p1[0].y = 1000;
	p1[1].x = 1200;
	p1[1].y = 1000;
	p1[2].x = 1200;
	p1[2].y = 1200;
	p1[3].x = 1000;
	p1[3].y = 1200;
	p1[4].x = 1000;
	p1[4].y = 1000;
*/
	for ( i = 0; i < 5; i++ )	ProjTrans( &(p1[i]), &(p2[i]), param );
	for ( i = 0; i < 4; i++ ) {
//		printf("(%d, %d)\n", p2[i].x, p2[i].y);
		cvLine( img, cvPoint( p2[i].x, p2[i].y ), cvPoint( p2[i+1].x, p2[i+1].y ), cRed, 5, CV_AA, 0 );
	}
}

int IsEqualParam( strProjParam param1, strProjParam param2 )
{
	if ( param1.a1 == param2.a1 &&
	param1.a1 == param2.a1 &&
	param1.a2 == param2.a2 &&
	param1.a3 == param2.a3 &&
	param1.b1 == param2.b1 &&
	param1.b2 == param2.b2 &&
	param1.b3 == param2.b3 &&
	param1.c1 == param2.c1 &&
	param1.c2 == param2.c2 )	return 1;
	else	return 0;
}
	
int main( int argc, char *argv[] )
{
	SOCKET sock, sockpt, sockres;
	int i, num = 0, num0 = 0, key, ret, corpsnum, ar, draw_fp = -1, draw_fp_overlap = -1, draw_con = -1, draw_con_overlap = -1;
	unsigned char *img_buff;
	char doc_name[kMaxDocNameLen] = "", doc_name_prev[kMaxDocNameLen] = "invalid";
	long buff_size;
	TIME_COUNT start, end, start_cap, end_cap, start_show, end_show, start_connect, end_connect, start_conv, end_conv, start_ret, end_ret, start_fp, end_fp, start_com, end_com, start_loop, end_loop;
	
	CvPoint *ps = NULL, *ps0 = NULL, corps[kMaxPointNum][2], ar_p1, ar_p2, point;
	double *areas = NULL, *areas0 = NULL;
	CvSize img_size, query_size, reg_size, thumb_size, merge_size, small_img_size, small_thumb_size, res_size, resized_size;
	strDirectShowCap dsc;
	IplImage *img_cap = NULL, *img_pt = NULL, *img_cap0 = NULL, *tmp = NULL, *thumb = NULL, *thumb_draw = NULL, *merge = NULL, *small_cap, *small_thumb, *clone = NULL, *cap_file, *cap_resized, *img_con = NULL, *img_con0 = NULL, *img_disp = NULL;
	IplImage *flag;
	// 表示画像関係
	IplImage *disp_cap, *disp_con;
	CvPoint *disp_ps;
	int disp_num;
#ifdef VIDEO_OUTPUT
	CvSize small_pt_size;
	IplImage *small_pt;
#endif
	strProjParam param, zero_param;;
	struct CvVideoWriter *video_wr;	// ビデオ出力用
	struct sockaddr_in addr;
	CvCapture *cap_mov;	// ビデオ入力用
	// サムネイル保存用
	char **fname_list = NULL;
	int doc_num = 0;

//	puts("start");
	InterpretArguments( argc, argv );	// 引数を解釈
	switch ( eEntireMode ) {
		case CAP_MOVIE_MODE:
			CaptureMovie( eMovieFileName );
			return 0;
		case DECOMPOSE_MOVIE_MODE:
			DecomposeMovie( eMovieFileName );
			return 0;
		case TUNE_FP_MODE:
			TuneFeaturePointParam( eTuneFpRegFileName );
			return 0;
		case CHK_CAM_MODE:
			CheckDirectShowCap();
			return 0;
		case CONV_MOVIE_MODE:
			ConvMovie();
			return 0;
		case CAM_HARRIS_MODE:
			DetectHarrisCam( eCamHarrisRegFileName, eDetectHarrisCamMode );
			return 0;
		case HARRIS_TEST_MODE:
			DetectHarrisTest( eHarrisTestOrigFileName, eHarrisTestAnnoFileName );
			return 0;
	}

	timeBeginPeriod( 1 );//精度を1msに設定
 
//	if ( ReadIniFile() == 0 ) {	// iniファイルの読み込みに失敗
//		fprintf( stderr, "Error : ReadIniFile\n" );
//		return 1;
//	}
	ReadIniFile();
	if ( eEntireMode == CAM_RET_MODE ) {
		ret = InitDirectShowCap( &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
		if ( ret == 0 )	return 1;
	}
	else if ( eEntireMode == INPUT_MOVIE_MODE ) {
		cap_mov = cvCaptureFromFile( eMovieFileName );	// ファイルからのキャプチャ
		if ( cap_mov == NULL )	return 1;	// キャプチャ失敗
		img_size.width = (int)cvGetCaptureProperty( cap_mov, CV_CAP_PROP_FRAME_WIDTH );	// サイズ読み込み
		img_size.height = (int)cvGetCaptureProperty( cap_mov, CV_CAP_PROP_FRAME_HEIGHT );
	}
	buff_size = img_size.width * img_size.height * 3;
	thumb_size = cvSize( 425, 550 );	// サムネイルのサイズ
	// ビデオ用の縮小サイズ
	small_img_size = cvSize( (int)(img_size.width * kResizeScaleImg), (int)(img_size.height * kResizeScaleImg) );
	small_thumb_size = cvSize( (int)(thumb_size.width * kResizeScaleThumb), (int)(thumb_size.height *kResizeScaleThumb) );
//	merge_size = cvSize( img_size.width + thumb_size.width, max( img_size.height, thumb_size.height ) );
	merge_size = cvSize( small_img_size.width + small_thumb_size.width, max( small_img_size.height, small_thumb_size.height ) );
	// アスペクト比に合わせる
	if ( merge_size.width > (int)((double)merge_size.height * kVideoAspectRatio) )
		merge_size.height = (int)((double)merge_size.width / kVideoAspectRatio);
	else if ( merge_size.width < (int)((double)merge_size.height * kVideoAspectRatio) )
		merge_size.width = (int)((double)merge_size.height * kVideoAspectRatio);
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// 画像を作成
	img_cap0 = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// 画像を作成
	if ( eCaptureWidth != 0 )	{	// キャプチャウィンドウの幅指定が有効
		resized_size.width = eCaptureWidth;
		resized_size.height = (int)((double)img_size.height * ((double)eCaptureWidth / (double)img_size.width));
		cap_resized = cvCreateImage( resized_size, IPL_DEPTH_8U, 3 );
	}
	img_con = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );
	img_con0 = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );
	img_disp = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// 画像を作成
#ifdef	VIDEO_OUTPUT
	small_pt_size.width = 212;
	small_pt_size.height = 159;
	small_cap = cvCreateImage( small_img_size, IPL_DEPTH_8U, 3 );
	small_thumb = cvCreateImage( small_thumb_size, IPL_DEPTH_8U, 3 );
	merge = cvCreateImage( merge_size, IPL_DEPTH_8U, 3 );
	small_pt = cvCreateImage( small_pt_size, IPL_DEPTH_8U, 3 );
#endif
#ifdef	AR
	ar = 1;
#else
	ar = -1;
#endif
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;

	if ( eEntireMode == CAM_RET_MODE )
		StartDirectShowCap( &dsc );	// キャプチャを開始
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// ウィンドウを作成
//	cvNamedWindow( "Connected", CV_WINDOW_AUTOSIZE );
//	cvNamedWindow( "Points", CV_WINDOW_AUTOSIZE );
#ifdef	DRAW_COR	// 対応点描画
	cvNamedWindow( "Corres", CV_WINDOW_AUTOSIZE );
#else
	cvNamedWindow( "Thumb", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Language", CV_WINDOW_AUTOSIZE );
#endif
#ifdef	DRAW_HIST	// ヒストグラム描画
	cvNamedWindow( "Hist", CV_WINDOW_AUTOSIZE );
#endif
	// ネゴシエイト開始
	sock = InitWinSockClTCP( eTCPPort, eServerName );
	if ( sock == INVALID_SOCKET ) {
		fprintf( stderr, "error: connection failure\n" );
		return 1;
	}
	ret = SendComSetting( sock, eProtocol, ePointPort, eResultPort, eClientName );	// 通信設定を送信
	if ( ret < 0 )	return 1;
//	ret = RecvFileNameList( sock, &doc_num, &fname_list, kMaxDocNameLen );
//	if  ( ret < 0 )	return 1;
//	for ( i = 0; i < doc_num; i++ ) {
//		printf( "%d : %s\n", i, fname_list[i] );
//	}
//	return 0;	// とりあえず終了

	if ( eProtocol == kUDP ) {	// UDP
		sockpt = InitWinSockClUDP( ePointPort, eServerName, &addr );
		sockres = InitWinSockSvUDP( eResultPort );
	}

#ifdef	VIDEO_OUTPUT
	video_wr = cvCreateVideoWriter( kVideoFileName, -1, kVideoFps, merge_size, 1 );	// ビデオ出力
#endif
	start = GetProcTimeMiliSec();
	for ( ; ; ) {
		start_loop = GetProcTimeMiliSec();
		if ( eEntireMode == CAM_RET_MODE ) {
			// キャプチャ
			start_cap = GetProcTimeMiliSec();
			CaptureDirectShowCap( &dsc, img_buff, buff_size );
			end_cap = GetProcTimeMiliSec();
			// バッファを画像に変換
			start_conv = GetProcTimeMiliSec();
			Buff2ImageData( img_buff, img_cap );
			end_conv = GetProcTimeMiliSec();
		}
		else if ( eEntireMode == INPUT_MOVIE_MODE ) {
			start_cap = GetProcTimeMiliSec();
			cap_file = cvQueryFrame( cap_mov );
			end_cap = GetProcTimeMiliSec();
			if ( cap_file == NULL )	goto end_cap;
			CopyImageData( cap_file, img_cap );
		}
		// 結合画像を作成
//		start_fp = GetProcTimeMiliSec();
//		start_connect = GetProcTimeMiliSec();
		if ( eIsJp )	img_con = GetConnectedImageCamJp2( img_cap );
		else			GetConnectedImageCam( img_cap, img_con );
//		end_connect = GetProcTimeMiliSec();
		if ( ps0 != NULL && num0 > 0 )	ReleaseCentres( ps0 );
		if ( areas0 != NULL && num0 > 0 )	free( areas0 );
		ps0 = ps;
		areas0 = areas;
		num0 = num;
		// 結合画像から重心を計算する
		num = MakeCentresFromImage( &ps, img_con, &img_size, &areas );
		end_fp = GetProcTimeMiliSec();
//		printf("cap : %dms\nfp : %dms\n", end_cap - start_cap, end_fp - start_fp);
//		if ( img_pt != NULL )	cvReleaseImage( &img_pt );
		// 表示画像の作成
		if ( draw_fp > 0 || draw_con > 0)	disp_cap = NULL;
		else	disp_cap = img_cap0;
		if ( draw_con > 0 || draw_con_overlap > 0 )	disp_con = img_con0;
		else	disp_con = NULL;
		if ( draw_fp > 0 || draw_fp_overlap > 0 ) {
			disp_ps = ps0;
			disp_num = num0;
		} else {
			disp_ps = NULL;
			disp_num = 0;
		}
		MakeDispImage( img_disp, disp_cap, disp_con, disp_ps, disp_num );
		// 特徴点を描画
/*		if ( draw_fp > 0 ) {
			if ( img_pt != NULL )	cvReleaseImage( &img_pt );
			img_pt = DrawPoints( ps0, num0, &img_size );
		}
		else if ( draw_fp_overlap > 0 ) {
			if ( img_pt != NULL )	cvReleaseImage( &img_pt );
			img_pt = DrawPointsOverlap( ps0, num0, &img_size, img_cap0 );
		}*/
//		cvShowImage( "Points", img_pt );
//		DrawPoints2( ps, num, img_size );
//		printf("cap : %dms\nconv : %dms\nshow : %dms\nconnect : %dms\nfp : %d\n", end_cap - start_cap, end_conv - start_conv, end_show - start_show, end_connect - start_connect, end_fp - start_fp );
//		cvWaitKey( 1 );	// ないとウィンドウが更新されない
		start_com = GetProcTimeMiliSec();
		// 結果を受信（受信を先にすることで、サーバとの並列処理が可能になる。なお一回目はダミーのデータが送られる）
//		RecvResult( sock, doc_name, 20 );
#ifdef	DRAW_COR
		printf("%x\n", ps );
		RecvResultCor( sock, doc_name, 20, corps, &corpsnum, &query_size, &reg_size );
#else
//		puts("recv start");
		if ( eProtocol == kTCP ) {
			RecvResultParam( sock, doc_name, kMaxDocNameLen, &param, &res_size );
		}
		else {
			RecvResultParam( sockres, doc_name, kMaxDocNameLen, &param, &res_size );
		}
#endif
//		printf("recv : %d ms\n", GetProcTimeMiliSec() - start_com );
		// 特徴点をサーバへ送信
//		start_com = GetProcTimeMiliSec();
//		puts("send start");
		if ( eProtocol == kTCP ) {
			SendPointsAreas( sock, ps, areas, num, &img_size, &addr, eProtocol );
		}
		else {
			SendPointsAreas( sockpt, ps, areas, num, &img_size, &addr, eProtocol );
		}
		end_com = GetProcTimeMiliSec();
//		printf("com : %dms\n", end_com - start_com);
		// 結果を表示
		printf("%s\n", doc_name);
//		printf("size:%d, %d\n", res_size.width, res_size.height );
#ifdef	DRAW_COR
//		if ( corpsnum > 0 )	DrawCor( corps, corpsnum, img_cap, doc_name );
		DrawCor2( corps, corpsnum, img_cap, doc_name );
#else
		// サムネイルを表示
		printf( "%x\n", doc_name );
#if 1
		DrawThumb( doc_name, doc_name_prev, &thumb, &thumb_draw, param, zero_param, img_size, res_size );
//		if ( doc_name[0] != '\0' && strcmp( doc_name, doc_name_prev ) ) {	// 検索結果が得られ、かつ前回の結果と異なる
//			thumb = LoadThumb( doc_name );
//			if ( thumb != NULL ) {
//				cvShowImage( "Thumb", thumb );
//				cvReleaseImage( &thumb );
//			}
//			strcpy( doc_name_prev, doc_name );
//		}
		cvShowImage( "Thumb", thumb_draw );
#ifdef	DRAW_FLAG
		// 言語の画像を表示
		printf( "%x\n", doc_name );
		flag = GetFlagImage( doc_name );
		cvShowImage( "Language", flag );
#endif

#ifdef	THUMB_OUT
		OutPutImage( thumb_draw );
#endif
#endif
//		DrawParam( img_cap0, thumb, param, zero_param );
		if ( ar > 0 ) {	// 拡張現実の描画
//			if ( clone != NULL )	cvReleaseImage( &clone );
//			clone = cvCloneImage( img_cap0 );	// 必要？　いらないなら消す
			DrawAR( img_disp, doc_name, param );	// ARを描画
//			if ( draw_fp > 0 || draw_fp_overlap > 0 ) 			DrawAR( img_pt, doc_name, param );	// ARを描画
//			else						DrawAR( img_cap0, doc_name, param );	// ARを描画
//			OutPutImage( img_cap0 );
		}
#endif
		strcpy( doc_name_prev, doc_name );
#ifdef	VIDEO_OUTPUT
		cvResize( img_cap0, small_cap, CV_INTER_NN );
		cvResize( thumb_draw, small_thumb, CV_INTER_NN );
		// 縮小特徴点画像の作成
		cvZero( small_pt );
		cvNot( small_pt, small_pt );
		for ( i = 0; i < num; i++ ) {
			point.x = (int)((double)ps[i].x * (double)small_pt_size.width / (double)img_size.width);
			point.y = (int)((double)ps[i].y * (double)small_pt_size.height / (double)img_size.height);
			cvCircle( small_pt, point, 1, CV_RGB(0,0,0), -1, CV_AA, 0 );
		}
		MergeImage3( small_cap, small_thumb, small_pt, merge );
		// キャプチャ画像をビデオに出力
		cvWriteFrame( video_wr, merge );
#endif
		// 枠を描画
//		if ( !IsEqualParam( param, zero_param ) )	DrawSash( img_cap0, &param );
		// キャプチャウィンドウの描画
		if ( eCaptureWidth == 0 ) {	// 幅指定なし
			cvShowImage( "Capture", img_disp );
		}
		else {	// 幅指定あり
			cvResize( img_disp, cap_resized, CV_INTER_LINEAR );	// リサイズ
			cvShowImage( "Capture", cap_resized );
		}
/*		if ( eCaptureWidth == 0 ) {	// 幅指定なし
			if ( draw_fp > 0 || draw_fp_overlap > 0 )	cvShowImage( "Capture", img_pt );
			else if ( draw_con > 0 && img_con0 != NULL )	cvShowImage( "Capture", img_con0 );
			else				cvShowImage( "Capture", img_cap0 );
		}
		else {
			if ( draw_fp > 0 || draw_fp_overlap > 0 )	cvResize( img_pt, cap_resized, CV_INTER_LINEAR );
			else if ( draw_con > 0 && img_con0 != NULL )	cvResize( img_con0, cap_resized, CV_INTER_LINEAR );
			else				cvResize( img_cap0, cap_resized, CV_INTER_LINEAR );
			cvShowImage( "Capture", cap_resized );
		}*/
		end = GetProcTimeMiliSec();
//		printf("Total : %dms\n", end - start );
		start = end;	
		key = cvWaitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'p':	// 一時停止
					if ( eEntireMode == CAM_RET_MODE )
						PauseDirectShowCap( &dsc );
					printf("Hit any key to resume\n");
					for ( ; cvWaitKey(100) < 0; );
					if ( eEntireMode == CAM_RET_MODE )
						ResumeDirectShowCap( &dsc );
					break;
				case 'q':	// 終了
					goto end_cap;
#ifndef	FUNC_LIMIT
				case ' ':	// キャプチャ画像を保存
					OutPutImage( img_cap );
					printf("Save complete\n");
					break;
				case 's':	// 撮影モード
					ret = GotoSnapshotMode( &dsc, img_buff, buff_size, img_cap, thumb );
					if ( ret == 1 ) {	// キャプチャして戻った
						ret = GotoPasteMode( img_cap, thumb, &ar_p1, &ar_p2 );
						if ( ret == 1 ) {	// 場所を指定して戻った
							SaveAR( doc_name, img_cap, cvSize( (int)((double)thumb->width / eThumbScale), (int)((double)thumb->height / eThumbScale)), &ar_p1, &ar_p2 );
						}
					}
					printf("*** Retrieval mode ***\n");
					printf("Pause(p) Capture(c) Snapshot(s) Quit(q)\n");
					break;
				case 'a':	// ARの描画のオン／オフ
					ar *= -1;
					break;
#endif
				case 'F':	// 特徴点モードのオン／オフ
					draw_fp *= -1;
					draw_fp_overlap = -1;
					break;
				case 'f':	// 特徴点上書きモードのオン／オフ
					draw_fp_overlap *= -1;
					draw_fp = -1;
					break;
				case 'C':	// 連結成分モードのオン／オフ
					draw_con *= -1;
					draw_con_overlap = -1;
					break;
				case 'c':	// 連結成分上書きモードのオン／オフ
					draw_con_overlap *= -1;
					draw_con = -1;
					break;
				default:
					break;
			}
		}
		// キャプチャ画像の1フレーム前との入れ替え
		tmp = img_cap0;
		img_cap0 = img_cap;
		img_cap = tmp;
		// 連結成分画像の1フレーム前との入れ替え
		tmp = img_con0;
		img_con0 = img_con;
		img_con = tmp;
//		Sleep(100);
		end_loop = GetProcTimeMiliSec();
		if ( eExperimentMode )	printf( "%d ms\n", end_loop - start_loop );
	}
end_cap:
	// 終了処理
	// 終了のメッセージとして負の特徴点数を送る
	if ( eProtocol == kTCP ) {
		SendPointsAreas( sock, NULL, NULL, -1, &img_size, &addr, eProtocol );
	}
	else {
		SendPointsAreas( sockpt, NULL, NULL, -1, &img_size, &addr, eProtocol );
	}
	CloseWinSock( sock );
	if ( eProtocol == kUDP ) {
		CloseWinSock( sockpt );
		CloseWinSock( sockres );
	}
	cvDestroyWindow( "Capture" );	// ウィンドウを破棄
//	cvDestroyWindow( "Connected" );
//	cvDestroyWindow( "Points" );
#ifdef	DRAW_COR
	cvDestroyWindow( "Corres" );
#else
	cvDestroyWindow( "Thumb" );
#endif
#ifdef	DRAW_HIST
	cvDestroyWindow( "Hist" );
#endif
	if ( eEntireMode == CAM_RET_MODE ) {
		StopDirectShowCap( &dsc );	// キャプチャを終了
		ReleaseDirectShowCap( &dsc );	// DirectShowの諸々をリリース
	}
	else if ( eEntireMode == INPUT_MOVIE_MODE ) {
		cvReleaseCapture( &cap_mov );
	}
	cvReleaseImage( &img_cap );
//	cvReleaseImage( &img );
//	cvReleaseImage( &img_pt );
	ReleaseCentres( ps );
	free( areas );
	free( img_buff );
#ifdef	VIDEO_OUTPUT
	cvReleaseVideoWriter( &video_wr );
#endif
	timeEndPeriod( 1 );

	return 0;
}

int GotoSnapshotMode( strDirectShowCap *dsc, unsigned char *img_buff, long buff_size, IplImage *img_cap, IplImage *thumb )
// スナップショット撮影モード
{
	int key;

	printf( "*** Snapshot mode ***\n");
	printf("Capture(c) Quit(q)\n");
	for ( ; ; ) {
		CaptureDirectShowCap( dsc, img_buff, buff_size );
		Buff2ImageData( img_buff, img_cap );
		cvShowImage( "Capture", img_cap );
		cvShowImage( "Thumb", thumb );
		key = cvWaitKey(1);
		if ( key >= 0 ) {
			switch ( key ) {
				case 'c':	// キャプチャ
					return 1;
				case 'q':	// 終了
					return 0;
				default:
					break;
			}
		}
	}
	return 0;
}

int GotoPasteMode( IplImage *img_cap, IplImage *thumb, CvPoint *p1, CvPoint *p2 )
// 貼り付けモード
{
	int key;

	g_thumb = cvCloneImage( thumb );
	g_thumb_draw = cvCloneImage( thumb );
	g_ratio = (double)img_cap->height / (double)img_cap->width;
	g_paste_complete = 0;

	printf("*** Paste mode ***\n");
	printf("Drag on \"Thumb\" window to paste the snapshot\n");
	cvSetMouseCallback( "Thumb", OnMouseThumb, 0 );

	cvShowImage( "Thumb", g_thumb_draw );

	for ( ; ; ) {
		key = cvWaitKey(1);
		switch ( key ) {
			case 'q':
				cvShowImage( "Thumb", thumb );

				cvReleaseImage( &g_thumb );
				cvReleaseImage( &g_thumb_draw );
			
				cvSetMouseCallback( "Thumb", NULL, 0 );
				return 0;
			case 's':
				goto paste_end;
			default:
				break;
		}
	}
paste_end:

	cvShowImage( "Thumb", thumb );

	cvReleaseImage( &g_thumb );
	cvReleaseImage( &g_thumb_draw );

	cvSetMouseCallback( "Thumb", NULL, 0 );

	if ( g_paste_complete == 1 ) {
		p1->x = min( (int)((double)g_p1.x/eThumbScale), (int)((double)g_p2.x/eThumbScale) );
		p1->y = min( (int)((double)g_p1.y/eThumbScale), (int)((double)g_p2.y/eThumbScale) );
		p2->x = max( (int)((double)g_p1.x/eThumbScale), (int)((double)g_p2.x/eThumbScale) );
		p2->y = max( (int)((double)g_p2.y/eThumbScale), (int)((double)g_p2.y/eThumbScale) );
		return 1;
	}
	else {
		return 0;
	}
}

void OnMouseThumb( int event, int x, int y, int flags, void *param )
// マウスのコールバック関数
{
//	static CvPoint p1, p2;
	static int status = 0;
	int width, height;
	double ratio;

	width = abs(g_p1.x - x);
	height = abs(g_p1.y - y);
	ratio = (double)height / (double)width;
	if ( ratio > g_ratio ) {	// y軸優先
		if ( x > g_p1.x ) {	// 右へ
			g_p2.x = g_p1.x + (int)((double)height / g_ratio);
		}
		else {	// 左へ
			g_p2.x = g_p1.x - (int)((double)height / g_ratio);
		}
		if ( y > g_p1.y ) {	// 下へ
			g_p2.y = g_p1.y + height;
		}
		else {	// 上へ
			g_p2.y = g_p1.y - height;
		}
	}
	else {	// x軸優先
		if ( x > g_p1.x ) {	// 右へ
			g_p2.x = g_p1.x + width;
		}
		else {	// 左へ
			g_p2.x = g_p1.x - width;
		}
		if ( y > g_p1.y ) {	// 下へ
			g_p2.y = g_p1.y + (int)((double)width * g_ratio);
		}
		else {	// 上へ
			g_p2.y = g_p1.y - (int)((double)width * g_ratio);
		}
	}

	switch( event ) {
		case CV_EVENT_LBUTTONDOWN:
			g_p1 = cvPoint( x, y );
			status = 1;	// マウスが押された
			g_paste_complete = 0;
			break;
		case CV_EVENT_LBUTTONUP:
			if ( status == 1 ) {
				cvReleaseImage( &g_thumb_draw );
				g_thumb_draw = cvCloneImage( g_thumb );
				cvRectangle( g_thumb_draw, g_p1, g_p2, CV_RGB( 255, 0, 0 ), 2, 8, 0 );
				status = 0;
				cvShowImage( "Thumb", g_thumb_draw );
			}
			g_paste_complete = 1;
			break;
		case CV_EVENT_MOUSEMOVE:
			if ( status == 1 ) {
				cvReleaseImage( &g_thumb_draw );
				g_thumb_draw = cvCloneImage( g_thumb );
				cvRectangle( g_thumb_draw, g_p1, g_p2, CV_RGB( 0, 0, 0 ), 1, 8, 0 );
				cvShowImage( "Thumb", g_thumb_draw );
			}
			break;
	}
}

int LoadPointFile( char *fname, CvPoint **ps0, CvSize *size )
// 点データを読み込む
{
	int num;
	char line[kMaxLineLen];
	FILE *fp;
	CvPoint *ps;
	
	if ( (fp = fopen(fname, "r")) == NULL )	return 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size->width), &(size->height));
	ps = (CvPoint *)calloc(kMaxPointNum, sizeof(CvPoint));
	*ps0 = ps;
	num = 0;
	while ( fgets(line, kMaxLineLen, fp) != NULL && num < kMaxPointNum ) {
		sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
		num++;
	}
	fclose(fp);
	return num;
}

IplImage *LoadThumb( char *fname )
// fnameで指定されたサムネイル画像をロードする
{
	char thumb_fname[kMaxPathLen];

	if ( *fname == '\0' )	return NULL;	// fnameが空文字列
//	sprintf( thumb_fname, "%s%s.%s", kThumbDir, fname, kThumbSuffix );
	sprintf( thumb_fname, "%s%s.%s", eThumbDir, fname, eThumbSuffix );
	puts(thumb_fname);
	return cvLoadImage( thumb_fname, 1 );
}

#define	kDrawParamScale	(0.5)
#define	kDrawParamHMargin	(100)
#define	kDrawParamVMargin	(100)
#define	kDrawParamSpace	(400)
#define	kDrawParamRectThick	(4)
#define	kDrawParamLineThick	(4)

void DrawParam( IplImage *img_cap, IplImage *thumb, strProjParam param, strProjParam zero_param )
// キャプチャ画像とサムネイルとパラメータを示す枠の対応を描画して出力
{
	int i, width_all, height_all;
	IplImage *img, *thumb_res;
	strPoint p1[5], p2[5];

	// サムネイルの拡大版を作る
	thumb_res = cvCreateImage( cvSize( (int)(thumb->width / eThumbScale * kDrawParamScale), (int)(thumb->height / eThumbScale * kDrawParamScale) ), IPL_DEPTH_8U, 3 );
	cvResize( thumb, thumb_res, CV_INTER_LINEAR );
	// 全体画像を作る
	width_all = kDrawParamHMargin * 2 + kDrawParamSpace + img_cap->width + thumb_res->width;
	height_all = kDrawParamVMargin * 2 + max( img_cap->height, thumb_res->height );
	img = cvCreateImage( cvSize( width_all, height_all ), IPL_DEPTH_8U, 3 );
	cvSet( img, cWhite, NULL );	// 白で塗りつぶす
	// キャプチャ画像を配置する
	PutImage( img_cap, img, cvPoint( kDrawParamHMargin, kDrawParamVMargin ) );
	// 拡大サムネイルを配置する
	PutImage( thumb_res, img, cvPoint( kDrawParamHMargin + img_cap->width + kDrawParamSpace, kDrawParamVMargin ) );
	// サムネイルの枠を描画
	cvRectangle( img, cvPoint( kDrawParamHMargin + img_cap->width + kDrawParamSpace, kDrawParamVMargin), \
		cvPoint( kDrawParamHMargin + img_cap->width + kDrawParamSpace + thumb_res->width, kDrawParamVMargin + thumb_res->height ), \
		cBlack, kDrawParamRectThick, CV_AA, 0 );
	// キャプチャ画像の枠を描画
	cvRectangle( img, cvPoint( kDrawParamHMargin, kDrawParamVMargin ), cvPoint( kDrawParamHMargin + img_cap->width, kDrawParamVMargin + img_cap->height ), \
		cRed, kDrawParamRectThick, CV_AA, 0 );
	// 四隅の対応線を描画
	if ( !IsEqualParam( param, zero_param ) ) {	// パラメータ計算に成功
		// 枠を描画
		p1[0].x = 0;
		p1[0].y = 0;
		p1[1].x = img_cap->width;
		p1[1].y = 0;
		p1[2].x = img_cap->width;
		p1[2].y = img_cap->height;
		p1[3].x = 0;
		p1[3].y = img_cap->height;
		p1[4].x = 0;
		p1[4].y = 0;
		for ( i = 0; i < 5; i++ ) {
			ProjTrans( &(p1[i]), &(p2[i]), &param );
			p2[i].x = (int)(p2[i].x * kDrawParamScale);
			p2[i].y = (int)(p2[i].y * kDrawParamScale);
		}
		for ( i = 0; i < 4; i++ )	cvLine( img, cvPoint( kDrawParamHMargin + img_cap->width + kDrawParamSpace + p2[i].x, kDrawParamVMargin + p2[i].y ), cvPoint( kDrawParamHMargin + img_cap->width + kDrawParamSpace + p2[i+1].x, kDrawParamHMargin + p2[i+1].y ), cRed, kDrawParamLineThick, CV_AA, 0 );
		for ( i = 0; i < 4; i++ )	cvLine( img, cvPoint( kDrawParamHMargin + p1[i].x, kDrawParamVMargin + p1[i].y ), cvPoint( kDrawParamHMargin + img_cap->width + kDrawParamSpace + p2[i].x, kDrawParamVMargin + p2[i].y ), cRed, kDrawParamLineThick, CV_AA, 0 );
	}


	OutPutImage( img );
//	OutPutImage( thumb_res );
	cvReleaseImage( &thumb_res );
	cvReleaseImage( &img );
}

void DrawThumb( char *doc_name, char *doc_name_prev, IplImage **thumb0, IplImage **thumb_draw0, strProjParam param, strProjParam zero_param, CvSize cap_size, CvSize res_size )
// サムネイルを枠つきで描画
{
	int i;
	char thumb_fname[kMaxPathLen];
	IplImage *thumb, *thumb_orig, *thumb_draw;
	CvSize thumb_size;
	strPoint p1[5], p2[5];

	// サムネイルのサイズ
	if ( eThumbWidth != 0 ) {	// 幅指定が有効
		thumb_size.width = eThumbWidth;
		thumb_size.height = (int)((double)res_size.height * ((double)eThumbWidth / (double)res_size.width));
		eThumbScale = (double)eThumbWidth / (double)res_size.width;
	}
	else {	// スケール指定
		thumb_size.width = (int)((double)res_size.width * eThumbScale);
		thumb_size.height = (int)((double)res_size.height * eThumbScale);
	}

	thumb = *thumb0;
	if ( strcmp( doc_name, doc_name_prev ) ) {	// 前回と検索結果が異なる
		if ( *thumb0 != NULL )	cvReleaseImage( thumb0 );	// リリースした上で
		thumb = cvCreateImage( thumb_size, IPL_DEPTH_8U, 3 );	// 縮小画像を作成
		cvSet( thumb, CV_RGB(255,255,255), NULL );	// 白にクリア
		sprintf( thumb_fname, "%s%s.%s", eThumbDir, doc_name, eThumbSuffix );
//		puts( thumb_fname );
		if ( ( thumb_orig = cvLoadImage( thumb_fname, 1 ) ) != NULL ) {	// サムネイル画像をロードし，
			cvResize( thumb_orig, thumb, CV_INTER_NN );	// リサイズして表示
			cvReleaseImage( &thumb_orig );
		}
		*thumb0 = thumb;
	}
	if ( *thumb_draw0 != NULL )	cvReleaseImage( thumb_draw0 );
	thumb_draw = cvCloneImage( thumb );	// クローンを作成
	*thumb_draw0 = thumb_draw;
	if ( !IsEqualParam( param, zero_param ) ) {	// パラメータ計算に成功
		// 枠を描画
//		cvLine( thumb_draw, cvPoint(0,0), cvPoint(100,100), cRed, 4, CV_AA, 0 );
		p1[0].x = 0;
		p1[0].y = 0;
		p1[1].x = cap_size.width;
		p1[1].y = 0;
		p1[2].x = cap_size.width;
		p1[2].y = cap_size.height;
		p1[3].x = 0;
		p1[3].y = cap_size.height;
		p1[4].x = 0;
		p1[4].y = 0;
		for ( i = 0; i < 5; i++ ) {
			ProjTrans( &(p1[i]), &(p2[i]), &param );
			p2[i].x = (int)(p2[i].x * eThumbScale);
			p2[i].y = (int)(p2[i].y * eThumbScale);
		}
		for ( i = 0; i < 4; i++ )	cvLine( thumb_draw, cvPoint( p2[i].x, p2[i].y ), cvPoint( p2[i+1].x, p2[i+1].y ), cRed, 5, CV_AA, 0 );
	}
#if 0
	thumb = *thumb0;
	thumb_draw = *thumb_draw0;
	if ( doc_name[0] == '\0' ) {	// 検索に失敗
		strcpy( doc_name, "0" );		// 0.bmpを表示
	}
	if ( strcmp( doc_name, doc_name_prev ) ) {	// 前回と検索結果が異なる
		if ( *thumb0 != NULL )	cvReleaseImage( thumb0 );	// リリースした上で
//		puts("start load thumb");
		thumb = LoadThumb( doc_name );		// ロードする
//		puts("finish load thumb");
		*thumb0 = thumb;
	}
	if ( *thumb_draw0 != NULL )	cvReleaseImage( thumb_draw0 );
	thumb_draw = cvCloneImage( thumb );	// クローンを作成
	*thumb_draw0 = thumb_draw;

	if ( !IsEqualParam( param, zero_param ) ) {	// パラメータ計算に成功
		// 枠を描画
//		cvLine( thumb_draw, cvPoint(0,0), cvPoint(100,100), cRed, 4, CV_AA, 0 );
		p1[0].x = 0;
		p1[0].y = 0;
		p1[1].x = cap_size.width;
		p1[1].y = 0;
		p1[2].x = cap_size.width;
		p1[2].y = cap_size.height;
		p1[3].x = 0;
		p1[3].y = cap_size.height;
		p1[4].x = 0;
		p1[4].y = 0;
		for ( i = 0; i < 5; i++ ) {
			ProjTrans( &(p1[i]), &(p2[i]), &param );
			p2[i].x = (int)(p2[i].x * kThumbScale);
			p2[i].y = (int)(p2[i].y * kThumbScale);
		}
		for ( i = 0; i < 4; i++ )	cvLine( thumb_draw, cvPoint( p2[i].x, p2[i].y ), cvPoint( p2[i+1].x, p2[i+1].y ), cRed, 5, CV_AA, 0 );
	}
#endif
}

void DrawCor2( CvPoint corps[][2], int corpsnum, IplImage *img_cap, char *doc_name )
// 対応点の描画２（キャプチャ画像とサムネイル付き）
{
	int i;
	char thumb_fname[kMaxPathLen];
	IplImage *cor_img, *thumb;
	CvSize qs, rs, cor_size;
	CvPoint p1, p2;
	
	if ( doc_name[0] == '\0' ) {	// 検索に失敗
		strcpy( doc_name, "0" );		// 0.bmpを表示
	}
//	puts("start load thumb");
	thumb = LoadThumb( doc_name );		// ロードする
//	puts("finish load thumb");

	cor_size = cvSize( img_cap->width + thumb->width, max( img_cap->height, thumb->height ) );
	cor_img = cvCreateImage( cor_size, IPL_DEPTH_8U, 3 );
	MergeImage( img_cap, thumb, MERGE_HOR, cor_img );

	for ( i = 0; i < corpsnum; i++ ) {
		p1.x = (int)(corps[i][0].x);
		p1.y = (int)(corps[i][0].y);
		p2.x = (int)(corps[i][1].x * eThumbScale) + img_cap->width;
		p2.y = (int)(corps[i][1].y * eThumbScale);
		cvLine( cor_img, p1, p2, cBlack, 1, CV_AA, 0 );
//		cvLine( cor_img, p1, p2, cWhite, 1, CV_AA, 0 );
	}
	cvShowImage( "Corres", cor_img );
	OutPutImage( cor_img );
	
	cvReleaseImage( &thumb );
	cvReleaseImage( &cor_img );
}

int SaveAR( char *doc_name, IplImage *img_cap, CvSize size, CvPoint *p1, CvPoint *p2 )
// AR情報を保存する
{
	char ar_fname[kMaxPathLen], img_fname[kMaxPathLen], img_path[kMaxPathLen];
	int num, ar_exist;
	FILE *fp;

	srand( GetTickCount() );
	num = rand() % 1000;

	sprintf( ar_fname, "%s%s.txt", kARDir, doc_name );
	sprintf( img_fname, "%s%03d.bmp", doc_name, num );
	sprintf( img_path, "%s%s", kARImageDir, img_fname );
//	puts( ar_fname );
//	puts( img_fname );
//	getchar();
	if ( ( fp = fopen( ar_fname, "r" ) ) == NULL ) {
		ar_exist = 0;
	}
	else {
		fclose( fp );
		ar_exist = 1;
	}
	if ( ( fp = fopen( ar_fname, "a" ) ) == NULL ) {
		return 0;
	}
	if ( !ar_exist )	fprintf( fp, "%d,%d\n", size.width, size.height );
	fprintf( fp, "%c\n%d,%d,%d,%d\n%s\n", kARTypeImg, p1->x, p1->y, p2->x, p2->y, img_fname );
	fclose( fp );
	cvSaveImage( img_path, img_cap );

	return 1;
}

int CaptureMovie( char *movfile )
// 動画の撮影
{
	int key, frame_msec;
	long buff_size;
	unsigned char *img_buff;
	CvSize img_size;
	IplImage *img_cap = NULL;
	strDirectShowCap dsc;
	struct CvVideoWriter *video_wr;	// ビデオ出力用
	TIME_COUNT start, end, start_cap, end_cap, start_show, end_show, start_connect, end_connect, start_conv, end_conv, start_ret, end_ret, start_fp, end_fp, start_com, end_com;

	timeBeginPeriod( 1 );//精度を1msに設定

	if ( ReadIniFile() == 0 ) {	// iniファイルの読み込みに失敗
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 1;
	}

	printf("%d\n", eCamConfNum);
	InitDirectShowCap( &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
	buff_size = img_size.width * img_size.height * 3;
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );	// 画像を作成

	StartDirectShowCap( &dsc );	// キャプチャを開始
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );	// ウィンドウを作成

	video_wr = cvCreateVideoWriter( movfile, -1, kVideoFps, img_size, 1 );	// ビデオ出力
	frame_msec = (int)(1000.0L / (double)kVideoFps );

	for ( ; ; ) {
		start = GetProcTimeMiliSec();
		// キャプチャ
		start_cap = GetProcTimeMiliSec();
		CaptureDirectShowCap( &dsc, img_buff, buff_size );
		end_cap = GetProcTimeMiliSec();
		// バッファを画像に変換
		start_conv = GetProcTimeMiliSec();
		Buff2ImageData( img_buff, img_cap );
		end_conv = GetProcTimeMiliSec();
		// キャプチャ画像を表示
		start_show = GetProcTimeMiliSec();
		cvShowImage( "Capture", img_cap );
		end_show = GetProcTimeMiliSec();
//		OutPutImage( img_cap );
		// キャプチャ画像をビデオに出力
		cvWriteFrame( video_wr, img_cap );
		key = cvWaitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'p':	// 一時停止
					PauseDirectShowCap( &dsc );
					printf("Hit any key to resume\n");
					for ( ; cvWaitKey(100) < 0; );
					ResumeDirectShowCap( &dsc );
					break;
				case 'q':	// 終了
					goto end_cap;
				case 'c':	// キャプチャ画像を保存
					OutPutImage( img_cap );
					printf("Save complete\n");
					break;
				default:
					break;
			}
		}
//		for ( end = GetProcTimeMiliSec(); end - start < frame_msec; end = GetProcTimeMiliSec() )
//			Sleep( 1 );
//		Sleep(100);
	}
end_cap:
	// 終了処理
	cvDestroyWindow( "Capture" );	// ウィンドウを破棄
	StopDirectShowCap( &dsc );	// キャプチャを終了
	ReleaseDirectShowCap( &dsc );	// DirectShowの諸々をリリース
	cvReleaseImage( &img_cap );
	free( img_buff );
	cvReleaseVideoWriter( &video_wr );
	timeEndPeriod( 1 );

	return 0;
}

int DecomposeMovie( char *mov_file )
// 動画ファイルを分解し，画像を出力
{
	int i;
	IplImage *img_cap;
	CvCapture *cap_mov;
	CvSize img_size;

	cap_mov = cvCaptureFromFile( mov_file );
	img_size.width = (int)cvGetCaptureProperty( cap_mov, CV_CAP_PROP_FRAME_WIDTH );
	img_size.height = (int)cvGetCaptureProperty( cap_mov, CV_CAP_PROP_FRAME_HEIGHT );
	printf("%d,%d\n", img_size.width, img_size.height );
	while ( ( img_cap = cvQueryFrame( cap_mov ) ) != NULL ) {
		OutPutImage( img_cap );
	}
	cvReleaseCapture( &cap_mov );
	return 1;
}

void ConvMovie( void )
// 動画の変換
{
	CvCapture *cap_mov;
	CvSize img_size;
	double mov_fps;
	CvVideoWriter *video_wr;
	IplImage *cap_file;

	if ( ( cap_mov = cvCaptureFromFile( eMovieFileName ) ) == NULL )	return;
	img_size.width = (int)cvGetCaptureProperty( cap_mov, CV_CAP_PROP_FRAME_WIDTH );	// サイズ読み込み
	img_size.height = (int)cvGetCaptureProperty( cap_mov, CV_CAP_PROP_FRAME_HEIGHT );
	mov_fps = cvGetCaptureProperty( cap_mov, CV_CAP_PROP_FPS );
	video_wr = cvCreateVideoWriter( eConvMovieFileName, -1, mov_fps, img_size, 1 );	// ビデオ出力
	cvNamedWindow( "Movie", CV_WINDOW_AUTOSIZE );
	for ( ; ; ) {
		cap_file = cvQueryFrame( cap_mov );
		if ( cap_file == NULL )	break;
		cvShowImage( "Movie", cap_file );
		cvWriteFrame( video_wr, cap_file );
	}
	cvReleaseCapture( &cap_mov );
	cvReleaseVideoWriter( &video_wr );

	return;
}

IplImage *LoadFlagImage( char *doc_name )
// doc_nameから国を判別し，国旗を読み込む
{
	char path[kMaxPathLen];
	char langs[][20] = { "arabic", "chinese", "english", "french", "hindi", "japanese", "korean", "lhao", "russian", "spanish", "tamil", "thai" };
	int i, num;
	IplImage *img;

	num = sizeof( langs );
	for ( i = 0; i < num; i++ ) {
		if ( strncmp( doc_name, langs[i], strlen( langs[i] ) ) == 0 ) {
			sprintf( path, "%s%s.%s", kFlagDir, langs[i], kFlagSuffix );
			printf("%s\n", path);
			img = cvLoadImage( path, 1 );
			return img;
		}
	}
	return NULL;
}

void MakeDispImage( IplImage *img_disp, IplImage *disp_cap, IplImage *disp_con, CvPoint *disp_ps, int disp_num )
// 表示用画像を作成する
{
	int i;

	// キャプチャ画像
	if ( disp_cap != NULL )	cvCopyImage( disp_cap, img_disp );
	else	cvZero( img_disp );
	// 連結成分
	if ( disp_con != NULL )	cvSet( img_disp, cGreen, disp_con );
	// 特徴点
	for ( i = 0; i < disp_num; i++ ) {
		cvCircle( img_disp, disp_ps[i], 3, cRed, -1, CV_AA, 0 );
	}
}

IplImage *GetFlagImage( char *doc_name )
// ファイル名から国旗画像を得る
{
	char path[kMaxPathLen];
	char langs[13][20] = { "none", "arabic", "chinese", "english", "french", "hindi", "japanese", "korean", "lhao", "russian", "spanish", "tamil", "thai" };
	int i;
	static int loaded = 0;	// 画像をロードしたかどうかのフラグ
	static int num = 13;
	static IplImage **flag_imgs = NULL;

	if ( loaded == 0 ) {	// まだ読み込んでいない
		flag_imgs = (IplImage **)calloc( num, sizeof(IplImage *) );
		for ( i = 0; i < num; i++ ) {
			sprintf( path, "%s%s.%s", kFlagDir, langs[i], kFlagSuffix );
			flag_imgs[i] = cvLoadImage( path, 1 );
		}
		loaded = 1;
	}
	if ( doc_name[0] == '\0' )	return flag_imgs[0];	// 検索結果なしならnoneを返す
	for ( i = 1; i < num; i++ ) {	// noneは省くので1から
		if ( strncmp( doc_name, langs[i], strlen( langs[i] ) ) == 0 ) {
			return flag_imgs[i];
		}
	}
	return flag_imgs[0];	// 該当するものがなければnoneを返す
}
