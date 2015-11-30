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
 * @file <MftSvcSplitterTransform.cpp>                          
 *                                       
 * @brief This file defines class necessary for SVC splitter transform
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "MftSvcSplitterTransform.h"

namespace {
template<class V>
/** 
 *******************************************************************************
 *  @fn     checkIndex
 *  @brief  checks the index and resizes the vector if necessary
 *           
 *  @param[in/out] vector : Reference Id
 *  @param[in] index      : index to be checked 
 *          
 *  @return void
 *******************************************************************************
 */
void checkIndex(V& vector, size_t index)
{
    if (vector.size() <= index)
    {
        vector.resize(index + 1);
    }
}
/** 
 *******************************************************************************
 *  @fn     cloneMediaSample
 *  @brief  clones the media sample
 *           
 *  @param[in] src   : Source sample pointer 
 *  @param[out] dst  : Destination sample pointer 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT cloneMediaSample(IMFSample* src, IMFSample** dst)
{
    (*dst) = src;
    (*dst)->AddRef();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     cloneMediaType
 *  @brief  Clones the media type
 *           
 *  @param[in] pSrc        : Soruce media type
 *  @param[out] ppDst      : Destination media type 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT cloneMediaType(IMFMediaType* pSrc, IMFMediaType** ppDst)
{
    HRESULT hr;

    CComPtr < IMFMediaType > type;
    hr = MFCreateMediaType(&type);
    RETURNIFFAILED(hr);

    hr = pSrc->CopyAllItems(type);
    RETURNIFFAILED(hr);

    *ppDst = type.Detach();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     parseSvcTemporalId
 *  @brief  parses the svc tomporal id
 *           
 *  @param[in] sample       : Pointer to the IMFSample interface
 *  @param[out] temporalId  : Temporal id 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT parseSvcTemporalId(IMFSample* sample, UINT32& temporalId)
{
    HRESULT hr;

    CComPtr < IMFMediaBuffer > pBuffer;

    hr = sample->ConvertToContiguousBuffer(&pBuffer);
    RETURNIFFAILED(hr);

    DWORD cbCurrentLength = 0;
    DWORD cbMaxLength = 0;

    BYTE* buf = NULL;
    hr = pBuffer->Lock(&buf, &cbMaxLength, &cbCurrentLength);
    RETURNIFFAILED(hr);

    for (size_t i = 0; buf != nullptr && i < cbCurrentLength;)
    {
        if (0 == buf[i] && 0 == buf[i + 1] && 0 == buf[i + 2] && 1
                        == buf[i + 3])
        {
            SvcSplitter svcSplitter;
            if (!svcSplitter.setNal(buf + i, cbCurrentLength))
            {
                temporalId = (int) svcSplitter.getTemporalId();
                break;
            }
            i += 4;
        }
        else
        {
            i += 1;
        }
    }

    hr = pBuffer->Unlock();
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     calculateRateDenumeratorByLayersNum
 *  @brief  Calculates the frame rate(denuminator) depending on the layer 
 *          number set on the GUI
 *           
 *  @param[in/out] inputDenumerator : input denomerator
 *  @param[in] layerNUM             : Layer number 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
UINT32 calculateRateDenumeratorByLayersNum(UINT32 inputDenumerator,
                UINT32 layerNUM)
{
    for (UINT32 i = SVC_LAYERS_NUMBER; i > layerNUM; --i)
    {
        inputDenumerator *= 2;
    }

    return inputDenumerator;
}
}
/** 
 *******************************************************************************
 *  @fn     createInstance
 *  @brief  Creates the transform 
 *           
 *  @param[in] iid        : Reference Id
 *  @param[out] ppMFT  : Pointer to the MFT 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::createInstance(REFIID iid, void **ppMFT)
{
    if (ppMFT == NULL)
    {
        return E_POINTER;
    }

    CComPtr < IMFTransform > pMFT = new Transform();
    if (pMFT == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr;
    hr = pMFT->QueryInterface(iid, ppMFT);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     Transform
 *  @brief  Constuctor
 *          
 *******************************************************************************
 */
Transform::Transform() :
    mRefCount(0), mOutputImageSize(0)
{
    DllAddRef();
}
/** 
 *******************************************************************************
 *  @fn     Transform
 *  @brief  Destructor
 *          
 *******************************************************************************
 */
Transform::~Transform()
{
    DllRelease();
}
/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG Transform::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}
/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  IUnknown methods
 *  
 *  @return ULONG 
 *******************************************************************************
 */
