//#define	kConnectHashImageThr	(220)
// �Ōy�ʂ̓����_���o�̂��߂ɕύX�@07/06/07
//#define	kConnectHashImageThr	(250)
// ���X�L�[�g�m�C�Y�΍�̂��߂ɕύX
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
