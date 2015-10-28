#pragma once
#include <process.h>
#include <time.h>
#include <dk_ff_video_decoder.h>

#define g_szFilterName L"DK ffmpeg video decode filter"

// {937EE76C-5881-4558-971D-E168E1A2C672}
DEFINE_GUID(CLSID_DK_FFMPEG_VIDEO_DECODE_FILTER,
	0x937ee76c, 0x5881, 0x4558, 0x97, 0x1d, 0xe1, 0x68, 0xe1, 0xa2, 0xc6, 0x72);

// {BAE73B0F-C78B-454C-ACAB-22FA14AD064D}
DEFINE_GUID(CLSID_DK_FFMPEG_VIDEO_DECODE_FILTER_PROPERTIES,
	0xbae73b0f, 0xc78b, 0x454c, 0xac, 0xab, 0x22, 0xfa, 0x14, 0xad, 0x6, 0x4d);

class dk_ff_video_decode_filter : public CTransformFilter, public ISpecifyPropertyPages
{
public:
	dk_ff_video_decode_filter(LPUNKNOWN unk, HRESULT *hr);
	virtual ~dk_ff_video_decode_filter(void);

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
	dk_ff_video_decoder::configuration_t _config;
	dk_ff_video_decoder	* _decoder;
};