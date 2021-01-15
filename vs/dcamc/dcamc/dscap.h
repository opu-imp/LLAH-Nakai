typedef struct _strDirectShowCap {
	ICreateDevEnum	*pDevEnum;	// 指定されたフィルタ カテゴリ内のフィルタおよびハードウェア デバイスのリストの列挙子を作成する。
	IGraphBuilder	*pGraph;	// アプリケーションからフィルタグラフを構築するメソッドを提供する。
	IMediaControl	*pMC;		// フィルタグラフを通るデータの流れを制御するメソッドを提供する。
	ISampleGrabber	*pGrab;		// フィルタグラフ内を通る個々のメディア サンプルを取得するメソッドを提供する。
	ICaptureGraphBuilder2 *pCapture;	// キャプチャグラフを作成するためのメソッドを備えている。
	// 08/09/24 カメラコントロールの制御のために変更
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
