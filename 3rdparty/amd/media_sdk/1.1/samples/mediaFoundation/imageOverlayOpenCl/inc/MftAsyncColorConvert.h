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
 *******************************************************************************
 * @file <MftAsyncColorConvert.h>                          
 *                                       
 * @brief Async Color Convert MFT class declaration
 *         
 *******************************************************************************
 */
#ifndef MFTASYNCCOLORCONVERT_H
#define MFTASYNCCOLORCONVERT_H

#include <Windows.h>
#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>
#include <atlbase.h>
#include <queue>
#include <memory>

#include "VideoEffect.h"
#include "PrintLog.h"
#include "MftUtils.h"

/**
 *   @brief CLSID for OpenCL MFT using DirectX 11.
 */
// {D79203E4-E0C4-4B32-AEEF-B79EC75CD6BF}
DEFINE_GUID(CLSID_ColorConvert_OpenCLMFTDx11, 0xd79203e4, 0xe0c4, 0x4b32, 0xae, 0xef, 0xb7, 0x9e, 0xc7, 0x5c, 0xd6, 0xbf);

/**
 *   @brief CLSID for OpenCL MFT using DirectX 9.
 */
// {2EECEF7B-93B4-4428-ADFB-BDF2462D1172}
DEFINE_GUID(CLSID_ColorConvert_OpenCLMFTDx9, 0x2eecef7b, 0x93b4, 0x4428, 0xad, 0xfb, 0xbd, 0xf2, 0x46, 0x2d, 0x11, 0x72);

/**
 *   @class Asynchronious MFT.
 */
class AsyncColorConvert: public IMFTransform,
                public IMFMediaEventGenerator,
                public IMFShutdown
{
public:

    /**
     *   @brief CreateInstance. Fabric method.
     */
    template<class VF>
    static HRESULT CreateInstance(REFIID iid, void **instance)
    {
        if (instance == NULL)
        {
            return E_POINTER;
        }

        std::unique_ptr<AsyncColorConvert> colorConvert(
                        new (std::nothrow) AsyncColorConvert());
        if (nullptr == colorConvert)
        {
            return E_OUTOFMEMORY;
        }

        std::unique_ptr < VideoEffect > videoEffect(new (std::nothrow) VF());
        if (nullptr == videoEffect)
        {
            return E_OUTOFMEMORY;
        }

        HRESULT hr;

        hr = colorConvert->Init(videoEffect.release());
        RETURNIFFAILED(hr);

        hr = colorConvert->QueryInterface(iid, instance);
        RETURNIFFAILED(hr);

        colorConvert.release();

        return S_OK;
    }

    /**
     *   @brief IUnknown::QueryInterface().
     */
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);

    /**
     *   @brief IUnknown::AddRef().
     */
    STDMETHODIMP_(ULONG) AddRef();

    /**
     *   @brief IUnknown::Release().
     */
    STDMETHODIMP_(ULONG) Release();

    /**
     *   @brief IMFTransform::GetAttributes().
     */
    STDMETHODIMP GetAttributes(IMFAttributes** ppAttributes);

    /**
     *   @brief IMFTransform::CreateAttributeStore().
     */
    STDMETHODIMP CreateAttributeStore();

    /**
     *   @brief IMFTransform::GetStreamLimits().
     */
    STDMETHODIMP GetStreamLimits(DWORD *pdwInputMinimum,
                    DWORD *pdwInputMaximum, DWORD *pdwOutputMinimum,
                    DWORD *pdwOutputMaximum);

    /**
     *   @brief IMFTransform::GetStreamCount().
     */
    STDMETHODIMP GetStreamCount(DWORD *pcInputStreams, DWORD *pcOutputStreams);

    /**
     *   @brief IMFTransform::GetStreamIDs().
     */
    STDMETHODIMP GetStreamIDs(DWORD dwInputIDArraySize, DWORD *pdwInputIDs,
                    DWORD dwOutputIDArraySize, DWORD *pdwOutputIDs);

    /**
     *   @brief IMFTransform::GetInputStreamInfo().
     */
    STDMETHODIMP GetInputStreamInfo(DWORD dwInputStreamID,
                    MFT_INPUT_STREAM_INFO * pStreamInfo);

    /**
     *   @brief IMFTransform::GetOutputStreamInfo().
     */
    STDMETHODIMP GetOutputStreamInfo(DWORD dwOutputStreamID,
                    MFT_OUTPUT_STREAM_INFO * pStreamInfo);

    /**
     *   @brief IMFTransform::GetInputStreamAttributes().
     */
    STDMETHODIMP GetInputStreamAttributes(DWORD dwInputStreamID,
                    IMFAttributes **ppAttributes);

    /**
     *   @brief IMFTransform::GetOutputStreamAttributes().
     */
    STDMETHODIMP GetOutputStreamAttributes(DWORD dwOutputStreamID,
                    IMFAttributes **ppAttributes);

    /**
     *   @brief IMFTransform::DeleteInputStream().
     */
    STDMETHODIMP DeleteInputStream(DWORD dwStreamID);

    /**
     *   @brief IMFTransform::AddInputStreams().
     */
    STDMETHODIMP AddInputStreams(DWORD cStreams, DWORD *adwStreamIDs);

    /**
     *   @brief IMFTransform::GetInputAvailableType().
     */
    STDMETHODIMP GetInputAvailableType(DWORD dwInputStreamID,
                    DWORD dwTypeIndex, // 0-based
                    IMFMediaType **ppType);

    /**
     *   @brief IMFTransform::GetOutputAvailableType().
     */
    STDMETHODIMP GetOutputAvailableType(DWORD dwOutputStreamID,
                    DWORD dwTypeIndex, // 0-based
                    IMFMediaType **ppType);

    /**
     *   @brief IMFTransform::SetInputType().
     */
    STDMETHODIMP SetInputType(DWORD dwInputStreamID, IMFMediaType *pType,
                    DWORD dwFlags);

    /**
     *   @brief IMFTransform::SetOutputType().
     */
    STDMETHODIMP SetOutputType(DWORD dwOutputStreamID, IMFMediaType *pType,
                    DWORD dwFlags);

    /**
     *   @brief IMFTransform::GetInputCurrentType().
     */
    STDMETHODIMP GetInputCurrentType(DWORD dwInputStreamID,
                    IMFMediaType **ppType);

    /**
     *   @brief IMFTransform::GetOutputCurrentType().
     */
    STDMETHODIMP GetOutputCurrentType(DWORD dwOutputStreamID,
                    IMFMediaType **ppType);

    /**
     *   @brief IMFTransform::GetInputStatus().
     */
    STDMETHODIMP GetInputStatus(DWORD dwInputStreamID, DWORD *pdwFlags);

    /**
     *   @brief IMFTransform::GetOutputStatus().
     */
    STDMETHODIMP GetOutputStatus(DWORD *pdwFlags);

    /**
     *   @brief IMFTransform::SetOutputBounds().
     */
    STDMETHODIMP
                    SetOutputBounds(LONGLONG hnsLowerBound,
                                    LONGLONG hnsUpperBound);

    /**
     *   @brief IMFTransform::ProcessEvent().
     */
    STDMETHODIMP ProcessEvent(DWORD dwInputStreamID, IMFMediaEvent *pEvent);

    /**
     *   @brief IMFTransform::ProcessMessage().
     */
    STDMETHODIMP ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam);

    /**
     *   @brief IMFTransform::ProcessInput().
     */
    STDMETHODIMP ProcessInput(DWORD dwInputStreamID, IMFSample *pSample,
                    DWORD dwFlags);

    /**
     *   @brief IMFTransform::ProcessOutput().
     */
    STDMETHODIMP ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount,
                    MFT_OUTPUT_DATA_BUFFER *pOutputSamples, // one per stream
                    DWORD *pdwStatus);

    /**
     *   @brief IMFMediaEventGenerator::GetEvent().
     */
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent);

    /**
     *   @brief IMFMediaEventGenerator::BeginGetEvent().
     */
    STDMETHODIMP
                    BeginGetEvent(IMFAsyncCallback *pCallback,
                                    IUnknown *punkState);

    /**
     *   @brief IMFMediaEventGenerator::EndGetEvent().
     */
    STDMETHODIMP EndGetEvent(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);

    /**
     *   @brief IMFMediaEventGenerator::BeginGetEvent().
     */
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType,
                    HRESULT hrStatus, const PROPVARIANT *pvValue);

    /**
     *   @brief IMFShutdown::Shutdown().
     */
    STDMETHODIMP Shutdown(void);

    /**
     *   @brief IMFShutdown::GetShutdownStatus().
     */
    STDMETHODIMP GetShutdownStatus(MFSHUTDOWN_STATUS *pStatus);

    /**
     *   @brief Init().
     */
    STDMETHODIMP Init(VideoEffect* videoEffect);

    /**
     *   @brief ColorConvertSample(). Does color conversion and put result to an output queue.
     */
    HRESULT ColorConvertSample(IMFSample* sample);

    /**
     *   @brief Public destructor to allow inheritance from this class.
     */
    ~AsyncColorConvert();

