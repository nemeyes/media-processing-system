/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1 Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 2 Redistributions in binary form must reproduce the above copyright notice,
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
 * @file <VideoInput.h>
 *
 * @brief This file contains implementation of VideoInput class
 *
 ********************************************************************************
 */
#include "VideoInput.h"

/**
 *******************************************************************************
 *  @fn     createInstance
 *  @brief  Creates parser
 *
 *  @param[in]  url      : URL style path
 *  @param[out] ppParser : Pointer to the parser
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::createInstance(const WCHAR* url, VideoInput **ppParser)
{
    HRESULT hr = S_OK;
    VideoInput* pParser = NULL;

    DWORD pathCount = MAX_PATH;
    wchar_t temp[MAX_PATH];
    if (PathIsURL(url))
    {
        /**********************************************************************
         * convert a URL-style path to an MS-DOS style path string            *
         **********************************************************************/
        hr = PathCreateFromUrl(url, temp, &pathCount, 0);
    }
    else
    {
        wcscpy_s(temp, MAX_PATH, url);
    }

    if (SUCCEEDED(hr))
    {
        /**********************************************************************
         * handle a case where the file either doesn't exist or inaccessable  *
         **********************************************************************/
        if (!PathFileExists(temp))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            /******************************************************************
             * create a new parser                                            *
             ******************************************************************/
            pParser = new (std::nothrow) VideoInput(temp);
            if (nullptr == pParser)
            {
                return E_OUTOFMEMORY;
            }
        }

        /**********************************************************************
         * initialize the parser                                              *
         **********************************************************************/
        hr = pParser->init();
        if (SUCCEEDED(hr))
        {
            *ppParser = pParser;
        }
    }
    if (FAILED(hr) && pParser != NULL)
    {
        delete pParser;
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn     VideoInput
 *  @brief  Constructor
 *
 *  @return
 *******************************************************************************
 */
VideoInput::VideoInput(const WCHAR* url) :
    mCurrentVideoSample(0), mDuration(0), mUrl(NULL), mNextKeyframe(0)
{
    if (url != NULL && wcslen(url) > 0)
    {
        mUrl = new (std::nothrow) WCHAR[wcslen(url) + 1];
        wcscpy_s(mUrl, wcslen(url) + 1, url);
    }
    ZeroMemory(&mVideoFormat, sizeof(mVideoFormat));

}

/**
 *******************************************************************************
 *  @fn     init
 *  @brief  Intializes VideoInput objects
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::init()
{
    DWORD firstFourBytes;
    HRESULT hr = S_OK;

    if (mUrl == NULL || wcslen(mUrl) == 0)
    {
        return E_FAIL;
    }

    WideCharToMultiByte(CP_ACP, 0, mUrl, -1, mFileName, 400, NULL, NULL);
    mP264File = fopen(mFileName, "rb");
    fread(&firstFourBytes, 1, 4, mP264File);
    if (firstFourBytes != 0x01000000)
    {
        printf("-------------------------------------\n");
        printf("NOTE: Unsupported Input file format.\n");
        printf("-------------------------------------\n");
        hr = E_FAIL;
        exit(-1);
    }
    fseek(mP264File, 0L, SEEK_END);
    mFileSize = ftell(mP264File);
    fseek(mP264File, 0L, SEEK_SET);
    mBytesRead = 0;
    return hr;
}

/**
 *******************************************************************************
 *  @fn     parseHeader
 *  @brief  Parsing the Header
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::parseHeader()
{
    HRESULT hr = S_OK;
    createVideoMediaType(NULL, 0, mWidth, mHeight);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     parseVideoStreamHeader
 *  @brief  No implementation
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::parseVideoStreamHeader(void)
{

    HRESULT hr = S_OK;

    return hr;
}

/**
 *******************************************************************************
 *  @fn     getVideoMediaType
 *  @brief  Get a copy of the video media type
 *
 *  @param[in] ppMediaType : Pointer to the Media type
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::getVideoMediaType(IMFMediaType** ppMediaType)
{
    HRESULT hr = S_OK;

    if (nullptr == ppMediaType)
        return E_POINTER;
    if (nullptr == mPVideoType)
        return E_UNEXPECTED;

    hr = mPVideoType.CopyTo(ppMediaType);
    return hr;
}

/**
 *******************************************************************************
 *  @fn     getNextVideoSample
 *  @brief
 *
 *  @param[out] ppSample : pointer to the sample
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::getNextVideoSample(IMFSample** ppSample)
{
    HRESULT hr = S_OK;

    long bufferSize = 1024;
    BYTE* pBuffer = NULL;
    LONGLONG sampleTime = 0;
    CComPtr < IMFMediaBuffer > pMediaBuffer;
    CComPtr < IMFSample > pSample;

    /**************************************************************************
     * create an IMFMediaBuffer object with the required size                 *
     **************************************************************************/
    hr = MFCreateMemoryBuffer(bufferSize, &pMediaBuffer);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * lock the IMFMediaBuffer object to get a pointer to its underlyng buffer*
     /**************************************************************************/
    hr = pMediaBuffer->Lock(&pBuffer, NULL, NULL);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * read the data from the stream into the buffer                          *
     **************************************************************************/
    fread(pBuffer, 1, bufferSize, mP264File);
    mBytesRead += bufferSize;

    /**************************************************************************
     * unlock the IMFMediaBuffer                                              *
     **************************************************************************/
    hr = pMediaBuffer->Unlock();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * store the number of bytes read in the IMFMediaBuffer object            *
     **************************************************************************/
    hr = pMediaBuffer->SetCurrentLength(bufferSize);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * create the actual IMFSample object                                     *
     **************************************************************************/
    hr = MFCreateSample(&pSample);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * store the buffer in the sample                                         *
     **************************************************************************/
    hr = pSample->AddBuffer(pMediaBuffer);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * detach the sample from the object and store it in the passed-in pointer*
     **************************************************************************/
    *ppSample = pSample.Detach();

    return hr;
}

