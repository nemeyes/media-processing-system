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
 * @file <MftAsyncTransform.h>                          
 *                                       
 * @brief Async transform MFT class declaration
 *         
 *******************************************************************************
 */
#ifndef MFTASYNCTRANSFORM_H
#define MFTASYNCTRANSFORM_H

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
// {5549E1D8-CB9A-4C0D-A645-6B225ECF68E7}
DEFINE_GUID(CLSID_OpenCLMFTDx11,
                0x5549e1d8, 0xcb9a, 0x4c0d, 0xa6, 0x45, 0x6b, 0x22, 0x5e, 0xcf, 0x68, 0xe7);

/**
 *   @brief CLSID for OpenCL MFT using DirectX 9.
 */
// {6D47D871-333E-4939-81D9-9293CB8F9256}
DEFINE_GUID(CLSID_OpenCLMFTDx9,
                0x6d47d871, 0x333e, 0x4939, 0x81, 0xd9, 0x92, 0x93, 0xcb, 0x8f, 0x92, 0x56);

/**
 *   @class Asynchronious MFT.
 */
class AsyncTransform: public IMFTransform,
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

        std::unique_ptr<AsyncTransform> transform(
                        new (std::nothrow) AsyncTransform());
        if (nullptr == transform)
        {
            return E_OUTOFMEMORY;
        }

        std::unique_ptr < VideoEffect > videoEffect(new (std::nothrow) VF());
        if (nullptr == videoEffect)
        {
            return E_OUTOFMEMORY;
        }

        HRESULT hr;

        hr = transform->Init(videoEffect.release());
        RETURNIFFAILED(hr);

        hr = transform->QueryInterface(iid, instance);
        RETURNIFFAILED(hr);

        transform.release();

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
     *   @brief TransformSample(). Does transformation and put result to an output queue.
     */
    HRESULT TransformSample(IMFSample* sample);

    /**
     *   @brief Public destructor to allow inheritance from this class.
     */
    ~AsyncTransform();

private:

    AsyncTransform();

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

    HRESULT AsyncTransform::SendDrainCompletedEvent();

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
