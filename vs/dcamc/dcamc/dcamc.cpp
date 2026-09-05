#define	GLOBAL_DEFINE	/* extern変数にここで実体を与える */

#include "def_general.h"	// proctime用
#include <stdio.h>
#include <winsock2.h>	// socket
#include <opencv2/opencv.hpp>			// OpneCV
// #include <highgui.h> // now in opencv2/opencv.hpp	// OpenCV
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
cv::Mat g_thumb, g_thumb_draw;
double g_ratio = 0.0;
int g_paste_complete = 0;
cv::Point g_p1, g_p2;

int LoadPointFile( char *fname, cv::Point **ps0, cv::Size *size );

int SendPoints( SOCKET sock, cv::Point *ps, int num, cv::Size *size, struct sockaddr_in *addr, int ptc )
// 点データを送信
{
	int i, num_send, buff_cur;
	char buff[kSendBuffSize];

	// 画像サイズをバッファに格納
	int j=0;
	for ( i = 0, buff_cur = 0; i < sizeof(cv::Size); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)size)[i];
	}
	// 特徴点数をバッファに格納
	num_send = (num > kMaxPointNumServer) ? kMaxPointNumServer : num;	// 特徴点数がサーバ側の最大値を超える場合は制限
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)&num_send)[i];
	}
	// 特徴点データをバッファに格納
	for ( i = 0; i < (int)sizeof(cv::Point) * num_send; i++, buff_cur++ ) {
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

int SendPointsAreas( SOCKET sock, cv::Point *ps, double *areas, int num, cv::Size *size, struct sockaddr_in *addr, int ptc )
// 点データを送信
// 面積も送信
{
	int i, num_send, buff_cur;
	char buff[kSendBuffSizeAreas];
	unsigned short areas_us[kMaxPointNumServer];

	// 画像サイズをバッファに格納
	int j=0;
	for ( i = 0, buff_cur = 0; i < sizeof(cv::Size); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)size)[i];
	}
	// 特徴点数をバッファに格納
	num_send = (num > kMaxPointNumServer) ? kMaxPointNumServer : num;	// 特徴点数がサーバ側の最大値を超える場合は制限
	for ( i = 0; i < sizeof(int); i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)&num_send)[i];
	}
	// 特徴点データをバッファに格納
	for ( i = 0; i < (int)sizeof(cv::Point) * num_send; i++, buff_cur++ ) {
		buff[buff_cur] = ((char *)ps)[i];
	}
	// 面積をunsigned shortに直して格納
	for ( i = 0; i < num_send; i++ ) {
		areas_us[i] = (unsigned short)areas[i];
		// printf("%lf\n", areas[i]);
	}
	buff_cur = sizeof(cv::Size) + sizeof(int) + sizeof(cv::Point) * kMaxPointNumServer;
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

int SendImage( SOCKET sock, cv::Mat img )
// 画像を送信
{
	int ret;
	cv::Size size;
	
	size.width = img.cols;
	size.height = img.rows;
	send( sock, (const char *)&size, sizeof(cv::Size), SEND_FLAG );	// サイズを送信
	int img_channels = img.channels();
	int img_depth = (int)(img.elemSize1() * 8);	// IplImage.depth (IPL_DEPTH_8U == 8) and bits per channel
	send( sock, (const char *)&img_channels, sizeof(int), SEND_FLAG );	// チャネルを送信
	send( sock, (const char *)&img_depth, sizeof(int), SEND_FLAG );	// ビット数を送信
	ret = send( sock, (const char *)((char*)img.data), ((int)img.total() * (int)img.elemSize()), SEND_FLAG );	// 画像本体を送信
	
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

int RecvResultCor( SOCKET sock, char *doc_name, int len, cv::Point corps[][2], int *corpsnum, cv::Size *query_size, cv::Size *reg_size )
// 文書名と対応点を受信
{
	int i, ret;
	
	ret = recv( sock, doc_name, len, RECV_FLAG );	// 文書名を受信
	puts(doc_name);
	ret = recv( sock, (char *)query_size, sizeof(cv::Size), RECV_FLAG );	// クエリ画像のサイズを受信
//	printf("%d,%d\n", query_size.cols, query_size.rows);
	ret = recv( sock, (char *)reg_size, sizeof(cv::Size), RECV_FLAG );	// 登録画像のサイズを受信
	ret = recv( sock, (char *)corpsnum, sizeof(int), RECV_FLAG );	// 対応の数を受信
//	printf("%d\n", *corpsnum );
	if ( *corpsnum > 0 )	ret = recv( sock, (char *)corps, sizeof(cv::Point)*2*(*corpsnum), SEND_FLAG );
	for ( i = 0; i < *corpsnum; i++ ) {
//		ret = recv( sock, (char *)&(corps[i][0]), sizeof(cv::Point), RECV_FLAG );
//		ret = recv( sock, (char *)&(corps[i][1]), sizeof(cv::Point), RECV_FLAG );
//		printf("%d : (%d, %d), (%d, %d)\n", i, corps[i][0].x, corps[i][0].y, corps[i][1].x, corps[i][1].y);
	}
	return ret;
}

int RecvResultParam( SOCKET sock, char *doc_name, int len, strProjParam *param, cv::Size *img_size )
// 文書名と射影変換パラメータを受信
{
	int ret;
	char buff[kRecvBuffSize];

	ret = recv( sock, buff, kRecvBuffSize, RECV_FLAG );
	memcpy( doc_name, buff, kMaxDocNameLen );
	memcpy( param, buff + kMaxDocNameLen, sizeof(strProjParam) );
	memcpy( img_size, buff + kMaxDocNameLen + sizeof(strProjParam), sizeof(cv::Size) );

	return ret;

#if 0

	ret = recv( sock, doc_name, len, RECV_FLAG );
	ret = recv( sock, (char *)param, sizeof(strProjParam), RECV_FLAG );
	
	return ret;
#endif
}

void Buff2ImageData( unsigned char *buff, cv::Mat img )
{
	int i;

	for ( i = 0; i < img.rows; i++ ) {
		memcpy( &(img.data[img.step[0]*(img.rows - i - 1)]), &(buff[img.cols*3*i]), img.cols*3 );
	}
}

void ReleaseCentres( cv::Point *ps )
{
	free( ps );
}

#define	CENTROID
//#define	SECOND_ORDER_MOMENT
//#define	kMaxNoiseArea	(10)
#define	kMaxNoiseArea	(20)	// 裏写り対策

int CalcCentres(cv::Point **ps0, const std::vector<std::vector<cv::Point>> &contours, const std::vector<cv::Vec4i> &hierarchy, cv::Size *size, double **areas0)
// 重心を計算する
// 06/01/12	面積の計算を追加
{
	int i, num;
	double *areas = NULL;
//	double max_area = 0.0L;	// 連結成分の面積の最大値を調べる用
	cv::Moments mom;
	cv::Point *ps;

	// 点の数をカウント（暫定）
	i = 0;
	for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ) i++;
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// ゴミ等で連結成分が多すぎるとき（本来は大きさを調べたりすべき）
	// 点を入れる配列を確保（numの値は正確ではないが，大きめということで）
	ps = (cv::Point *)calloc(num, sizeof(cv::Point));
	*ps0 = ps;
	// 面積を入れる配列を確保
	if ( areas0 ) {	// 面積配列のポインタが与えられている
		areas = (double *)calloc(num, sizeof(double));
		*areas0 = areas;
	}
	// 連結成分を描画・重心を計算
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0 && i < num; ci = hierarchy[ci][0] ){
		double d00;
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < kMaxNoiseArea ) continue;	// 小さすぎる連結成分は除外

#ifdef	CENTROID
		ps[i].x = (int)(mom.m10 / d00);
		ps[i].y = (int)(mom.m01 / d00);
#endif
#ifdef	SECOND_ORDER_MOMENT
		// 07/05/30
		// なぜこっちにしていたのか分からない。直し忘れか？
		// どちらにせよあまり影響は与えていなかった（2次の項は1ピクセルにも満たない）。
//		printf( "%f + %f, %f + %f\n", mom.m10 / d00, mom.nu20, mom.m01 / d00, mom.nu02 );
		ps[i].x = (int)(mom.m10 / d00 + mom.nu20 );
		ps[i].y = (int)(mom.m01 / d00 + mom.nu02 );
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

int CalcFloatCentres(cv::Point2f **pfs0, const std::vector<std::vector<cv::Point>> &contours, const std::vector<cv::Vec4i> &hierarchy, cv::Size *size, double **areas0)
// 重心を計算する（float版）
// 06/01/12	面積の計算を追加
{
	int i, num;
	cv::Moments mom;
	cv::Point2f *pfs;

	// 点の数をカウント（暫定）
	i = 0;
	for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0; ci = hierarchy[ci][0] ) i++;
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// ゴミ等で連結成分が多すぎるとき（本来は大きさを調べたりすべき）
	// 点を入れる配列を確保（numの値は正確ではないが，大きめということで）
	pfs = (cv::Point2f *)calloc(num, sizeof(cv::Point2f));
	*pfs0 = pfs;
	// 連結成分を描画・重心を計算
    i = 0;
    for( int ci = FirstTopLevelContour( hierarchy ); ci >= 0 && i < num; ci = hierarchy[ci][0] ){
		double d00;
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < kMaxNoiseArea ) continue;	// 小さすぎる連結成分は除外

