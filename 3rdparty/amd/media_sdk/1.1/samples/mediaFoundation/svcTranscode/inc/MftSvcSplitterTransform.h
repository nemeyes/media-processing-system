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
 *******************************************************************************/
/**  
 ********************************************************************************
 * @file <MftSvcSplitterTransform.h>                          
 *                                       
 * @brief This file contains transform functions for SVC splitter MFT
 *         
 ********************************************************************************
 */
#ifndef MFTSVCSPLITTERTRANSFORM_H_
#define MFTSVCSPLITTERTRANSFORM_H_

/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include <Windows.h>
#include <Evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <atlbase.h>
#include <assert.h>
#include <new>
#include <shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <strmif.h>
#include <initguid.h>
//#include <amp.h>
//#include <amp_graphics.h>
//#include <amp_math.h>
#include <iostream>
#include <vector>
#include <Mferror.h>

#include "MftSvcSplitterGuids.h"
#include "MftSvcSplitterLib.h"
#include "MftUtils.h"
#include "MftSvcSplitter.h"

/*******************************************************************************
 * SVC layers                                                                   *
 *******************************************************************************/
static const UINT32 SVC_LAYERS_NUMBER = 3;

/**
 *   @class Transform. COM object; implements video editing by using C++ Amp.
 */
class Transform: public IMFTransform, public IPersist
{
public:
    /**
     *   @brief createInstance. Fabric method.
     */
    static HRESULT createInstance(REFIID iid, void **ppSource);

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
    STDMETHODIMP createAttributeStore();

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
     *   @brief IPersist::GetClassID().
     */
STDMETHODIMP GetClassID(__RPC__out CLSID* pClassID);

private:
    Transform();

    ~Transform();
    HRESULT Transform::getNv12ImageSize(UINT32 width, UINT32 height,
                    DWORD* pcbImage);
    BOOL hasPendingOutput() const
    {
        return mSample != NULL;
    }

    BOOL isValidInputStream(DWORD dwInputStreamID) const;
    BOOL isValidOutputStream(DWORD dwOutputStreamID);

    HRESULT onGetPartialType(DWORD dwTypeIndex, IMFMediaType **ppmt);
    HRESULT onCheckInputType(IMFMediaType *pmt);
    HRESULT onCheckOutputType(IMFMediaType *pmt, DWORD dwOutputStreamID);
    HRESULT onCheckMediaType(IMFMediaType *pmt);
    HRESULT onSetInputType(IMFMediaType *pmt);
    HRESULT onSetOutputType(IMFMediaType *pmt, DWORD dwOutputStreamID);
    HRESULT onFlush();

    UINT32 getOutputSlotsNumber();

    HRESULT getTemporalLayersNumByStream(DWORD dwOutputStreamID,
                    UINT32& layerID);
    HRESULT calculateOutputStreamFrameRateByID(DWORD streamID, /*out*/
                    UINT32& frNumerator, UINT32& frDenominator);
    HRESULT updateFormatInfo(DWORD& dwOutputStreamID);

    CComPtr<IMFSample> mSample;
    CComPtr<IMFMediaType> mInputType;
    CComPtr<IMFAttributes> mAttributes;

    DWORD mOutputImageSize;

    std::vector<CComPtr<IMFMediaType> > mOutputTypes;
    std::vector<CComPtr<IMFAttributes> > mOutputsAttributes;

    CComMultiThreadModel::AutoCriticalSection mCritSec;
    long mRefCount;
};

inline HRESULT MFT_CreateInstance(REFIID riid, void **ppv)
{
    return Transform::createInstance(riid, ppv);
}
#endif //MFTSVCSPLITTERTRANSFORM_H_
