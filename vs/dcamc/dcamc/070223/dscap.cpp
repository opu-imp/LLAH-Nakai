#include <windows.h>
#include <qedit.h>			// SampleGrabber用
#include <dshow.h>
#include "def_general.h"
#include "dscap.h"
#include "extern.h"
#include "init.h"

int InitDirectShowCap( strDirectShowCap *dsc, int *width, int *height )
// DirectShowによるUSBカメラの使用の際の初期化
{
	int i = 0;
    ULONG cFetched;
	HRESULT hr;	// リザルトコード

	IEnumMoniker *pClassEnum;	// モニカを列挙するために使用するインターフェイス。
	IMoniker *pMoniker = NULL;	// モニカ オブジェクトを使用できるようにするメソッドが含まれたインターフェイス。
	IBaseFilter *pbf = NULL, *pF = NULL;	// フィルタを制御するメソッドを提供する。
	AM_MEDIA_TYPE amt, *pmtConfig;	// メディアサンプルのフォーマットを記述。
	IAMStreamConfig *pConfig = NULL;	// ビデオキャプチャフィルタの出力ピンに実装される。
	VIDEO_STREAM_CONFIG_CAPS scc;	// 一定の範囲内のビデオ フォーマットを記述する。

	CoInitialize( NULL );					// COMの初期化

	// ---- キャプチャフィルタの準備 ----
	// キャプチャデバイスを探す
	// デバイス列挙子を作成
	dsc->pDevEnum = NULL;
	CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&(dsc->pDevEnum) );
	// ビデオキャプチャデバイス列挙子を作成
	pClassEnum = NULL;
	dsc->pDevEnum -> CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pClassEnum, 0 );
	if ( pClassEnum == NULL ) {	// 見つからない
		fprintf( stderr, "error: no video capture device\n" );
		dsc->pDevEnum->Release();
		CoUninitialize();
		return 0;
	}
	pClassEnum->Reset();	// リセット
	// eCamNumまで進める
	if ( eCamNum < 0 ) {	// CamNumが指定されていない
		char line[kMaxLineLen];
		printf("These cameras are found.\n");
		for ( i = 0; pClassEnum->Next( 1, &pMoniker, &cFetched ) == S_OK; i++ ) {
			/*********** デバイス名を取得 ***********/
			IPropertyBag *pPropertyBag;
			TCHAR devname[128];
			// IPropertyBagにbindする
			pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropertyBag );
			// Friendly nameを取得するための入れ物
			VARIANT varFriendlyName;
			varFriendlyName.vt = VT_BSTR;
			// FriendlyNameを取得
			pPropertyBag->Read( L"FriendlyName", &varFriendlyName, 0 );
			// 取得したFriendlyNameをコピー
#ifdef UNICODE
			// 念のためUNICODEの時のコード
			_tcscpy(devname, varFriendlyName.bstrVal);
#else
			// 特に何もしなければ、UNICODEではなく、こちらになるはずです
			WideCharToMultiByte( CP_ACP, 0, varFriendlyName.bstrVal, -1, devname, sizeof(devname), 0, 0 );
#endif      
			// 結果を表示
			printf( "%02d : %s\n", i, devname );
			// 資源を解放
			VariantClear(&varFriendlyName);
			// 資源を解放
			pMoniker->Release();
			pPropertyBag->Release();
		}
		if ( i-1 > 0 ) {	// ２つ以上のカメラが見つかった
			printf( "Please select a camera.\n" );
			printf( "(You can omit this process by specifying CamNum in %s)\n", kIniFileName );
			printf( "[%02d-%02d] : ", 0, i-1 );
			gets( line );
			sscanf( line, "%d", &eCamNum );
		}
		else {
			eCamNum = 0;
		}
	}
	pClassEnum->Reset();	// リセット
	i = 0;
	do {
		hr = pClassEnum->Next( 1, &pMoniker, &cFetched );
		if ( hr != S_OK ) {
			fprintf( stderr, "error: wrong CamNum %d\n", eCamNum );
			return 0;
		}
	} while ( i++ < eCamNum );
	pMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void **)&pbf );
	// ---- フィルタグラフの準備 ----
	// フィルタグラフを作り、インターフェースを得る
	CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&(dsc->pGraph) );
	dsc->pGraph->QueryInterface( IID_IMediaControl, (LPVOID *)&(dsc->pMC) );
	// キャプチャフィルタをフィルタグラフに追加
	dsc->pGraph->AddFilter( pbf, L"Video Capture" );
	// ---- グラバフィルタの準備 ----
	// グラバフィルタを作る
	CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pF );
	pF->QueryInterface( IID_ISampleGrabber, (void **)&(dsc->pGrab) );
	// グラバフィルタの挿入場所の特定のための設定
	ZeroMemory( &amt, sizeof(AM_MEDIA_TYPE) );
	amt.majortype  = MEDIATYPE_Video;
	amt.subtype    = MEDIASUBTYPE_RGB24;
	amt.formattype = FORMAT_VideoInfo; 
	dsc->pGrab -> SetMediaType( &amt );
	// グラバフィルタをフィルタグラフに追加
	dsc->pGraph -> AddFilter( pF, L"SamGra" );
	// ---- キャプチャグラフの準備 ----
	// キャプチャグラフを作る   
	CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&(dsc->pCapture) );
	dsc->pCapture -> FindInterface( &PIN_CATEGORY_CAPTURE , 0, pbf, IID_IAMStreamConfig, (void **)&pConfig );
	if ( eCamConfNum < 0 ) {	// CamConfNumが指定されていない
		int picount, pisize;
		char line[kMaxLineLen];

		printf( "This camera supports these modes.\n" );
		hr = pConfig->GetNumberOfCapabilities( &picount, &pisize );
		for ( i = 0; i < picount; i++ ) {	// 解像度等を表示
			pConfig->GetStreamCaps( i, &pmtConfig, (BYTE *)&scc );
			printf("%02d : %dx%d[%.1ffps]\n", i, ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biWidth, ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biHeight, 1/(((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->AvgTimePerFrame*0.0000001) );
		}
		if ( picount > 1 ) {	// ２つ以上のモードが見つかった
			printf( "Please select a mode.\n" );
			printf( "(You can omit this process by specifying CamConfNum in %s)\n", kIniFileName );
			printf( "*** CAUTION *** Latter modes may not work.\n" );
			printf( "[%02d-%02d] : ", 0, picount-1 );
			gets( line );
			sscanf( line, "%d", &eCamConfNum );
		}
		else {
			eCamConfNum = 0;
		}
	}
	// 解像度を変更
	hr = pConfig->GetStreamCaps( eCamConfNum, &pmtConfig, (BYTE *)&scc );	// extern.hのものに変更
	if ( hr != S_OK ) {
		fprintf( stderr, "error: wrong CamConfNum %d\n", eCamConfNum );
		return 0;
	}
	hr = pConfig->SetFormat( pmtConfig );
	
	*width  = ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biWidth;
	*height = ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biHeight;
#ifdef	RESO_LIMIT
	if ( (*width) * (*height) > 1280*1024 ) {
		fprintf( stderr, "error: resolution of query image is limited to 1.3 mega pixels\n" );
		return 0;
	}
#endif
	// フィルタグラフをキャプチャグラフに組み込む
	dsc->pCapture -> SetFiltergraph( dsc->pGraph );
	// キャプチャグラフの設定、グラバをレンダリング出力に設定
	dsc->pCapture -> RenderStream( &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pbf, NULL, pF );
	// キャプチャフィルタの参照をリリース
    pbf -> Release();
	
	return 1;
}

int CheckDirectShowCap( void )
// DirectShowによるUSBカメラの情報のチェック
{
    ULONG cFetched;
	HRESULT hr;	// リザルトコード

	IEnumMoniker *pClassEnum;	// モニカを列挙するために使用するインターフェイス。
	IMoniker *pMoniker = NULL;	// モニカ オブジェクトを使用できるようにするメソッドが含まれたインターフェイス。
	IBaseFilter *pbf = NULL, *pF = NULL;	// フィルタを制御するメソッドを提供する。
	AM_MEDIA_TYPE amt, *pmtConfig;	// メディアサンプルのフォーマットを記述。
	IAMStreamConfig *pConfig = NULL;	// ビデオキャプチャフィルタの出力ピンに実装される。
	VIDEO_STREAM_CONFIG_CAPS scc;	// 一定の範囲内のビデオ フォーマットを記述する。
	int i, j, picount, pisize;

	strDirectShowCap dsc0, *dsc;

	dsc = &dsc0;
	CoInitialize( NULL );					// COMの初期化

	// ---- キャプチャフィルタの準備 ----
	// キャプチャデバイスを探す
	// デバイス列挙子を作成
	dsc->pDevEnum = NULL;
	hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&(dsc->pDevEnum) );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed : CoCreateInstance\n" );
		return 0;
	}
	// ビデオキャプチャデバイス列挙子を作成
	pClassEnum = NULL;
	hr = dsc->pDevEnum -> CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pClassEnum, 0 );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed : CreateClassEnumerator\n" );
		return 0;
	}

	if ( pClassEnum == NULL ) {	// 見つからない
		fprintf( stderr, "Error: No video capture device\n" );
		dsc->pDevEnum->Release();
		CoUninitialize();
		return 0;
	}
	pClassEnum->Reset();	// リセット
	for ( j = 0; pClassEnum->Next( 1, &pMoniker, &cFetched ) == S_OK; j++ ) {
//	while ( pClassEnum->Next( 1, &pMoniker, &cFetched ) == S_OK ) {
		/*********** デバイス名を取得 ***********/
		// get the device friendly name:
		IPropertyBag *pPropertyBag;
		TCHAR devname[128];
		// IPropertyBagにbindする
		pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropertyBag );
		// Friendly nameを取得するための入れ物
		VARIANT varFriendlyName;
		varFriendlyName.vt = VT_BSTR;
		// FriendlyNameを取得
		pPropertyBag->Read( L"FriendlyName", &varFriendlyName, 0 );
		// 取得したFriendlyNameをコピー