#ifdef	CENTROID
		pfs[i].x = (float)(mom.m10 / d00);
		pfs[i].y = (float)(mom.m01 / d00);
#endif
#ifdef	SECOND_ORDER_MOMENT
		pfs[i].x = (float)(mom.m10 / d00 + mom.nu20 );
		pfs[i].y = (float)(mom.m01 / d00 + mom.nu02 );
#endif
		i++;
    }
	num = i;	// 連結成分数の更新

	return num;
}

int MakeCentresFromImage(cv::Point **ps, cv::Mat img, cv::Size *size, double **areas)
// 画像のファイル名を与えて重心を計算し，重心の数を返す
// 06/01/12	面積の計算を追加
{
	int num, ret;
    // CvMemStorage removed (not needed in OpenCV 4)
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	size->width = img.cols;
	size->height = img.rows;
	
    cv::findContours( img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE ); ret = (int)contours.size();	// 連結成分を抽出する
	if ( ret <= 0 ) {	// 連結成分が見つからなかった // cvReleaseMemStorage removed
		return 0;
	}
	num = CalcCentres(ps, contours, hierarchy, size, areas);	// 各連結成分の重心を計算する


	return num;
}

int MakeFloatCentresFromImage(cv::Point2f **pfs, cv::Mat img, cv::Size *size, double **areas)
// 画像のファイル名を与えて重心を計算し，重心の数を返す
// 06/01/12	面積の計算を追加
{
	int num, ret;
    // CvMemStorage removed (not needed in OpenCV 4)
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	size->width = img.cols;
	size->height = img.rows;
	
    cv::findContours( img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE ); ret = (int)contours.size();	// 連結成分を抽出する
	if ( ret <= 0 ) {	// 連結成分が見つからなかった // cvReleaseMemStorage removed
		return 0;
	}
	num = CalcFloatCentres(pfs, contours, hierarchy, size, areas);	// 各連結成分の重心を計算する


	return num;
}

#define	kDrawPointsHMargin	(100)
#define	kDrawPointsVMargin	(100)
#define	kDrawPointsRectThick	(4)
#define	kDrawPointsPtRad	(4)

void DrawPoints2( cv::Point *ps, int num, cv::Size img_size )
// 特徴点を描画し，保存する
{
	int i;
	cv::Mat img;

	img = cv::Mat::zeros(cv::Size( kDrawPointsHMargin*2+img_size.width, kDrawPointsVMargin*2+img_size.height ), CV_8UC3);
	img.setTo( cWhite );	// 白で塗りつぶす
	// 枠を描画
	cv::rectangle( img, cv::Point( kDrawPointsHMargin, kDrawPointsVMargin ), \
		cv::Point( kDrawPointsHMargin + img_size.width, kDrawPointsVMargin + img_size.height ), \
		cBlack, kDrawPointsRectThick, cv::LINE_AA, 0 );
	// 特徴点を描画
	for ( i = 0; i < num; i++ ) {
		cv::circle( img, cv::Point( kDrawPointsHMargin + ps[i].x, kDrawPointsVMargin + ps[i].y ), kDrawPointsPtRad, cBlack, -1, cv::LINE_AA, 0 );
	}
	OutPutImage( img ); // cvReleaseImage removed (cv::Mat auto-manages memory)
}

cv::Mat DrawPoints( cv::Point *ps, int num, cv::Size *img_size )
// 特徴点を描画する
{
	int i;
	cv::Mat img_pt;
	
	img_pt = cv::Mat::zeros(*img_size, CV_8UC1);
	img_pt.setTo(cv::Scalar::all(0));
	for ( i = 0; i < num; i++ ) {
		cv::circle( img_pt, ps[i], 3, cWhite, -1, cv::LINE_AA, 0 );
	}
	
	return img_pt;
}

cv::Mat DrawPointsOverlap( cv::Point *ps, int num, cv::Size *img_size, cv::Mat img )
// 特徴点を描画する
{
	int i;
	cv::Mat img_pt;
	
	img_pt = img.clone();
	for ( i = 0; i < num; i++ ) {
		cv::circle( img_pt, ps[i], 3, cRed, -1, cv::LINE_AA, 0 );
	}
	
	return img_pt;
}

int OutPutImage(cv::Mat img)
// 画像をファイルに保存する．ファイル名は自動的に連番になる
{
	static int n = 0;
	char filename[kFileNameLen];

	sprintf(filename, "output%03d.jpg", n++);
	return cv::imwrite(filename, img);
}

