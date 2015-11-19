#pragma once
#include <process.h>
#include <time.h>
#include <dk_celt_decoder.h>

/* TODO
1. validate various bitpersample
2. make properties page
*/

#define g_szFilterName L"DK CELT Decode filter"

// {B8D75DD0-42AD-4E60-8849-55D58E55745C}
DEFINE_GUID(CLSID_DK_CELT_DECODE_FILTER,
	0xb8d75dd0, 0x42ad, 0x4e60, 0x88, 0x49, 0x55, 0xd5, 0x8e, 0x55, 0x74, 0x5c);

// {A6708F71-66AE-4DC2-A32F-969EA0EAC5C3}
DEFINE_GUID(CLSID_DK_CELT_DECODE_FILTER_PROPERTIES,
	0xa6708f71, 0x66ae, 0x4dc2, 0xa3, 0x2f, 0x96, 0x9e, 0xa0, 0xea, 0xc5, 0xc3);

#if 0

// {0DFEE29C-12CD-418E-A08A-36FF4D684236}
DEFINE_GUID(MEDIASUBTYPE_CELT,
	0xdfee29c, 0x12cd, 0x418e, 0xa0, 0x8a, 0x36, 0xff, 0x4d, 0x68, 0x42, 0x36);

// {212B4516-A75A-4220-90B1-7A16822856AE}
DEFINE_GUID(KSDATAFORMAT_SUBTYPE_CELT,
	0x212b4516, 0xa75a, 0x4220, 0x90, 0xb1, 0x7a, 0x16, 0x82, 0x28, 0x56, 0xae);

#else

// {B940AE21-195E-4CE6-B324-E703AE733AEC}
DEFINE_GUID(MEDIASUBTYPE_CELT,
	0xb940ae21, 0x195e, 0x4ce6, 0xb3, 0x24, 0xe7, 0x3, 0xae, 0x73, 0x3a, 0xec);

// {1DAD5025-02FA-4330-9A60-EC0121E3CE3D}
DEFINE_GUID(KSDATAFORMAT_SUBTYPE_CELT,
	0x1dad5025, 0x2fa, 0x4330, 0x9a, 0x60, 0xec, 0x1, 0x21, 0xe3, 0xce, 0x3d);

#endif

class dk_celt_decode_filter : public CTransformFilter, 
							 public ISpecifyPropertyPages
{
public:
	dk_celt_decode_filter(LPUNKNOWN unk, HRESULT *hr);
	~dk_celt_decode_filter(VOID);

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

private:
	bool _got_time;
	REFERENCE_TIME _start_time;
	unsigned long long _time_count;

	//unsigned char _extra_data[100];
	//unsigned long _extra_data_size;
	//int _channels;
	//int _samplerate;
	//int _bitpersamples;
	dk_celt_decoder::configuration_t _config;
	dk_celt_decoder	* _decoder;
};