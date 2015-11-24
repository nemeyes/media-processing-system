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
***************************************************************
* @file <Common.cpp> 
*   
* @brief This file contains implementation of common functions
*
***************************************************************
*/
#include "..\Inc\Common.h"

/** 
*********************************************************************************
* @fn createTexture
* @brief This function create D3D11 texture 2D according input parameters
* 
* @param[in] width : Width of the texture
* @param[in] height : Height of the texture
* @param[in] bindFlag: Flags (see D3D11_BIND_FLAG) for binding to pipeline stages 
* @param[in] format: Texture format (see DXGI_FORMAT)
* @param[in] p3dDevice: pointer to the ID3D11Device
* @param[out] texture : pointer to pointer of created texture
*
* @return S_OK on success and other HRESULT on failure
*********************************************************************************
*/
HRESULT createTexture( UINT width, UINT height, UINT bindFlag, DXGI_FORMAT format,  ID3D11Device* p3dDevice, ID3D11Texture2D** texture )
{
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof(D3D11_TEXTURE2D_DESC) );
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = bindFlag;
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    return  p3dDevice->CreateTexture2D(&desc, NULL, texture);
}

/** 
*****************************************************************************************
* @fn convertTextureFormat
* @brief This function convert format of D3D11 texture 2D
* 
* @param[out] pDst : Destination D3D11 texture
* @param[in] pSrc : Source D3D11 texture
* @param[in] srcViewIndex: Number of texture in array which currently should be processed
*
* @return S_OK on success and other HRESULT on failure
*****************************************************************************************/
HRESULT convertTextureFormat(ID3D11Texture2D* pDst, ID3D11Texture2D* pSrc, UINT srcViewIndex)
{
    HRESULT hr = S_OK;

    D3D11_TEXTURE2D_DESC srcDesc;
    pSrc->GetDesc(&srcDesc);

    CComPtr<ID3D11Device> pDevice;
    pSrc->GetDevice(&pDevice);

    CComPtr<ID3D11VideoDevice> pVideoDevice;
    pVideoDevice = pDevice;

    D3D11_VIDEO_PROCESSOR_CONTENT_DESC videoProcessorContentDesc;
    memset(&videoProcessorContentDesc, 0, sizeof(videoProcessorContentDesc));

    videoProcessorContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
    videoProcessorContentDesc.Usage = D3D11_VIDEO_USAGE_OPTIMAL_QUALITY;
    videoProcessorContentDesc.InputWidth   = srcDesc.Width;
    videoProcessorContentDesc.InputHeight  = srcDesc.Height;
    videoProcessorContentDesc.OutputWidth  = srcDesc.Width;
    videoProcessorContentDesc.OutputHeight = srcDesc.Height;

    CComPtr<ID3D11VideoProcessor> pVideoProcessor;
    CComPtr<ID3D11VideoProcessorEnumerator> pVideoProcessorEnumerator;
    hr = pVideoDevice->CreateVideoProcessorEnumerator(&videoProcessorContentDesc, &pVideoProcessorEnumerator);
    if (FAILED(hr)) return hr;

    hr = pVideoDevice->CreateVideoProcessor(pVideoProcessorEnumerator, 0, &pVideoProcessor);
    if (FAILED(hr)) return hr;

    CComPtr<ID3D11DeviceContext> pDeviceContext;
    pDevice->GetImmediateContext(&pDeviceContext);

    CComPtr<ID3D11VideoContext> pVideoContext;
    pVideoContext = pDeviceContext;

    /************************************************************************************************ 
    * We have to disable auto processing as it can use internal VQ processing which takes extra time.
    *************************************************************************************************/
    pVideoContext->VideoProcessorSetStreamAutoProcessingMode(pVideoProcessor, 0, FALSE);

    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputViewDesc;
    memset(&outputViewDesc, 0, sizeof(outputViewDesc));
    outputViewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
    
    CComPtr<ID3D11VideoProcessorOutputView> pOutputView;
    hr = pVideoDevice->CreateVideoProcessorOutputView(pDst, pVideoProcessorEnumerator, &outputViewDesc, &pOutputView);
    if (FAILED(hr)) return hr;

    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc;
    memset(&inputViewDesc, 0, sizeof(inputViewDesc));
    inputViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
    inputViewDesc.Texture2D.ArraySlice = srcViewIndex;

    CComPtr<ID3D11VideoProcessorInputView> pInputView;
    hr = pVideoDevice->CreateVideoProcessorInputView(pSrc, pVideoProcessorEnumerator, &inputViewDesc, &pInputView);
    if (FAILED(hr)) return hr;

    D3D11_VIDEO_PROCESSOR_STREAM vpStreams;
    memset(&vpStreams,0,sizeof(vpStreams));
    vpStreams.Enable = TRUE;
    vpStreams.pInputSurface = pInputView;

    hr = pVideoContext->VideoProcessorBlt(pVideoProcessor, pOutputView, 0, 1, &vpStreams);
    if (FAILED(hr)) return hr;

    return S_OK;
}

/** 
******************************************************
* @fn getNV12ImageSize
* @brief This function calculate size of NV12 image
* 
* @param[out] pcbImage : Size of image in byte
* @param[in] width : Width of the image
* @param[in] height : Height of the image
*
* @return S_OK on success and other HRESULT on failure
******************************************************
*/
HRESULT getNV12ImageSize(UINT32 width, UINT32 height, DWORD* pcbImage)
{
    if ((height/2 > MAXDWORD - height) || ((height + height/2) > MAXDWORD / width))
    {
        return E_INVALIDARG;
    }

    // 12 bpp
    *pcbImage = width * (height + (height/2));

    return S_OK;
}