void DrawCor( cv::Point corps[][2], int corpsnum, cv::Mat img_cap, char *doc_name )
// 対応点の描画
{
	int i;
	char thumb_fname[kMaxPathLen];
	cv::Mat query_small, reg_small, cor_img;
	cv::Size qs, rs, cor_size;
	cv::Point p1, p2;
	
	sprintf( thumb_fname, "\\\\leo\\nakai\\didb\\thumb\\%s.bmp", doc_name );
	printf("%s\n", thumb_fname );
	if ( doc_name[0] == '\0')	return;
	reg_small = cv::imread( thumb_fname, cv::IMREAD_COLOR);
	
	qs.width = (int)(img_cap.cols / 2);
	qs.height = (int)(img_cap.rows / 2);
	rs.width = reg_small.cols;
	rs.height = reg_small.rows;
	
	cor_size.width = qs.width + rs.width;
	cor_size.height = (qs.height > rs.height) ? qs.height : rs.height;
	
	cor_img = cv::Mat::zeros(cor_size, CV_8UC3);
	for ( i = 0; i < corpsnum; i++ ) {
		p1.x = (int)(corps[i][0].x / 2);
		p1.y = (int)(corps[i][0].y / 2);
		p2.x = (int)(corps[i][1].x / 4) + qs.width;
		p2.y = (int)(corps[i][1].y / 4);
		cv::line( cor_img, p1, p2, cWhite, 1, cv::LINE_AA, 0 );
	}
	cv::imshow( "Corres", cor_img ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
}

void DrawSash( cv::Mat img, strProjParam *param )
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
		cv::line( img, cv::Point( p2[i].x, p2[i].y ), cv::Point( p2[i+1].x, p2[i+1].y ), cRed, 5, cv::LINE_AA, 0 );
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
	
	cv::Point *ps = NULL, *ps0 = NULL, corps[kMaxPointNum][2], ar_p1, ar_p2, point;
	double *areas = NULL, *areas0 = NULL;
	cv::Size img_size, query_size, reg_size, thumb_size, merge_size, small_img_size, small_thumb_size, res_size, resized_size;
	strDirectShowCap dsc;
	cv::Mat img_cap, img_pt, img_cap0, tmp, thumb, thumb_draw, merge, small_cap, small_thumb, clone, cap_file, cap_resized, img_con, img_con0, img_disp;
	cv::Mat flag;
	// 表示画像関係
	cv::Mat disp_cap, disp_con;
	cv::Point *disp_ps;
	int disp_num;
	// フォーカス関係
	CameraControlProperty prop;
	int focus_available, auto_focus_tb, focus_tb;
	long focus, focus_min, focus_max, focus_step, focus_def, focus_flags;
	// ガウシアンフィルタのトラックバー関係
	int gauss_tb;
#ifdef VIDEO_OUTPUT
	cv::Size small_pt_size;
	cv::Mat small_pt;
#endif
	strProjParam param, zero_param;;
	struct cv::VideoWriter video_wr;	// ビデオ出力用
	struct sockaddr_in addr;
	cv::VideoCapture cap_mov;	// ビデオ入力用
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
	// デュアルハッシュモードは別関数
	if ( eDualHash ) {
		RetrieveDualHash();
		return 0;
	}
	if ( eEntireMode == CAM_RET_MODE ) {
		ret = InitDirectShowCap( eCamConfNum, &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
		if ( ret == 0 )	return 1;
	}
	else if ( eEntireMode == INPUT_MOVIE_MODE ) {
		cap_mov = cv::VideoCapture( eMovieFileName );	// ファイルからのキャプチャ
		if ( !cap_mov.isOpened() )	return 1;	// キャプチャ失敗
		img_size.width = (int)cap_mov.get(cv::CAP_PROP_FRAME_WIDTH);	// サイズ読み込み
		img_size.height = (int)cap_mov.get(cv::CAP_PROP_FRAME_HEIGHT);
	}
	buff_size = img_size.width * img_size.height * 3;
	thumb_size = cv::Size( 425, 550 );	// サムネイルのサイズ
	// ビデオ用の縮小サイズ
	small_img_size = cv::Size( (int)(img_size.width * kResizeScaleImg), (int)(img_size.height * kResizeScaleImg) );
	small_thumb_size = cv::Size( (int)(thumb_size.width * kResizeScaleThumb), (int)(thumb_size.height *kResizeScaleThumb) );
//	merge_size = cv::Size( img_size.width + thumb_size.width, (std::max)( img_size.height, thumb_size.height ) );
	merge_size = cv::Size( small_img_size.width + small_thumb_size.width, (std::max)( small_img_size.height, small_thumb_size.height ) );
	// アスペクト比に合わせる
	if ( merge_size.width > (int)((double)merge_size.height * kVideoAspectRatio) )
		merge_size.height = (int)((double)merge_size.width / kVideoAspectRatio);
	else if ( merge_size.width < (int)((double)merge_size.height * kVideoAspectRatio) )
		merge_size.width = (int)((double)merge_size.height * kVideoAspectRatio);
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
	img_cap0 = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
	if ( eCaptureWidth != 0 )	{	// キャプチャウィンドウの幅指定が有効
		resized_size.width = eCaptureWidth;
		resized_size.height = (int)((double)img_size.height * ((double)eCaptureWidth / (double)img_size.width));
		cap_resized = cv::Mat::zeros(resized_size, CV_8UC3);
	}
	img_con = cv::Mat::zeros(img_size, CV_8UC1);
	img_con0 = cv::Mat::zeros(img_size, CV_8UC1);
	img_disp = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
#ifdef	VIDEO_OUTPUT
	small_pt_size.width = 212;
	small_pt_size.height = 159;
	small_cap = cv::Mat::zeros(small_img_size, CV_8UC3);
	small_thumb = cv::Mat::zeros(small_thumb_size, CV_8UC3);
	merge = cv::Mat::zeros(merge_size, CV_8UC3);
	small_pt = cv::Mat::zeros(small_pt_size, CV_8UC3);
#endif
#ifdef	AR
	ar = 1;
#else
	ar = -1;
#endif
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	cv::namedWindow( "Capture", cv::WINDOW_AUTOSIZE );	// ウィンドウを作成
//	cv::namedWindow( "Connected", cv::WINDOW_AUTOSIZE );
//	cv::namedWindow( "Points", cv::WINDOW_AUTOSIZE );
#ifdef	DRAW_COR	// 対応点描画
	cv::namedWindow( "Corres", cv::WINDOW_AUTOSIZE );
#else
	cv::namedWindow( "Thumb", cv::WINDOW_AUTOSIZE );
#endif
#ifdef	DRAW_HIST	// ヒストグラム描画
	cv::namedWindow( "Hist", cv::WINDOW_AUTOSIZE );
#endif
#ifdef	DRAW_FLAG
	cv::namedWindow( "Language", cv::WINDOW_AUTOSIZE );
#endif
	////////////////////////// フォーカスのトラックバー関係 //////////////////////////
	prop = CameraControl_Focus;
	focus_available = GetCameraControlRange( dsc.pCameraControl, prop, &focus_min, &focus_max, &focus_step, &focus_def, &focus_flags );
//	// ペンカメラのために一時的にフォーカス関係を殺している
//	focus_available = 0;
	if ( focus_available )	focus = GetCameraControlProperty( dsc.pCameraControl, prop, &focus_flags );
	// 取得したプロパティからトラックバーを作成する
	if ( focus_available ) {
//		auto_focus_tb = ( focus_flags == CameraControl_Flags_Auto ) ? 1 : 0;
		focus_tb = (int)(focus - focus_min);
//		cv::createTrackbar( "Manu/Auto", "Capture", &auto_focus_tb, 1, NULL );
		cv::createTrackbar( "Focus", "Capture", &focus_tb, focus_max - focus_min, NULL );
	}
	////////////////////////// ガウシアンマスクサイズのトラックバー関係 //////////////////////////
#ifdef	GAUSS_VARIABLE
	gauss_tb = (int)(eCamGaussMaskSize / 2) + 1;
//	cv::createTrackbar( "Gauss", "Capture", &gauss_tb, 10, NULL );
	eCamGaussMaskSize = gauss_tb * 2 + 1;
#endif

	if ( eEntireMode == CAM_RET_MODE )
		StartDirectShowCap( &dsc );	// キャプチャを開始
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
	video_wr = cv::VideoWriter( kVideoFileName, -1, kVideoFps, merge_size, 1 );	// ビデオ出力
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
			cap_file = [&]() -> cv::Mat { cv::Mat f; cap_mov.read(f); return f; }();
			end_cap = GetProcTimeMiliSec();
			if ( cap_file.empty() )	goto end_cap;
			CopyImageData( cap_file, img_cap );
		}
		// 結合画像を作成
//		start_fp = GetProcTimeMiliSec();
//		start_connect = GetProcTimeMiliSec();
		if ( eIsJp )	img_con = GetConnectedImageCamJp2( img_cap );
		else			GetConnectedImageCam( img_cap, img_con, eCamGaussMaskSize );
//		end_connect = GetProcTimeMiliSec();
		if ( ps0 != NULL && num0 > 0 )	ReleaseCentres( ps0 );
		if ( areas0 != NULL && num0 > 0 )	free( areas0 );
		ps0 = ps;
		areas0 = areas;
		num0 = num;
		// フォーカス連動ガウシアンパラメータ
//		if ( eFocusCoupledGaussian )	eCamGaussMaskSize = (int)(0.0652 * (double)focus_tb - 2)*2+1;	// 単語の場合
		if ( eFocusCoupledGaussian )	eCamGaussMaskSize = (int)(0.0326 * (double)focus_tb - 2)*2+1;	// 文字の場合
		if ( eCamGaussMaskSize < 1 )	eCamGaussMaskSize = 1;
		printf("%d, %d\n", focus_tb, eCamGaussMaskSize);
		// 結合画像から重心を計算する
		num = MakeCentresFromImage( &ps, img_con, &img_size, &areas );
		end_fp = GetProcTimeMiliSec();
//		printf("cap : %dms\nfp : %dms\n", end_cap - start_cap, end_fp - start_fp);
//		if ( !img_pt.empty() ) // cvReleaseImage removed (cv::Mat auto-manages memory)
		// 表示画像の作成
		if ( draw_fp > 0 || draw_con > 0)	disp_cap = cv::Mat();
		else	disp_cap = img_cap0;
		if ( draw_con > 0 || draw_con_overlap > 0 )	disp_con = img_con0;
		else	disp_con = cv::Mat();
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
			img_pt = DrawPoints( ps0, num0, &img_size );
		}
		else if ( draw_fp_overlap > 0 ) {
			img_pt = DrawPointsOverlap( ps0, num0, &img_size, img_cap0 );
		}*/
//		cv::imshow( "Points", img_pt );
//		DrawPoints2( ps, num, img_size );
//		printf("cap : %dms\nconv : %dms\nshow : %dms\nconnect : %dms\nfp : %d\n", end_cap - start_cap, end_conv - start_conv, end_show - start_show, end_connect - start_connect, end_fp - start_fp );
//		cv::waitKey( 1 );	// ないとウィンドウが更新されない
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
//			if ( !thumb.empty() ) {
//				cv::imshow( "Thumb", thumb );
// // cvReleaseImage removed (cv::Mat auto-manages memory)
//			}
//			strcpy( doc_name_prev, doc_name );
//		}
		cv::imshow( "Thumb", thumb_draw );
#ifdef	DRAW_FLAG
		// 言語の画像を表示
		printf( "%x\n", doc_name );
		flag = GetFlagImage( doc_name );
		cv::imshow( "Language", flag );
#endif

#ifdef	THUMB_OUT
		OutPutImage( thumb_draw );
#endif
#endif
//		DrawParam( img_cap0, thumb, param, zero_param );
		if ( ar > 0 ) {	// 拡張現実の描画
//			if ( !clone.empty() ) // cvReleaseImage removed (cv::Mat auto-manages memory)
//			clone = img_cap0.clone();	// 必要？　いらないなら消す
			DrawAR( img_disp, doc_name, param );	// ARを描画
//			if ( draw_fp > 0 || draw_fp_overlap > 0 ) 			DrawAR( img_pt, doc_name, param );	// ARを描画
//			else						DrawAR( img_cap0, doc_name, param );	// ARを描画
//			OutPutImage( img_cap0 );
		}
#endif
		strcpy( doc_name_prev, doc_name );
#ifdef	VIDEO_OUTPUT
		cv::resize( img_cap0, small_cap, small_cap.size(), 0, 0, cv::INTER_NEAREST );
		cv::resize( thumb_draw, small_thumb, small_thumb.size(), 0, 0, cv::INTER_NEAREST );
		// 縮小特徴点画像の作成
		small_pt.setTo(cv::Scalar::all(0));
		cv::bitwise_not( small_pt, small_pt );
		for ( i = 0; i < num; i++ ) {
			point.x = (int)((double)ps[i].x * (double)small_pt_size.width / (double)img_size.width);
			point.y = (int)((double)ps[i].y * (double)small_pt_size.height / (double)img_size.height);
			cv::circle( small_pt, point, 1, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );
		}
		MergeImage3( small_cap, small_thumb, small_pt, merge );
		// キャプチャ画像をビデオに出力
		video_wr.write(merge);
