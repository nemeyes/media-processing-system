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
***************************************************************************
* @file <Common.h> 
*   
* @brief This file contains declaration of general function for all project
*
***************************************************************************
*/
#ifndef _COMMON_H_
#define _COMMON_H_


#include <new>
#include <mfapi.h>
#include <mftransform.h>
#include <mfidl.h>
#include <mferror.h>
#include <assert.h>

#include <wrl\implements.h>
#include <wrl\module.h>
#include <windows.media.h>

#include <d3d11.h>
#include <atlcomcli.h>

using namespace Microsoft::WRL;

/**
* AutoLock
* This class implement auto lock of critical section.
*/
class AutoLock
{
private:
    CRITICAL_SECTION* m_criticalSection;
public:

    AutoLock(CRITICAL_SECTION* crit)
    {
       m_criticalSection = crit;
       EnterCriticalSection(m_criticalSection);
    }

    ~AutoLock()
    {
       LeaveCriticalSection(m_criticalSection);
    }
};

/**
* LockDevice
* This class implement lock functionality of D3D11 device.
*/
class LockDevice
{
private:
    CComPtr<IMFDXGIDeviceManager> m_ptrDeviceManager;
    HANDLE m_deviceHandle;

public:

    LockDevice(CComPtr<IMFDXGIDeviceManager> ptrDeviceMgr)
    {
        m_ptrDeviceManager = ptrDeviceMgr;
    }

    HRESULT Lock()
    {
        CComPtr<ID3D11Device> ptrDevice;
        HRESULT hr = m_ptrDeviceManager->OpenDeviceHandle(&m_deviceHandle);
        if (FAILED(hr)) return hr;

        hr = m_ptrDeviceManager->LockDevice(m_deviceHandle, IID_PPV_ARGS(&ptrDevice), true); 
        return hr;
    }

    ~LockDevice()
    {
        m_ptrDeviceManager->UnlockDevice(m_deviceHandle, true);
    }
};

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

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
HRESULT createTexture(UINT width, UINT height, UINT bindFlag, DXGI_FORMAT format, ID3D11Device* p3dDevice, ID3D11Texture2D** texture);

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
*****************************************************************************************
*/
HRESULT convertTextureFormat(ID3D11Texture2D* pDst, ID3D11Texture2D* pSrc, UINT srcViewIndex);

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
HRESULT getNV12ImageSize(UINT32 width, UINT32 height, DWORD* pcbImage);

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
HRESULT getDefaultStride(IMFMediaType *pType, LONG *plStride);

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
                        BOOL fixedSize = TRUE,
                        BOOL samplesIndependent = TRUE,
                        const MFRatio* frameRate = nullptr,
                        const MFRatio* pixelAspectRatio = nullptr,
                        UINT32 width = 0U, 
                        UINT32 height = 0U,
                        MFVideoInterlaceMode interlaceMode = MFVideoInterlace_Progressive);

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
                            UINT* textureViewIndex );

#endif