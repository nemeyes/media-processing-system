#pragma once

class dk_screencapture_filter;
class dk_screencapture_filter_stream : public CSourceStream, 
										  public IAMStreamConfig, 
										  public IKsPropertySet //CSourceStream is ... CBasePin
{

public:
    int m_iFrameNumber;

protected:

    //int m_FramesWritten;				// To track where we are
    REFERENCE_TIME					m_rtFrameLength; // also used to get the fps
	// float m_fFps; use the method to get this now
	REFERENCE_TIME					previousFrameEndTime;

    RECT							m_rScreen;// Rect containing screen coordinates we are currently "capturing"

    int								getNegotiatedFinalWidth();
    int								getNegotiatedFinalHeight();                   

	int								m_iCaptureConfigWidth;
	int								m_iCaptureConfigHeight;

    //CMediaType m_MediaType;
    //CImageDisplay m_Display;            // Figures out our media type for us
	
	dk_screencapture_filter		*m_pParent;

	HDC								hScrDc;
	HBITMAP							hRawBitmap;

	//CCritSec m_cSharedState;            // Protects our internal state use CAutoLock cAutoLock(m_pFilter->pStateLock()); instead

	bool							m_bFormatAlreadySet;
	bool							m_bConvertToI420;
	bool							m_bUseCaptureBlt;
	//int m_iScreenBitDepth;

	float							GetFps();

	boolean							m_bReReadRegistry;
	boolean							m_bDeDupe;
	int								m_millisToSleepBeforePollForChanges;
	HWND							m_iHwndToTrack;
    void CopyScreenToDataBlock(HDC hScrDc, BYTE *pData, BITMAPINFO *pHeader, IMediaSample *pSample);
	void doJustBitBltOrScaling(HDC hMemDC, int nWidth, int nHeight,int nDestWidth,int nDestHeight, HDC hScrDC, int nX, int nY);
	void doDIBits(HDC hScrDC, HBITMAP hRawBitmap, int nHeightScanLines, BYTE *pData, BITMAPINFO *pHeader);

    BYTE *pOldData;

	int m_iStretchToThisConfigWidth;
    int m_iStretchToThisConfigHeight;
    int m_iStretchMode;

	int getCaptureDesiredFinalWidth();
	int getCaptureDesiredFinalHeight();

public:

	//CSourceStream
	HRESULT OnThreadCreate(void);

    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv); 
    STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); } // gets called often...
    STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }


     //////////////////////////////////////////////////////////////////////////
    //  IAMStreamConfig
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC);

	dk_screencapture_filter_stream(HRESULT *phr, dk_screencapture_filter *pFilter);
	~dk_screencapture_filter_stream();

    // Override the version that offers exactly one media type
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);
    
    // Set the agreed media type and set up the necessary parameters
    HRESULT SetMediaType(const CMediaType *pMediaType);

    // Support multiple display formats (CBasePin)
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    // IQualityControl
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

	
    //////////////////////////////////////////////////////////////////////////
    //  IKsPropertySet
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
    HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData,DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

private:
	void reReadCurrentPosition(int isReRead);

};
