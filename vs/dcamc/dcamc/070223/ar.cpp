#include "def_general.h"	// proctime用
#include <stdio.h>
#include <cv.h>			// OpneCV
#include <highgui.h>	// OpenCV
#include <windows.h>
#include <mmsystem.h>
//#include <dshow.h>
//#include <conio.h>
//#include <qedit.h>

//#include "ws_cl.h"
#include "dirs.h"
//#include "proctime.h"
//#include "dscap.h"
//#include "auto_connect.h"
#include "proj4p.h"
//#include "dcamc.h"
#include "nimg.h"
#include "ar.h"

void DrawAR( IplImage *img_cap, char *doc_name, strProjParam param )
// ARを描画
{
	int i, j, m, ar_num, fi_num, ti_num;
	char line[kMaxLineLen], ar_fname[kMaxPathLen], img_fname[kMaxPathLen], img_path[kMaxPathLen], **ar_array, val[3], ar_type;
	FILE *fp, *fp_img;
	CvSize reg_size;
	CvPoint **edge_array, cvpt_rect[4];
	strPoint pt_q, pt_r, pt_m, q_edge[4], r_edge[4], p1_r, p2_r, p1_q, p2_q, rect_r[4], rect_q[4];
	IplImage **img_array;
	strAR fi[kMaxAR], ti[kMaxAR];
	strProjParam param_r2q;


	if ( doc_name[0] == '\0' )	return;	// リジェクトの場合
	sprintf( ar_fname, "%s%s%s", kARDir, doc_name, ".txt" );
	if ( ( fp = fopen( ar_fname, "r" ) ) == NULL )	return;
	fgets( line, kMaxLineLen, fp );
	sscanf( line, "%d,%d", &(reg_size.width), &(reg_size.height) );	// 登録文書の画像サイズ
//	fgets( line, kMaxLineLen, fp );
//	sscanf( line, "%d", &ar_num );	// 関連情報の数
	// メモリの確保
//	ar_array = (char **)calloc( ar_num, sizeof(char *) );
//	edge_array = (CvPoint **)calloc( ar_num, sizeof(CvPoint *) );
//	img_array = (IplImage **)calloc( ar_num, sizeof(IplImage *) );
	for ( fi_num = 0, ti_num = 0; fgets( line, kMaxLineLen, fp ) != NULL ; ) {
		sscanf( line, "%c", &ar_type );	// 種別の読み込み
		switch ( ar_type ) {
			case kARTypeImg:	// 画像
				fi[fi_num].type = kARTypeImg;
				// 端点の座標
				fgets( line, kMaxLineLen, fp );
				sscanf( line, "%d,%d,%d,%d", &(fi[fi_num].e[0].x), &(fi[fi_num].e[0].y), &(fi[fi_num].e[1].x), &(fi[fi_num].e[1].y) );
				// 画像のファイル名
				fgets( line, kMaxLineLen, fp );
				sscanf( line, "%s", img_fname );
				sprintf( img_path, "%s%s", kARImageDir, img_fname );
				fi[fi_num].img = cvLoadImage( img_path, 1 );
				fi_num++;
				break;
			case kARTypeRect:	// 矩形
				ti[ti_num].type = kARTypeRect;
				// 端点の座標
				fgets( line, kMaxLineLen, fp );
				sscanf( line, "%d,%d,%d,%d", &(ti[ti_num].e[0].x), &(ti[ti_num].e[0].y), &(ti[ti_num].e[1].x), &(ti[ti_num].e[1].y) );
				ti[ti_num].img = NULL;
				ti_num++;
				break;
			case kARTypeUL:		// アンダーライン
				ti[ti_num].type = kARTypeUL;
				fgets( line, kMaxLineLen, fp );
				sscanf( line, "%d,%d,%d,%d", &(ti[ti_num].e[0].x), &(ti[ti_num].e[0].y), &(ti[ti_num].e[1].x), &(ti[ti_num].e[1].y) );
				ti[ti_num].img = NULL;
				ti_num++;
				break;
			default:
				break;
		}
	}
	fclose( fp );
/*	for ( i = 0; i < ar_num; i++ ) {
		ar_array[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
		edge_array[i] = (CvPoint *)calloc( 2, sizeof(CvPoint) );
		fgets( line, kMaxLineLen, fp );
		// 端点の座標
		sscanf( line, "%d,%d,%d,%d", &(edge_array[i][0].x), &(edge_array[i][0].y), &(edge_array[i][1].x), &(edge_array[i][1].y));
		fgets( line, kMaxLineLen, fp );
		sscanf( line, "%s", ar_array[i] );	// 画像ファイルの場所
		// 画像ファイルの読み込み
		sprintf( img_fname, "%s%s", kARImageDir, ar_array[i] );
		if ( ( fp_img = fopen( img_fname, "r" ) ) == NULL ) {
			fprintf( stderr, "%s cannot open\n", img_fname );
			i--;
		}
		img_array[i] = cvLoadImage( img_fname, 1 );
		fclose( fp_img );
	}*/

	// 確認
//	printf("reg_size : %d,%d\n", reg_size.width, reg_size.height);
//	printf("fi_num : %d, ti_num : %d\n", fi_num, ti_num);
//	for ( i = 0; i < fi_num; i++ ) {
//		printf("%x\n", fi[i].img );
//		printf("%d,%d\n", fi[i].img->width, fi[i].img->height);
//	}
//	for ( i = 0; i < fi_num; i++ ) {
//		printf("e%d1 : %d,%d\ne%d2 : %d,%d\n", i, edge_array[i][0].x, edge_array[i][0].y, i, edge_array[i][1].x, edge_array[i][1].y);
//		printf("file%d : %s\n", i, ar_array[i]);
//	}
	// ARの描画（キャプチャ画像から関連情報へ）
	for ( j = 0; j < img_cap->height; j++ ) {
		for ( i = 0; i < img_cap->width; i++ ) {
			pt_q.x = i;
			pt_q.y = j;
			ProjTrans( &pt_q, &pt_r, &param );	// 登録文書の座標を求める
			for ( m = 0; m < fi_num; m++ ) {	// 各関連情報について
//				if ( pt_r.x >= edge_array[m][0].x && pt_r.x < edge_array[m][1].x && pt_r.y >= edge_array[m][0].y && pt_r.y < edge_array[m][1].y ) {
//					pt_m.x = (int)(( (double)pt_r.x - (double)edge_array[m][0].x) / (double)(edge_array[m][1].x - edge_array[m][0].x) * (double)img_array[m]->width);
//					pt_m.y = (int)(( (double)pt_r.y - (double)edge_array[m][0].y) / (double)(edge_array[m][1].y - edge_array[m][0].y) * (double)img_array[m]->height);
				if ( pt_r.x >= fi[m].e[0].x && pt_r.x < fi[m].e[1].x && pt_r.y >= fi[m].e[0].y && pt_r.y < fi[m].e[1].y ) {
					pt_m.x = (int)(( (double)pt_r.x - (double)fi[m].e[0].x) / (double)(fi[m].e[1].x - fi[m].e[0].x) * (double)fi[m].img->width);
					pt_m.y = (int)(( (double)pt_r.y - (double)fi[m].e[0].y) / (double)(fi[m].e[1].y - fi[m].e[0].y) * (double)fi[m].img->height);
					GetPix( fi[m].img, pt_m.x, pt_m.y, val );
//					val[0] = 0x0ff; val[1] = 0x0ff; val[2] = 0x0ff;
					SetPix( img_cap, i, j, val );
				}
			}

		}
	}
	// 登録画像から検索質問への射影変換パラメータを求める
	q_edge[0].x = 0;
	q_edge[0].y = 0;
	q_edge[1].x = img_cap->width;
	q_edge[1].y = 0;
	q_edge[2].x = img_cap->width;
	q_edge[2].y = img_cap->height;
	q_edge[3].x = 0;
	q_edge[3].y = img_cap->height;
	for ( i = 0; i < 4; i++ ) {
		ProjTrans( &(q_edge[i]), &(r_edge[i]), &param );	// 登録文書の座標を求める
	}
	CalcProjParam( r_edge, q_edge, &param_r2q );
	p1_r.x = 0;
	p1_r.y = 0;
	p2_r.x = reg_size.width;
	p2_r.y = reg_size.height;

	ProjTrans( &p1_r, &p1_q, &param_r2q );
	ProjTrans( &p2_r, &p2_q, &param_r2q );
//	cvLine( img_cap, cvPoint(p1_q.x, p1_q.y), cvPoint(p2_q.x, p2_q.y), cWhite, 2, 8, 0 );

	// ARの描画（関連情報からキャプチャ画像へ
	for ( i = 0; i < ti_num; i++ ) {
//		printf("%d %c\n", i, ti[i].type);
		switch ( ti[i].type ) {
			case kARTypeRect:	// 矩形
				rect_r[0].x = ti[i].e[0].x;
				rect_r[0].y = ti[i].e[0].y;
				rect_r[1].x = ti[i].e[1].x;
				rect_r[1].y = ti[i].e[0].y;
				rect_r[2].x = ti[i].e[1].x;
				rect_r[2].y = ti[i].e[1].y;
				rect_r[3].x = ti[i].e[0].x;
				rect_r[3].y = ti[i].e[1].y;
				for ( j = 0; j < 4; j++ ) {
					ProjTrans( &(rect_r[j]), &(rect_q[j]), &param_r2q );
					cvpt_rect[j].x = rect_q[j].x;
					cvpt_rect[j].y = rect_q[j].y;
				}
				DrawPolygon( img_cap, cvpt_rect, 4, cRed, 3, 8, 0 );
				break;
			case kARTypeUL:	// アンダーライン
				rect_r[0].x = ti[i].e[0].x;
				rect_r[0].y = ti[i].e[0].y;
				rect_r[1].x = ti[i].e[1].x;
				rect_r[1].y = ti[i].e[1].y;
				for ( j = 0; j < 2; j++ ) {
					ProjTrans( &(rect_r[j]), &(rect_q[j]), &param_r2q );
					cvpt_rect[j].x = rect_q[j].x;
					cvpt_rect[j].y = rect_q[j].y;
				}
				cvLine( img_cap, cvpt_rect[0], cvpt_rect[1], cRed, 3, 8, 0 );
				break;
			default:
				break;
		}
	}
	// メモリの開放
	for ( i = 0; i < fi_num; i++ ) {
		cvReleaseImage( &(fi[i].img) );
	}
}
