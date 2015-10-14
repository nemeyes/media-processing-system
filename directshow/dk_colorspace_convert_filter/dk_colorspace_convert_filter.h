#pragma once
#include <process.h>
#include <time.h>

#define g_szFilterName L"DK Colorspace Convert Filter"

// {96143A18-CBD3-406D-8DD0-319E8E2FCD9D}
DEFINE_GUID(CLSID_DK_COLORSPACE_CONVERT_FILTER,
	0x96143a18, 0xcbd3, 0x406d, 0x8d, 0xd0, 0x31, 0x9e, 0x8e, 0x2f, 0xcd, 0x9d);

// {62569E6B-B718-4256-BFB5-85A27D4D8E8C}
DEFINE_GUID(CLSID_DK_COLORSPACE_CONVERT_FILTER_PROPERTIES,
	0x62569e6b, 0xb718, 0x4256, 0xbf, 0xb5, 0x85, 0xa2, 0x7d, 0x4d, 0x8e, 0x8c);

class dk_colorspace_converter;
class dk_colorspace_convert_filter : public CTransformFilter,
	public ISpecifyPropertyPages
{
public:
	dk_colorspace_convert_filter(LPUNKNOWN unk, HRESULT *hr);
	~dk_colorspace_convert_filter(VOID);

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
	bool _flip;
	int _width;
	int _height;
	int _dst_stride;
	int _ics;
	int _ocs;
	dk_colorspace_converter * _converter;
};