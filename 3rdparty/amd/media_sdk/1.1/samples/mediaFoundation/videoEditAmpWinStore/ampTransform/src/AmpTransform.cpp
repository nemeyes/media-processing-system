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
********************************************************************************
* @file <AmpTransform.cpp> 
*   
* @brief This file contains implementation of AmpTransform class
*
********************************************************************************
*/

#include "Common.h"

#include "AmpTransform.h"

using namespace Microsoft::WRL;

/** 
 *******************************************************************************
 *  @fn     GetPropertyUINT32
 *  @brief  Retrieves the requested property value from input property structure. 
 *
 *  @param[in] pConfiguration   : Input property set structure.
 *  @param[in] propertyName     : Property to be retrieved.
 *  @param[out] val             : Value of requested property
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT GetPropertyUINT32( ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration, const wchar_t* propertyName, UINT32& val )
{
    HRESULT hr = S_OK;

    if(pConfiguration)
    {
        Microsoft::WRL::ComPtr<IInspectable> spInsp;
        Microsoft::WRL::ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> spSetting;
        Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValue> spPropVal;

        hr = pConfiguration->QueryInterface(IID_PPV_ARGS(&spSetting));
        if (FAILED(hr))
        {
            return hr;
        }

        Microsoft::WRL::Wrappers::HStringReference strKey(propertyName);
        hr = spSetting->Lookup(strKey.Get(), spInsp.ReleaseAndGetAddressOf());
        if(FAILED(hr))
        {
            return E_INVALIDARG;
        }

        hr = spInsp.As(&spPropVal);
        if(FAILED(hr))
        {
            return E_INVALIDARG;
        }

        hr = spPropVal->GetUInt32(&val);
        if(FAILED(hr))
        {
            return E_INVALIDARG;
        }
    }

    return hr;
}

/** 
 *******************************************************************************
 *  @fn     CAmpTransform
 *  @brief  constructor 
 *
 *******************************************************************************
 */
CAmpTransform::CAmpTransform() :
    mSample(NULL), mInputType(NULL), mOutputType(NULL),
    mOutputImageWidthInPixels(0), mOutputImageHeightInPixels(0), mOutputCbImageSize(0),
    mAttributes(NULL)
{
    InitializeCriticalSectionEx(&mCritSec, 3000, 0);

    OutputDebugString(L"AmpTransform:Initilized\n");

    mTransform = new CTransform();
}

/** 
 *******************************************************************************
 *  @fn     ~CAmpTransform
 *  @brief  destructor 
 *
 *******************************************************************************
 */
CAmpTransform::~CAmpTransform()
{
    if (mTransform != nullptr)
    {
        delete mTransform;
    }

    OutputDebugString(L"AmpTransform:Deleted\n");

    DeleteCriticalSection(&mCritSec);
}

/** 
 *******************************************************************************
 *  @fn     RuntimeClassInitialize
 *  @brief  Initializes the class
 *          
 *  @return : 
 *******************************************************************************
 */
STDMETHODIMP CAmpTransform::RuntimeClassInitialize()
{
    HRESULT hr;

    hr = MFCreateAttributes(&mAttributes, 2);
    if (FAILED(hr)) return hr;

    hr = mAttributes->SetUINT32(MF_SA_D3D11_AWARE, 1);
    if (FAILED(hr)) return hr;

    hr = mAttributes->SetUINT32(MF_SA_D3D_AWARE, 1);
    if (FAILED(hr)) return hr;

    OutputDebugString(L"AmpTransform:RuntimeClassInitialize\n");

    return hr;
}

/** 
 *******************************************************************************
 *  @fn     SetProperties
 *  @brief  Parse and set the input properties . 
 *
 *  @param[in] pConfiguration   : Receives the number of input streams.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
    /***************************************************************************
    * Subscribe on property change event                                       *
    ***************************************************************************/
    Microsoft::WRL::ComPtr<ABI::Windows::Foundation::Collections::IObservableMap<HSTRING, IInspectable *>> spSettingObs;
    HRESULT hr = pConfiguration->QueryInterface(IID_PPV_ARGS(&spSettingObs));
    if (FAILED(hr)) return hr;

    OutputDebugString(L"AmpTransform:SetProperties\n");
    return S_OK;    
}

