#pragma once
#include <process.h>
#include <time.h>
#include <dk_x264_encoder.h>

#define g_szFilterName L"DK x264 Encode filter"

// {8331675F-E5F4-4B37-A8E9-46365B6E619F}
DEFINE_GUID(CLSID_DK_X264_ENCODE_FILTER,
	0x8331675f, 0xe5f4, 0x4b37, 0xa8, 0xe9, 0x46, 0x36, 0x5b, 0x6e, 0x61, 0x9f);

// {1E097165-13D4-45A8-9510-880EAB1F6C18}
DEFINE_GUID(CLSID_DK_X264_ENCODE_FILTER_PROPERTIES,
	0x1e097165, 0x13d4, 0x45a8, 0x95, 0x10, 0x88, 0xe, 0xab, 0x1f, 0x6c, 0x18);

class dk_x264_encode_filter : public CTransformFilter, public ISpecifyPropertyPages
{
public:
	dk_x264_encode_filter(LPUNKNOWN unk, HRESULT *hr);
	virtual ~dk_x264_encode_filter(void);

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
	dk_x264_encoder::configuration_t _config;
	dk_x264_encoder	*_encoder;
};