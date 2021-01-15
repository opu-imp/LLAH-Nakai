//#define	kConnectHashImageThr	(220)
// 最軽量の特徴点抽出のために変更　07/06/07
//#define	kConnectHashImageThr	(250)
// モスキートノイズ対策のために変更
#define	kConnectHashImageThr	(240)
#define	kImgMaxVal	(255)

IplImage *GetConnectedImage( char *fname, int mode );
int GetConnectedImageJp( IplImage *src, IplImage *dst );
int AutoConnectTest( char *fname, CvSize *size );
IplImage *GetConnectedImage2( const char *fname, int mode, int gaussian_parameter );
IplImage *GetConnectedImageJp2( const char *fname, int mode );
int VariousLangMaskSize( const char *fname );
int VariousLangMaskSizeDualChar( const char *fname );
int VariousLangMaskSizeDualWord( const char *fname );
