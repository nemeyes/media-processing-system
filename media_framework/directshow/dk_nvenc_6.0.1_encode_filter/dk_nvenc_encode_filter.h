#pragma once
#include <process.h>
#include <time.h>
#include <dk_nvenc_encoder.h>

#define g_szFilterName L"DK NVENC Encode filter"

// {41F2B1D9-12C9-4178-A6BA-37896967F335}
DEFINE_GUID(CLSID_DK_NVENC_ENCODE_FILTER,
	0x41f2b1d9, 0x12c9, 0x4178, 0xa6, 0xba, 0x37, 0x89, 0x69, 0x67, 0xf3, 0x35);

// {0EB063EA-6AD9-4561-AC09-4F7B9105B3C1}
DEFINE_GUID(CLSID_DK_NVENC_ENCODE_FILTER_PROPERTIES,
	0xeb063ea, 0x6ad9, 0x4561, 0xac, 0x9, 0x4f, 0x7b, 0x91, 0x5, 0xb3, 0xc1);

class dk_nvenc_encode_filter : public CTransformFilter, public ISpecifyPropertyPages
{
public:
	dk_nvenc_encode_filter(LPUNKNOWN unk, HRESULT *hr);
	virtual ~dk_nvenc_encode_filter(void);

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

	//ISpecifyPropertyPages
	STDMETHODIMP	GetPages(CAUUID *pages);

	int RowWidth(int w)
	{
		if (w % 4)
			w += 4 - w % 4;
		return w;
	};

private:
	unsigned int _pitch;
	dk_nvenc_encoder::configuration_t _config;
	dk_nvenc_encoder * _encoder;
};