/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice, 
 this list of conditions and the following disclaimer in the 
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/**  
 ********************************************************************************
 * @file <MftAsyncColorConvert.cpp>                          
 *                                       
 * @brief This file contains functions for async MFT class
 *         
 ********************************************************************************
 */
#include <evr.h>
#include <mfapi.h>
#include <mftransform.h>
#include <Mferror.h>
#include "MftAsyncColorConvert.h"

class ColorConvertTask: public IMFAsyncCallback
{
public:

    /**
     *   @brief CreateInstance. Fabric method.
     */
    static HRESULT CreateInstance(AsyncColorConvert* colorConvert,
                    IMFSample* sample, IMFAsyncCallback** instance)
    {
        if (nullptr == instance)
        {
            return E_POINTER;
        }

        *instance = new ColorConvertTask(colorConvert, sample);
        (*instance)->AddRef();

        return S_OK;
    }

    /**
     *   @brief IMFAsyncCallback::GetParameters().
     */
    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue)
    {
        return S_OK;
    }

    /**
     *   @brief IMFAsyncCallback::Invoke().
     */
    STDMETHODIMP Invoke(IMFAsyncResult *asyncResult)
    {
        if (nullptr == asyncResult)
        {
            return E_POINTER;
        }

        HRESULT colorConvertResult = _colorConvert->ColorConvertSample(_sample);

        HRESULT hr = asyncResult->SetStatus(colorConvertResult);
        RETURNIFFAILED(hr);

        return S_OK;
    }
    ;

    /**
     *   @brief IUnknown::QueryInterface().
     */
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
    {
        if (nullptr == ppv)
        {
            return E_POINTER;
        }

        if (iid == IID_IMFAsyncCallback)
        {
            *ppv = static_cast<IMFAsyncCallback*> (this);
        }
        else if (iid == IID_IUnknown)
        {
            *ppv = static_cast<IUnknown*> (this);
        }
        else
        {
            *ppv = nullptr;
            return E_NOINTERFACE;
        }

        return S_OK;
    }

    /**
     *   @brief IUnknown::AddRef().
     */
STDMETHODIMP_(ULONG) AddRef()
{
    _colorConvert->AddRef();

    return InterlockedIncrement(&mReferenceCount);
}

/**
 *   @brief IUnknown::Release().
 */
STDMETHODIMP_(ULONG) Release()
{
    _colorConvert->Release();

    ULONG uCount = InterlockedDecrement(&mReferenceCount);
    if (uCount == 0)
    {
        delete this;
    }

    return uCount;
}

private:

    AsyncColorConvert* _colorConvert;
    CComPtr<IMFSample> _sample;
    long mReferenceCount;

    ColorConvertTask(AsyncColorConvert* colorConvert, IMFSample* sample) :
        _colorConvert(colorConvert), _sample(sample), mReferenceCount(0)
    {
    }

    ~ColorConvertTask(void)
    {
    }
    ;
};

/** 
 *******************************************************************************
 *  @fn     AsyncColorConvert
 *  @brief  Constructor
 *******************************************************************************
 */
AsyncColorConvert::AsyncColorConvert() :
    mIsStreamStarted(false), mIsStreamEnded(false), mIsShutdown(false),
                    mIsDraining(false), mMarker(0), mReferenceCount(0),
                    mWorkQueueID(0), mOutputImageWidth(0),
                    mOutputImageHeight(0), mOutputImageSize(0)
{
    mMftBuilderObjPtr = new msdk_CMftBuilder;
}

/** 
 *******************************************************************************
 *  @fn     ~AsyncColorConvert
 *  @brief  Destructor
 *******************************************************************************
 */
AsyncColorConvert::~AsyncColorConvert()
{
    if (mEventQueue != nullptr)
    {
        mEventQueue->Shutdown();
    }

    if (mWorkQueueID != 0)
    {
        MFUnlockWorkQueue( mWorkQueueID);
    }
    delete mMftBuilderObjPtr;
}

