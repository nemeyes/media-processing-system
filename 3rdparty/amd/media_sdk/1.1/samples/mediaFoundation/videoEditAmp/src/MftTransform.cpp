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
 * @file <MftTransform.cpp>                          
 *                                       
 * @brief This file contains functions for MftTransform class
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/

#include <Evr.h>

#include "MftAmpMFT.h"
#include "MftTransform.h"

/** 
 *******************************************************************************
 *  @fn     CreateInstance
 *  @brief  Creates MFT instance 
 *  @param[in] iid       : GUID of the MFT 
 *  @param[out] ppMFT    : Pointer to MFT's object 
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
 *  @brief  constructor 
 *
 *******************************************************************************
 */
Transform::Transform() :
    mRefCount(0), mOutputImageWidth(0), mOutputImageHeight(0),
                    mOutputImageSize(0), mResizer(nullptr)
{
    DllAddRef();

    mResizer = new Resizer();
    mMftBuilderObjPtr = new msdk_CMftBuilder;
}

/** 
 *******************************************************************************
 *  @fn     ~Transform
 *  @brief  Destructor 
 *******************************************************************************
 */
Transform::~Transform()
{
    if (mResizer != nullptr)
    {
        delete mResizer;
    }

    if (mMftBuilderObjPtr != nullptr)
    {
        delete mMftBuilderObjPtr;
    }
    DllRelease();
}

/** 
 *******************************************************************************
 *  @fn     Addref
 *  @brief  Adds reference count
 *          
 *  @return ULONG : New reference count
 *******************************************************************************
 */
ULONG Transform::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}