/** 
 *******************************************************************************
 *  @fn     GetStreamLimits
 *  @brief  Retrieves the minimum and maximum number of input and output streams. 
 *
 *  @param[out] pdwInputMinimum   : Receives the minimum number of input streams.
 *  @param[out] pdwInputMaximum   : Receives the maximum number of input streams. 
 *              If there is no maximum, receives the value MFT_STREAMS_UNLIMITED.
 *  @param[out] pdwOutputMinimum   : Receives the minimum number of output streams.
 *  @param[out] pdwOutputMaximum   : Receives the maximum number of output streams. 
 *              If there is no maximum, receives the value MFT_STREAMS_UNLIMITED.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetStreamLimits(
    DWORD   *pdwInputMinimum,
    DWORD   *pdwInputMaximum,
    DWORD   *pdwOutputMinimum,
    DWORD   *pdwOutputMaximum
)
{
    if ((pdwInputMinimum == NULL) ||
        (pdwInputMaximum == NULL) ||
        (pdwOutputMinimum == NULL) ||
        (pdwOutputMaximum == NULL))
    {
        return E_POINTER;
    }

    *pdwInputMinimum = 1;
    *pdwInputMaximum = 1;
    *pdwOutputMinimum = 1;
    *pdwOutputMaximum = 1;

    OutputDebugString(L"AmpTransform:GetStreamLimits\n");
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetStreamCount
 *  @brief  Retrieves the current number of input and output streams on this MFT. 
 *
 *  @param[out] pcInputStreams   : Receives the number of input streams.
 *  @param[out] pcOutputStreams   : Receives the number of output streams.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetStreamCount(DWORD   *pcInputStreams, DWORD   *pcOutputStreams)
{
    if ((pcInputStreams == NULL) || (pcOutputStreams == NULL))

    {
        return E_POINTER;
    }

    *pcInputStreams = 1;
    *pcOutputStreams = 1;

    OutputDebugString(L"AmpTransform:GetStreamCount\n");
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetStreamIDs
 *  @brief  Retrieves the stream identifiers for the input and output streams on this MFT. 
 *
 *  @param[in] dwInputIDArraySize   :Number of elements in the pdwInputIDs array.
 *  @param[out] pdwInputIDs   : Pointer to an array allocated by the caller.
 *  @param[in] dwOutputIDArraySize   : Number of elements in the pdwOutputIDs array.
 *  @param[out] pdwOutputIDs   : Pointer to an array allocated by the caller.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetStreamIDs(
    DWORD   dwInputIDArraySize,
    DWORD   *pdwInputIDs,
    DWORD   dwOutputIDArraySize,
    DWORD   *pdwOutputIDs
)
{
    OutputDebugString(L"AmpTransform:GetStreamIDs\n");
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     GetInputStreamInfo
 *  @brief  Retrieves the buffer requirements and other information for an input stream. 
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.
 *  @param[out] pStreamInfo   : Pointer to an MFT_INPUT_STREAM_INFO structure.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetInputStreamInfo(
    DWORD                     dwInputStreamID,
    MFT_INPUT_STREAM_INFO *   pStreamInfo
)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    if (!isValidInputStream(dwInputStreamID))
    {

        return MF_E_INVALIDSTREAMNUMBER;
    }

    AutoLock al = AutoLock(&mCritSec);

    pStreamInfo->hnsMaxLatency = 0;
    pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES | MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER;

    if (mInputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        HRESULT hr;

        UINT32 inputImageWidthInPixels = 0;
        UINT32 inputImageHeightInPixels = 0;
        DWORD  inputCbImageSize = 0;

        hr = MFGetAttributeSize(mInputType, MF_MT_FRAME_SIZE,
                                &inputImageWidthInPixels,
                                &inputImageHeightInPixels);
        if (FAILED(hr)) return hr;

        hr = getNV12ImageSize(inputImageWidthInPixels, inputImageHeightInPixels, &inputCbImageSize);
        if (FAILED(hr)) return hr;

        pStreamInfo->cbSize = inputCbImageSize;
    }

    pStreamInfo->cbMaxLookahead = 0;
    pStreamInfo->cbAlignment = 0;

    OutputDebugString(L"AmpTransform:GetInputStreamInfo\n");
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputStreamInfo
 *  @brief  Retrieves the buffer requirements and other information for an output stream on this MFT. 
 *
 *  @param[in] dwOutputStreamID   :Output stream identifier.
 *  @param[out] pStreamInfo   : Pointer to an MFT_OUTPUT_STREAM_INFO structure.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetOutputStreamInfo(
    DWORD                     dwOutputStreamID,
    MFT_OUTPUT_STREAM_INFO *  pStreamInfo
)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    AutoLock al = AutoLock(&mCritSec);

    pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
                           MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE | MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

    if (mOutputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        pStreamInfo->cbSize = mOutputCbImageSize;
    }

    pStreamInfo->cbAlignment = 0;

    OutputDebugString(L"AmpTransform:GetOutputStreamInfo\n");
    return S_OK;
}

HRESULT CAmpTransform::GetAttributes(IMFAttributes** ppAttributes)
{
    if (ppAttributes == NULL)
    {
        return E_POINTER;
    }

    AutoLock al = AutoLock(&mCritSec);

    *ppAttributes = mAttributes;
    (*ppAttributes)->AddRef();

    OutputDebugString(L"AmpTransform:GetAttributes\n");
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetInputStreamAttributes
 *  @brief  Retrieves the attribute store for an input stream on this MFT 
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.
 *  @param[out] ppAttributes   :  Receives a pointer to the IMFAttributes interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetInputStreamAttributes(
    DWORD           dwInputStreamID,
    IMFAttributes   **ppAttributes
)
{
    OutputDebugString(L"AmpTransform:GetInputStreamAttributes\n");
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputStreamAttributes
 *  @brief  Retrieves the attribute store for an output stream on this MFT 
 *
 *  @param[in] dwOutputStreamID   :Input stream identifier.
 *  @param[out] ppAttributes   :  Receives a pointer to the IMFAttributes interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetOutputStreamAttributes(
    DWORD           dwOutputStreamID,
    IMFAttributes   **ppAttributes
)
{
    OutputDebugString(L"AmpTransform:GetOutputStreamAttributes\n");
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     DeleteInputStream
 *  @brief  Removes an input stream from this MFT. 
 *
 *  @param[in] dwStreamID   :Identifier of the input stream to remove.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::DeleteInputStream(DWORD dwStreamID)
{
    OutputDebugString(L"AmpTransform:DeleteInputStream\n");
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     AddInputStreams
 *  @brief  Adds one or more new input streams to this MFT.
 *
 *  @param[in] cStreams   :Number of streams to add.
 *  @param[in] adwStreamIDs   :Array of stream identifiers.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::AddInputStreams(
    DWORD   cStreams,
    DWORD   *adwStreamIDs
)
{
    OutputDebugString(L"AmpTransform:AddInputStreams\n");
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     GetInputAvailableType
 *  @brief  Retrieves a possible media type for an input stream on this MFT.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier. 
 *  @param[in] dwTypeIndex   :Index of the media type to retrieve.
 *  @param[out] ppType   :Receives a pointer to the IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetInputAvailableType(
    DWORD           dwInputStreamID,
    DWORD           dwTypeIndex,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    if (dwTypeIndex > 0)
    {
        return MF_E_NO_MORE_TYPES;
    }

    if (!isValidInputStream(dwInputStreamID))
    {     
        return MF_E_INVALIDSTREAMNUMBER;
    }
    
    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr;

    CComPtr<IMFMediaType> type;
    hr = createVideoType(&type, MFVideoFormat_NV12);
    if (FAILED(hr)) return hr;

    *ppType = type.Detach();

    OutputDebugString(L"AmpTransform:GetInputAvailableType\n");
    return hr;
}

HRESULT OnGetPartialType(DWORD dwTypeIndex, IMFMediaType **ppmt)
{
    CComPtr<IMFMediaType> pmt = nullptr;

    HRESULT hr = MFCreateMediaType(&pmt);
    if (FAILED(hr)) return hr;

    hr = pmt->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr)) return hr;

    hr = pmt->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    if (FAILED(hr)) return hr;
    
    *ppmt = pmt;
    (*ppmt)->AddRef();

    return hr;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputAvailableType
 *  @brief  Retrieves a possible media type for an output stream on this MFT.
 *
 *  @param[in] dwOutputStreamID   :Output stream identifier. 
 *  @param[in] dwTypeIndex   :Index of the media type to retrieve.
 *  @param[out] ppType   :Receives a pointer to the IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetOutputAvailableType(
    DWORD           dwOutputStreamID,
    DWORD           dwTypeIndex,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr = S_OK;

    if (mInputType == NULL)
    {
        /***********************************************************************
        * The input type is not set. Create a partial media type.              *
        ***********************************************************************/
        hr = OnGetPartialType(dwTypeIndex, ppType);
    }
    else if (dwTypeIndex > 0)
    {
        hr = MF_E_NO_MORE_TYPES;
    }
    else
    {
        *ppType = mInputType;
        (*ppType)->AddRef();
    }

    return hr;
}

