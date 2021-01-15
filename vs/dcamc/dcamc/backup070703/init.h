#define	kIniFileName	"client.ini"
#define	kDefaultCamNum	(-1)
#define	kDefaultCamConfNum	(-1)
#define	kDefaultTCPPort		(12345)
#define	kDefaultProtocol	(2)	/* 1はTCP，2はUDP */
#define	kDefaultPointPort	(65431)
#define	kDefaultResultPort	(65432)
#define	kDefaultClientName	"localhost"
#define	kDefaultServerName	"localhost"
#define	kDefaultThumbDir	".\\"
#define	kDefaultThumbSuffix	"jpg"
#define	kDefaultThumbScale	(0.25)

#define	kCopyright	"Copyright (C) 2006 Tomohiro Nakai, Intelligent Media Processing Laboratory, Osaka Prefecture University"

#define	CAM_RET_MODE		(0)
#define	INPUT_MOVIE_MODE	(1)
#define	CAP_MOVIE_MODE		(2)
#define	DECOMPOSE_MOVIE_MODE	(3)
#define	TUNE_FP_MODE		(4)
#define	CHK_CAM_MODE		(5)	// カメラの情報をチェックするモード
#define	CONV_MOVIE_MODE		(6)	/* 動画の変換モード */
#define	CAM_HARRIS_MODE		(7)	/* カメラでHarrisをやるモード */
#define	HARRIS_TEST_MODE	(8)	/* Harrisのテストモード */

int ReadIniFile( void );
void InterpretArguments( int argc, char *argv[] );