ULONG Transform::Release()
{
    ULONG uCount = InterlockedDecrement(&mRefCount);
    if (uCount == 0)
    {
        delete this;
    }

    return uCount;
}
/** 
 *******************************************************************************
 *  @fn     QueryInterface
 *  @brief  virtual function
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::QueryInterface(REFIID iid, void** ppv)
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
    else if (iid == __uuidof(IPersist))
    {
        *ppv = static_cast<IPersist*> (this);
    }
    else if (iid == __uuidof(IMFAttributes))
    {
        GetAttributes((IMFAttributes**) ppv);
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
 *  @fn     GetCharacteristics
 *  @brief  IMFTransform methods.Gets the global attribute store for this 
 *          Media Foundation transform (MFT).
 *           
 *  @param[out] ppAttributes: Pointer to the IMFAttributes interface
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetAttributes(IMFAttributes** ppAttributes)
{
    if (ppAttributes == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr;

    hr = createAttributeStore();
    RETURNIFFAILED(hr);

    *ppAttributes = mAttributes;
    (*ppAttributes)->AddRef();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     createAttributeStore
 *  @brief  Creates the attribute
 *           
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::createAttributeStore()
{
    if (mAttributes == nullptr)
    {
        HRESULT hr;

        hr = MFCreateAttributes(&mAttributes, 3);
        RETURNIFFAILED(hr);

        hr = mAttributes->SetUINT32(CLSID_SVC_MFT_OUTPUTS_NUMBER_PROPERTY, 1);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetStreamLimits
 *  @brief  IMFTransform methods.Gets the minimum and maximum number of 
 *          input and output streams for this Media Foundation transform (MFT).
 *           
 *  @param[out] pdwInputMinimum : Receives the minimum number of input streams.
 *  @param[out] pdwInputMaximum : Receives the Maximum number of input streams.
 *  @param[out] pdwOutputMinimum: Receives the minimum number of output streams.
 *  @param[out] pdwOutputMaximum: Receives the Maximum number of output streams.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetStreamLimits(DWORD* pdwInputMinimum,
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
    *pdwOutputMaximum = getOutputSlotsNumber();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetStreamCount
 *  @brief  IMFTransform methods.Gets the current number of input and output
 *          streams on this Media Foundation transform (MFT).
 *           
 *  @param[out] pcInputStreams  : Receives the number of input streams.
 *  @param[out] pcOutputStreams : Receives the number of output streams.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams)
{
    if ((pcInputStreams == NULL) || (pcOutputStreams == NULL))

    {
        return E_POINTER;
    }

    *pcInputStreams = 1;
    *pcOutputStreams = getOutputSlotsNumber();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetStreamIDs
 *  @brief  IMFTransform methods.Gets the stream identifiers for the input 
 *          and output streams on this Media Foundation transform (MFT).
 *           
 *  @param[in] dwInputIDArraySize  : Number of elements in the pdwInputIDs array.
 *  @param[out] pdwInputIDs        : The method fills the array with the input 
 *                                   stream identifiers.
 *  @param[in] dwOutputIDArraySize : Number of elements in the pdwOutputIDs array.
 *  @param[out] pdwOutputIDs       : The method fills the array with the output 
 *                                   stream identifiers
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetStreamIDs(DWORD /*dwInputIDArraySize*/,
                DWORD* /*pdwInputIDs*/, DWORD /*dwOutputIDArraySize*/, DWORD* /*pdwOutputIDs*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     GetInputStreamInfo
 *  @brief  Gets the buffer requirements and other information for an input 
 *          stream on this Media Foundation transform (MFT).
 *           
 *  @param[in] dwInputStreamID : Input stream identifier.
 *  @param[out] pStreamInfo    : Pointer to an MFT_INPUT_STREAM_INFO structure.
 *                  Fills the structure with information about the input stream.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetInputStreamInfo(DWORD dwInputStreamID,
                MFT_INPUT_STREAM_INFO* pStreamInfo)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidInputStream(dwInputStreamID))
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
        DWORD inputCbImageSize = 0;

        hr = MFGetAttributeSize(mInputType, MF_MT_FRAME_SIZE,
                        &inputImageWidthInPixels, &inputImageHeightInPixels);
        RETURNIFFAILED(hr);

        hr = getNv12ImageSize(inputImageWidthInPixels,
                        inputImageHeightInPixels, &inputCbImageSize);
        RETURNIFFAILED(hr);

        pStreamInfo->cbSize = inputCbImageSize;
    }

    pStreamInfo->cbMaxLookahead = 0;
    pStreamInfo->cbAlignment = 0;

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     getNv12ImageSize
 *  @brief  Calculates NV12 image size using width and height of the image
 *           
 *  @param[in] width     : width
 *  @param[in] height    : Height
 *  @param[out] pcbImage : Pointer to the NV12 image size
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::getNv12ImageSize(UINT32 width, UINT32 height,
                DWORD* pcbImage)
{
    if ((height / 2 > MAXDWORD - height) || ((height + height / 2) > MAXDWORD
                    / width))
    {
        return E_INVALIDARG;
    }

    *pcbImage = width * (height + (height / 2));

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetOutputStreamInfo
 *  @brief  Gets the buffer requirements and other information for an output 
 *          stream on this Media Foundation transform (MFT).
 *           
 *  @param[in] dwOutputStreamID : Output stream identifier
 *  @param[in] pStreamInfo      : Pointer to an MFT_OUTPUT_STREAM_INFO structure. 
 *                                The method fills the structure with information 
 *                                 about the output stream.
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetOutputStreamInfo(DWORD dwOutputStreamID,
                MFT_OUTPUT_STREAM_INFO* pStreamInfo)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES
                    | MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER
                    | MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE
                    | MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

    if (mOutputTypes[dwOutputStreamID] == NULL)
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
 *  @brief  Not implemented. Gets the attribute store for an input stream on 
 *          this Media Foundation transform (MFT).
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[in] ppAttributes    : Pointer to the attributes
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetInputStreamAttributes(DWORD /*dwInputStreamID*/,
                IMFAttributes** /*ppAttributes*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     GetOutputStreamAttributes
 *  @brief  Gets the attribute store for an input stream on this Media 
 *          Foundation transform (MFT).
 *           
 *  @param[in] dwOutputStreamID : Output stream identifier
 *  @param[in] ppAttributes     : Pointer to the attributes
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetOutputStreamAttributes(DWORD dwOutputStreamID,
                IMFAttributes** ppAttributes)
{
    if (ppAttributes == nullptr)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    checkIndex<> (mOutputsAttributes, dwOutputStreamID);

    HRESULT hr;
    if (mOutputsAttributes[dwOutputStreamID] == nullptr)
    {
        hr = MFCreateAttributes(&mOutputsAttributes[dwOutputStreamID], 1);
        RETURNIFFAILED(hr);

        hr = mOutputsAttributes[dwOutputStreamID]->SetUINT32(
                        CLSID_SVC_MFT_OUTPUT_LAYERS_NUM_PROPERTY,
                        SVC_LAYERS_NUMBER);
        RETURNIFFAILED(hr);
    }

    (*ppAttributes) = mOutputsAttributes[dwOutputStreamID];
    (*ppAttributes)->AddRef();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     DeleteInputStream
 *  @brief  Not supported.Delete input stream.
 *           
 *  @param[in] dwStreamID : stream identifier
 *   
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::DeleteInputStream(DWORD /*dwStreamID*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     AddInputStreams
 *  @brief  Not implemented
 *           
 *  @param[in] cStreams      : Number of streams to add
 *  @param[out] adwStreamIDs : Array of stream identifiers. The new stream 
 *                            identifiers must not match any existing input streams.
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::AddInputStreams(DWORD /*cStreams*/, DWORD* /*adwStreamIDs*/)
{

    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     GetInputAvailableType
 *  @brief  Gets an available media type for an input stream on this (MFT).
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[in] dwTypeIndex     : Index of the media type to retrieve
 *  @param[in] ppType          : Receives a pointer to the IMFMediaType interface.
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetInputAvailableType(DWORD dwInputStreamID,
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

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;

    hr = MFCreateMediaType(&type);
    RETURNIFFAILED(hr);

    hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    RETURNIFFAILED(hr);

    *ppType = type.Detach();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetOutputAvailableType
 *  @brief  Gets an available media type for an output stream on this (MFT).
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[in] dwTypeIndex     : Index of the media type to retrieve
 *  @param[in] ppType          : Receives a pointer to the IMFMediaType interface.
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetOutputAvailableType(DWORD dwOutputStreamID,
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

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (mInputType == nullptr)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;

    hr = MFCreateMediaType(&type);
    RETURNIFFAILED(hr);

    hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    RETURNIFFAILED(hr);

    UINT32 frNumerator;
    UINT32 frDenominator;
    hr = calculateOutputStreamFrameRateByID(dwOutputStreamID, frNumerator,
                    frDenominator);
    RETURNIFFAILED(hr);

    MFSetAttributeRatio(type, MF_MT_FRAME_RATE, frNumerator, frDenominator);
    RETURNIFFAILED(hr);

    *ppType = type.Detach();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     SetInputType
 *  @brief  Sets, tests, or clears the media type for an input stream 
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[in] pType           : Pointer to the IMFMediaType interface
 *  @param[in] dwFlags         : Zero or more flags from the 
 *                               _MFT_SET_TYPE_FLAGS enumeration
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType,
                DWORD dwFlags)
{
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (hasPendingOutput())
    {
        return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
    }

    HRESULT hr;

    if (pType)
    {
        hr = onCheckInputType(pType);
        RETURNIFFAILED(hr);
    }

    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);
    if (bReallySet)
    {
        hr = onSetInputType(pType);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     SetOutputType
 *  @brief  Sets, tests, or clears the media type for an output stream 
 *
 *  @param[in] dwOutputStreamID: stream identifier
 *  @param[in] pType           : Pointer to the IMFMediaType interface
 *  @param[in] dwFlags         : Zero or more flags from the 
 *                               _MFT_SET_TYPE_FLAGS enumeration
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType,
                DWORD dwFlags)
{
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (hasPendingOutput())
    {
        return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
    }

    if (mInputType == nullptr)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr;

    if (pType)
    {
        hr = onCheckOutputType(pType, dwOutputStreamID);
        RETURNIFFAILED(hr);
    }

    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);
    if (bReallySet)
    {
        hr = onSetOutputType(pType, dwOutputStreamID);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetInputCurrentType
 *  @brief  Gets the current media type for an input stream 
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[out] ppType         : Receives a pointer to the IMFMediaType interface.
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetInputCurrentType(DWORD dwInputStreamID,
                IMFMediaType** ppType)
{
    if (ppType == nullptr)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (!mInputType)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr = cloneMediaType(mInputType, ppType);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetOutputCurrentType
 *  @brief  Gets the current media type for an output stream 
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[out] ppType         : Receives a pointer to the IMFMediaType interface.
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetOutputCurrentType(DWORD dwOutputStreamID,
                IMFMediaType** ppType)
{
    if (ppType == nullptr)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    checkIndex<> (mOutputTypes, dwOutputStreamID);

    if (nullptr == mOutputTypes[dwOutputStreamID])
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr = cloneMediaType(mOutputTypes[dwOutputStreamID], ppType);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetInputStatus
 *  @brief  Queries whether an input stream on this MFT can accept more data.
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[out] pdwFlags       : Receives a member of the 
 *                               _MFT_INPUT_STATUS_FLAGS enumeration, or zero. 
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags)
{
    if (pdwFlags == nullptr)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (mSample == nullptr)
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
 *  @brief  Queries whether an output stream on this MFT can accept more data.
 *           
 *  @param[out] pdwFlags       :Receives a member of the 
 *    _MFT_OUTPUT_STATUS_FLAGS enumeration, or zero. If the value is 
 *     MFT_OUTPUT_STATUS_SAMPLE_READY, the MFT can produce an output sample.
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetOutputStatus(DWORD *pdwFlags)
{
    if (pdwFlags == nullptr)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (mSample != nullptr)
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
 *  @brief  Sets the range of time stamps the client needs for output.
 *           
 *  @param[in] hnsLowerBound    : Specifies the earliest time stamp.
 *  @param[in] hnsUpperBound   : Specifies the latest time stamp
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::SetOutputBounds(LONGLONG /*hnsLowerBound*/, LONGLONG /*hnsUpperBound*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     ProcessEvent
 *  @brief  Not implemented.Sends an event to an input stream 
 *           
 *  @param[in] dwInputStreamID : stream identifier
 *  @param[out] pEvent         :Pointer to the IMFMediaEvent interface
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::ProcessEvent(DWORD /*dwInputStreamID*/, IMFMediaEvent* /*pEvent*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     ProcessMessage
 *  @brief  Sends a message to the MFT
 *           
 *  @param[in] eMessage : The message to send, 
 *  @param[in] ulParam  : Message parameter. 
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    HRESULT hr;

    switch (eMessage)
    {
    case MFT_MESSAGE_COMMAND_FLUSH:
        hr = onFlush();
        RETURNIFFAILED(hr);
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
    case MFT_MESSAGE_SET_D3D_MANAGER:
    case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
    case MFT_MESSAGE_NOTIFY_END_STREAMING:
    case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
    case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
        break;
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     ProcessInput
 *  @brief  Delivers data to an input stream 
 *           
 *  @param[in] dwInputStreamID : Input stream identifier
 *  @param[in] pSample         : Pointer to the IMFSample 
 *  @param[in] dwFlags         : Reserved. Must be zero.
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::ProcessInput(DWORD dwInputStreamID, IMFSample *pSample,
                DWORD dwFlags)
{
    if (pSample == nullptr)
    {
        return E_POINTER;
    }

    if (dwFlags != 0)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (!mInputType)
    {
        return MF_E_NOTACCEPTING;
    }

    if (mSample != nullptr)
    {
        return MF_E_NOTACCEPTING;
    }

    HRESULT hr;

    DWORD dwBufferCount = 0;
    hr = pSample->GetBufferCount(&dwBufferCount);
    RETURNIFFAILED(hr);

    if (dwBufferCount == 0)
    {
        return E_FAIL;
    }

    if (dwBufferCount > 1)
    {
        return MF_E_SAMPLE_HAS_TOO_MANY_BUFFERS;
    }
    /***************************************************************************
     * Save the sample. Actual work is in ProcessOutput function.               *
     ***************************************************************************/
    mSample = pSample;

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     ProcessOutput
 *  @brief  Sends a message to the MFT
 *           
 *  @param[in] dwFlags            :  _MFT_PROCESS_OUTPUT_FLAGS 
 *  @param[in] cOutputBufferCount : Number of elements in the pOutputSamples array
 *  @param[in/out] pOutputSamples : Pointer to an array of MFT_OUTPUT_DATA_BUFFER structures
 *  @param[in] pdwStatus          : _MFT_PROCESS_OUTPUT_STATUS 
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount,
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

    if (cOutputBufferCount > getOutputSlotsNumber())
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    if (mSample == NULL)
    {
        return MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    HRESULT hr;

    UINT32 frameTemporalId = 0;
    hr = parseSvcTemporalId(mSample, frameTemporalId);
    RETURNIFFAILED(hr);

    for (DWORD i = 0; i < cOutputBufferCount; ++i)
    {
        UINT32 layersNum = 0;
        hr = getTemporalLayersNumByStream(i, layersNum);
        RETURNIFFAILED(hr);

        if (layersNum > frameTemporalId)
        {
            hr = cloneMediaSample(mSample, &pOutputSamples[i].pSample);
            RETURNIFFAILED(hr);
        }
        else
        {
            pOutputSamples[i].dwStatus = MFT_OUTPUT_DATA_BUFFER_NO_SAMPLE;
        }
    }

    mSample.Release();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     getOutputSlotsNumber
 *  @brief  gets output slot numbers
 *  @param[in] dwFlags            :  _MFT_PROCESS_OUTPUT_FLAGS 
 *  
 *  @return UINT32 : output slot numbers
 *******************************************************************************
 */
UINT32 Transform::getOutputSlotsNumber()
{
    UINT32 outputSlotsNumber(0);

    CComPtr < IMFAttributes > spAttributes;
    HRESULT hr = GetAttributes(&spAttributes);
    if (SUCCEEDED(hr))
    {
        spAttributes->GetUINT32(CLSID_SVC_MFT_OUTPUTS_NUMBER_PROPERTY,
                        &outputSlotsNumber);
    }
    return outputSlotsNumber;
}
/** 
 *******************************************************************************
 *  @fn     isValidOutputStream
 *  @brief  Check if the given output stream is valid or not
 *  @param[in] dwOutputStreamID   :  Stream Id 
 *  
 *  @return bool : ture or flase
 *******************************************************************************
 */
BOOL Transform::isValidOutputStream(DWORD dwOutputStreamID)
{
    return dwOutputStreamID < getOutputSlotsNumber();
}
/** 
 *******************************************************************************
 *  @fn     isValidInputStream
 *  @brief  Check if the given input stream is valid or not
 *  @param[in] dwInputStreamID            :  Stream Id  
 *  
 *  @return BOOL :  ture or flase
 *******************************************************************************
 */
BOOL Transform::isValidInputStream(DWORD dwInputStreamID) const
{
    return dwInputStreamID == 0;
}
/** 
 *******************************************************************************
 *  @fn     onCheckInputType
 *  @brief  Check input type
 *  @param[in] pmt            :  Media type 
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes
 *******************************************************************************
 */
HRESULT Transform::onCheckInputType(IMFMediaType *pmt)
{
    return onCheckMediaType(pmt);
}
/** 
 *******************************************************************************
 *  @fn     onCheckOutputType
 *  @brief  Check output media type.
 *  @param[in] pmt              :  Media type 
 *  @param[in] dwOutputStreamID :  stream id
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes
 *******************************************************************************
 */
HRESULT Transform::onCheckOutputType(IMFMediaType *pmt, DWORD /*dwOutputStreamID*/)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    HRESULT hr;

    hr = onCheckMediaType(pmt);
    RETURNIFFAILED(hr);

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
 *  @brief  check media type
 *  @param[in] mediaType            :  media type 
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::onCheckMediaType(IMFMediaType* mediaType)
{
    HRESULT hr;

    GUID major_type;
    hr = mediaType->GetGUID(MF_MT_MAJOR_TYPE, &major_type);
    RETURNIFFAILED(hr);

    if (major_type != MFMediaType_Video)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     getTemporalLayersNumByStream
 *  @brief  Get temporal layer count 
 *  @param[in] dwOutputStreamID    :  media type 
 *  @param[out] layerID            :  layer count 
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::getTemporalLayersNumByStream(DWORD dwOutputStreamID,
                UINT32& layerID)
{
    CComPtr < IMFAttributes > outputStreamAttr;
    HRESULT hr = GetOutputStreamAttributes(dwOutputStreamID, &outputStreamAttr);
    RETURNIFFAILED(hr);

    UINT32 temporalLayerID;
    hr = outputStreamAttr->GetUINT32(CLSID_SVC_MFT_OUTPUT_LAYERS_NUM_PROPERTY,
                    &temporalLayerID);
    RETURNIFFAILED(hr);

    layerID = temporalLayerID;

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     calculateOutputStreamFrameRateByID
 *  @brief  calculate frame rate numerator and denominator
 *  @param[in] streamID            :  stream id 
 *  @param[out] frNumerator        :  frame rate numerator 
 *  @param[out] frDenominator      :  frame rate denominator
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::calculateOutputStreamFrameRateByID(DWORD streamID,
                UINT32& frNumerator, UINT32& frDenominator)
{
    HRESULT hr = MFGetAttributeRatio(mInputType, MF_MT_FRAME_RATE,
                    &frNumerator, &frDenominator);
    RETURNIFFAILED(hr);

    UINT32 layersNum;
    hr = getTemporalLayersNumByStream(streamID, layersNum);
    RETURNIFFAILED(hr);

    frDenominator = calculateRateDenumeratorByLayersNum(frDenominator,
                    layersNum);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     onSetInputType
 *  @brief  set input type
 *  @param[in/out] pmt            :  media type 
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::onSetInputType(IMFMediaType *pmt)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    mInputType.Release();
    mInputType = pmt;

    if (mInputType != nullptr)
    {
        for (UINT32 i = 0; i < getOutputSlotsNumber(); ++i)
        {
            checkIndex<> (mOutputTypes, i);
            if (nullptr == mOutputTypes[i])
            {
                CComPtr < IMFMediaType > outputType;
                HRESULT hr = cloneMediaType(mInputType, &outputType);
                RETURNIFFAILED(hr);

                UINT32 frNumerator;
                UINT32 frDenominator;
                hr = calculateOutputStreamFrameRateByID(i, frNumerator,
                                frDenominator);
                RETURNIFFAILED(hr);

                MFSetAttributeRatio(outputType, MF_MT_FRAME_RATE, frNumerator,
                                frDenominator);
                RETURNIFFAILED(hr);

                mOutputTypes[i] = outputType;
            }
        }
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     onSetOutputType
 *  @brief  set output media type
 *  @param[out] pmt              :  media type 
 *  @param[in] dwOutputStreamID  :  stream id
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::onSetOutputType(IMFMediaType *pmt, DWORD dwOutputStreamID)
{
    checkIndex<> (mOutputTypes, dwOutputStreamID);

    mOutputTypes[dwOutputStreamID].Release();
    mOutputTypes[dwOutputStreamID] = pmt;

    return updateFormatInfo(dwOutputStreamID);
}
/** 
 *******************************************************************************
 *  @fn     onFlush
 *  @brief  release the sample on flush
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::onFlush()
{
    mSample.Release();
    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     updateFormatInfo
 *  @brief  update frame info
 *  @param[in] dwOutputStreamID            :  stream id
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::updateFormatInfo(DWORD& dwOutputStreamID)
{
    HRESULT hr;
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCritSec, true);

    checkIndex<> (mOutputTypes, dwOutputStreamID);
    if (nullptr != mOutputTypes[dwOutputStreamID])
    {
        UINT32 outputImageWidth = 0;
        UINT32 outputImageHeight = 0;
        mOutputImageSize = 0;

        hr
                        = MFGetAttributeSize(mOutputTypes[dwOutputStreamID],
                                        MF_MT_FRAME_SIZE, &outputImageWidth,
                                        &outputImageHeight);
        RETURNIFFAILED(hr);

        hr = getNv12ImageSize(outputImageWidth, outputImageHeight,
                        &mOutputImageSize);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetClassID
 *  @brief  Retrieves the class identifier (CLSID) of the object.
 *  @param[out] pClassID :  A pointer to the location that receives the CLSID 
 *                          on return.
 *  
 * @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetClassID(__RPC__out CLSID* pClassID)
{
    if (nullptr == pClassID)
    {
        return E_POINTER;
    }

    *pClassID = CLSID_SvcMFT;

    return S_OK;
}
