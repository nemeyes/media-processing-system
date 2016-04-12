#pragma once

#include <process.h>
#include <time.h>
#include <dk_media_source.h>
#include "dk_rtmp_subscriber.h"

#define E_SEEK (0x80008888L)
#define g_sz_filter_name L"DK rtmp source filter"

// {208BB56D-B523-4CB4-81FE-B367CC6B1BAB}
DEFINE_GUID(CLSID_DK_RTMP_SOURCE_FILTER,
	0x208bb56d, 0xb523, 0x4cb4, 0x81, 0xfe, 0xb3, 0x67, 0xcc, 0x6b, 0x1b, 0xab);

// {F5F16F5F-DC3D-4E2B-A314-4FEAAAA090E2}
DEFINE_GUID(CLSID_DK_RTMP_SOURCE_FILTER_PROPERTIES,
	0xf5f16f5f, 0xdc3d, 0x4e2b, 0xa3, 0x14, 0x4f, 0xea, 0xaa, 0xa0, 0x90, 0xe2);



class dk_rtmp_source_filter : public CSource,
							  public IFileSourceFilter,
							  public IAMFilterMiscFlags,
							  public ISpecifyPropertyPages,
							  public IRTMPClient
{
	friend class dk_rtmp_video_source_stream;
	friend class dk_rtmp_audio_source_stream;

public:
	dk_rtmp_source_filter(LPUNKNOWN unk, HRESULT *hr);
	virtual ~dk_rtmp_source_filter(void);

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN unk, HRESULT * hr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	STDMETHODIMP Run(REFERENCE_TIME start);
	STDMETHODIMP Stop(void);
	STDMETHODIMP Pause(void);
	//STDMETHODIMP GetState( DWORD millsecond_timeout, FILTER_STATE *state );

	//IFileSourceFilter
	STDMETHODIMP Load(LPCOLESTR file_name, const AM_MEDIA_TYPE *type);
	STDMETHODIMP GetCurFile(LPOLESTR *file_name, AM_MEDIA_TYPE *type);

	//IAMFilterMiscFlags
	ULONG STDMETHODCALLTYPE GetMiscFlags(void);

	//ISpecifyPropertyPages
	STDMETHODIMP GetPages(CAUUID * pages);

	//IRTMPClient
	STDMETHODIMP SetUrl(BSTR url);
	STDMETHODIMP SetUsername(BSTR username);
	STDMETHODIMP SetPassword(BSTR password);
	STDMETHODIMP SetRecvOption(USHORT option);
	STDMETHODIMP SetRecvTimeout(ULONGLONG timeout);
	STDMETHODIMP SetConnectionTimeout(ULONGLONG timeout);
	STDMETHODIMP SetRepeat(BOOL repeat);
	STDMETHODIMP GetUrl(BSTR * url);
	STDMETHODIMP GetUsername(BSTR * username);
	STDMETHODIMP GetPassword(BSTR * password);
	STDMETHODIMP GetRecvOption(USHORT & option);
	STDMETHODIMP GetRecvTimeout(ULONGLONG & timeout);
	STDMETHODIMP GetConnectionTimeout(ULONGLONG & timeout);
	STDMETHODIMP GetRepeat(BOOL & repeat);

private:
	dk_rtmp_subscriber _subscriber;
};