/** 
 *******************************************************************************
 *  @fn     Init
 *  @brief  Initializes the async color convert MFT object
 *           
 *  @param[in] videoEffect     : Video effect object 
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::Init(VideoEffect* videoEffect)
{

    if (nullptr == videoEffect)
    {
        return E_POINTER;
    }

    mVideoEffect.reset(videoEffect);

    HRESULT hr;

    hr = MFCreateAttributes(&mAttributes, 3);
    RETURNIFFAILED(hr);

    if (mVideoEffect->isD3D11Aware())
    {
        hr = mAttributes->SetUINT32(BORROWED_MF_SA_D3D11_AWARE, 1);
        RETURNIFFAILED(hr);
    }

    if (mVideoEffect->isD3DAware())
    {
        hr = mAttributes->SetUINT32(MF_SA_D3D_AWARE, 1);
        RETURNIFFAILED(hr);
    }

    hr = mAttributes->SetUINT32(MF_TRANSFORM_ASYNC, TRUE);
    RETURNIFFAILED(hr);

    hr = mAttributes->SetUINT32(MFT_SUPPORT_DYNAMIC_FORMAT_CHANGE, TRUE);
    RETURNIFFAILED(hr);

    hr = MFCreateEventQueue(&mEventQueue);
    RETURNIFFAILED(hr);

    hr = MFAllocateWorkQueue(&mWorkQueueID);
    RETURNIFFAILED(hr);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  Adds reference count
 *          
 *
 *  @return ULONG : Updated reference count
 *******************************************************************************
 */
ULONG AsyncColorConvert::AddRef()
{
    return InterlockedIncrement(&mReferenceCount);
}

/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  Decreases reference count
 *          
 *
 *  @return ULONG : Updated reference count
 *******************************************************************************
 */
ULONG AsyncColorConvert::Release()
{
    ULONG uCount = InterlockedDecrement(&mReferenceCount);
    if (uCount == 0)
    {
        delete this;
    }

    return uCount;
}