#endif
		// 枠を描画
//		if ( !IsEqualParam( param, zero_param ) )	DrawSash( img_cap0, &param );
		// キャプチャウィンドウの描画
		if ( eCaptureWidth == 0 ) {	// 幅指定なし
			cv::imshow( "Capture", img_disp );
		}
		else {	// 幅指定あり
			cv::resize( img_disp, cap_resized, cap_resized.size(), 0, 0, cv::INTER_LINEAR );	// リサイズ
			cv::imshow( "Capture", cap_resized );
		}
/*		if ( eCaptureWidth == 0 ) {	// 幅指定なし
			if ( draw_fp > 0 || draw_fp_overlap > 0 )	cv::imshow( "Capture", img_pt );
			else if ( draw_con > 0 && !img_con0.empty() )	cv::imshow( "Capture", img_con0 );
			else				cv::imshow( "Capture", img_cap0 );
		}
		else {
			if ( draw_fp > 0 || draw_fp_overlap > 0 )	cv::resize( img_pt, cap_resized, cap_resized.size(), 0, 0, cv::INTER_LINEAR );
			else if ( draw_con > 0 && !img_con0.empty() )	cv::resize( img_con0, cap_resized, cap_resized.size(), 0, 0, cv::INTER_LINEAR );
			else				cv::resize( img_cap0, cap_resized, cap_resized.size(), 0, 0, cv::INTER_LINEAR );
			cv::imshow( "Capture", cap_resized );
		}*/
		end = GetProcTimeMiliSec();
//		printf("Total : %dms\n", end - start );
		// フォーカスのトラックバー関係
		if ( focus_available ) {
//			if ( auto_focus_tb == 0 )
				focus_flags = 2;	// マニュアル
//			else
//				focus_flags = 3;		// オート
			SetCameraControlProperty( dsc.pCameraControl, prop, focus_tb + focus_min, focus_flags );
			focus = GetCameraControlProperty( dsc.pCameraControl, prop, &focus_flags );
			// トラックバー変数にフィードバック
			if ( focus_flags == 3 )	auto_focus_tb = 1;
			else	auto_focus_tb = 0;
			focus_tb = focus - focus_min;
//			cv::setTrackbarPos( "Manu/Auto", "Capture", auto_focus_tb );
			cv::setTrackbarPos( "Focus", "Capture", focus_tb );
//			printf( "%ld:%ld\n", focus_flags, focus );
		}
		// ガウシアンフィルタのトラックバー関係
#ifdef	GAUSS_VARIABLE
		gauss_tb = (int)((double)focus * 0.0565 - 3.5326);
		if ( gauss_tb < 0 )	gauss_tb = 0;
		eCamGaussMaskSize = gauss_tb * 2 + 1;
//		cv::setTrackbarPos( "Gauss", "Capture", gauss_tb );
#endif
		// キー入力関係
		start = end;
		key = cv::waitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'p':	// 一時停止
					if ( eEntireMode == CAM_RET_MODE )
						PauseDirectShowCap( &dsc );
					printf("Hit any key to resume\n");
					for ( ; cv::waitKey(100) < 0; );
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
							SaveAR( doc_name, img_cap, cv::Size( (int)((double)thumb.cols / eThumbScale), (int)((double)thumb.rows / eThumbScale)), &ar_p1, &ar_p2 );
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
	cv::destroyWindow( "Capture" );	// ウィンドウを破棄
//	cv::destroyWindow( "Connected" );
//	cv::destroyWindow( "Points" );
#ifdef	DRAW_COR
	cv::destroyWindow( "Corres" );
#else
	cv::destroyWindow( "Thumb" );
#endif
#ifdef	DRAW_HIST
	cv::destroyWindow( "Hist" );
#endif
	if ( eEntireMode == CAM_RET_MODE ) {
		StopDirectShowCap( &dsc );	// キャプチャを終了
		ReleaseDirectShowCap( &dsc );	// DirectShowの諸々をリリース
	}
	else if ( eEntireMode == INPUT_MOVIE_MODE ) {
		// cvReleaseCapture removed (VideoCapture auto-releases);
	} // cvReleaseImage removed (cv::Mat auto-manages memory)
// // cvReleaseImage removed (cv::Mat auto-manages memory)
// // cvReleaseImage removed (cv::Mat auto-manages memory)
	ReleaseCentres( ps );
	free( areas );
	free( img_buff );
#ifdef	VIDEO_OUTPUT
	// cvReleaseVideoWriter removed (VideoWriter auto-releases);
#endif
	timeEndPeriod( 1 );

	return 0;
}