#ifdef UNICODE
		// 念のためUNICODEの時のコード
		_tcscpy(devname, varFriendlyName.bstrVal);
#else
		// 特に何もしなければ、UNICODEではなく、こちらになるはずです
		WideCharToMultiByte( CP_ACP, 0, varFriendlyName.bstrVal, -1, devname, sizeof(devname), 0, 0 );
#endif      
		// 結果を表示
//		MessageBox(NULL, devname, "DEVICE NAME", MB_OK);
		printf( "CamNum : Device name\n" );
		printf( "%02d : %s\n", j, devname );
		// 資源を解放
		VariantClear(&varFriendlyName);
		// 資源を解放
		pMoniker->Release();
		pPropertyBag->Release();

		/*********** 解像度等を取得 ***********/
		pMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void **)&pbf );
		// ---- フィルタグラフの準備 ----
		// フィルタグラフを作り、インターフェースを得る
		CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&(dsc->pGraph) );
		dsc->pGraph->QueryInterface( IID_IMediaControl, (LPVOID *)&(dsc->pMC) );
		// キャプチャフィルタをフィルタグラフに追加
		dsc->pGraph->AddFilter( pbf, L"Video Capture" );
		// ---- グラバフィルタの準備 ----
		// グラバフィルタを作る
		CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pF );
		pF->QueryInterface( IID_ISampleGrabber, (void **)&(dsc->pGrab) );
		// グラバフィルタの挿入場所の特定のための設定
		ZeroMemory( &amt, sizeof(AM_MEDIA_TYPE) );
		amt.majortype  = MEDIATYPE_Video;
		amt.subtype    = MEDIASUBTYPE_RGB24;
		amt.formattype = FORMAT_VideoInfo; 
		dsc->pGrab -> SetMediaType( &amt );
		// グラバフィルタをフィルタグラフに追加
		dsc->pGraph -> AddFilter( pF, L"SamGra" );
		// ---- キャプチャグラフの準備 ----
		// キャプチャグラフを作る   
		CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&(dsc->pCapture) );
		dsc->pCapture -> FindInterface( &PIN_CATEGORY_CAPTURE , 0, pbf, IID_IAMStreamConfig, (void **)&pConfig );
		hr = pConfig->GetNumberOfCapabilities( &picount, &pisize );