/** 
 *******************************************************************************
 *  @fn     QueryInterface
 *  @brief  Queries interface
 *           
 *  @param[in] iid      : Reference ID of required interface 
 *  @param[out] ppv     : Pointer of interface object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (static_cast<IMFTransform*> (this));
    }
    else if (iid == __uuidof(IMFTransform))
    {
        *ppv = static_cast<IMFTransform*> (this);
    }
    else if (iid == __uuidof(IMFAttributes))
    {
        GetAttributes((IMFAttributes**) ppv);
    }
    else if (iid == __uuidof(IMFShutdown))
    {
        *ppv = static_cast<IMFShutdown*> (this);
    }
    else if (iid == __uuidof(IMFMediaEventGenerator))
    {
        *ppv = static_cast<IMFMediaEventGenerator*> (this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetAttributes
 *  @brief  Retrieves attribute
 *           
 *  @param[out] ppAttributes   : Pointer to attributes object 
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetAttributes(IMFAttributes** ppAttributes)
{
    if (ppAttributes == NULL)
    {
        return E_POINTER;
    }

    *ppAttributes = mAttributes;
    (*ppAttributes)->AddRef();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetStreamLimits
 *  @brief  Gets stream limit
 *           
 *  @param[out] pdwInputMinimum    : Pointer for minimum input value 
 *  @param[out] pdwInputMaximum    : Pointer for maximum input value 
 *  @param[out] pdwOutputMinimum   : Pointer for minimum output value
 *  @param[out] pdwOutputMaximum   : Pointer for maximum output value
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetStreamLimits(DWORD* pdwInputMinimum,
                DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum,
                DWORD* pdwOutputMaximum)
{
    if ((pdwInputMinimum == NULL) || (pdwInputMaximum == NULL)
                    || (pdwOutputMinimum == NULL) || (pdwOutputMaximum == NULL))
    {
        return E_POINTER;
    }

    *pdwInputMinimum = 1;
    *pdwInputMaximum = 1;
    *pdwOutputMinimum = 1;
    *pdwOutputMaximum = 1;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetStreamCount
 *  @brief  Gets stream count
 *           
 *  @param[out] pcInputStreams   : Pointer for input streams count
 *  @param[out] pcOutputStreams  : Pointer for output streams count
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetStreamCount(DWORD* pcInputStreams,
                DWORD* pcOutputStreams)
{
    if ((pcInputStreams == NULL) || (pcOutputStreams == NULL))
    {
        return E_POINTER;
    }

    *pcInputStreams = 1;
    *pcOutputStreams = 1;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetStreamIDs
 *  @brief  Gets stream IDs
 *           
 *  @param[in] dwInputIDArraySize   : Input ID array size
 *  @param[out] pdwInputIDs         : Pointer for input IDs array
 *  @param[in] dwOutputIDArraySize  : Pointer for output IDs array
 *  @param[out] pdwOutputIDs        : Pointer for output IDs array
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetStreamIDs(DWORD dwInputIDArraySize,
                DWORD* pdwInputIDs, DWORD dwOutputIDArraySize,
                DWORD* pdwOutputIDs)
{
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     GetInputStreamInfo
 *  @brief  Gets input stream info
 *           
 *  @param[in] dwInputStreamID      : Input stream ID
 *  @param[out] pStreamInfo         : Pointer for input stream info
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetInputStreamInfo(DWORD dwInputStreamID,
                MFT_INPUT_STREAM_INFO* pStreamInfo)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    Lock lock(mCritSec, true);

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    pStreamInfo->hnsMaxLatency = 0;
    pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES
                    | MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER;

    if (mInputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        HRESULT hr;

        UINT32 inputImageWidthInPixels = 0;
        UINT32 inputImageHeightInPixels = 0;
        hr = MFGetAttributeSize(mInputType, MF_MT_FRAME_SIZE,
                        &inputImageWidthInPixels, &inputImageHeightInPixels);
        RETURNIFFAILED(hr);

        GUID inputSubType;
        hr = mInputType->GetGUID(MF_MT_SUBTYPE, &inputSubType);
        RETURNIFFAILED(hr);

        DWORD inputCbImageSize = 0;
        if (IsEqualGUID(MFVideoFormat_NV12, inputSubType))
        {
            // Input to CC MFT is NV12
            hr = mMftBuilderObjPtr->getNV12ImageSize(inputImageWidthInPixels,
                            inputImageHeightInPixels, &inputCbImageSize);
        }
        else
        {
            hr = mMftBuilderObjPtr->getRGBImageSize(inputImageWidthInPixels,
                            inputImageHeightInPixels, &inputCbImageSize);
        }
        RETURNIFFAILED(hr);

        pStreamInfo->cbSize = inputCbImageSize;
    }

    pStreamInfo->cbMaxLookahead = 0;
    pStreamInfo->cbAlignment = 0;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputStreamInfo
 *  @brief  Gets output stream info
 *           
 *  @param[in] dwOutputStreamID     : Output stream ID
 *  @param[out] pStreamInfo         : Pointer for output stream info
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetOutputStreamInfo(DWORD dwOutputStreamID,
                MFT_OUTPUT_STREAM_INFO* pStreamInfo)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    Lock lock(mCritSec, true);

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES
                    | MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER
                    | MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE
                    | MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

    if (mOutputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        pStreamInfo->cbSize = mOutputImageSize;
    }

    pStreamInfo->cbAlignment = 0;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetInputStreamAttributes
 *  @brief  Gets input stream attributes
 *           
 *  @param[in] dwInputStreamID      : Input stream ID
 *  @param[out] ppAttributes        : Pointer for input stream attributes
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetInputStreamAttributes(DWORD dwInputStreamID,
                IMFAttributes** ppAttributes)
{
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputStreamAttributes
 *  @brief  Gets output stream attributes
 *           
 *  @param[in] dwOutputStreamID   : Output stream ID
 *  @param[out] ppAttributes      : Pointer for output stream attributes
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetOutputStreamAttributes(DWORD dwOutputStreamID,
                IMFAttributes** ppAttributes)
{
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     DeleteInputStream
 *  @brief  Deletes input stream
 *           
 *  @param[in] dwStreamID   :Input stream ID
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::DeleteInputStream(DWORD dwStreamID)
{
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     AddInputStreams
 *  @brief  Adds input streams
 *           
 *  @param[in] cStreams   : Count of stream to add
 *  @param[out] adwStreamIDs      : Pointer for added stream's IDs
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::AddInputStreams(DWORD cStreams, DWORD* adwStreamIDs)
{

    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     GetInputAvailableType
 *  @brief  Gets input available type
 *           
 *  @param[in] dwInputStreamID  : Input Stream ID
 *  @param[in] dwTypeIndex      : Type index
 *  @param[out] ppType          : Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetInputAvailableType(DWORD dwInputStreamID,
                DWORD dwTypeIndex, IMFMediaType** ppType)
{
    if (ppType == nullptr)
    {
        return E_INVALIDARG;
    }

    if (dwTypeIndex > 0)
    {
        return MF_E_NO_MORE_TYPES;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;

    GUID inputSubType;
    hr = mInputType->GetGUID(MF_MT_SUBTYPE, &inputSubType);
    RETURNIFFAILED(hr);

    if (IsEqualGUID(MFVideoFormat_RGB32, inputSubType)) // Input RGB and output NV12
    {
        hr = mMftBuilderObjPtr->createVideoType(&type, MFVideoFormat_RGB32,
                        TRUE, TRUE, NULL, NULL, 0U, 0U,
                        MFVideoInterlace_Progressive);
        RETURNIFFAILED(hr);
    }
    else
    {
        hr = mMftBuilderObjPtr->createVideoType(&type, MFVideoFormat_NV12,
                        TRUE, TRUE, NULL, NULL, 0U, 0U,
                        MFVideoInterlace_Progressive);
        RETURNIFFAILED(hr);

    }

    *ppType = type.Detach();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputAvailableType
 *  @brief  Gets output available type
 *           
 *  @param[in] dwOutputStreamID  : Output Stream ID
 *  @param[in] dwTypeIndex      : Type index
 *  @param[out] ppType          : Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetOutputAvailableType(DWORD dwOutputStreamID,
                DWORD dwTypeIndex, IMFMediaType** ppType)
{
    if (ppType == nullptr)
    {
        return E_INVALIDARG;
    }

    if (dwTypeIndex > 0)
    {
        return MF_E_NO_MORE_TYPES;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;

    GUID outputSubType;
    hr = mOutputType->GetGUID(MF_MT_SUBTYPE, &outputSubType);
    RETURNIFFAILED(hr);

    if (IsEqualGUID(MFVideoFormat_RGB32, outputSubType)) // Output RGB and input NV12
    {
        hr = mMftBuilderObjPtr->createVideoType(&type, MFVideoFormat_RGB32,
                        TRUE, TRUE, NULL, NULL, 0U, 0U,
                        MFVideoInterlace_Progressive);
        RETURNIFFAILED(hr);
    }
    else
    {
        hr = mMftBuilderObjPtr->createVideoType(&type, MFVideoFormat_NV12,
                        TRUE, TRUE, NULL, NULL, 0U, 0U,
                        MFVideoInterlace_Progressive);
        RETURNIFFAILED(hr);

    }

    *ppType = type.Detach();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     SetInputType
 *  @brief  Sets input media type
 *           
 *  @param[in] dwInputStreamID  : Input Stream ID
 *  @param[in] pType            : Media type object
 *  @param[in] dwFlags          : Flags
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::SetInputType(DWORD dwInputStreamID,
                IMFMediaType* pType, DWORD dwFlags)
{
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr;

    if (pType)
    {
        hr = OnCheckInputType(pType);
        RETURNIFFAILED(hr);
    }

    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);
    if (bReallySet)
    {
        hr = OnSetInputType(pType);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     SetOutputType
 *  @brief  Sets output media type
 *           
 *  @param[in] dwOutputStreamID     : Output Stream ID
 *  @param[in] pType                : Media type object
 *  @param[in] dwFlags              : Flags
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::SetOutputType(DWORD dwOutputStreamID,
                IMFMediaType* pType, DWORD dwFlags)
{
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    if (dwOutputStreamID != 0)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    HRESULT hr;

    if (pType)
    {
        hr = OnCheckOutputType(pType);
        RETURNIFFAILED(hr);
    }

    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);
    if (bReallySet)
    {
        hr = OnSetOutputType(pType);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetInputCurrentType
 *  @brief  Gets current input media type
 *           
 *  @param[in] dwInputStreamID  : Input Stream ID
 *  @param[out] ppType          : Pointer to Media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetInputCurrentType(DWORD dwInputStreamID,
                IMFMediaType** ppType)
{
    if (ppType == nullptr)
    {
        return E_POINTER;
    }

    if (dwInputStreamID != 0)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (!mInputType)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;
    hr = MFCreateMediaType(&type);
    RETURNIFFAILED(hr);

    hr = mInputType->CopyAllItems(type);
    RETURNIFFAILED(hr);

    *ppType = type.Detach();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputCurrentType
 *  @brief  Gets current output media type
 *           
 *  @param[in] dwOutputStreamID     : Output Stream ID
 *  @param[out] ppType              : Pointer to Media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetOutputCurrentType(DWORD dwOutputStreamID,
                IMFMediaType** ppType)
{
    if (ppType == nullptr)
    {
        return E_POINTER;
    }

    if (dwOutputStreamID != 0)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (nullptr == mOutputType)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;
    hr = MFCreateMediaType(&type);
    RETURNIFFAILED(hr);

    hr = mOutputType->CopyAllItems(type);
    RETURNIFFAILED(hr);

    *ppType = type.Detach();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetInputStatus
 *  @brief  Gets input status
 *           
 *  @param[in] dwInputStreamID      :Input Stream ID
 *  @param[out] pdwFlags            :Pointer to status flags
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetInputStatus(DWORD dwInputStreamID,
                DWORD* pdwFlags)
{
    if (pdwFlags == nullptr)
    {
        return E_POINTER;
    }

    if (dwInputStreamID != 0)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (mIsStreamStarted)
    {
        *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
    }
    else
    {
        *pdwFlags = 0;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputStatus
 *  @brief  Gets output status
 *           
 *  @param[out] pdwFlags        :Pointer to status flags
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetOutputStatus(DWORD *pdwFlags)
{
    if (pdwFlags == nullptr)
    {
        return E_POINTER;
    }

    Lock lock(mCritSec, true);

    if (!mOutputQueue.empty())
    {
        *pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
    }
    else
    {
        *pdwFlags = 0;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     SetOutputBounds
 *  @brief  Sets output bounds
 *           
 *  @param[in] hnsLowerBound        :Lower bound
 *  @param[in] hnsUpperBound        :Upper bound
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::SetOutputBounds(LONGLONG hnsLowerBound,
                LONGLONG hnsUpperBound)
{
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     ProcessEvent
 *  @brief  Processes event
 *           
 *  @param[in] dwInputStreamID      :Input stream ID
 *  @param[in] pEvent               :Event to process
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::ProcessEvent(DWORD dwInputStreamID,
                IMFMediaEvent* pEvent)
{
    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (nullptr == pEvent)
    {
        return E_POINTER;
    }

    if (dwInputStreamID != 0)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     ProcessMessage
 *  @brief  Processes message
 *           
 *  @param[in] eMessage     :Message to process
 *  @param[in] ulParam      :Parameters object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::ProcessMessage(MFT_MESSAGE_TYPE eMessage,
                ULONG_PTR ulParam)
{
    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (eMessage != MFT_MESSAGE_SET_D3D_MANAGER && (nullptr == mInputType
                    || nullptr == mOutputType))
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr;

    switch (eMessage)
    {
    case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
        mIsStreamStarted = true;
        mIsStreamEnded = false;

        hr = RequestInput();

        break;

    case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
        SetStreamEnded();
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
        StartDraining();
        break;

    case MFT_MESSAGE_COMMAND_FLUSH:
        FlushOutput();
        break;

    case MFT_MESSAGE_COMMAND_MARKER:
        SetMarker(ulParam);
        break;

    case MFT_MESSAGE_SET_D3D_MANAGER:
        mDeviceManager.Release();

        if (ulParam != 0)
        {
            mDeviceManager = (IUnknown*) ulParam;
        }

        break;

    default:
        break;
    };

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     ProcessInput
 *  @brief  Processes input
 *           
 *  @param[in] inputStreamID    :Input stream ID
 *  @param[in] sample           :Input media sample
 *  @param[in] flags            :Flags
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::ProcessInput(DWORD inputStreamID, IMFSample *sample,
                DWORD flags)
{
    if (sample == nullptr)
    {
        return E_POINTER;
    }

    if (flags != 0)
    {
        return E_INVALIDARG;
    }

    Lock lock(mCritSec, true);

    if (!IsValidInputStream(inputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (!mInputType || !mOutputType)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr;

    DWORD bufferCount = 0;
    hr = sample->GetBufferCount(&bufferCount);
    RETURNIFFAILED(hr);

    if (bufferCount == 0)
    {
        return E_FAIL;
    }

    if (bufferCount > 1)
    {
        return MF_E_SAMPLE_HAS_TOO_MANY_BUFFERS;
    }

    CComPtr < IMFAsyncCallback > colorConvertTask;
    ColorConvertTask::CreateInstance(this, sample, &colorConvertTask);

    CComPtr < IMFAsyncResult > colorConvertResult;

    hr = MFCreateAsyncResult(NULL, colorConvertTask, nullptr,
                    &colorConvertResult);
    RETURNIFFAILED(hr);

    hr = MFPutWorkItemEx(mWorkQueueID, colorConvertResult);
    RETURNIFFAILED(hr);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     ProcessOutput
 *  @brief  Processes output
 *           
 *  @param[in] dwFlags              :Flags
 *  @param[in] cOutputBufferCount   :Output buffer count
 *  @param[out] pOutputSamples      :Output media sample
 *  @param[out] pdwStatus           :Output status
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::ProcessOutput(DWORD dwFlags,
                DWORD cOutputBufferCount,
                MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus)
{
    if (dwFlags != 0)
    {
        return E_INVALIDARG;
    }

    if (pOutputSamples == NULL || pdwStatus == NULL)
    {
        return E_POINTER;
    }

    if (cOutputBufferCount != 1)
    {
        return E_INVALIDARG;
    }

    Lock lock(mCritSec, true);

    if (IsLocked())
    {
        return MF_E_TRANSFORM_ASYNC_LOCKED;
    }

    if (!mInputType || !mOutputType)
    {
        return MF_E_NOTACCEPTING;
    }

    if (mOutputQueue.empty())
    {
        return MF_E_UNEXPECTED;
    }

    *pdwStatus = 0;
    pOutputSamples[0].dwStatus = 0;
    pOutputSamples[0].pSample = mOutputQueue.front();

    mOutputQueue.pop();

    HRESULT hr;

    if (!mIsDraining)
    {
        hr = RequestInput();
        RETURNIFFAILED(hr);
    }

    if (mIsDraining && mOutputQueue.empty())
    {
        hr = SendDrainCompletedEvent();
        RETURNIFFAILED(hr);

        mIsDraining = false;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     OnCheckInputType
 *  @brief  Handle check input type
 *           
 *  @param[in] pmt          :Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::OnCheckInputType(IMFMediaType *pmt)
{
    return OnCheckMediaType(pmt);
}

/** 
 *******************************************************************************
 *  @fn     OnCheckOutputType
 *  @brief  Handle check output type
 *           
 *  @param[in] pmt          :Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::OnCheckOutputType(IMFMediaType *pmt)
{
    Lock lock(mCritSec, true);

    HRESULT hr;

    hr = OnCheckMediaType(pmt);
    RETURNIFFAILED(hr);

    MFVideoInterlaceMode interlace = MFVideoInterlace_Unknown;
    hr = pmt->GetUINT32(MF_MT_INTERLACE_MODE, (UINT32*) &interlace);
    RETURNIFFAILED(hr);

    if (interlace != MFVideoInterlace_Progressive)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    UINT32 width = 0;
    UINT32 height = 0;
    hr = MFGetAttributeSize(pmt, MF_MT_FRAME_SIZE, &width, &height);
    if (0 == width || 0 == height)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     OnCheckMediaType
 *  @brief  Handle check media type
 *           
 *  @param[in] mediaType            :Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::OnCheckMediaType(IMFMediaType* mediaType)
{
    HRESULT hr;

    GUID major_type;
    hr = mediaType->GetGUID(MF_MT_MAJOR_TYPE, &major_type);
    RETURNIFFAILED(hr);

    if (major_type != MFMediaType_Video)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    GUID subtype;
    hr = mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
    RETURNIFFAILED(hr);

    if ((subtype != MFVideoFormat_NV12) && (subtype != MFVideoFormat_RGB32))
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    UINT32 fixedSize;
    hr = mediaType->GetUINT32(MF_MT_FIXED_SIZE_SAMPLES, &fixedSize);
    RETURNIFFAILED(hr);

    if (fixedSize != TRUE)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    UINT32 samplesIndependent;
    hr = mediaType->GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT,
                    &samplesIndependent);
    RETURNIFFAILED(hr);

    if (samplesIndependent != TRUE)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     OnSetInputType
 *  @brief  Handle set input media type
 *           
 *  @param[in] pmt          :Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::OnSetInputType(IMFMediaType *pmt)
{
    mInputType.Release();
    mInputType = pmt;

    if (mInputType != nullptr && nullptr == mOutputType)
    {
        HRESULT hr;

        CComPtr < IMFMediaType > outputType;
        hr = MFCreateMediaType(&outputType);
        RETURNIFFAILED(hr);

        hr = mInputType->CopyAllItems(outputType);
        RETURNIFFAILED(hr);

        hr = outputType->SetUINT32(MF_MT_INTERLACE_MODE,
                        MFVideoInterlace_Progressive);
        RETURNIFFAILED(hr);

        mOutputType = outputType;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     OnSetOutputType
 *  @brief  Handle set output media type
 *           
 *  @param[in] pmt          :Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::OnSetOutputType(IMFMediaType *pmt)
{
    mOutputType.Release();
    mOutputType = pmt;

    return UpdateFormatInfo();
}

/** 
 *******************************************************************************
 *  @fn     ColorConvertSample
 *  @brief   Does transformation and put result to an output queue
 *           
 *  @param[in] sample       :Pointer to media type object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::ColorConvertSample(IMFSample* sample)
{
    Lock lock(mCritSec, true);

    HRESULT hr;

    CComPtr < IMFSample > outputSample;
    hr = MFCreateVideoSampleFromSurface(nullptr, &outputSample);
    RETURNIFFAILED(hr);

    ULONG_PTR deviceManagerPtr = reinterpret_cast<ULONG_PTR> (mDeviceManager.p);

    UINT32 useInteropVal = MFGetAttributeUINT32(mAttributes,
                    ATTRIBUTE_USE_INTEROP, TRUE);

    bool useInterop = SUCCEEDED(hr) && 1 == useInteropVal ? true : false;

    hr = mVideoEffect->process(deviceManagerPtr, mInputType, sample,
                    mOutputType, outputSample, useInterop);
    if (FAILED(hr))
    {
        HRESULT queueEventStatus = QueueEvent(MEError, GUID_NULL, hr, nullptr);
        RETURNIFFAILED(queueEventStatus);
    }

    static size_t callCounter = 0;
    callCounter++;

    mOutputQueue.push(outputSample.Detach());

    CComPtr < IMFMediaEvent > haveOutputEvent;
    hr = MFCreateMediaEvent(METransformHaveOutput, GUID_NULL, S_OK, NULL,
                    &haveOutputEvent);
    RETURNIFFAILED(hr);

    hr = mEventQueue->QueueEvent(haveOutputEvent);
    RETURNIFFAILED(hr);

    if (!mIsDraining && mOutputQueue.size() < 16)
    {
        hr = RequestInput();
        RETURNIFFAILED(hr);
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     RequestInput
 *  @brief   Requests input
 * 
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::RequestInput()
{
    if (!mIsStreamStarted || mIsStreamEnded)
    {
        return MF_E_NOTACCEPTING;
    }

    HRESULT hr;

    CComPtr < IMFMediaEvent > event;

    hr
                    = MFCreateMediaEvent(METransformNeedInput, GUID_NULL, S_OK,
                                    NULL, &event);
    RETURNIFFAILED(hr);

    hr = event->SetUINT32(MF_EVENT_MFT_INPUT_STREAM_ID, 0);
    RETURNIFFAILED(hr);

    hr = mEventQueue->QueueEvent(event);
    RETURNIFFAILED(hr);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     IsLocked
 *  @brief   is locked
 * 
 *  @return bool : true if locked else false
 *******************************************************************************
 */
