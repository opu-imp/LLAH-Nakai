// GRIFFON�̏ꍇ
//#define	kCamConfNum	13	// 1280x960
//#define	kCamConfNum	8	// 640x480
// RAPTOR�̏ꍇ
//#define	kCamConfNum	2	// 1280x960
//#define	kCamConfNum	0	// 640x480

/* kCamConfNum�Ɖ𑜓x�E�t���[�����[�g�̊֌W�iQVX-13�̏ꍇ�j
[0] 320x240(30fps)
[1] 176x144(30fps)
[2] 160x120(30fps)
[3] 352x288(30fps)
[4] 432x240(30fps)
[5] 480x360(30fps)
[6] 512x288(30fps)
[7] 640x360(30fps)
[8] 640x480(30fps)
[9] 704x576(15fps)
[10] 864x480(15fps)
[11] 960x720(15fps)
[12] 1024x576(10fps)
[13] 1280x960(7fps)
[14] 320x240(30fps)
[15] 176x144(30fps)
[16] 160x120(30fps)
[17] 352x288(30fps)
[18] 432x240(30fps)
[19] 480x360(30fps)
[20] 512x288(30fps)
[21] 640x360(30fps)
[22] 640x480(30fps)
[23] 704x576(15fps)
[24] 864x480(15fps)
[25] 960x720(15fps)
[26] 1024x576(10fps)
[27] 1280x960(7fps)
*/
typedef struct _strDirectShowCap {
	ICreateDevEnum	*pDevEnum;	// �w�肳�ꂽ�t�B���^ �J�e�S�����̃t�B���^����уn�[�h�E�F�A �f�o�C�X�̃��X�g�̗񋓎q���쐬����B
	IGraphBuilder	*pGraph;	// �A�v���P�[�V��������t�B���^�O���t���\�z���郁�\�b�h��񋟂���B
	IMediaControl	*pMC;		// �t�B���^�O���t��ʂ�f�[�^�̗���𐧌䂷�郁�\�b�h��񋟂���B
	ISampleGrabber	*pGrab;		// �t�B���^�O���t����ʂ�X�̃��f�B�A �T���v�����擾���郁�\�b�h��񋟂���B
	ICaptureGraphBuilder2 *pCapture;	// �L���v�`���O���t���쐬���邽�߂̃��\�b�h������Ă���B
} strDirectShowCap;

int InitDirectShowCap( strDirectShowCap *dsc, int *width, int *height );
int StartDirectShowCap( strDirectShowCap *dsc );
int CaptureDirectShowCap( strDirectShowCap *dsc, unsigned char *buff, long buff_size );
int PauseDirectShowCap( strDirectShowCap *dsc );
int ResumeDirectShowCap( strDirectShowCap *dsc );
int StopDirectShowCap( strDirectShowCap *dsc );
void ReleaseDirectShowCap( strDirectShowCap *dsc );
int CheckDirectShowCap( void );