int GotoSnapshotMode( strDirectShowCap *dsc, unsigned char *img_buff, long buff_size, cv::Mat img_cap, cv::Mat thumb )
// スナップショット撮影モード
{
	int key;

	printf( "*** Snapshot mode ***\n");
	printf("Capture(c) Quit(q)\n");
	for ( ; ; ) {
		CaptureDirectShowCap( dsc, img_buff, buff_size );
		Buff2ImageData( img_buff, img_cap );
		cv::imshow( "Capture", img_cap );
		cv::imshow( "Thumb", thumb );
		key = cv::waitKey(1);
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

int GotoPasteMode( cv::Mat img_cap, cv::Mat thumb, cv::Point *p1, cv::Point *p2 )
// 貼り付けモード
{
	int key;

	g_thumb = thumb.clone();
	g_thumb_draw = thumb.clone();
	g_ratio = (double)img_cap.rows / (double)img_cap.cols;
	g_paste_complete = 0;

	printf("*** Paste mode ***\n");
	printf("Drag on \"Thumb\" window to paste the snapshot\n");
	cv::setMouseCallback( "Thumb", OnMouseThumb, 0 );

	cv::imshow( "Thumb", g_thumb_draw );

	for ( ; ; ) {
		key = cv::waitKey(1);
		switch ( key ) {
			case 'q':
				cv::imshow( "Thumb", thumb ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
			
				cv::setMouseCallback( "Thumb", NULL, 0 );
				return 0;
			case 's':
				goto paste_end;
			default:
				break;
		}
	}
paste_end:

	cv::imshow( "Thumb", thumb ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)

	cv::setMouseCallback( "Thumb", NULL, 0 );

	if ( g_paste_complete == 1 ) {
		p1->x = (std::min)( (int)((double)g_p1.x/eThumbScale), (int)((double)g_p2.x/eThumbScale) );
		p1->y = (std::min)( (int)((double)g_p1.y/eThumbScale), (int)((double)g_p2.y/eThumbScale) );
		p2->x = (std::max)( (int)((double)g_p1.x/eThumbScale), (int)((double)g_p2.x/eThumbScale) );
		p2->y = (std::max)( (int)((double)g_p2.y/eThumbScale), (int)((double)g_p2.y/eThumbScale) );
		return 1;
	}
	else {
		return 0;
	}
}

void OnMouseThumb( int event, int x, int y, int flags, void *param )
// マウスのコールバック関数
{
//	static cv::Point p1, p2;
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
		case cv::EVENT_LBUTTONDOWN:
			g_p1 = cv::Point( x, y );
			status = 1;	// マウスが押された
			g_paste_complete = 0;
			break;
		case cv::EVENT_LBUTTONUP:
			if ( status == 1 ) { // cvReleaseImage removed (cv::Mat auto-manages memory)
				g_thumb_draw = g_thumb.clone();
				cv::rectangle( g_thumb_draw, g_p1, g_p2, CV_RGB( 255, 0, 0 ), 2, 8, 0 );
				status = 0;
				cv::imshow( "Thumb", g_thumb_draw );
			}
			g_paste_complete = 1;
			break;
		case cv::EVENT_MOUSEMOVE:
			if ( status == 1 ) { // cvReleaseImage removed (cv::Mat auto-manages memory)
				g_thumb_draw = g_thumb.clone();
				cv::rectangle( g_thumb_draw, g_p1, g_p2, CV_RGB( 0, 0, 0 ), 1, 8, 0 );
				cv::imshow( "Thumb", g_thumb_draw );
			}
			break;
	}
}

int LoadPointFile( char *fname, cv::Point **ps0, cv::Size *size )
// 点データを読み込む
{
	int num;
	char line[kMaxLineLen];
	FILE *fp;
	cv::Point *ps;
	
	if ( (fp = fopen(fname, "r")) == NULL )	return 0;
	fgets(line, kMaxLineLen, fp);
	sscanf(line, "%d,%d", &(size->width), &(size->height));
	ps = (cv::Point *)calloc(kMaxPointNum, sizeof(cv::Point));
	*ps0 = ps;
	num = 0;
	while ( fgets(line, kMaxLineLen, fp) != NULL && num < kMaxPointNum ) {
		sscanf(line, "%d,%d", &(ps[num].x), &(ps[num].y));
		num++;
	}
	fclose(fp);
	return num;
}

cv::Mat LoadThumb( char *fname )
// fnameで指定されたサムネイル画像をロードする
{
	char thumb_fname[kMaxPathLen];

	if ( *fname == '\0' )	return cv::Mat();	// fnameが空文字列
//	sprintf( thumb_fname, "%s%s.%s", kThumbDir, fname, kThumbSuffix );
	sprintf( thumb_fname, "%s%s.%s", eThumbDir, fname, eThumbSuffix );
	puts(thumb_fname);
	return cv::imread( thumb_fname, cv::IMREAD_COLOR);
}

#define	kDrawParamScale	(0.5)
#define	kDrawParamHMargin	(100)
#define	kDrawParamVMargin	(100)
#define	kDrawParamSpace	(400)
#define	kDrawParamRectThick	(4)
#define	kDrawParamLineThick	(4)

void DrawParam( cv::Mat img_cap, cv::Mat thumb, strProjParam param, strProjParam zero_param )
// キャプチャ画像とサムネイルとパラメータを示す枠の対応を描画して出力
{
	int i, width_all, height_all;
	cv::Mat img, thumb_res;
	strPoint p1[5], p2[5];

	// サムネイルの拡大版を作る
	thumb_res = cv::Mat::zeros( cv::Size( (int)(thumb.cols / eThumbScale * kDrawParamScale), (int)(thumb.rows / eThumbScale * kDrawParamScale) ), CV_8UC3 );
	cv::resize( thumb, thumb_res, thumb_res.size(), 0, 0, cv::INTER_LINEAR );
	// 全体画像を作る
	width_all = kDrawParamHMargin * 2 + kDrawParamSpace + img_cap.cols + thumb_res.cols;
	height_all = kDrawParamVMargin * 2 + (std::max)( img_cap.rows, thumb_res.rows );
	img = cv::Mat::zeros(cv::Size( width_all, height_all ), CV_8UC3);
	img.setTo( cWhite );	// 白で塗りつぶす
	// キャプチャ画像を配置する
	PutImage( img_cap, img, cv::Point( kDrawParamHMargin, kDrawParamVMargin ) );
	// 拡大サムネイルを配置する
	PutImage( thumb_res, img, cv::Point( kDrawParamHMargin + img_cap.cols + kDrawParamSpace, kDrawParamVMargin ) );
	// サムネイルの枠を描画
	cv::rectangle( img, cv::Point( kDrawParamHMargin + img_cap.cols + kDrawParamSpace, kDrawParamVMargin), \
		cv::Point( kDrawParamHMargin + img_cap.cols + kDrawParamSpace + thumb_res.cols, kDrawParamVMargin + thumb_res.rows ), \
		cBlack, kDrawParamRectThick, cv::LINE_AA, 0 );
	// キャプチャ画像の枠を描画
	cv::rectangle( img, cv::Point( kDrawParamHMargin, kDrawParamVMargin ), cv::Point( kDrawParamHMargin + img_cap.cols, kDrawParamVMargin + img_cap.rows ), \
		cRed, kDrawParamRectThick, cv::LINE_AA, 0 );
	// 四隅の対応線を描画
	if ( !IsEqualParam( param, zero_param ) ) {	// パラメータ計算に成功
		// 枠を描画
		p1[0].x = 0;
		p1[0].y = 0;
		p1[1].x = img_cap.cols;
		p1[1].y = 0;
		p1[2].x = img_cap.cols;
		p1[2].y = img_cap.rows;
		p1[3].x = 0;
		p1[3].y = img_cap.rows;
		p1[4].x = 0;
		p1[4].y = 0;
		for ( i = 0; i < 5; i++ ) {
			ProjTrans( &(p1[i]), &(p2[i]), &param );
			p2[i].x = (int)(p2[i].x * kDrawParamScale);
			p2[i].y = (int)(p2[i].y * kDrawParamScale);
		}
		for ( i = 0; i < 4; i++ )	cv::line( img, cv::Point( kDrawParamHMargin + img_cap.cols + kDrawParamSpace + p2[i].x, kDrawParamVMargin + p2[i].y ), cv::Point( kDrawParamHMargin + img_cap.cols + kDrawParamSpace + p2[i+1].x, kDrawParamHMargin + p2[i+1].y ), cRed, kDrawParamLineThick, cv::LINE_AA, 0 );
		for ( i = 0; i < 4; i++ )	cv::line( img, cv::Point( kDrawParamHMargin + p1[i].x, kDrawParamVMargin + p1[i].y ), cv::Point( kDrawParamHMargin + img_cap.cols + kDrawParamSpace + p2[i].x, kDrawParamVMargin + p2[i].y ), cRed, kDrawParamLineThick, cv::LINE_AA, 0 );
	}


	OutPutImage( img );
//	OutPutImage( thumb_res ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
}

void DrawThumb( char *doc_name, char *doc_name_prev, cv::Mat *thumb0, cv::Mat *thumb_draw0, strProjParam param, strProjParam zero_param, cv::Size cap_size, cv::Size res_size )
// サムネイルを枠つきで描画
{
	int i;
	char thumb_fname[kMaxPathLen];
	cv::Mat thumb, thumb_orig, thumb_draw;
	cv::Size thumb_size;
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
		thumb = cv::Mat::zeros(thumb_size, CV_8UC3);	// 縮小画像を作成
		thumb.setTo( CV_RGB(255,255,255) );	// 白にクリア
		sprintf( thumb_fname, "%s%s.%s", eThumbDir, doc_name, eThumbSuffix );
//		puts( thumb_fname );
		if ( !( thumb_orig = cv::imread( thumb_fname, cv::IMREAD_COLOR) ).empty() ) {	// サムネイル画像をロードし，
			cv::resize( thumb_orig, thumb, thumb.size(), 0, 0, cv::INTER_NEAREST );	// リサイズして表示 // cvReleaseImage removed (cv::Mat auto-manages memory)
		}
		*thumb0 = thumb;
	}
	thumb_draw = thumb.clone();	// クローンを作成
	*thumb_draw0 = thumb_draw;
	if ( !IsEqualParam( param, zero_param ) ) {	// パラメータ計算に成功
		// 枠を描画
//		cv::line( thumb_draw, cv::Point(0,0), cv::Point(100,100), cRed, 4, cv::LINE_AA, 0 );
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
		for ( i = 0; i < 4; i++ )	cv::line( thumb_draw, cv::Point( p2[i].x, p2[i].y ), cv::Point( p2[i+1].x, p2[i+1].y ), cRed, 5, cv::LINE_AA, 0 );
	}
#if 0
	thumb = *thumb0;
	thumb_draw = *thumb_draw0;
	if ( doc_name[0] == '\0' ) {	// 検索に失敗
		strcpy( doc_name, "0" );		// 0.bmpを表示
	}
	if ( strcmp( doc_name, doc_name_prev ) ) {	// 前回と検索結果が異なる
//		puts("start load thumb");
		thumb = LoadThumb( doc_name );		// ロードする
//		puts("finish load thumb");
		*thumb0 = thumb;
	}
	thumb_draw = thumb.clone();	// クローンを作成
	*thumb_draw0 = thumb_draw;

	if ( !IsEqualParam( param, zero_param ) ) {	// パラメータ計算に成功
		// 枠を描画
//		cv::line( thumb_draw, cv::Point(0,0), cv::Point(100,100), cRed, 4, cv::LINE_AA, 0 );
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
		for ( i = 0; i < 4; i++ )	cv::line( thumb_draw, cv::Point( p2[i].x, p2[i].y ), cv::Point( p2[i+1].x, p2[i+1].y ), cRed, 5, cv::LINE_AA, 0 );
	}
#endif
}

void DrawCor2( cv::Point corps[][2], int corpsnum, cv::Mat img_cap, char *doc_name )
// 対応点の描画２（キャプチャ画像とサムネイル付き）
{
	int i;
	char thumb_fname[kMaxPathLen];
	cv::Mat cor_img, thumb;
	cv::Size qs, rs, cor_size;
	cv::Point p1, p2;
	
	if ( doc_name[0] == '\0' ) {	// 検索に失敗
		strcpy( doc_name, "0" );		// 0.bmpを表示
	}
//	puts("start load thumb");
	thumb = LoadThumb( doc_name );		// ロードする
//	puts("finish load thumb");

	cor_size = cv::Size( img_cap.cols + thumb.cols, (std::max)( img_cap.rows, thumb.rows ) );
	cor_img = cv::Mat::zeros(cor_size, CV_8UC3);
	MergeImage( img_cap, thumb, MERGE_HOR, cor_img );

	for ( i = 0; i < corpsnum; i++ ) {
		p1.x = (int)(corps[i][0].x);
		p1.y = (int)(corps[i][0].y);
		p2.x = (int)(corps[i][1].x * eThumbScale) + img_cap.cols;
		p2.y = (int)(corps[i][1].y * eThumbScale);
		cv::line( cor_img, p1, p2, cBlack, 1, cv::LINE_AA, 0 );
//		cv::line( cor_img, p1, p2, cWhite, 1, cv::LINE_AA, 0 );
	}
	cv::imshow( "Corres", cor_img );
	OutPutImage( cor_img ); // cvReleaseImage removed (cv::Mat auto-manages memory) // cvReleaseImage removed (cv::Mat auto-manages memory)
}

int SaveAR( char *doc_name, cv::Mat img_cap, cv::Size size, cv::Point *p1, cv::Point *p2 )
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
	cv::imwrite( img_path, img_cap );

	return 1;
}

