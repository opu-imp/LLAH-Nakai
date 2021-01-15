#define	cWhite	CV_RGB( 255,255,255 )
#define	cBlack	CV_RGB( 0,0,0 )
#define	cRed	CV_RGB( 255,0,0 )
#define	cGreen	CV_RGB( 0,255,0 )
#define	cBlue	CV_RGB( 0,0,255 )
#define	cRandom	CV_RGB( rand()%256, rand()%256, rand()%256 )

#define	kBinMaxVal	(256)	/* 二値化の際の画素値の最大値 */
#define	kDrawContourMaxLevel	(-1)	/* 連結成分描画時のパラメータ */
#define	kDrawContourLineType	(8)	/* 同上 */
#define	kMaxColorDist	(256*256*256) /* 8ビット3チャネルカラーの最大距離 */