/** 
 *******************************************************************************
 *  @fn     SetInputType
 *  @brief  Sets, tests, or clears the media type for an input stream on this MFT.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.  
 *  @param[in] pType   :Pointer to the IMFMediaType interface, or NULL.
 *  @param[in] dwFlags   :Zero or more flags from the _MFT_SET_TYPE_FLAGS enumeration.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::SetInputType(
    DWORD           dwInputStreamID,
    IMFMediaType    *pType,
    DWORD           dwFlags
)
{
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr = S_OK;
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    if (hasPendingOutput())
    {
        return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;      
    }

    if (pType)
    {
        hr = onCheckInputType(pType);
        if (FAILED(hr)) return hr;
    }

    if (bReallySet)
    {
        onSetInputType(pType);
    }

    OutputDebugString(L"AmpTransform:SetInputType\n");
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     SetOutputType
 *  @brief  Sets, tests, or clears the media type for an output stream on this 
 *          MFT.
 *
 *  @param[in] dwOutputStreamID   :Output stream identifier.  
 *  @param[in] pType   :Pointer to the IMFMediaType interface, or NULL.
 *  @param[in] dwFlags   :Zero or more flags from the _MFT_SET_TYPE_FLAGS 
 *                        enumeration.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::SetOutputType(
    DWORD           dwOutputStreamID,
    IMFMediaType    *pType, 
    DWORD           dwFlags
)
{
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr = S_OK;
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    if (hasPendingOutput())
    {
       return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
    }

    if (pType)
    {
        hr = onCheckOutputType(pType);
        if (FAILED(hr)) return hr;
    }

    if (bReallySet)
    {
        onSetOutputType(pType);
    }

    OutputDebugString(L"AmpTransform:SetOutputType\n");
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     GetInputCurrentType
 *  @brief  Retrieves the current media type for an input stream on this MFT.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.  
 *  @param[out] ppType   :Receives a pointer to the IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetInputCurrentType(
    DWORD           dwInputStreamID,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_POINTER;
    }

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }
    
    if (!mInputType)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr = S_OK;
    CComPtr<IMFMediaType> type;
    hr = MFCreateMediaType(&type);
    if (FAILED(hr)) return hr;

    hr = mInputType->CopyAllItems(type);
    if (FAILED(hr)) return hr;

    *ppType = type.Detach();

    OutputDebugString(L"AmpTransform:GetInputCurrentType\n");
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputCurrentType
 *  @brief  Retrieves the current media type for an output stream on this MFT.
 *
 *  @param[in] dwOutputStreamID   :Output stream identifier.  
 *  @param[out] ppType   :Receives a pointer to the IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetOutputCurrentType(
    DWORD           dwOutputStreamID,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_POINTER;
    }

    if (!isValidOutputStream(dwOutputStreamID))
    {
       return  MF_E_INVALIDSTREAMNUMBER;
    }
    
    if (!mOutputType)
    {
       return  MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr = S_OK;

    CComPtr<IMFMediaType> type;
    hr = MFCreateMediaType(&type);
    if (FAILED(hr)) return hr;

    hr = mOutputType->CopyAllItems(type);
    if (FAILED(hr)) return hr;
    hr = MFSetAttributeSize(type, MF_MT_FRAME_SIZE, mOutputImageWidthInPixels, mOutputImageHeightInPixels);
    *ppType = type.Detach();

    OutputDebugString(L"AmpTransform:GetOutputCurrentType\n");
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     GetInputStatus
 *  @brief  Queries whether an input stream on this MFT can accept more data.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.  
 *  @param[out] pdwFlags   :Receives a member of the _MFT_INPUT_STATUS_FLAGS enumeration, or zero. 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetInputStatus(
    DWORD           dwInputStreamID,
    DWORD           *pdwFlags
)
{
    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    AutoLock al = AutoLock(&mCritSec);

    if (mSample == NULL)
    {
        *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
    }
    else
    {
        *pdwFlags = 0;
    }

    OutputDebugString(L"AmpTransform:GetInputStatus\n");
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetOutputStatus
 *  @brief  Queries whether the transform is ready to produce output data.
 *
 *  @param[out] pdwFlags   :Receives a member of the _MFT_OUTPUT_STATUS_FLAGS 
 *                          enumeration, or zero. 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::GetOutputStatus(DWORD *pdwFlags)
{
    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    AutoLock al = AutoLock(&mCritSec);

    if (mSample != NULL)
    {
        *pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
    }
    else
    {
        *pdwFlags = 0;
    }

    OutputDebugString(L"AmpTransform:GetOutputStatus\n");
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     SetOutputBounds
 *  @brief  Sets the range of timestamps the client needs for output.
 *
 *  @param[in] hnsLowerBound   :Specifies the earliest time stamp. 
 *  @param[in] hnsUpperBound   :Specifies the latest time stamp. 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::SetOutputBounds(
    LONGLONG        hnsLowerBound,
    LONGLONG        hnsUpperBound
)
{
    OutputDebugString(L"AmpTransform:SetOutputBounds\n");
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     ProcessEvent
 *  @brief  Sends an event to an input stream on this MFT.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.  
 *  @param[in] pEvent   :Pointer to the IMFMediaEvent interface of an event object.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::ProcessEvent(
    DWORD              dwInputStreamID,
    IMFMediaEvent      *pEvent
)
{
    OutputDebugString(L"AmpTransform:ProcessEvent\n");
    return E_NOTIMPL;
}

/** 
 *******************************************************************************
 *  @fn     ProcessMessage
 *  @brief  Sends a message to the MFT.
 *
 *  @param[in] eMessage   :The message to send, specified as a member of the MFT_MESSAGE_TYPE enumeration.
 *  @param[in] ulParam   :Message parameter.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::ProcessMessage(
    MFT_MESSAGE_TYPE    eMessage,
    ULONG_PTR           ulParam
)
{
    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr = S_OK;

    switch (eMessage)
    {
    case MFT_MESSAGE_COMMAND_FLUSH:
        OutputDebugString(L"AmpTransform:ProcessMessage:Flush\n");
        hr = onFlush();
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
        OutputDebugString(L"AmpTransform:ProcessMessage:Drain\n");
        break;

    case MFT_MESSAGE_SET_D3D_MANAGER:
        if (ulParam != 0)
        {
            OutputDebugString(L"AmpTransform:ProcessMessage:D3DManager\n");

            CComPtr<IUnknown> spD3DManagerUnknown = (IUnknown*)ulParam;
            mD3D11DeviceManager = nullptr;
            hr = spD3DManagerUnknown->QueryInterface(&mD3D11DeviceManager);
            if(FAILED(hr)) return hr;

            hr = mTransform->init(mD3D11DeviceManager);
            if(FAILED(hr)) return hr;

            HANDLE deviceHandle;
            hr = mD3D11DeviceManager->OpenDeviceHandle(&deviceHandle);
            if(FAILED(hr)) return hr;

            mD3D11Device = nullptr;
            hr = mD3D11DeviceManager->GetVideoService(deviceHandle, IID_PPV_ARGS(&mD3D11Device));
            if(FAILED(hr)) return hr;
        }
        break;

    case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
        OutputDebugString(L"AmpTransform:ProcessMessage:StreamingBegin\n");
        break;

    case MFT_MESSAGE_NOTIFY_END_STREAMING:
        OutputDebugString(L"AmpTransform:ProcessMessage:StreamingEnd\n");
        break;

    case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
        OutputDebugString(L"AmpTransform:ProcessMessage:EndOfStream\n");
        break;

    case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
        OutputDebugString(L"AmpTransform:ProcessMessage:StartOfStream\n");
        break;
    }

    OutputDebugString(L"AmpTransform:ProcessMessage\n");
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     ProcessInput
 *  @brief  Delivers data to an input stream on this MFT.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier. 
 *  @param[in] pSample   :Pointer to the IMFSample interface of the input sample.
 *  @param[in] dwFlags   :Reserved. Must be zero.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::ProcessInput(
    DWORD               dwInputStreamID,
    IMFSample           *pSample,
    DWORD               dwFlags
)
{
    if (pSample == NULL)
    {
        return E_POINTER;
    }

    if (dwFlags != 0)
    {
        return E_INVALIDARG;
    }

    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr = S_OK;

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (!mInputType || !mOutputType)
    {
       return MF_E_NOTACCEPTING;   
    }

    if (mSample != NULL)
    {
        return MF_E_NOTACCEPTING;  

    }

    /***************************************************************************
    * Cache the sample. We do the actual work in ProcessOutput.                *
    ***************************************************************************/
    mSample = pSample;

    OutputDebugString(L"AmpTransform:ProcessInput\n");
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     ProcessOutput
 *  @brief  Generates output from the current input data.
 *
 *  @param[in] dwFlags   :Bitwise OR of zero or more flags from the
 *                        _MFT_PROCESS_OUTPUT_FLAGS enumeration.
 *  @param[in] cOutputBufferCount   :Number of elements in the pOutputSamples 
 *                                   array. The value must be at least 1.
 *  @param[in,out] pOutputSamples   :Pointer to an array of MFT_OUTPUT_DATA_BUFFER 
 *                                   structures, allocated by the caller. 
 *  @param[out] pdwStatus   :Receives a bitwise OR of zero or more flags from the 
 *                           _MFT_PROCESS_OUTPUT_STATUS enumeration. 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::ProcessOutput(
    DWORD                   dwFlags,
    DWORD                   cOutputBufferCount,
    MFT_OUTPUT_DATA_BUFFER  *pOutputSamples,
    DWORD                   *pdwStatus
)
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

    if (mSample == NULL)
    {
        return MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    AutoLock al = AutoLock(&mCritSec);

    HRESULT hr;

    /***************************************************************************
    * Generate a new output sample                                             *
    ***************************************************************************/
    if (pOutputSamples[0].pSample == NULL)
    {
        CComPtr<IMFSample> newSample;
        CComPtr<ID3D11Texture2D> newTexture;
        CComPtr<IMFMediaBuffer> newMediaBuffer;
        CComPtr<IDXGISurface> newSurface;

        hr = MFCreateSample(&newSample);
        if (FAILED(hr)) return hr;

        hr = createTexture( mOutputImageWidthInPixels, mOutputImageHeightInPixels,
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_FORMAT_NV12, mD3D11Device, &newTexture );
        if (FAILED(hr)) return hr;

        hr = newTexture->QueryInterface(&newSurface);
        if (FAILED(hr)) return hr;
    
        hr = MFCreateDXGISurfaceBuffer( IID_ID3D11Texture2D, newSurface, 0, FALSE, &newMediaBuffer );
        if (FAILED(hr)) return hr;

        hr = newSample->AddBuffer( newMediaBuffer );
        if (FAILED(hr)) return hr;
        
        pOutputSamples[0].pSample = newSample.Detach();
        if (FAILED(hr)) return hr;
    }

    CComPtr<IMFMediaBuffer> pInput;
    hr = mSample->ConvertToContiguousBuffer(&pInput);
    if (FAILED(hr)) return hr;

    CComPtr<IMFMediaBuffer> pOutput;
    hr = pOutputSamples[0].pSample->ConvertToContiguousBuffer(&pOutput);
    if (FAILED(hr)) return hr;

    UINT textureInViewIndex;
    CComPtr<ID3D11Texture2D> textureIn; 
    hr = mediaBuffer2Texture( pInput, &textureIn, &textureInViewIndex);
    if (FAILED(hr)) return hr;

    UINT textureOutViewIndex;
    CComPtr<ID3D11Texture2D> textureOut;
    hr = mediaBuffer2Texture( pOutput, &textureOut, &textureOutViewIndex);
    if (FAILED(hr)) return hr;

    hr = mTransform->process(textureIn, textureInViewIndex, textureOut);
    if (FAILED(hr)) return hr;

    hr = pOutput->SetCurrentLength(mOutputCbImageSize);
    if (FAILED(hr)) return hr;

    pOutputSamples[0].dwStatus = 0;
    *pdwStatus = 0;

    // time
    LONGLONG hnsDuration = 0;
    LONGLONG hnsTime = 0;

    hr = mSample->GetSampleDuration(&hnsDuration);
    if (FAILED(hr)) return hr;

    hr = pOutputSamples[0].pSample->SetSampleDuration(hnsDuration);
    if (FAILED(hr)) return hr;

    hr = mSample->GetSampleTime(&hnsTime);
    if (FAILED(hr)) return hr;

    hr = pOutputSamples[0].pSample->SetSampleTime(hnsTime);
    if (FAILED(hr)) return hr;

    mSample.Release();

    OutputDebugString(L"AmpTransform:ProcessOutput\n");
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     onCheckInputType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::onCheckInputType(IMFMediaType *pmt)
{
    assert(pmt != NULL);

    HRESULT hr = S_OK;

    /***************************************************************************
    * Output type is not set. Just check this type                             *
    ***************************************************************************/
    hr = onCheckMediaType(pmt);
  
    return hr;
}

/** 
 *******************************************************************************
 *  @fn     onCheckOutputType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::onCheckOutputType(IMFMediaType *pmt)
{
    HRESULT hr;

    hr = onCheckMediaType(pmt);
    if (FAILED(hr)) return hr;

    MFVideoInterlaceMode interlace = MFVideoInterlace_Unknown;
    hr = pmt->GetUINT32(MF_MT_INTERLACE_MODE, (UINT32*)&interlace);
     if (FAILED(hr)) return hr;

    if ((interlace != MFVideoInterlace_Progressive) && (interlace != MFVideoInterlace_MixedInterlaceOrProgressive))
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
 *  @fn     onCheckMediaType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::onCheckMediaType(IMFMediaType *pmt)
{
    HRESULT hr;

    GUID major_type;
    hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &major_type);
    if (FAILED(hr)) return hr;

    if (major_type != MFMediaType_Video)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    GUID subtype;
    hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (FAILED(hr)) return hr;

    if (subtype != MFVideoFormat_NV12)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    UINT32 fixedSize;
    hr = pmt->GetUINT32(MF_MT_FIXED_SIZE_SAMPLES, &fixedSize);
    if (FAILED(hr)) return hr;

    if (fixedSize != TRUE)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    UINT32 samplesIndependent;
    hr = pmt->GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, &samplesIndependent);
    if (FAILED(hr)) return hr;

    if (samplesIndependent != TRUE)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     onSetInputType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::onSetInputType(IMFMediaType *pmt)
{
    mInputType = nullptr;
    mInputType = pmt;

    if (mInputType != nullptr && nullptr == mOutputType)
    {
        HRESULT hr = S_OK;

        CComPtr<IMFMediaType> outputType;
        hr = MFCreateMediaType(&outputType);
        if(FAILED(hr)) return hr;

        hr = mInputType->CopyAllItems(outputType);
        if(FAILED(hr)) return hr;

        hr = outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
        if(FAILED(hr)) return hr;
        UINT32 width = 0;
        UINT32 height = 0;
        hr = MFGetAttributeSize(outputType, MF_MT_FRAME_SIZE, &width, &height);
        mOutputType = outputType;
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     onSetOutputType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::onSetOutputType(IMFMediaType *pmt)
{
    return updateFormatInfo();
}

/** 
 *******************************************************************************
 *  @fn     onFlush
 *  @brief  
 *
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::onFlush()
{
    mSample = nullptr;
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     updateFormatInfo
 *  @brief  
 *
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CAmpTransform::updateFormatInfo()
{
    HRESULT hr = S_OK;
    if (nullptr != mOutputType)
    {
        mOutputImageWidthInPixels = 0;
        mOutputImageHeightInPixels = 0;
        mOutputCbImageSize = 0;

        hr = MFGetAttributeSize(
            mOutputType,
            MF_MT_FRAME_SIZE,
            &mOutputImageWidthInPixels,
            &mOutputImageHeightInPixels
            );
        if (FAILED(hr)) return hr;

       hr = getNV12ImageSize(mOutputImageWidthInPixels, mOutputImageHeightInPixels, &mOutputCbImageSize);
       if (FAILED(hr)) return hr;
    }
    return S_OK;	
}