/** 
***********************************************************************
* @fn getDefaultStride
* @brief This function get the default stride from the input media type
* 
* @param[out] plStride : Poiner to default stride
* @param[in] pType : Pointer to media type
*
* @return S_OK on success and other HRESULT on failure
***********************************************************************
*/
HRESULT getDefaultStride(IMFMediaType *pType, LONG *plStride)
{
    LONG lStride = 0;

    /*************************************************** 
    * Try to get the default stride from the media type.
    ****************************************************/
    HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
    if (FAILED(hr))
    {
        /******************************************************** 
        * Attribute not set. Try to calculate the default stride.
        *********************************************************/
        GUID subtype = GUID_NULL;

        UINT32 width = 0;
        UINT32 height = 0;

        hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr))
        {
            hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
        }
        if (SUCCEEDED(hr))
        {
            if (subtype == MFVideoFormat_NV12)
            {
                lStride = width;
            }
            else if (subtype == MFVideoFormat_YUY2 || subtype == MFVideoFormat_UYVY)
            {
                lStride = ((width * 2) + 3) & ~3;
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }

        /***************************************
        * Set the attribute for later reference.
        ****************************************/
        if (SUCCEEDED(hr))
        {
            (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
        }
    }
    if (SUCCEEDED(hr))
    {
        *plStride = lStride;
    }
    return hr;
}

/** 
*******************************************************************************************
* @fn createVideoType
* @brief This function create IMFMediaType according input parameters
* 
* @param[out] ppType : Pointer to pointer of media type
* @param[in]  videoSybtype : GUID of media subtype
* @param[in]  fixedSize : Specifies for a media type whether the samples have a fixed size
* @param[in]  samplesIndependent : Specifies for a media type whether each sample is 
*                                  independent of the other samples in the stream
* @param[in]  frameRate : Frame rate of a video media type, in frames per second
* @param[in]  pixelAspectRatio : Pixel aspect ratio for a video media type
* @param[in]  width : Width of the frame
* @param[in]  height : Height of the frame
* @param[in]  interlaceMode : Describes how the frames in a video media type are interlaced
*
* @return S_OK on success and other HRESULT on failure
*******************************************************************************************
*/
HRESULT createVideoType(IMFMediaType** ppType,
                        const GUID& videoSybtype,
                        BOOL fixedSize,
                        BOOL samplesIndependent,
                        const MFRatio* frameRate,
                        const MFRatio* pixelAspectRatio,
                        UINT32 width, 
                        UINT32 height,
                        MFVideoInterlaceMode interlaceMode)
{
    if (ppType == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr;

    CComPtr<IMFMediaType> type = NULL;
    hr = MFCreateMediaType(&type);
    if (FAILED(hr)) return hr;

    hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr)) return hr;

    hr = type->SetGUID(MF_MT_SUBTYPE, videoSybtype);
    if (FAILED(hr)) return hr;

    hr = type->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, fixedSize);
    if (FAILED(hr)) return hr;

    hr = type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, samplesIndependent);
    if (FAILED(hr)) return hr;

    hr = type->SetUINT32(MF_MT_INTERLACE_MODE, interlaceMode);
    if (FAILED(hr)) return hr;

    if (frameRate != nullptr)
    {
        hr = MFSetAttributeRatio(type, MF_MT_FRAME_RATE, frameRate->Numerator, frameRate->Denominator);
        if (FAILED(hr)) return hr;
    }

    if (pixelAspectRatio != nullptr)
    {
        hr = MFSetAttributeRatio(type, MF_MT_PIXEL_ASPECT_RATIO, pixelAspectRatio->Numerator, pixelAspectRatio->Denominator);
        if (FAILED(hr)) return hr;
    }

    if (width != 0 && height != 0)
    {
        hr = MFSetAttributeSize(type, MF_MT_FRAME_SIZE, width, height);
        if (FAILED(hr)) return hr;

        LONG stride = 0;
        hr = getDefaultStride(*ppType, &stride);

        /*************************************************
        * If video sybtype is not compressed it has stride
        **************************************************/
        if (SUCCEEDED(hr))
        {
            hr = type->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(stride));
            if (FAILED(hr)) return hr;

            DWORD sampleSize = 0;

            hr = getNV12ImageSize(width, height, &sampleSize);

            if (FAILED(hr)) return hr;

            hr = type->SetUINT32(MF_MT_SAMPLE_SIZE, sampleSize);
            if (FAILED(hr)) return hr;
        }
    }

    *ppType = type.Detach();

    return S_OK;
}

/** 
****************************************************************
* @fn mediaBuffer2Texture
* @brief This function convert IMFMediaBuffer to ID#D11Texture2D
* 
* @param[out] ppTexture2d : Pointer to pointer of texture
* @param[in] pMediaBuffer : Pointer to media buffer
*
* @return S_OK on success and other HRESULT on failure
****************************************************************
*/
HRESULT mediaBuffer2Texture(IMFMediaBuffer*   pMediaBuffer, 
                            ID3D11Texture2D** ppTexture2d,
                            UINT* textureViewIndex )
{
    HRESULT hr = S_OK;

    CComPtr<IMFDXGIBuffer> spD11Buffer;
    hr = pMediaBuffer->QueryInterface(IID_IMFDXGIBuffer, (void**)&spD11Buffer);
    if (FAILED(hr)) return hr;

    hr = spD11Buffer->GetSubresourceIndex(textureViewIndex);
    if (FAILED(hr)) return hr;

    hr = spD11Buffer->GetResource(IID_ID3D11Texture2D, (void**)ppTexture2d);
    if (FAILED(hr)) return hr;

    return S_OK;
}