#pragma once

#include <dk_image_loader.h>

class dk_image_source_stream : public CSourceStream/*,
							   public IAMPushSource*/
{
	DECLARE_IUNKNOWN;
public:
	dk_image_source_stream(HRESULT * hr, CSource * filter, LPCTSTR image_path);
	virtual ~dk_image_source_stream(void);

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
	dk_image_loader _loader;
	bool _bimgloaded;
	unsigned int _stride;

	CRefTime _sample_time;
	int _frame_number;
	const REFERENCE_TIME _frame_length;
};