//		printf( "%d, %d\n", picount, pisize );
		printf( "\tCamConfNum : Resolution[Frame rate]\n" );
		for ( i = 0; i < picount; i++ ) {	// 解像度等を表示
			pConfig->GetStreamCaps( i, &pmtConfig, (BYTE *)&scc );
			printf("\t%02d : %dx%d[%.1ffps]\n", i, ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biWidth, ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biHeight, 1/(((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->AvgTimePerFrame*0.0000001) );
		}
		// キャプチャフィルタの参照をリリース
	    pbf -> Release();
	}
	// 資源を解放
	pClassEnum->Release();
	dsc->pDevEnum->Release();
	// COM終了
	CoUninitialize();

	return 1;

#if 0
	// 最初に見つかったビデオキャプチャデバイスのオブジェクトのインタフェースを得る
	pClassEnum->Next( 1, &pMoniker, &cFetched );
	pMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void **)&pbf );
	// ---- フィルタグラフの準備 ----
	// フィルタグラフを作り、インターフェースを得る
	CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&(dsc->pGraph) );
	dsc->pGraph->QueryInterface( IID_IMediaControl, (LPVOID *)&(dsc->pMC) );
	// キャプチャフィルタをフィルタグラフに追加
	puts("add filter");
	dsc->pGraph->AddFilter( pbf, L"Video Capture" );
	// ---- グラバフィルタの準備 ----
	// グラバフィルタを作る
	CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pF );
	pF->QueryInterface( IID_ISampleGrabber, (void **)&(dsc->pGrab) );
	// グラバフィルタの挿入場所の特定のための設定
	ZeroMemory( &amt, sizeof(AM_MEDIA_TYPE) );
	amt.majortype  = MEDIATYPE_Video;
	amt.subtype    = MEDIASUBTYPE_RGB24;
	amt.formattype = FORMAT_VideoInfo; 
	dsc->pGrab -> SetMediaType( &amt );
	// グラバフィルタをフィルタグラフに追加
	puts("add grab filter");
	dsc->pGraph -> AddFilter( pF, L"SamGra" );
	// ---- キャプチャグラフの準備 ----
	// キャプチャグラフを作る   
	puts("create instance");
	CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&(dsc->pCapture) );
	dsc->pCapture -> FindInterface( &PIN_CATEGORY_CAPTURE , 0, pbf, IID_IAMStreamConfig, (void **)&pConfig );
	// 解像度を変更
	puts("get stream caps");
	printf("%d\n", eCamConfNum);
	pConfig->GetStreamCaps( eCamConfNum, &pmtConfig, (BYTE *)&scc );	// extern.hのものに変更
	puts("set format");
	hr = pConfig->SetFormat( pmtConfig );
	puts("set width");
	
	*width  = ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biWidth;
	printf("%x\n", pmtConfig );
	puts("set height");
	*height = ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biHeight;
	// フィルタグラフをキャプチャグラフに組み込む
	puts("set graph");
	dsc->pCapture -> SetFiltergraph( dsc->pGraph );
	// キャプチャグラフの設定、グラバをレンダリング出力に設定
	dsc->pCapture -> RenderStream( &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pbf, NULL, pF );
	// キャプチャフィルタの参照をリリース
	puts("release");
    pbf -> Release();
	puts("init end");
