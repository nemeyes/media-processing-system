#pragma once

class dk_rtmp_source_filter;
class dk_rtmp_audio_source_stream : public CSourceStream, public IAMPushSource
{
	DECLARE_IUNKNOWN;
	friend class dk_rtmp_source_filter;
public:
	dk_rtmp_audio_source_stream(HRESULT * hr, CSource * ms, LPCWSTR name);
	virtual ~dk_rtmp_audio_source_stream(void);


	/// override this to publicise our interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	HRESULT GetMediaType(CMediaType *type);
	HRESULT DecideBufferSize(IMemAllocator *alloc, ALLOCATOR_PROPERTIES *properties);
	HRESULT FillBuffer(IMediaSample *ms);
	STDMETHODIMP Notify(IBaseFilter *self, Quality quality) { return NOERROR; }

	HRESULT OnThreadCreate(VOID);
	HRESULT OnThreadDestroy(VOID);
	HRESULT OnThreadStartPlay(VOID);

	STDMETHODIMP GetPushSourceFlags(ULONG *flags);
	STDMETHODIMP SetPushSourceFlags(ULONG flags);

	STDMETHODIMP GetStreamOffset(REFERENCE_TIME *offset);
	STDMETHODIMP SetStreamOffset(REFERENCE_TIME offset);

	STDMETHODIMP GetMaxStreamOffset(REFERENCE_TIME *maxoffset);
	STDMETHODIMP SetMaxStreamOffset(REFERENCE_TIME maxoffset);

	STDMETHODIMP GetLatency(REFERENCE_TIME *latency);

private:
	DWORD ThreadProc(VOID);
	HRESULT DoBufferProcessingLoop(VOID);

protected:
	//CRefTime _sample_time; // The time stamp for each sample
	//int _repeat_time; // Time in msec between frames

	//BOOL _is_first_sample_delivered;
	//CRefTime _rt_sample_time; // The time to be stamped on each sample
	//LONGLONG _sample_media_time_start;
};