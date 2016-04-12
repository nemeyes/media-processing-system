#pragma once
#include <process.h>
#include <time.h>
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <mfidl.h>	
#include <evr.h>
#include <dk_video_decode_filter.h>

#define g_szFilterName L"DK DXVA2 Decode filter"

// {A8C81396-763E-403A-B2D7-6D6F27482CA2}
DEFINE_GUID(CLSID_DK_DXVA2_DECODE_FILTER,
	0xa8c81396, 0x763e, 0x403a, 0xb2, 0xd7, 0x6d, 0x6f, 0x27, 0x48, 0x2c, 0xa2);

// {AD5C68B0-AAC8-4F03-94A1-8A5506B78409}
DEFINE_GUID(CLSID_DK_DXVA2_DECODE_FILTER_PROPERTIES,
	0xad5c68b0, 0xaac8, 0x4f03, 0x94, 0xa1, 0x8a, 0x55, 0x6, 0xb7, 0x84, 0x9);

class dk_dxva2_decode_filter : public dk_video_decode_filter, public ISpecifyPropertyPages
{
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN unk, HRESULT *hr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	//CTransformFilter
	virtual HRESULT CheckInputType(const CMediaType * type);
	virtual HRESULT GetMediaType(int position, CMediaType * type);
	virtual HRESULT CheckTransform(const CMediaType * itype, const CMediaType * otype);
	virtual HRESULT DecideBufferSize(IMemAllocator * allocator, ALLOCATOR_PROPERTIES * pprop);
	virtual HRESULT Transform(IMediaSample * src, IMediaSample * dst);

	//dk_dxva2_decode_filter
	virtual HRESULT Transform(IMediaSample * src);

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
	STDMETHODIMP	GetPages(CAUUID * pages);


private:
	dk_dxva2_decode_filter(LPUNKNOWN unk, HRESULT * hr);
	~dk_dxva2_decode_filter(void);

	CBasePin * get_pin(int n);
	HRESULT configure_dxva2(IPin * pin);
	HRESULT set_evr4dxva2(IPin * pin);
	HRESULT find_decoder_configuration(IDirectXVideoDecoderService *  decoder_service, const GUID & guid_decoder, DXVA2_VideoDesc * video_description, DXVA2_ConfigPictureDecode * selected_config, BOOL * found_dxva2_configuration);
	
	BOOL is_supported_decoder_mode(const GUID & mode);
	BOOL is_supported_decoder_config(const DXVA2_ConfigPictureDecode & config);
	void fill_video_description(DXVA2_VideoDesc * description);

	DWORD get_aligned_dimension(DWORD dimension);

	HRESULT decode(IMediaSample *src, IMediaSample *dst);


	void alloc_execute_params(int size);

private:
	//IDirectXVideoDecoderService * _dxva2_decoder_service;
	DXVA2_ConfigPictureDecode  _dxva2_decoder_config;
	GUID _dxva2_decoder_guid;
	HANDLE _dxva2_device;
	FOURCC _fcc_output_format;

	IDirectXVideoDecoder * _dxva2_decoder;
	DXVA2_DecodeExecuteParams _dxva2_execute_params;


	DWORD _surface_width;
	DWORD _surface_height;

	//unsigned int _width;
	//unsigned int _height;
	//unsigned int _stride;
};