#endif
}

int StartDirectShowCap( strDirectShowCap *dsc )
// DirectShowによるUSBカメラの使用を開始
{
	HRESULT hr;
	
	// ---- キャプチャ開始 ----
	hr = dsc->pGrab -> SetBufferSamples(TRUE);	// グラブ開始
	hr = dsc->pMC -> Run();						// レンダリング開始
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int CaptureDirectShowCap( strDirectShowCap *dsc, unsigned char *buff, long buff_size )
// DirectShowによるUSBカメラを用いたキャプチャ
{
	HRESULT hr;

	hr = dsc->pGrab->GetCurrentBuffer( &buff_size, (long *)buff );	// バッファのコピーを取得
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int PauseDirectShowCap( strDirectShowCap *dsc )
// DirectShowによるUSBカメラの使用を一時停止
{
	HRESULT hr;
	
	hr = dsc->pMC -> Pause();
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int ResumeDirectShowCap( strDirectShowCap *dsc )
// DirectShowによるUSBカメラの使用を再開
{
	HRESULT hr;
	
	hr = dsc->pMC -> Run();
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int StopDirectShowCap( strDirectShowCap *dsc )
// DirectShowによるUSBカメラの使用を停止
{
	HRESULT hr;
	
	hr = dsc->pMC -> Stop();
	dsc->pGrab -> SetBufferSamples( 0 );
	if ( hr == S_OK )	return 1;
	else	return 0;
}

void ReleaseDirectShowCap( strDirectShowCap *dsc )
// DirectShowによるUSBカメラの使用の諸々を解放
{
	// ---- 終了処理 ----
	// インターフェースのリリース
	dsc->pMC -> Release();
	dsc->pDevEnum -> Release();
	dsc->pGraph -> Release();
	dsc->pCapture -> Release();
	// COMのリリース
	CoUninitialize();
}

int SetCamProperty( void )
// カメラのプロパティを設定
{
	return 1;	
}