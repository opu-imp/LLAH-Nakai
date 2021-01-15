#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <qedit.h>			// SampleGrabber�p
#include <dshow.h>
#include "dscap.h"

int InitDirectShowCap( int camconf, strDirectShowCap *dsc, int *width, int *height )
// DirectShow�ɂ��USB�J�����̎g�p�̍ۂ̏�����
{
    ULONG cFetched;
	HRESULT hr;	// ���U���g�R�[�h

	IEnumMoniker *pClassEnum;	// ���j�J��񋓂��邽�߂Ɏg�p����C���^�[�t�F�C�X�B
	IMoniker *pMoniker = NULL;	// ���j�J �I�u�W�F�N�g���g�p�ł���悤�ɂ��郁�\�b�h���܂܂ꂽ�C���^�[�t�F�C�X�B
	IBaseFilter *pbf = NULL, *pF = NULL;	// �t�B���^�𐧌䂷�郁�\�b�h��񋟂���B
	AM_MEDIA_TYPE amt, *pmtConfig;	// ���f�B�A�T���v���̃t�H�[�}�b�g���L�q�B
	IAMStreamConfig *pConfig = NULL;	// �r�f�I�L���v�`���t�B���^�̏o�̓s���Ɏ��������B
	VIDEO_STREAM_CONFIG_CAPS scc;	// ���͈͓̔��̃r�f�I �t�H�[�}�b�g���L�q����B

	CoInitialize( NULL );					// COM�̏�����

	// ---- �L���v�`���t�B���^�̏��� ----
	// �L���v�`���f�o�C�X��T��
	// �f�o�C�X�񋓎q���쐬
	dsc->pDevEnum = NULL;
	CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&(dsc->pDevEnum) );
	// �r�f�I�L���v�`���f�o�C�X�񋓎q���쐬
	pClassEnum = NULL;
	dsc->pDevEnum -> CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pClassEnum, 0 );
	if ( pClassEnum == NULL ) {	// ������Ȃ�
		fprintf( stderr, "No video capture device\n" );
		dsc->pDevEnum->Release();
		CoUninitialize();
		return 0;
	}
	// �ŏ��Ɍ��������r�f�I�L���v�`���f�o�C�X�̃I�u�W�F�N�g�̃C���^�t�F�[�X�𓾂�
	pClassEnum->Next( 1, &pMoniker, &cFetched );
	pMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void **)&pbf );
	// ---- �t�B���^�O���t�̏��� ----
	// �t�B���^�O���t�����A�C���^�[�t�F�[�X�𓾂�
	CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&(dsc->pGraph) );
	dsc->pGraph->QueryInterface( IID_IMediaControl, (LPVOID *)&(dsc->pMC) );
	// �L���v�`���t�B���^���t�B���^�O���t�ɒǉ�
	dsc->pGraph->AddFilter( pbf, L"Video Capture" );
	// ---- �O���o�t�B���^�̏��� ----
	// �O���o�t�B���^�����
	CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pF );
	pF->QueryInterface( IID_ISampleGrabber, (void **)&(dsc->pGrab) );
	// �O���o�t�B���^�̑}���ꏊ�̓���̂��߂̐ݒ�
	ZeroMemory( &amt, sizeof(AM_MEDIA_TYPE) );
	amt.majortype  = MEDIATYPE_Video;
	amt.subtype    = MEDIASUBTYPE_RGB24;
	amt.formattype = FORMAT_VideoInfo; 
	dsc->pGrab -> SetMediaType( &amt );
	// �O���o�t�B���^���t�B���^�O���t�ɒǉ�
	dsc->pGraph -> AddFilter( pF, L"SamGra" );
	// ---- �L���v�`���O���t�̏��� ----
	// �L���v�`���O���t�����   
	CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&(dsc->pCapture) );
	dsc->pCapture -> FindInterface( &PIN_CATEGORY_CAPTURE , 0, pbf, IID_IAMStreamConfig, (void **)&pConfig );
	// �𑜓x��ύX
	hr = pConfig->GetStreamCaps( camconf, &pmtConfig, (BYTE *)&scc );	// �w�肳�ꂽ���̂ɕύX
	if ( hr != S_OK ) {
		fprintf( stderr, "Error : GetStreamCaps" );
		return 0;
	}
	hr = pConfig->SetFormat( pmtConfig );
	
	*width  = ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biWidth;
	*height = ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biHeight;
	// �t�B���^�O���t���L���v�`���O���t�ɑg�ݍ���
	dsc->pCapture -> SetFiltergraph( dsc->pGraph );
	// �L���v�`���O���t�̐ݒ�A�O���o�������_�����O�o�͂ɐݒ�
	dsc->pCapture -> RenderStream( &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pbf, NULL, pF );
	// 08/09/24 �J�����R���g���[���̐���̂��߂ɕύX
	pbf->QueryInterface( IID_IAMCameraControl, (void **)&(dsc->pCameraControl) );
	// �L���v�`���t�B���^�̎Q�Ƃ������[�X
    pbf -> Release();
	
	return 1;
}

int CheckDirectShowCap( void )
// DirectShow�ɂ��USB�J�����̏��̃`�F�b�N
{
    ULONG cFetched;
	HRESULT hr;	// ���U���g�R�[�h

	IEnumMoniker *pClassEnum;	// ���j�J��񋓂��邽�߂Ɏg�p����C���^�[�t�F�C�X�B
	IMoniker *pMoniker = NULL;	// ���j�J �I�u�W�F�N�g���g�p�ł���悤�ɂ��郁�\�b�h���܂܂ꂽ�C���^�[�t�F�C�X�B
	IBaseFilter *pbf = NULL, *pF = NULL;	// �t�B���^�𐧌䂷�郁�\�b�h��񋟂���B
	AM_MEDIA_TYPE amt, *pmtConfig;	// ���f�B�A�T���v���̃t�H�[�}�b�g���L�q�B
	IAMStreamConfig *pConfig = NULL;	// �r�f�I�L���v�`���t�B���^�̏o�̓s���Ɏ��������B
	VIDEO_STREAM_CONFIG_CAPS scc;	// ���͈͓̔��̃r�f�I �t�H�[�}�b�g���L�q����B
	int i, picount, pisize;

	strDirectShowCap dsc0, *dsc;

	dsc = &dsc0;
	CoInitialize( NULL );					// COM�̏�����

	// ---- �L���v�`���t�B���^�̏��� ----
	// �L���v�`���f�o�C�X��T��
	// �f�o�C�X�񋓎q���쐬
	dsc->pDevEnum = NULL;
	hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&(dsc->pDevEnum) );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed : CoCreateInstance\n" );
		return 0;
	}
	// �r�f�I�L���v�`���f�o�C�X�񋓎q���쐬
	pClassEnum = NULL;
	hr = dsc->pDevEnum -> CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pClassEnum, 0 );
	if ( FAILED( hr ) ) {
		fprintf( stderr, "Failed : CreateClassEnumerator\n" );
		return 0;
	}

	if ( pClassEnum == NULL ) {	// ������Ȃ�
		fprintf( stderr, "No video capture device\n" );
		dsc->pDevEnum->Release();
		CoUninitialize();
		return 0;
	}
	pClassEnum->Reset();	// ���Z�b�g
	while ( pClassEnum->Next( 1, &pMoniker, &cFetched ) == S_OK ) {
		/*********** �f�o�C�X�����擾 ***********/
		// get the device friendly name:
		IPropertyBag *pPropertyBag;
		TCHAR devname[128];
		// IPropertyBag��bind����
		pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropertyBag );
		// Friendly name���擾���邽�߂̓��ꕨ
		VARIANT varFriendlyName;
		varFriendlyName.vt = VT_BSTR;
		// FriendlyName���擾
		pPropertyBag->Read( L"FriendlyName", &varFriendlyName, 0 );
		// �擾����FriendlyName���R�s�[
#ifdef UNICODE
		// �O�̂���UNICODE�̎��̃R�[�h
		_tcscpy(devname, varFriendlyName.bstrVal);
#else
		// ���ɉ������Ȃ���΁AUNICODE�ł͂Ȃ��A������ɂȂ�͂��ł�
		WideCharToMultiByte( CP_ACP, 0, varFriendlyName.bstrVal, -1, devname, sizeof(devname), 0, 0 );
#endif      
		// ���ʂ�\��
//		MessageBox(NULL, devname, "DEVICE NAME", MB_OK);
		printf( "%s\n", devname );
		// ���������
		VariantClear(&varFriendlyName);
		// ���������
		pMoniker->Release();
		pPropertyBag->Release();

		/*********** �𑜓x�����擾 ***********/
		pMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void **)&pbf );
		// ---- �t�B���^�O���t�̏��� ----
		// �t�B���^�O���t�����A�C���^�[�t�F�[�X�𓾂�
		CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&(dsc->pGraph) );
		dsc->pGraph->QueryInterface( IID_IMediaControl, (LPVOID *)&(dsc->pMC) );
		// �L���v�`���t�B���^���t�B���^�O���t�ɒǉ�
		dsc->pGraph->AddFilter( pbf, L"Video Capture" );
		// ---- �O���o�t�B���^�̏��� ----
		// �O���o�t�B���^�����
		CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pF );
		pF->QueryInterface( IID_ISampleGrabber, (void **)&(dsc->pGrab) );
		// �O���o�t�B���^�̑}���ꏊ�̓���̂��߂̐ݒ�
		ZeroMemory( &amt, sizeof(AM_MEDIA_TYPE) );
		amt.majortype  = MEDIATYPE_Video;
		amt.subtype    = MEDIASUBTYPE_RGB24;
		amt.formattype = FORMAT_VideoInfo; 
		dsc->pGrab -> SetMediaType( &amt );
		// �O���o�t�B���^���t�B���^�O���t�ɒǉ�
		dsc->pGraph -> AddFilter( pF, L"SamGra" );
		// ---- �L���v�`���O���t�̏��� ----
		// �L���v�`���O���t�����   
		CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&(dsc->pCapture) );
		dsc->pCapture -> FindInterface( &PIN_CATEGORY_CAPTURE , 0, pbf, IID_IAMStreamConfig, (void **)&pConfig );
		hr = pConfig->GetNumberOfCapabilities( &picount, &pisize );
//		printf( "%d, %d\n", picount, pisize );
		for ( i = 0; i < picount; i++ ) {	// �𑜓x����\��
			pConfig->GetStreamCaps( i, &pmtConfig, (BYTE *)&scc );
			printf("%02d : %dx%d[%.1ffps]\n", i, ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biWidth, ((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->bmiHeader.biHeight, 1/(((VIDEOINFOHEADER *)(pmtConfig->pbFormat))->AvgTimePerFrame*0.0000001) );
		}
		// �L���v�`���t�B���^�̎Q�Ƃ������[�X
	    pbf -> Release();
	}
	// ���������
	pClassEnum->Release();
	dsc->pDevEnum->Release();
	// COM�I��
	CoUninitialize();

	return 1;
}

int StartDirectShowCap( strDirectShowCap *dsc )
// DirectShow�ɂ��USB�J�����̎g�p���J�n
{
	HRESULT hr;
	
	// ---- �L���v�`���J�n ----
	hr = dsc->pGrab -> SetBufferSamples(TRUE);	// �O���u�J�n
	hr = dsc->pMC -> Run();						// �����_�����O�J�n
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int CaptureDirectShowCap( strDirectShowCap *dsc, unsigned char *buff, long buff_size )
// DirectShow�ɂ��USB�J������p�����L���v�`��
{
	HRESULT hr;

	hr = dsc->pGrab->GetCurrentBuffer( &buff_size, (long *)buff );	// �o�b�t�@�̃R�s�[���擾
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int PauseDirectShowCap( strDirectShowCap *dsc )
// DirectShow�ɂ��USB�J�����̎g�p���ꎞ��~
{
	HRESULT hr;
	
	hr = dsc->pMC -> Pause();
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int ResumeDirectShowCap( strDirectShowCap *dsc )
// DirectShow�ɂ��USB�J�����̎g�p���ĊJ
{
	HRESULT hr;
	
	hr = dsc->pMC -> Run();
	if ( hr == S_OK )	return 1;
	else	return 0;
}

int StopDirectShowCap( strDirectShowCap *dsc )
// DirectShow�ɂ��USB�J�����̎g�p���~
{
	HRESULT hr;
	
	hr = dsc->pMC -> Stop();
	dsc->pGrab -> SetBufferSamples( 0 );
	if ( hr == S_OK )	return 1;
	else	return 0;
}

void ReleaseDirectShowCap( strDirectShowCap *dsc )
// DirectShow�ɂ��USB�J�����̎g�p�̏��X�����
{
	// ---- �I������ ----
	// �C���^�[�t�F�[�X�̃����[�X
	dsc->pMC -> Release();
	dsc->pDevEnum -> Release();
	dsc->pGraph -> Release();
	dsc->pCapture -> Release();
	// COM�̃����[�X
	CoUninitialize();
}

long GetCameraControlProperty( IAMCameraControl *pCameraControl, CameraControlProperty prop, long *flags )
// �J�����̃v���p�e�B�𓾂�
{
	HRESULT res;
	long prop_val, lmin, lmax, lstep, ldef, lflag, cflags;

//	pCameraControl->GetRange( prop, &lmin, &lmax, &lstep, &ldef, &lflag );
	res = pCameraControl->Get( prop, &prop_val, flags );

	if ( res != S_OK )	return 0;
	return prop_val;
}

int SetCameraControlProperty( IAMCameraControl *pCameraControl, CameraControlProperty prop, long val, long flags )
// �J�����̃v���p�e�B��ݒ肷��
{
	HRESULT res;

	res = pCameraControl->Set( prop, val, flags );
	if ( res == S_OK )	return 1;
	else	return 0;
}

int GetCameraControlRange( IAMCameraControl *pCameraControl, CameraControlProperty prop, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags )
// �J�����v���p�e�B�͈̔͂ƋK��l���擾����
{
	HRESULT res;

	res = pCameraControl->GetRange( prop, pMin, pMax, pSteppingDelta, pDefault, pCapsFlags );

	if ( res == S_OK )	return 1;
	else	return 0;
}
