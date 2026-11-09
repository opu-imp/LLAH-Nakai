//#define	kConnectHashImageThr	(220)
// 最軽量の特徴点抽出のために変更　07/06/07
//#define	kConnectHashImageThr	(250)
// モスキートノイズ対策のために変更
#define	kConnectHashImageThr	(240)
#define	kImgMaxVal	(255)

cv::Mat GetConnectedImage( char *fname, int mode );
int GetConnectedImageJp( cv::Mat src, cv::Mat dst );
int AutoConnectTest( char *fname, cv::Size *size );
cv::Mat GetConnectedImage2( const char *fname, int mode, int gaussian_parameter );
cv::Mat GetConnectedImageJp2( const char *fname, int mode );
int VariousLangMaskSize( const char *fname );
int VariousLangMaskSizeDualChar( const char *fname );
int VariousLangMaskSizeDualWord( const char *fname );