/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  Releases the count
 *          
 *  @return ULONG : New reference count
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
 *  @brief  query interface  
 *
 *  @param[in] riid   : Reference iid
 *  @param[in] ppv    : 
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
 *  @brief  Retrieve attributes 
 *
 *  @param[out] ppAttributes   : Pointer to attributes
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

    hr = CreateAttributeStore();
    RETURNIFFAILED(hr);

    *ppAttributes = mAttributes;
    (*ppAttributes)->AddRef();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     CreateAttributeStore
 *  @brief  Creates attribute store 
 *
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::CreateAttributeStore()
{
    if (mAttributes == nullptr)
    {
        HRESULT hr;

        hr = MFCreateAttributes(&mAttributes, 2);
        RETURNIFFAILED(hr);

        hr = mAttributes->SetUINT32(MF_SA_D3D11_AWARE, 1);
        RETURNIFFAILED(hr);

        hr = mAttributes->SetUINT32(MF_SA_D3D_AWARE, 1);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     GetStreamLimits
 *  @brief  Retrieves the minimum and maximum number of input and output streams. 
 *
 *  @param[out] pdwInputMinimum   : Receives the minimum number of input streams.
 *  @param[out] pdwInputMaximum   : Receives the maximum number of input streams.
 *  If there is no maximum, receives the value MFT_STREAMS_UNLIMITED.
 *  @param[out] pdwOutputMinimum   : Receives the minimum number of output streams.
 *  @param[out] pdwOutputMaximum   : Receives the maximum number of output streams. 
 *  If there is no maximum, receives the value MFT_STREAMS_UNLIMITED.
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
    *pdwOutputMaximum = 1;

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
HRESULT Transform::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams)
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
 *  @brief  Retrieves the stream identifiers for the input and output streams on 
 *  this MFT. 
 *
 *  @param[in] dwInputIDArraySize   :Number of elements in the pdwInputIDs array.
 *  @param[out] pdwInputIDs   : Pointer to an array allocated by the caller.
 *  @param[in] dwOutputIDArraySize   : Number of elements in the pdwOutputIDs array.
 *  @param[out] pdwOutputIDs   : Pointer to an array allocated by the caller.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetStreamIDs(DWORD dwInputIDArraySize, DWORD* pdwInputIDs,
                DWORD dwOutputIDArraySize, DWORD* pdwOutputIDs)
{
    (void) dwInputIDArraySize;
    (void) pdwInputIDs;
    (void) dwOutputIDArraySize;
    (void) pdwOutputIDs;
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
HRESULT Transform::GetInputStreamInfo(DWORD dwInputStreamID,
                MFT_INPUT_STREAM_INFO* pStreamInfo)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCriticalSection, true);

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

        hr = mMftBuilderObjPtr->getNV12ImageSize(inputImageWidthInPixels,
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
 *  @fn     GetOutputStreamInfo
 *  @brief  Retrieves the buffer requirements and other information for an output stream on this MFT. 
 *
 *  @param[in] dwOutputStreamID   :Output stream identifier.
 *  @param[out] pStreamInfo   : Pointer to an MFT_OUTPUT_STREAM_INFO structure.
 *          
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
                    mCriticalSection, true);

    if (!isValidOutputStream(dwOutputStreamID))
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
 *  @brief  Retrieves the attribute store for an input stream on this MFT 
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.
 *  @param[out] ppAttributes   :  Receives a pointer to the IMFAttributes interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::GetInputStreamAttributes(DWORD dwInputStreamID,
                IMFAttributes** ppAttributes)
{
    (void) dwInputStreamID;
    (void) ppAttributes;
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
HRESULT Transform::GetOutputStreamAttributes(DWORD dwOutputStreamID,
                IMFAttributes** ppAttributes)
{
    (void) dwOutputStreamID;
    (void) ppAttributes;
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
HRESULT Transform::DeleteInputStream(DWORD dwStreamID)
{
    (void) dwStreamID;
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
HRESULT Transform::AddInputStreams(DWORD cStreams, DWORD* adwStreamIDs)
{
    (void) cStreams; //Not used
    (void) adwStreamIDs; //Not used
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
                    mCriticalSection, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;
    hr = mMftBuilderObjPtr->createVideoType(&type, MFVideoFormat_NV12, TRUE,
                    TRUE, NULL, NULL, 0U, 0U, MFVideoInterlace_Progressive);
    RETURNIFFAILED(hr);

    *ppType = type.Detach();

    return S_OK;
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
                    mCriticalSection, true);

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > type;
    hr = mMftBuilderObjPtr->createVideoType(&type, MFVideoFormat_NV12, TRUE,
                    TRUE, NULL, NULL, 0U, 0U, MFVideoInterlace_Progressive);
    RETURNIFFAILED(hr);

    *ppType = type.Detach();

    return S_OK;
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
HRESULT Transform::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType,
                DWORD dwFlags)
{
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCriticalSection, true);

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
 *  @brief  Sets, tests, or clears the media type for an output stream on this MFT.
 *
 *  @param[in] dwOutputStreamID   :Output stream identifier.  
 *  @param[in] pType   :Pointer to the IMFMediaType interface, or NULL.
 *  @param[in] dwFlags   :Zero or more flags from the _MFT_SET_TYPE_FLAGS enumeration.
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
                    mCriticalSection, true);

    if (!isValidOutputStream(dwOutputStreamID))
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
        hr = onCheckOutputType(pType);
        RETURNIFFAILED(hr);
    }

    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);
    if (bReallySet)
    {
        hr = onSetOutputType(pType);
        RETURNIFFAILED(hr);
    }

    return S_OK;
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
HRESULT Transform::GetInputCurrentType(DWORD dwInputStreamID,
                IMFMediaType** ppType)
{
    if (ppType == nullptr)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCriticalSection, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
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
 *  @brief  Retrieves the current media type for an output stream on this MFT.
 *
 *  @param[in] dwOutputStreamID   :Output stream identifier.  
 *  @param[out] ppType   :Receives a pointer to the IMFMediaType interface.
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
                    mCriticalSection, true);

    if (!isValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
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
 *  @brief  Queries whether an input stream on this MFT can accept more data.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier.  
 *  @param[out] pdwFlags   :Receives a member of the _MFT_INPUT_STATUS_FLAGS enumeration, or zero. 
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
                    mCriticalSection, true);

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
 *  @brief  Queries whether the transform is ready to produce output data.
 *
 *  @param[out] pdwFlags   :Receives a member of the _MFT_OUTPUT_STATUS_FLAGS enumeration, or zero. 
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
                    mCriticalSection, true);

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
 *  @brief  Sets the range of timestamps the client needs for output.
 *
 *  @param[in] hnsLowerBound   :Specifies the earliest time stamp. 
 *  @param[in] hnsUpperBound   :Specifies the latest time stamp. 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::SetOutputBounds(LONGLONG hnsLowerBound,
                LONGLONG hnsUpperBound)
{
    (void) hnsLowerBound;
    (void) hnsUpperBound;
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
HRESULT Transform::ProcessEvent(DWORD dwInputStreamID, IMFMediaEvent* pEvent)
{
    (void) dwInputStreamID;
    (void) pEvent;
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
HRESULT Transform::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCriticalSection, true);

    HRESULT hr;

    switch (eMessage)
    {
    case MFT_MESSAGE_COMMAND_FLUSH:
        hr = onFlush();
        RETURNIFFAILED(hr);
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
        break;

    case MFT_MESSAGE_SET_D3D_MANAGER:
        if (ulParam != 0)
        {
            CComPtr < IUnknown > deviceManagerUnknown = (IUnknown*) ulParam;

            mDeviceManager.Release();

            hr = deviceManagerUnknown->QueryInterface(&mDeviceManager);
            RETURNIFFAILED(hr);

            hr = mResizer->init(mDeviceManager);
            RETURNIFFAILED(hr);

            HANDLE deviceHandle;
            hr = mDeviceManager->OpenDeviceHandle(&deviceHandle);
            RETURNIFFAILED(hr);

            hr = mDeviceManager->GetVideoService(deviceHandle, IID_PPV_ARGS(
                            &mD3d11Device));
            RETURNIFFAILED(hr);
        }

        break;

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
 *  @brief  Delivers data to an input stream on this MFT.
 *
 *  @param[in] dwInputStreamID   :Input stream identifier. 
 *  @param[in] pSample   :Pointer to the IMFSample interface of the input sample.
 *  @param[in] dwFlags   :Reserved. Must be zero.
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
                    mCriticalSection, true);

    if (!isValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (!mInputType || !mOutputType)
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

    /**************************************************************
     * Save the sample. Actual work is in ProcessOutput function   *
     **************************************************************/
    mSample = pSample;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     ProcessOutput
 *  @brief  Generates output from the current input data.
 *
 *  @param[in] dwFlags   :Bitwise OR of zero or more flags from the _MFT_PROCESS_OUTPUT_FLAGS enumeration.
 *  @param[in] cOutputBufferCount   :Number of elements in the pOutputSamples array. The value must be at least 1.
 *  @param[in,out] pOutputSamples   :Pointer to an array of MFT_OUTPUT_DATA_BUFFER structures, allocated by the caller. 
 *  @param[out] pdwStatus   :Receives a bitwise OR of zero or more flags from the _MFT_PROCESS_OUTPUT_STATUS enumeration. 
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

    if (cOutputBufferCount != 1)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCriticalSection, true);

    if (mSample == NULL)
    {
        return MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    HRESULT hr;

    /************************************************
     * Generate a new output sample                  *
     ************************************************/
    if (pOutputSamples[0].pSample == NULL)
    {
        CComPtr < IMFSample > newSample;
        CComPtr < ID3D11Texture2D > newTexture;
        CComPtr < IMFMediaBuffer > newMediaBuffer;
        CComPtr < IDXGISurface > newSurface;

        hr = MFCreateVideoSampleFromSurface(nullptr, &newSample);
        RETURNIFFAILED(hr);

        hr = mMftBuilderObjPtr->createTexture(mOutputImageWidth,
                        mOutputImageHeight, D3D11_BIND_RENDER_TARGET
                                        | D3D11_BIND_UNORDERED_ACCESS,
                        DXGI_FORMAT_NV12, mD3d11Device, &newTexture);
        RETURNIFFAILED(hr);

        hr = newTexture->QueryInterface(&newSurface);
        RETURNIFFAILED(hr);

        hr = MFCreateDXGISurfaceBuffer(IID_ID3D11Texture2D, newSurface, 0,
                        FALSE, &newMediaBuffer);
        RETURNIFFAILED(hr);

        hr = newSample->AddBuffer(newMediaBuffer);
        RETURNIFFAILED(hr);

        pOutputSamples[0].pSample = newSample.Detach();
        RETURNIFFAILED(hr);
    }

    CComPtr < IMFMediaBuffer > pInput;
    hr = mSample->ConvertToContiguousBuffer(&pInput);
    RETURNIFFAILED(hr);

    CComPtr < IMFMediaBuffer > pOutput;
    hr = pOutputSamples[0].pSample->ConvertToContiguousBuffer(&pOutput);
    RETURNIFFAILED(hr);

    UINT textureInViewIndex;
    CComPtr < ID3D11Texture2D > textureIn;
    hr = mMftBuilderObjPtr->mediaBuffer2Texture(mD3d11Device, mInputType,
                    pInput, &textureIn, &textureInViewIndex);
    RETURNIFFAILED(hr);

    UINT textureOutViewIndex;
    CComPtr < ID3D11Texture2D > textureOut;
    hr = mMftBuilderObjPtr->mediaBuffer2Texture(mD3d11Device, mOutputType,
                    pOutput, &textureOut, &textureOutViewIndex);
    RETURNIFFAILED(hr);

    hr = mResizer->process(textureIn, textureInViewIndex, textureOut);
    RETURNIFFAILED(hr);

    hr = pOutput->SetCurrentLength(mOutputImageSize);
    RETURNIFFAILED(hr);

    pOutputSamples[0].dwStatus = 0;
    *pdwStatus = 0;

    /************************************************
     * Set time for output sample                    *
     ************************************************/
    LONGLONG hnsDuration = 0;
    LONGLONG hnsTime = 0;

    hr = mSample->GetSampleDuration(&hnsDuration);
    RETURNIFFAILED(hr);

    hr = pOutputSamples[0].pSample->SetSampleDuration(hnsDuration);
    RETURNIFFAILED(hr);

    hr = mSample->GetSampleTime(&hnsTime);
    RETURNIFFAILED(hr);

    hr = pOutputSamples[0].pSample->SetSampleTime(hnsTime);
    RETURNIFFAILED(hr);

    textureIn.Release();
    textureOut.Release();
    mSample.Release();

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
HRESULT Transform::onCheckInputType(IMFMediaType *pmt)
{
    return onCheckMediaType(pmt);
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
HRESULT Transform::onCheckOutputType(IMFMediaType *pmt)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCriticalSection, true);

    HRESULT hr;

    hr = onCheckMediaType(pmt);
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
 *  @fn     onCheckMediaType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
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

    GUID subtype;
    hr = mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
    RETURNIFFAILED(hr);

    if (subtype != MFVideoFormat_NV12)
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
 *  @fn     onSetInputType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::onSetInputType(IMFMediaType *pmt)
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
 *  @fn     onSetOutputType
 *  @brief  
 *
 *  @param[in] pmt   :Object of IMFMediaType interface.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Transform::onSetOutputType(IMFMediaType *pmt)
{
    mOutputType.Release();
    mOutputType = pmt;

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
HRESULT Transform::onFlush()
{
    mSample.Release();
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
HRESULT Transform::updateFormatInfo()
{
    HRESULT hr;
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mCriticalSection, true);

    if (nullptr != mOutputType)
    {
        mOutputImageWidth = 0;
        mOutputImageHeight = 0;
        mOutputImageSize = 0;

        hr = MFGetAttributeSize(mOutputType, MF_MT_FRAME_SIZE,
                        &mOutputImageWidth, &mOutputImageHeight);
        RETURNIFFAILED(hr);

        hr = mMftBuilderObjPtr->getNV12ImageSize(mOutputImageWidth,
                        mOutputImageHeight, &mOutputImageSize);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}
