//#define	kConnectHashImageThr	(220)
// �Ōy�ʂ̓����_���o�̂��߂ɕύX�@07/06/07
//#define	kConnectHashImageThr	(250)
// ���X�L�[�g�m�C�Y�΍�̂��߂ɕύX
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
