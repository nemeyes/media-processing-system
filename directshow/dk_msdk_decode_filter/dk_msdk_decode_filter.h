#pragma once
#include <process.h>
#include <time.h>
#include <initguid.h>
#include <dk_media_sdk_decoder.h>

#define g_szFilterName L"DK Media SDK Decode filter"

// {B8921CD0-A02A-4678-BD00-E8E6DF75DCFC}
DEFINE_GUID(CLSID_DK_MEDIA_SDK_DECODE_FILTER,
	0xb8921cd0, 0xa02a, 0x4678, 0xbd, 0x0, 0xe8, 0xe6, 0xdf, 0x75, 0xdc, 0xfc);

// {3E1DB5F1-23D5-448E-9A4B-B9B1EC741742}
DEFINE_GUID(CLSID_DK_MEDIA_SDK_DECODE_FILTER_PROPERTIES,
	0x3e1db5f1, 0x23d5, 0x448e, 0x9a, 0x4b, 0xb9, 0xb1, 0xec, 0x74, 0x17, 0x42);

class dk_msdk_decode_filter : public CTransformFilter, public ISpecifyPropertyPages
{
public:
	dk_msdk_decode_filter(LPUNKNOWN unk, HRESULT *hr);
	~dk_msdk_decode_filter(VOID);

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN unk, HRESULT *hr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);


	//CTransformFilter
	virtual HRESULT CheckInputType(const CMediaType *type);
	virtual HRESULT GetMediaType(int position, CMediaType *type);
	virtual HRESULT CheckTransform(const CMediaType *itype, const CMediaType *otype);
	virtual HRESULT DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *pprop);
	virtual HRESULT Transform(IMediaSample *src, IMediaSample *dst);

	// you can also override these if you want to know about streaming
	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();

	// chance to grab extra interfaces on connection
	virtual HRESULT CheckConnect(PIN_DIRECTION direction, IPin *pin);
	virtual HRESULT BreakConnect(PIN_DIRECTION direction);
	virtual HRESULT	CompleteConnect(PIN_DIRECTION direction, IPin *pin);

	// override if you can do anything constructive with quality notifications
	virtual HRESULT AlterQuality(Quality quality);

	// override this to know when the media type is actually set
	virtual HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *type);

	// if you override Receive, you may need to override these three too
	virtual HRESULT EndOfStream(void);
	virtual HRESULT BeginFlush(void);
	virtual HRESULT EndFlush(void);
	virtual HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	////IVmxnetVideoDecodeSetting
	//STDMETHODIMP	SetMediaType(VMXNET_SUB_MEDIA_TYPE media_type);
	//STDMETHODIMP	SetResolution(USHORT width, USHORT height);
	//STDMETHODIMP	GetMediaType(VMXNET_SUB_MEDIA_TYPE &media_type);
	//STDMETHODIMP	GetResolution(USHORT &width, USHORT &height);

	//ISpecifyPropertyPages
	STDMETHODIMP	GetPages(CAUUID *pages);

	int RowWidth(int w)
	{
		if (w % 4)
			w += 4 - w % 4;
		return w;
	};

private:
	unsigned int _width;
	unsigned int _height;
	unsigned int _stride;
	//unsigned int _pitch;
	dk_media_sdk_decoder * _decoder;
};