int CaptureMovie( char *movfile )
// 動画の撮影
{
	int key, frame_msec;
	long buff_size;
	unsigned char *img_buff;
	cv::Size img_size;
	cv::Mat img_cap;
	strDirectShowCap dsc;
	struct cv::VideoWriter video_wr;	// ビデオ出力用
	TIME_COUNT start, end, start_cap, end_cap, start_show, end_show, start_connect, end_connect, start_conv, end_conv, start_ret, end_ret, start_fp, end_fp, start_com, end_com;

	timeBeginPeriod( 1 );//精度を1msに設定

	if ( ReadIniFile() == 0 ) {	// iniファイルの読み込みに失敗
		fprintf( stderr, "Error : ReadIniFile\n" );
		return 1;
	}

	printf("%d\n", eCamConfNum);
	InitDirectShowCap( eCamConfNum, &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
	buff_size = img_size.width * img_size.height * 3;
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成

	StartDirectShowCap( &dsc );	// キャプチャを開始
	cv::namedWindow( "Capture", cv::WINDOW_AUTOSIZE );	// ウィンドウを作成

	video_wr = cv::VideoWriter( movfile, -1, kVideoFps, img_size, 1 );	// ビデオ出力
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
		cv::imshow( "Capture", img_cap );
		end_show = GetProcTimeMiliSec();
//		OutPutImage( img_cap );
		// キャプチャ画像をビデオに出力
		video_wr.write(img_cap);
		key = cv::waitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'p':	// 一時停止
					PauseDirectShowCap( &dsc );
					printf("Hit any key to resume\n");
					for ( ; cv::waitKey(100) < 0; );
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
	cv::destroyWindow( "Capture" );	// ウィンドウを破棄
	StopDirectShowCap( &dsc );	// キャプチャを終了
	ReleaseDirectShowCap( &dsc );	// DirectShowの諸々をリリース // cvReleaseImage removed (cv::Mat auto-manages memory)
	free( img_buff );
	// cvReleaseVideoWriter removed (VideoWriter auto-releases);
	timeEndPeriod( 1 );

	return 0;
}

int DecomposeMovie( char *mov_file )
// 動画ファイルを分解し，画像を出力
{
	int i;
	cv::Mat img_cap;
	cv::VideoCapture cap_mov;
	cv::Size img_size;

	cap_mov = cv::VideoCapture( mov_file );
	img_size.width = (int)cap_mov.get(cv::CAP_PROP_FRAME_WIDTH);
	img_size.height = (int)cap_mov.get(cv::CAP_PROP_FRAME_HEIGHT);
	printf("%d,%d\n", img_size.width, img_size.height );
	while ( !( img_cap = [&]() -> cv::Mat { cv::Mat f; cap_mov.read(f); return f; }() ).empty() ) {
		OutPutImage( img_cap );
	}
	// cvReleaseCapture removed (VideoCapture auto-releases);
	return 1;
}

void ConvMovie( void )
// 動画の変換
{
	cv::VideoCapture cap_mov;
	cv::Size img_size;
	double mov_fps;
	cv::VideoWriter video_wr;
	cv::Mat cap_file;

	cap_mov = cv::VideoCapture( eMovieFileName );
	if ( !cap_mov.isOpened() )	return;
	img_size.width = (int)cap_mov.get(cv::CAP_PROP_FRAME_WIDTH);	// サイズ読み込み
	img_size.height = (int)cap_mov.get(cv::CAP_PROP_FRAME_HEIGHT);
	mov_fps = cap_mov.get(cv::CAP_PROP_FPS);
	video_wr = cv::VideoWriter( eConvMovieFileName, -1, mov_fps, img_size, 1 );	// ビデオ出力
	cv::namedWindow( "Movie", cv::WINDOW_AUTOSIZE );
	for ( ; ; ) {
		cap_file = [&]() -> cv::Mat { cv::Mat f; cap_mov.read(f); return f; }();
		if ( cap_file.empty() )	break;
		cv::imshow( "Movie", cap_file );
		video_wr.write(cap_file);
	}
	// cvReleaseCapture removed (VideoCapture auto-releases);
	// cvReleaseVideoWriter removed (VideoWriter auto-releases);

	return;
}

cv::Mat LoadFlagImage( char *doc_name )
// doc_nameから国を判別し，国旗を読み込む
{
	char path[kMaxPathLen];
	char langs[][20] = { "arabic", "chinese", "english", "french", "hindi", "japanese", "korean", "lhao", "russian", "spanish", "tamil", "thai" };
	int i, num;
	cv::Mat img;

	num = sizeof( langs );
	for ( i = 0; i < num; i++ ) {
		if ( strncmp( doc_name, langs[i], strlen( langs[i] ) ) == 0 ) {
			sprintf( path, "%s%s.%s", kFlagDir, langs[i], kFlagSuffix );
			printf("%s\n", path);
			img = cv::imread( path, cv::IMREAD_COLOR);
			return img;
		}
	}
	return cv::Mat();
}

void MakeDispImage( cv::Mat img_disp, cv::Mat disp_cap, cv::Mat disp_con, cv::Point *disp_ps, int disp_num )
// 表示用画像を作成する
{
	int i;

	// キャプチャ画像
	if ( !disp_cap.empty() )	disp_cap.copyTo( img_disp );
	else	img_disp.setTo(cv::Scalar::all(0));
	// 連結成分
	if ( !disp_con.empty() )	img_disp.setTo( cGreen, disp_con );
	// 特徴点
	for ( i = 0; i < disp_num; i++ ) {
		cv::circle( img_disp, disp_ps[i], 3, cRed, -1, cv::LINE_AA, 0 );
	}
}

cv::Mat GetFlagImage( char *doc_name )
// ファイル名から国旗画像を得る
{
	char path[kMaxPathLen];
	char langs[13][20] = { "none", "arabic", "chinese", "english", "french", "hindi", "japanese", "korean", "lhao", "russian", "spanish", "tamil", "thai" };
	int i;
	static int loaded = 0;	// 画像をロードしたかどうかのフラグ
	static int num = 13;
	static std::vector<cv::Mat> flag_imgs;

	if ( loaded == 0 ) {	// まだ読み込んでいない
		flag_imgs.assign( num, cv::Mat() );
		for ( i = 0; i < num; i++ ) {
			sprintf( path, "%s%s.%s", kFlagDir, langs[i], kFlagSuffix );
			flag_imgs[i] = cv::imread( path, cv::IMREAD_COLOR);
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

int RetrieveDualHash( void )
{
	SOCKET sock, sockpt, sockres;
	int i, num1 = 0, num10 = 0, num2 = 0, num20 = 0, key, ret, corpsnum, ar, draw_fp = -1, draw_fp_overlap = -1, draw_con = -1, draw_con_overlap = -1;
	unsigned char *img_buff;
	char doc_name[kMaxDocNameLen] = "", doc_name_prev[kMaxDocNameLen] = "invalid";
	long buff_size;
	TIME_COUNT start, end, start_cap, end_cap, start_show, end_show, start_connect, end_connect, start_conv, end_conv, start_ret, end_ret, start_fp, end_fp, start_com, end_com, start_loop, end_loop, start_all, end_all;
	
	cv::Point *ps1 = NULL, *ps10 = NULL, *ps2 = NULL, *ps20 = NULL, corps[kMaxPointNum][2], ar_p1, ar_p2, point;
	double *areas1 = NULL, *areas10 = NULL, *areas2 = NULL, *areas20 = NULL;
	cv::Size img_size, query_size, reg_size, thumb_size, merge_size, small_img_size, small_thumb_size, res_size, resized_size;
	strDirectShowCap dsc;
	cv::Mat img_cap, img_pt, img_cap0, tmp, thumb, thumb_draw, merge, small_cap, small_thumb, clone, cap_file, cap_resized, img_con1, img_con10, img_con2, img_con20, img_disp;
	cv::Mat flag;
	// 表示画像関係
	cv::Mat disp_cap, disp_con;
	cv::Point *disp_ps;
	int disp_num;
	// フォーカス関係
	CameraControlProperty prop;
	int focus_available = 0, auto_focus_tb, focus_tb;
	long focus, focus_min, focus_max, focus_step, focus_def, focus_flags;
	// ガウシアンフィルタのトラックバー関係
	int gauss_tb;
#ifdef VIDEO_OUTPUT
	cv::Size small_pt_size;
	cv::Mat small_pt1, small_pt2;;
#endif
	strProjParam param, zero_param;;
	struct cv::VideoWriter video_wr;	// ビデオ出力用
	struct sockaddr_in addr;
	cv::VideoCapture cap_mov;	// ビデオ入力用
	// サムネイル保存用
	char **fname_list = NULL;
	int doc_num = 0;

	if ( eEntireMode == CAM_RET_MODE ) {
		ret = InitDirectShowCap( eCamConfNum, &dsc, &(img_size.width), &(img_size.height) );	// USBカメラを初期化
		if ( ret == 0 )	return 1;
	}
	else if ( eEntireMode == INPUT_MOVIE_MODE ) {
		cap_mov = cv::VideoCapture( eMovieFileName );	// ファイルからのキャプチャ
		if ( !cap_mov.isOpened() )	return 1;	// キャプチャ失敗
		img_size.width = (int)cap_mov.get(cv::CAP_PROP_FRAME_WIDTH);	// サイズ読み込み
		img_size.height = (int)cap_mov.get(cv::CAP_PROP_FRAME_HEIGHT);
	}
	buff_size = img_size.width * img_size.height * 3;
	thumb_size = cv::Size( 425, 550 );	// サムネイルのサイズ
	// ビデオ用の縮小サイズ
	small_img_size = cv::Size( (int)(img_size.width * kResizeScaleImg), (int)(img_size.height * kResizeScaleImg) );
	small_thumb_size = cv::Size( (int)(thumb_size.width * kResizeScaleThumb), (int)(thumb_size.height *kResizeScaleThumb) );
	merge_size = cv::Size( small_img_size.width + small_thumb_size.width, (std::max)( small_img_size.height, small_thumb_size.height ) );
	// アスペクト比に合わせる
	if ( merge_size.width > (int)((double)merge_size.height * kVideoAspectRatio) )
		merge_size.height = (int)((double)merge_size.width / kVideoAspectRatio);
	else if ( merge_size.width < (int)((double)merge_size.height * kVideoAspectRatio) )
		merge_size.width = (int)((double)merge_size.height * kVideoAspectRatio);
	img_buff = (unsigned char *)malloc( buff_size );	// 画像のバッファを確保
	img_cap = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
	img_cap0 = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
	if ( eCaptureWidth != 0 )	{	// キャプチャウィンドウの幅指定が有効
		resized_size.width = eCaptureWidth;
		resized_size.height = (int)((double)img_size.height * ((double)eCaptureWidth / (double)img_size.width));
		cap_resized = cv::Mat::zeros(resized_size, CV_8UC3);
	}
	img_con1 = cv::Mat::zeros(img_size, CV_8UC1);
	img_con10 = cv::Mat::zeros(img_size, CV_8UC1);
	img_con2 = cv::Mat::zeros(img_size, CV_8UC1);
	img_con20 = cv::Mat::zeros(img_size, CV_8UC1);
	img_disp = cv::Mat::zeros(img_size, CV_8UC3);	// 画像を作成
#ifdef	VIDEO_OUTPUT
	small_pt_size.width = 212;
	small_pt_size.height = 159;
	small_cap = cv::Mat::zeros(small_img_size, CV_8UC3);
	small_thumb = cv::Mat::zeros(small_thumb_size, CV_8UC3);
	merge = cv::Mat::zeros(merge_size, CV_8UC3);
	small_pt1 = cv::Mat::zeros(small_pt_size, CV_8UC3);
	small_pt2 = cv::Mat::zeros(small_pt_size, CV_8UC3);
#endif
#ifdef	AR
	ar = 1;
#else
	ar = -1;
#endif
	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	cv::namedWindow( "Capture", cv::WINDOW_AUTOSIZE );	// ウィンドウを作成
#ifdef	DRAW_COR	// 対応点描画
	cv::namedWindow( "Corres", cv::WINDOW_AUTOSIZE );
#else
	cv::namedWindow( "Thumb", cv::WINDOW_AUTOSIZE );
#endif
#ifdef	DRAW_HIST	// ヒストグラム描画
	cv::namedWindow( "Hist", cv::WINDOW_AUTOSIZE );
#endif
#ifdef	DRAW_FLAG
	cv::namedWindow( "Language", cv::WINDOW_AUTOSIZE );
#endif
	////////////////////////// フォーカスのトラックバー関係 //////////////////////////
	if ( eEntireMode == CAM_RET_MODE ) {
		prop = CameraControl_Focus;
		focus_available = GetCameraControlRange( dsc.pCameraControl, prop, &focus_min, &focus_max, &focus_step, &focus_def, &focus_flags );
//	// ペンカメラのために一時的にフォーカス関係を殺している
//	focus_available = 0;
		if ( focus_available )	focus = GetCameraControlProperty( dsc.pCameraControl, prop, &focus_flags );
	// 取得したプロパティからトラックバーを作成する
		if ( focus_available ) {
//		auto_focus_tb = ( focus_flags == CameraControl_Flags_Auto ) ? 1 : 0;
			focus_tb = (int)(focus - focus_min);
//		cv::createTrackbar( "Manu/Auto", "Capture", &auto_focus_tb, 1, NULL );
			cv::createTrackbar( "Focus", "Capture", &focus_tb, focus_max - focus_min, NULL );
		}
	}
	////////////////////////// ガウシアンマスクサイズのトラックバー関係 //////////////////////////
#ifdef	GAUSS_VARIABLE
	gauss_tb = (int)(eCamGaussMaskSize / 2) + 1;
//	cv::createTrackbar( "Gauss", "Capture", &gauss_tb, 10, NULL );
	eCamGaussMaskSize = gauss_tb * 2 + 1;
#endif

	if ( eEntireMode == CAM_RET_MODE )
		StartDirectShowCap( &dsc );	// キャプチャを開始
	// ネゴシエイト開始
	sock = InitWinSockClTCP( eTCPPort, eServerName );
	if ( sock == INVALID_SOCKET ) {
		fprintf( stderr, "error: connection failure\n" );
		return 1;
	}
	ret = SendComSetting( sock, eProtocol, ePointPort, eResultPort, eClientName );	// 通信設定を送信
	if ( ret < 0 )	return 1;

	if ( eProtocol == kUDP ) {	// UDP
		sockpt = InitWinSockClUDP( ePointPort, eServerName, &addr );
		sockres = InitWinSockSvUDP( eResultPort );
	}

#ifdef	VIDEO_OUTPUT
//	video_wr = cv::VideoWriter( kVideoFileName, -1, kVideoFps, merge_size, 1 );	// ビデオ出力
	video_wr = cv::VideoWriter( kVideoFileName, cv::VideoWriter::fourcc( 'X', 'V', 'I', 'D' ), kVideoFps, merge_size, 1 );	// ビデオ出力
#endif
#ifdef	VIDEO_RECORD
//	video_wr = cv::VideoWriter( "recorded.avi", -1, kVideoFps, img_size, 1 );	// ビデオ出力
	video_wr = cv::VideoWriter( "recorded.avi", cv::VideoWriter::fourcc( 'X', 'V', 'I', 'D' ), kVideoFps, img_size, 1 );	// ビデオ出力
#endif

	start = GetProcTimeMiliSec();
	start_all = GetProcTimeMiliSec();
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
			cap_file = [&]() -> cv::Mat { cv::Mat f; cap_mov.read(f); return f; }();
			end_cap = GetProcTimeMiliSec();
			if ( cap_file.empty() )	goto end_cap;
			CopyImageData( cap_file, img_cap );
		}
#ifdef	VIDEO_RECORD
		video_wr.write(img_cap);
#endif
		// 結合画像を作成
		// 文字単位
		GetConnectedImageCam( img_cap, img_con1, eCamGaussMaskSize1 );
		if ( ps10 != NULL && num10 > 0 )	ReleaseCentres( ps10 );
		if ( areas10 != NULL && num10 > 0 )	free( areas10 );
		ps10 = ps1;
		areas10 = areas1;
		num10 = num1;
		// 単語単位
		GetConnectedImageCam( img_cap, img_con2, eCamGaussMaskSize2 );
		if ( ps20 != NULL && num20 > 0 )	ReleaseCentres( ps20 );
		if ( areas20 != NULL && num20 > 0 )	free( areas20 );
		ps20 = ps2;
		areas20 = areas2;
		num20 = num2;
		// フォーカス連動ガウシアンパラメータ
		if ( eEntireMode == INPUT_MOVIE_MODE )	focus_tb = 112;	// 動画入力の時はプリセットで
		if ( eFocusCoupledGaussian )	eCamGaussMaskSize1 = (int)(0.0326 * (double)focus_tb - 2)*2+1;	// 文字の場合
//		if ( eFocusCoupledGaussian )	eCamGaussMaskSize2 = (int)(0.0652 * (double)focus_tb - 2)*2+1;	// 単語の場合
		if ( eFocusCoupledGaussian )	eCamGaussMaskSize2 = (int)(0.08 * (double)focus_tb - 2)*2+1;	// 単語の場合
		if ( eCamGaussMaskSize1 < 1 )	eCamGaussMaskSize1 = 1;
		if ( eCamGaussMaskSize2 < 1 )	eCamGaussMaskSize2 = 1;
		// 結合画像から重心を計算する
		num1 = MakeCentresFromImage( &ps1, img_con1, &img_size, &areas1 );
		num2 = MakeCentresFromImage( &ps2, img_con2, &img_size, &areas2 );
		printf("%d, %d\n", num1, num2);
		end_fp = GetProcTimeMiliSec();
//		printf("cap : %dms\nfp : %dms\n", end_cap - start_cap, end_fp - start_fp);
//		if ( !img_pt.empty() ) // cvReleaseImage removed (cv::Mat auto-manages memory)
		// 表示画像の作成
		if ( draw_fp > 0 || draw_con > 0)	disp_cap = cv::Mat();
		else	disp_cap = img_cap0;
		if ( draw_con > 0 || draw_con_overlap > 0 )	disp_con = img_con10;
		else	disp_con = cv::Mat();
		if ( draw_fp > 0 || draw_fp_overlap > 0 ) {
			disp_ps = ps10;
			disp_num = num10;
		} else {
			disp_ps = NULL;
			disp_num = 0;
		}
		MakeDispImage( img_disp, disp_cap, disp_con, disp_ps, disp_num );
		start_com = GetProcTimeMiliSec();
		// 結果を受信（受信を先にすることで、サーバとの並列処理が可能になる。なお一回目はダミーのデータが送られる）
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
			SendPointsAreas( sock, ps1, areas1, num1, &img_size, &addr, eProtocol );
			SendPointsAreas( sock, ps2, areas2, num2, &img_size, &addr, eProtocol );
		}
		else {
			SendPointsAreas( sockpt, ps1, areas1, num1, &img_size, &addr, eProtocol );
			SendPointsAreas( sockpt, ps2, areas2, num2, &img_size, &addr, eProtocol );
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
//		printf( "%x\n", doc_name );
#if 1
		DrawThumb( doc_name, doc_name_prev, &thumb, &thumb_draw, param, zero_param, img_size, res_size );
		cv::imshow( "Thumb", thumb_draw );
//		printf("thumb drawn\n");
#ifdef	DRAW_FLAG
		// 言語の画像を表示
		printf( "%x\n", doc_name );
		flag = GetFlagImage( doc_name );
		cv::imshow( "Language", flag );
#endif

#ifdef	THUMB_OUT
		OutPutImage( thumb_draw );
#endif
#endif
		if ( ar > 0 ) {	// 拡張現実の描画
			DrawAR( img_disp, doc_name, param );	// ARを描画
		}
#endif
		strcpy( doc_name_prev, doc_name );
#ifdef	VIDEO_OUTPUT
		cv::resize( img_cap0, small_cap, small_cap.size(), 0, 0, cv::INTER_NEAREST );
		cv::resize( thumb_draw, small_thumb, small_thumb.size(), 0, 0, cv::INTER_NEAREST );
		// 縮小特徴点画像の作成
		small_pt1.setTo(cv::Scalar::all(0));
		cv::bitwise_not( small_pt1, small_pt1 );
		for ( i = 0; i < num1; i++ ) {
			point.x = (int)((double)ps1[i].x * (double)small_pt_size.width / (double)img_size.width);
			point.y = (int)((double)ps1[i].y * (double)small_pt_size.height / (double)img_size.height);
			cv::circle( small_pt1, point, 1, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );
		}
		small_pt2.setTo(cv::Scalar::all(0));
		cv::bitwise_not( small_pt2, small_pt2 );
		for ( i = 0; i < num2; i++ ) {
			point.x = (int)((double)ps2[i].x * (double)small_pt_size.width / (double)img_size.width);
			point.y = (int)((double)ps2[i].y * (double)small_pt_size.height / (double)img_size.height);
			cv::circle( small_pt2, point, 1, CV_RGB(0,0,0), -1, cv::LINE_AA, 0 );
		}
#ifdef	POINT_OUTPUT
		OutPutImage( small_pt1 );
		OutPutImage( small_pt2 );
#endif
//		MergeImage3( small_cap, small_thumb, small_pt, merge );
		MergeImage4( small_cap, small_thumb, small_pt1, small_pt2, merge );
		// キャプチャ画像をビデオに出力
		video_wr.write(merge);
#endif
		// キャプチャウィンドウの描画
		if ( eCaptureWidth == 0 ) {	// 幅指定なし
			cv::imshow( "Capture", img_disp );
		}
		else {	// 幅指定あり
			cv::resize( img_disp, cap_resized, cap_resized.size(), 0, 0, cv::INTER_LINEAR );	// リサイズ
			cv::imshow( "Capture", cap_resized );
		}
		end = GetProcTimeMiliSec();
//		printf("Total : %dms\n", end - start );
		// フォーカスのトラックバー関係
		if ( focus_available ) {
//			if ( auto_focus_tb == 0 )
				focus_flags = 2;	// マニュアル
//			else
//				focus_flags = 3;		// オート
			SetCameraControlProperty( dsc.pCameraControl, prop, focus_tb + focus_min, focus_flags );
			focus = GetCameraControlProperty( dsc.pCameraControl, prop, &focus_flags );
			// トラックバー変数にフィードバック
			if ( focus_flags == 3 )	auto_focus_tb = 1;
			else	auto_focus_tb = 0;
			focus_tb = focus - focus_min;
//			cv::setTrackbarPos( "Manu/Auto", "Capture", auto_focus_tb );
			cv::setTrackbarPos( "Focus", "Capture", focus_tb );
//			printf( "%ld:%ld\n", focus_flags, focus );
		}
		// ガウシアンフィルタのトラックバー関係
#ifdef	GAUSS_VARIABLE
		gauss_tb = (int)((double)focus * 0.0565 - 3.5326);
		if ( gauss_tb < 0 )	gauss_tb = 0;
		eCamGaussMaskSize = gauss_tb * 2 + 1;
//		cv::setTrackbarPos( "Gauss", "Capture", gauss_tb );
#endif
		// キー入力関係
		start = end;
		key = cv::waitKey( 1 );	// ないとウィンドウが更新されない
		if ( key >= 0 ) {
			switch ( key ) {
				case 'p':	// 一時停止
					if ( eEntireMode == CAM_RET_MODE )
						PauseDirectShowCap( &dsc );
					printf("Hit any key to resume\n");
					for ( ; cv::waitKey(100) < 0; );
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
							SaveAR( doc_name, img_cap, cv::Size( (int)((double)thumb.cols / eThumbScale), (int)((double)thumb.rows / eThumbScale)), &ar_p1, &ar_p2 );
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
		tmp = img_con10;
		img_con10 = img_con1;
		img_con1 = tmp;
		tmp = img_con20;
		img_con20 = img_con2;
		img_con2 = tmp;
//		Sleep(100);
		end_loop = GetProcTimeMiliSec();
		if ( eExperimentMode )	printf( "%d ms\n", end_loop - start_loop );
	}
end_cap:
	// 終了処理
	end_all = GetProcTimeMiliSec();
	printf("Total: %d ms\n", end_all - start_all );
	// 終了のメッセージとして負の特徴点数を送る
	if ( eProtocol == kTCP ) {
		SendPointsAreas( sock, NULL, NULL, -1, &img_size, &addr, eProtocol );
		SendPointsAreas( sock, NULL, NULL, -1, &img_size, &addr, eProtocol );
	}
	else {
		SendPointsAreas( sockpt, NULL, NULL, -1, &img_size, &addr, eProtocol );
		SendPointsAreas( sockpt, NULL, NULL, -1, &img_size, &addr, eProtocol );
	}
	CloseWinSock( sock );
	if ( eProtocol == kUDP ) {
		CloseWinSock( sockpt );
		CloseWinSock( sockres );
	}
	cv::destroyWindow( "Capture" );	// ウィンドウを破棄
#ifdef	DRAW_COR
	cv::destroyWindow( "Corres" );
#else
	cv::destroyWindow( "Thumb" );
#endif
#ifdef	DRAW_HIST
	cv::destroyWindow( "Hist" );
#endif
	if ( eEntireMode == CAM_RET_MODE ) {
		StopDirectShowCap( &dsc );	// キャプチャを終了
		ReleaseDirectShowCap( &dsc );	// DirectShowの諸々をリリース
	}
	else if ( eEntireMode == INPUT_MOVIE_MODE ) {
		// cvReleaseCapture removed (VideoCapture auto-releases);
	} // cvReleaseImage removed (cv::Mat auto-manages memory)
	ReleaseCentres( ps1 );
	free( areas1 );
	ReleaseCentres( ps2 );
	free( areas2 );
	free( img_buff );
#ifdef	VIDEO_OUTPUT
	// cvReleaseVideoWriter removed (VideoWriter auto-releases);
#endif
#ifdef	VIDEO_RECORD
	// cvReleaseVideoWriter removed (VideoWriter auto-releases);
#endif
	timeEndPeriod( 1 );

	return 0;
}
