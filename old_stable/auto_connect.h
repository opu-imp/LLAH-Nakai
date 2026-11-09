//#define	kConnectHashImageThr	(220)
// 最軽量の特徴点抽出のために変更　07/06/07
#define	kConnectHashImageThr	(250)
#define	kImgMaxVal	(255)

cv::Mat GetConnectedImage( char *fname, int mode );
int GetConnectedImageJp( cv::Mat src, cv::Mat dst );
int AutoConnectTest( char *fname, cv::Size *size );
cv::Mat GetConnectedImage2( const char *fname, int mode );
cv::Mat GetConnectedImageJp2( const char *fname, int mode );
