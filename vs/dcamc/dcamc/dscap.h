typedef struct _strDirectShowCap {
	ICreateDevEnum	*pDevEnum;	// �w�肳�ꂽ�t�B���^ �J�e�S�����̃t�B���^����уn�[�h�E�F�A �f�o�C�X�̃��X�g�̗񋓎q���쐬����B
	IGraphBuilder	*pGraph;	// �A�v���P�[�V��������t�B���^�O���t���\�z���郁�\�b�h��񋟂���B
	IMediaControl	*pMC;		// �t�B���^�O���t��ʂ�f�[�^�̗���𐧌䂷�郁�\�b�h��񋟂���B
	ISampleGrabber	*pGrab;		// �t�B���^�O���t����ʂ�X�̃��f�B�A �T���v�����擾���郁�\�b�h��񋟂���B
	ICaptureGraphBuilder2 *pCapture;	// �L���v�`���O���t���쐬���邽�߂̃��\�b�h������Ă���B
	// 08/09/24 �J�����R���g���[���̐���̂��߂ɕύX
	IAMCameraControl *pCameraControl;
} strDirectShowCap;

int InitDirectShowCap( int camconf, strDirectShowCap *dsc, int *width, int *height );
int StartDirectShowCap( strDirectShowCap *dsc );
int CaptureDirectShowCap( strDirectShowCap *dsc, unsigned char *buff, long buff_size );
int PauseDirectShowCap( strDirectShowCap *dsc );
int ResumeDirectShowCap( strDirectShowCap *dsc );
int StopDirectShowCap( strDirectShowCap *dsc );
void ReleaseDirectShowCap( strDirectShowCap *dsc );
int CheckDirectShowCap( void );
long GetCameraControlProperty( IAMCameraControl *pCameraControl, CameraControlProperty prop, long *flags );
int SetCameraControlProperty( IAMCameraControl *pCameraControl, CameraControlProperty prop, long val, long flags );
int GetCameraControlRange( IAMCameraControl *pCameraControl, CameraControlProperty prop, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags );
