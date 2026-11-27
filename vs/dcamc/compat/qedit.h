// Replacement for qedit.h, which current Windows SDKs no longer ship.
//
// qedit.h (DirectShow Editing Services) was dropped from the Windows SDK after the
// Windows Vista SDK, and it also needed dxtrans.h, which is gone as well. dcamc only
// uses the Sample Grabber filter, which Windows still provides (qedit.dll), so this
// header declares just that filter's interfaces and GUIDs, the same way OpenCV's
// videoio/src/cap_dshow.cpp does. The declarations match the original qedit.h, so the
// dcamc sources keep their "#include <qedit.h>" unchanged.
//
// CMakeLists.txt puts this directory on the include path.

#ifndef DCAMC_COMPAT_QEDIT_H
#define DCAMC_COMPAT_QEDIT_H

#include <dshow.h>

struct __declspec(uuid("0579154A-2B53-4994-B0D0-E773148EFF85"))
ISampleGrabberCB : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SampleCB( double SampleTime, IMediaSample *pSample ) = 0;
	virtual HRESULT STDMETHODCALLTYPE BufferCB( double SampleTime, BYTE *pBuffer, long BufferLen ) = 0;
};

struct __declspec(uuid("6B652FFF-11FE-4fce-92AD-0266B5D7C78F"))
ISampleGrabber : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( long *pBufferSize, long *pBuffer ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( IMediaSample **ppSample ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback, long WhichMethodToCallback ) = 0;
};

EXTERN_C const CLSID DECLSPEC_SELECTANY CLSID_SampleGrabber =
	{ 0xc1f400a0, 0x3f08, 0x11d3, { 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37 } };
EXTERN_C const CLSID DECLSPEC_SELECTANY CLSID_NullRenderer =
	{ 0xc1f400a4, 0x3f08, 0x11d3, { 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37 } };
EXTERN_C const IID DECLSPEC_SELECTANY IID_ISampleGrabber =
	{ 0x6b652fff, 0x11fe, 0x4fce, { 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f } };
EXTERN_C const IID DECLSPEC_SELECTANY IID_ISampleGrabberCB =
	{ 0x0579154a, 0x2b53, 0x4994, { 0xb0, 0xd0, 0xe7, 0x73, 0x14, 0x8e, 0xff, 0x85 } };

#endif // DCAMC_COMPAT_QEDIT_H