bool AsyncColorConvert::IsLocked(void)
{
    UINT32 isUnlocked = MFGetAttributeUINT32(mAttributes,
                    MF_TRANSFORM_ASYNC_UNLOCK, TRUE);

    return !isUnlocked;
}

/** 
 *******************************************************************************
 *  @fn     SetStreamEnded
 *  @brief   Sets stream ended
 * 
 *  @return void:
 *******************************************************************************
 */
void AsyncColorConvert::SetStreamEnded()
{
    mIsStreamEnded = true;
}

/** 
 *******************************************************************************
 *  @fn     SendDrainCompletedEvent
 *  @brief   Sends drain completed event
 * 
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::SendDrainCompletedEvent()
{
    HRESULT hr;

    CComPtr < IMFMediaEvent > drainCompleteEvent;
    hr = MFCreateMediaEvent(METransformDrainComplete, GUID_NULL, S_OK, NULL,
                    &drainCompleteEvent);
    RETURNIFFAILED(hr);

    hr = drainCompleteEvent->SetUINT32(MF_EVENT_MFT_INPUT_STREAM_ID, 0);
    RETURNIFFAILED(hr);

    hr = mEventQueue->QueueEvent(drainCompleteEvent);
    RETURNIFFAILED(hr);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     StartDraining
 *  @brief   Start draining
 * 
 *  @return void:
 *******************************************************************************
 */
