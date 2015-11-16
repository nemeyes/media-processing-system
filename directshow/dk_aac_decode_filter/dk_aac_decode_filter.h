#pragma once
#include <process.h>
#include <time.h>
#include <dk_aac_decoder.h>

/* TODO
1. validate various bitpersample
2. make properties page
*/

#define g_szFilterName L"DK AAC Decode filter"

		
// {B5D4DC1B-8EF0-48DF-B641-48DE829C8459}
DEFINE_GUID(CLSID_DK_AAC_DECODE_FILTER,
	0xb5d4dc1b, 0x8ef0, 0x48df, 0xb6, 0x41, 0x48, 0xde, 0x82, 0x9c, 0x84, 0x59);

// {840A8628-8D7E-4E12-94AA-91A6C1908C6C}
DEFINE_GUID(CLSID_DK_AAC_DECODE_FILTER_PROPERTIES,
	0x840a8628, 0x8d7e, 0x4e12, 0x94, 0xaa, 0x91, 0xa6, 0xc1, 0x90, 0x8c, 0x6c);


class dk_aac_decode_filter : public CTransformFilter, 
							 public ISpecifyPropertyPages
{
public:
	dk_aac_decode_filter(LPUNKNOWN unk, HRESULT *hr);
	~dk_aac_decode_filter(VOID);

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

	unsigned char _extra_data[100];
	unsigned long _extra_data_size;
	//int _channels;
	//int _samplerate;
	//int _bitpersamples;
	dk_aac_decoder::configuration_t _config;
	dk_aac_decoder	*_decoder;
};