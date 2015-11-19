#pragma once
#include <process.h>
#include <time.h>
#include <dk_aac_encoder.h>

/* TODO
1. verify various input format(PCM 8bit, 24bit, 32bit)
2. verify various output format(ADTS, LATM)
3. verify various bitrate (32000,
							40000,
							48000,
							56000,
							64000,
							72000,
							80000,
							88000,
							96000,
							104000,
							112000,
							120000,
							128000,
							140000,
							160000,
							192000,
							224000,
							256000)
4. find out the reason why timestamp use input samples size(_input_samples)
5. make properties page
*/

#define g_szFilterName L"DK AAC Encode filter"


// {D06ECB77-5483-4278-97F8-F8BE3185A8C6}
DEFINE_GUID(CLSID_DK_AAC_ENCODE_FILTER,
	0xd06ecb77, 0x5483, 0x4278, 0x97, 0xf8, 0xf8, 0xbe, 0x31, 0x85, 0xa8, 0xc6);
		

// {A5C18F00-78DB-4997-89C6-D0EA38FC235B}
DEFINE_GUID(CLSID_DK_AAC_ENCODE_FILTER_PROPERTIES,
	0xa5c18f00, 0x78db, 0x4997, 0x89, 0xc6, 0xd0, 0xea, 0x38, 0xfc, 0x23, 0x5b);

class dk_aac_encode_filter : public CTransformFilter, public ISpecifyPropertyPages
{
public:
	dk_aac_encode_filter(LPUNKNOWN unk, HRESULT *hr);
	~dk_aac_encode_filter(VOID);

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
	//REFERENCE_TIME _start_time;
	//unsigned long long _time_count;

	unsigned long _frame_size;
	//unsigned long _max_output_bytes;
	int64_t _frame_done;
	bool _got_time;
	REFERENCE_TIME _rt_begin;
	int16_t * _buffer;
	int32_t _samples;

	dk_aac_encoder::configuration_t _config;
	dk_aac_encoder	*_encoder;
};