void AsyncColorConvert::StartDraining()
{
    mIsDraining = true;

    if (mOutputQueue.empty())
    {
        HRESULT hr;

        hr = SendDrainCompletedEvent();

        mIsDraining = false;
    }
}

/** 
 *******************************************************************************
 *  @fn     SetMarker
 *  @brief   Sets marker
 * 
 *  @return void:
 *******************************************************************************
 */
void AsyncColorConvert::SetMarker(ULONG_PTR marker)
{
    mMarker = marker;
}

/** 
 *******************************************************************************
 *  @fn     FlushOutput
 *  @brief   Flushes output
 * 
 *  @return void:
 *******************************************************************************
 */
void AsyncColorConvert::FlushOutput()
{
    while (!mOutputQueue.empty())
    {
        IMFSample* sample = mOutputQueue.front();
        mOutputQueue.pop();
        sample->Release();
    }
}

/** 
 *******************************************************************************
 *  @fn     UpdateFormatInfo
 *  @brief   Updates format info
 * 
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::UpdateFormatInfo()
{
    Lock lock(mCritSec, true);

    if (nullptr != mOutputType)
    {
        HRESULT hr;

        mOutputImageWidth = 0;
        mOutputImageHeight = 0;
        hr = MFGetAttributeSize(mOutputType, MF_MT_FRAME_SIZE,
                        &mOutputImageWidth, &mOutputImageHeight);
        RETURNIFFAILED(hr);

        GUID outputSubType;
        hr = mOutputType->GetGUID(MF_MT_SUBTYPE, &outputSubType);
        RETURNIFFAILED(hr);

        mOutputImageSize = 0;
        if (IsEqualGUID(MFVideoFormat_NV12, outputSubType)) // Output is NV12
        {
            hr = mMftBuilderObjPtr->getNV12ImageSize(mOutputImageWidth,
                            mOutputImageHeight, &mOutputImageSize);
            RETURNIFFAILED(hr);
        }
        else
        {
            hr = mMftBuilderObjPtr->getRGBImageSize(mOutputImageWidth,
                            mOutputImageHeight, &mOutputImageSize);
            RETURNIFFAILED(hr);
        }
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetEvent
 *  @brief   Gets event
 *           
 *  @param[in] dwFlags          :Flags
 *  @param[out] ppEvent         :Pointer to event object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent)
{
    return mEventQueue->GetEvent(dwFlags, ppEvent);
}

/** 
 *******************************************************************************
 *  @fn     BeginGetEvent
 *  @brief   Begins get event
 *           
 *  @param[in] pCallback            :Call back interface
 *  @param[out] punkState           :
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::BeginGetEvent(IMFAsyncCallback *pCallback,
                IUnknown *punkState)
{
    return mEventQueue->BeginGetEvent(pCallback, punkState);
}

/** 
 *******************************************************************************
 *  @fn     EndGetEvent
 *  @brief   Ends get event
 *           
 *  @param[out] pResult         :Async result interface
 *  @param[out] ppEvent         :
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::EndGetEvent(IMFAsyncResult *pResult,
                IMFMediaEvent **ppEvent)
{
    return mEventQueue->EndGetEvent(pResult, ppEvent);
}

/** 
 *******************************************************************************
 *  @fn     QueueEvent
 *  @brief   Queues event
 *           
 *  @param[in] met                  :Media event type
 *  @param[in] guidExtendedType     :GUID of extended type
 *  @param[in] hrStatus             :HR status
 *  @param[in] pvValue              :Object of property variant
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::QueueEvent(MediaEventType met,
                REFGUID guidExtendedType, HRESULT hrStatus,
                const PROPVARIANT *pvValue)
{
    return mEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus,
                    pvValue);
}

/** 
 *******************************************************************************
 *  @fn     Shutdown
 *  @brief   Shuts down
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::Shutdown(void)
{
    Lock lock(mCritSec, true);

    HRESULT hr;

    hr = mEventQueue->Shutdown();
    RETURNIFFAILED(hr);

    hr = MFUnlockWorkQueue(mWorkQueueID);
    RETURNIFFAILED(hr);

    hr = mVideoEffect->shutdown();
    RETURNIFFAILED(hr);

    mIsShutdown = true;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetShutdownStatus
 *  @brief   Gets shut down status
 *
 *  @param[out] status          :Shut down status
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT AsyncColorConvert::GetShutdownStatus(MFSHUTDOWN_STATUS* status)
{
    if (status == NULL)
    {
        return E_POINTER;
    }

    Lock lock(mCritSec, true);

    if (!mIsShutdown)
    {
        return MF_E_INVALIDREQUEST;
    }

    *status = MFSHUTDOWN_COMPLETED;

    return S_OK;
}

