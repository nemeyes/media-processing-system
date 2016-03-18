#pragma once

#include <dk_yuvsource_reader.h>

class dk_yuvsource_stream : public CSourceStream/*,
							   public IAMPushSource*/
{
	DECLARE_IUNKNOWN;
public:
	dk_yuvsource_stream(HRESULT * hr, CSource * filter, LPCTSTR image_path, int32_t width, int32_t height, int32_t fps);
	virtual ~dk_yuvsource_stream(void);

	/// override this to publicise our interfaces
	STDMETHODIMP	NonDelegatingQueryInterface(REFIID riid, void **ppv);

	HRESULT CheckMediaType(const CMediaType * mt);
	HRESULT GetMediaType(int position, __inout CMediaType * mt);
	//HRESULT GetMediaType(CMediaType * mt);
	HRESULT DecideBufferSize(IMemAllocator * alloc, ALLOCATOR_PROPERTIES * request);
	HRESULT FillBuffer(IMediaSample * ms);

	/*STDMETHODIMP GetPushSourceFlags(ULONG *flags);
	STDMETHODIMP SetPushSourceFlags(ULONG flags);
	STDMETHODIMP GetStreamOffset(REFERENCE_TIME *offset);
	STDMETHODIMP SetStreamOffset(REFERENCE_TIME offset);
	STDMETHODIMP GetMaxStreamOffset(REFERENCE_TIME *maxoffset);
	STDMETHODIMP SetMaxStreamOffset(REFERENCE_TIME maxoffset);
	STDMETHODIMP GetLatency(REFERENCE_TIME *prtLatency);*/
	// Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
	STDMETHODIMP Notify(IBaseFilter * filter, Quality q)
	{
		return E_FAIL;
	}

private:
	int32_t _width;
	int32_t _height;
	dk_yuvsource_reader _reader;
	unsigned int _stride;

	CRefTime _sample_time;
	int _frame_number;
	const REFERENCE_TIME _frame_length;
};