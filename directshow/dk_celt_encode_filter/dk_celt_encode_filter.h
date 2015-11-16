#pragma once
#include <process.h>
#include <time.h>
#include <dk_celt_encoder.h>

#define g_szFilterName L"DK CELT Encode filter"

// {DA022662-729D-48BD-A187-1A0B5E43DF50}
DEFINE_GUID(CLSID_DK_CELT_ENCODE_FILTER,
	0xda022662, 0x729d, 0x48bd, 0xa1, 0x87, 0x1a, 0xb, 0x5e, 0x43, 0xdf, 0x50);

// {51F45A4F-CBD9-41BC-80F9-D89EEED33BF0}
DEFINE_GUID(CLSID_DK_CELT_ENCODE_FILTER_PROPERTIES,
	0x51f45a4f, 0xcbd9, 0x41bc, 0x80, 0xf9, 0xd8, 0x9e, 0xee, 0xd3, 0x3b, 0xf0);

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

class dk_celt_encode_filter : public CTransformFilter, public ISpecifyPropertyPages
{
public:
	dk_celt_encode_filter(LPUNKNOWN unk, HRESULT *hr);
	~dk_celt_encode_filter(VOID);

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN unk, HRESULT *hr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);


	//CTransformFilter
	virtual HRESULT CheckInputType(const CMediaType *type);
	virtual HRESULT GetMediaType(int position, CMediaType *type);
	virtual HRESULT CheckTransform(const CMediaType *itype, const CMediaType *otype);
	virtual HRESULT DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *pprop);
	//virtual HRESULT Transform(IMediaSample *src, IMediaSample *dst);
	

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
	virtual HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType * mt);

	virtual HRESULT Receive(IMediaSample *pSample);
	virtual HRESULT GetDeliveryBuffer(IMediaSample **sample);

	// if you override Receive, you may need to override these three too
	virtual HRESULT EndOfStream(void);
	virtual HRESULT BeginFlush(void);
	virtual HRESULT EndFlush(void);
	virtual HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	//ISpecifyPropertyPages
	STDMETHODIMP	GetPages(CAUUID *pages);

private:
	unsigned long _frame_size;
	int64_t _frame_done;
	bool _got_time;
	REFERENCE_TIME _rt_begin;
	int16_t * _buffer;
	int32_t _samples;

	dk_celt_encoder::configuration_t _config;
	dk_celt_encoder	*_encoder;
};