private:

    AsyncColorConvert();

    BOOL IsValidInputStream(DWORD dwInputStreamID) const
    {
        return 0 == dwInputStreamID;
    }

    BOOL IsValidOutputStream(DWORD dwOutputStreamID) const
    {
        return 0 == dwOutputStreamID;
    }

    HRESULT OnGetPartialType(DWORD dwTypeIndex, IMFMediaType **ppmt);
    HRESULT OnCheckInputType(IMFMediaType *pmt);
    HRESULT OnCheckOutputType(IMFMediaType *pmt);
    HRESULT OnCheckMediaType(IMFMediaType *pmt);
    HRESULT OnSetInputType(IMFMediaType *pmt);
    HRESULT OnSetOutputType(IMFMediaType *pmt);

    bool IsLocked(void);

    void SetMarker(ULONG_PTR marker);

    void StartDraining();

    void SetStreamEnded();

    void FlushOutput();

    HRESULT RequestInput();

    HRESULT AsyncColorConvert::SendDrainCompletedEvent();

    HRESULT UpdateFormatInfo();

    typedef CComMultiThreadModel::AutoCriticalSection AutoCS;
    typedef CComCritSecLock<AutoCS> Lock;

    bool mIsStreamStarted;
    bool mIsStreamEnded;
    bool mIsDraining;
    bool mIsShutdown;
    ULONG_PTR mMarker;

    CComPtr<IMFMediaType> mInputType;
    CComPtr<IMFMediaType> mOutputType;
    CComPtr<IMFAttributes> mAttributes;
    CComPtr<IUnknown> mDeviceManager;
    CComPtr<IMFMediaEventQueue> mEventQueue;

    DWORD mWorkQueueID;

    std::queue<IMFSample*> mOutputQueue;
    AutoCS mOutputQueueLock;

    UINT32 mOutputImageWidth;
    UINT32 mOutputImageHeight;
    DWORD mOutputImageSize;

    AutoCS mCritSec;
    long mReferenceCount;

    std::unique_ptr<VideoEffect> mVideoEffect;
    msdk_CMftBuilder* mMftBuilderObjPtr;
};
#endif