/**
 *******************************************************************************
 *  @fn     createVideoMediaType
 *  @brief  Create the video media type
 *
 *  @param[in] pUserData : User data pointer
 *  @param[in] dwUserData : User data value
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::createVideoMediaType(BYTE* pUserData, DWORD dwUserData,
                DWORD dwWidth, DWORD dwHeight)
{
    HRESULT hr = S_OK;
    CComPtr < IMFVideoMediaType > pType;

    mVideoFormat.biBitCount = 24;
    mVideoFormat.biBitCount = 0;
    mVideoFormat.biClrUsed = 0;
    mVideoFormat.biCompression = BI_RLE8;
    mVideoFormat.biHeight = dwHeight;
    mVideoFormat.biPlanes = 1;
    mVideoFormat.biWidth = dwWidth;
    mVideoFormat.biXPelsPerMeter = 0;
    mVideoFormat.biSizeImage = (dwWidth * dwHeight * 3);
    mVideoFormat.biSize = 40;
    mFpsNumerator = 30;
    mFpsDenominator = 1;

    DWORD original4CC = mVideoFormat.biCompression;

    /**************************************************************************
     * use a special case to handle custom 4CC types.  For example variations *
     * of the DivX decoder - DIV3 and DIVX - can be handled by MS decoders    *
     * MP43 and MP4V.  Therefore modify the 4CC value to match the decoders   *
     * that will handle the data                                              *
     **************************************************************************/
    /**************************************************************************
     *  special case - "DIV3" handled by "MP43" decoder                       *
     **************************************************************************/
    if (original4CC == 0x33564944)
    {
        mVideoFormat.biCompression = '34PM';
    }
    /**************************************************************************
     * special case - "DIVX" handled by "MP4V" decoder                        *
     **************************************************************************/
    else if (original4CC == 0x44495658)
    {
        mVideoFormat.biCompression = 'V4PM';
    }

    /**************************************************************************
     * construct the media type from the BitMapInfoHeader                     *
     **************************************************************************/
    hr = MFCreateVideoMediaTypeFromBitMapInfoHeaderEx(&mVideoFormat, // video info header to convert
                    mVideoFormat.biSize, // size of the header structure
                    1, // pixel aspect ratio X
                    1, // pixel aspect ratio Y
                    MFVideoInterlace_Progressive, // interlace mode
                    0, // video flags
                    mFpsNumerator, // FPS numerator
                    mFpsDenominator, // FPS denominator
                    128000, // max bitrate
                    &pType); // result - out
    RETURNIFFAILED(hr);

    /**************************************************************************
     * store the original 4CC value                                           *
     **************************************************************************/
    hr = pType->SetUINT32(MF_MT_ORIGINAL_4CC, original4CC);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * Set the video format to H264                                           *
     **************************************************************************/
    GUID subType = MFVideoFormat_H264;
    hr = pType->SetGUID(MF_MT_SUBTYPE, subType);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * store any extra video information data in the media type               *
     **************************************************************************/
    if (pUserData != NULL && dwUserData > 0)
    {
        hr = pType->SetBlob(MF_MT_USER_DATA, pUserData, dwUserData);
        RETURNIFFAILED(hr);
    }

    mPVideoType = pType;

    return hr;
}

/**
 *******************************************************************************
 *  @fn     setOffset
 *  @brief  sets the offset
 *
 *  @param[in] varStart : offset position
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::setOffset(const PROPVARIANT& varStart)
{
    (void) varStart;
    HRESULT hr = S_OK;
    return hr;
}

/**
 *******************************************************************************
 *  @fn     ~VideoInput
 *  @brief  Destructor
 *
 *  @return void :
 *******************************************************************************
 */
VideoInput::~VideoInput(void)
{
    if (mUrl != NULL)
    {
        delete mUrl;
    }
}

/**
 *******************************************************************************
 *  @fn     getPropertyStore
 *  @brief  Get the properties
 *
 *  @param[out] ppPropertyStore : Pointer to the property store
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT VideoInput::getPropertyStore(IPropertyStore** ppPropertyStore)
{
    HRESULT hr = S_OK;
    CComPtr < IPropertyStore > pPropStore;
    PROPVARIANT propValue;

    if (nullptr == ppPropertyStore)
    {
        return E_POINTER;
    }

    /**************************************************************************
     * create a new property store                                            *
     **************************************************************************/
    hr = PSCreateMemoryPropertyStore(IID_IPropertyStore, (void**) &pPropStore);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * set the duration property                                              *
     **************************************************************************/
    InitPropVariantFromInt64(mDuration, &propValue);
    hr = pPropStore->SetValue(PKEY_Media_Duration, propValue);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * detach and return the property store                                   *
     **************************************************************************/
    *ppPropertyStore = pPropStore.Detach();

    return hr;
}
