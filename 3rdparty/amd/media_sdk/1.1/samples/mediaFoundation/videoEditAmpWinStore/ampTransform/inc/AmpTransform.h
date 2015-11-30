/*******************************************************************************
Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1              Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer.
2              Redistributions in binary form must reproduce the above copyright notice, 
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
**************************************************************
* @file <AmpTransform.h> 
*   
* @brief This file contains declaration of CAmpTransform class
*        which describe interface of extension
**************************************************************
*/
#ifndef _AMP_TRANSFORM_H_
#define _AMP_TRANSFORM_H_

#include <new>
#include <mfapi.h>
#include <mftransform.h>
#include <mfidl.h>
#include <mferror.h>
#include <assert.h>

#include <wrl\implements.h>
#include <wrl\module.h>
#include <windows.media.h>

#include "AmpTransformExt_h.h"
#include "Transform.h"

/*******************************************************************************
* Class id for AMP Resizer Transform MFT                                       *
*******************************************************************************/
DEFINE_GUID(CLSID_AmpMFT, 
0x9edab03d, 0x2a7a, 0x4e72, 0x84, 0xf, 0xc7, 0xbf, 0x26, 0xab, 0x10, 0xf0);

/**
* CAmpTransform
* This class describe interface of extension.
*/
class CAmpTransform 
    : public Microsoft::WRL::RuntimeClass<
           Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >, 
           ABI::Windows::Media::IMediaExtension,
           IMFTransform/*,
           ABI::Windows::Foundation::Collections::MapChangedEventHandler<HSTRING, IInspectable *>*/>
{
    InspectableClass(RuntimeClass_ampTransformExt_ampTransform, BaseTrust)

public:
    CAmpTransform();
    ~CAmpTransform();

    STDMETHOD(RuntimeClassInitialize)();

    /****************
    * IMediaExtension
    *****************/
    STDMETHODIMP SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration);

    /*************
    * IMFTransform
    **************/
    STDMETHODIMP GetStreamLimits(
        DWORD   *pdwInputMinimum,
        DWORD   *pdwInputMaximum,
        DWORD   *pdwOutputMinimum,
        DWORD   *pdwOutputMaximum
    );

    STDMETHODIMP GetStreamCount(
        DWORD   *pcInputStreams,
        DWORD   *pcOutputStreams
    );

    STDMETHODIMP GetStreamIDs(
        DWORD   dwInputIDArraySize,
        DWORD   *pdwInputIDs,
        DWORD   dwOutputIDArraySize,
        DWORD   *pdwOutputIDs
    );

    STDMETHODIMP GetInputStreamInfo(
        DWORD                     dwInputStreamID,
        MFT_INPUT_STREAM_INFO *   pStreamInfo
    );

    STDMETHODIMP GetOutputStreamInfo(
        DWORD                     dwOutputStreamID,
        MFT_OUTPUT_STREAM_INFO *  pStreamInfo
    );

    STDMETHODIMP GetAttributes(IMFAttributes** pAttributes);

    STDMETHODIMP GetInputStreamAttributes(
        DWORD           dwInputStreamID,
        IMFAttributes   **ppAttributes
    );

    STDMETHODIMP GetOutputStreamAttributes(
        DWORD           dwOutputStreamID,
        IMFAttributes   **ppAttributes
    );

    STDMETHODIMP DeleteInputStream(DWORD dwStreamID);

    STDMETHODIMP AddInputStreams(
        DWORD   cStreams,
        DWORD   *adwStreamIDs
    );

    STDMETHODIMP GetInputAvailableType(
        DWORD           dwInputStreamID,
        DWORD           dwTypeIndex, 
        IMFMediaType    **ppType
    );

    STDMETHODIMP GetOutputAvailableType(
        DWORD           dwOutputStreamID,
        DWORD           dwTypeIndex,
        IMFMediaType    **ppType
    );

    STDMETHODIMP SetInputType(
        DWORD           dwInputStreamID,
        IMFMediaType    *pType,
        DWORD           dwFlags
    );

    STDMETHODIMP SetOutputType(
        DWORD           dwOutputStreamID,
        IMFMediaType    *pType,
        DWORD           dwFlags
    );

    STDMETHODIMP GetInputCurrentType(
        DWORD           dwInputStreamID,
        IMFMediaType    **ppType
    );

    STDMETHODIMP GetOutputCurrentType(
        DWORD           dwOutputStreamID,
        IMFMediaType    **ppType
    );

    STDMETHODIMP GetInputStatus(
        DWORD           dwInputStreamID,
        DWORD           *pdwFlags
    );

    STDMETHODIMP GetOutputStatus(DWORD *pdwFlags);

    STDMETHODIMP SetOutputBounds(
        LONGLONG        hnsLowerBound,
        LONGLONG        hnsUpperBound
    );

    STDMETHODIMP ProcessEvent(
        DWORD              dwInputStreamID,
        IMFMediaEvent      *pEvent
    );

    STDMETHODIMP ProcessMessage(
        MFT_MESSAGE_TYPE    eMessage,
        ULONG_PTR           ulParam
    );

    STDMETHODIMP ProcessInput(
        DWORD               dwInputStreamID,
        IMFSample           *pSample,
        DWORD               dwFlags
    );

    STDMETHODIMP ProcessOutput(
        DWORD                   dwFlags,
        DWORD                   cOutputBufferCount,
        MFT_OUTPUT_DATA_BUFFER  *pOutputSamples,
        DWORD                   *pdwStatus
    );

private:
    /**
    * hasPendingOutput
    * @return TRUE if the MFT is holding an input sample
    */
    BOOL hasPendingOutput() const 
    { 
        return mSample != NULL; 
    }

    /**
    * isValidInputStream
    * @return TRUE if dwInputStreamID is a valid input stream identifier
    */
    BOOL isValidInputStream(DWORD dwInputStreamID) const
    {
        return dwInputStreamID == 0;
    }

    /**
    * isValidOutputStream
    * @return TRUE if dwOutputStreamID is a valid output stream identifier
    */
    BOOL isValidOutputStream(DWORD dwOutputStreamID) const
    {
        return dwOutputStreamID == 0;
    }
 
    HRESULT onCheckInputType(IMFMediaType *pmt);
    HRESULT onCheckOutputType(IMFMediaType *pmt);
    HRESULT onCheckMediaType(IMFMediaType *pmt);
    HRESULT onSetInputType(IMFMediaType *pmt);
    HRESULT onSetOutputType(IMFMediaType *pmt); 
  
    HRESULT onProcessOutput(IMFMediaBuffer *pIn, IMFMediaBuffer *pOut);
    HRESULT onFlush();
    HRESULT updateFormatInfo();

    CRITICAL_SECTION mCritSec;

    CComPtr<IMFSample>                   mSample;     /**< COM ptr to the input sample */
    CComPtr<IMFMediaType>                mInputType;  /**< COM ptr to the input media type */
    CComPtr<IMFMediaType>                mOutputType; /**< COM ptr to the output media type */

    /****************** 
    * Format information
    *******************/
    UINT32                      mOutputImageWidthInPixels;
    UINT32                      mOutputImageHeightInPixels;
    DWORD                       mOutputCbImageSize;         /**< Image size, in bytes */

    CComPtr<IMFAttributes>          mAttributes;

    CComPtr<ID3D11Device>           mD3D11Device;
    CComPtr<IMFDXGIDeviceManager>   mD3D11DeviceManager;

    CTransform* mTransform;
};

#endif