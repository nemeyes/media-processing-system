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
********************************************************
* @file <Transform.h> 
*   
* @brief This file contains declaration of CTransform class
*
********************************************************
*/
#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <d3d11.h>
#include <atlcomcli.h>
#include <mfobjects.h>

/**
* CTransform
* This class use D3D11Device for editing video frames.
*/
class CTransform
{
public:
    /**
    * Constructor
    */
    CTransform();
    ~CTransform() {}

    /**
    * Initialization of D3D11 device and compute shaders
    * @param deviceManager pointer to device manager
    * @return S_OK on success and other HRESULT on failure
    */
    HRESULT init(CComPtr<IMFDXGIDeviceManager> deviceManager);

    /**
    * cleanup all necessary recources
    */
    HRESULT cleanup();

    /**
    * Perform transformation of input video frame
    * @param d3d11Texture2dIn Pointer to the input video frame
    * @param srcViewIndex Number of texture in array which currently should be processed
    * @param d3d11Texture2dOut Pointer to the output transformed video frame
    * @return S_OK on success and other HRESULT on failure
    */
    HRESULT process(ID3D11Texture2D* d3d11Texture2dIn, UINT srcViewIndex, ID3D11Texture2D* d3d11Texture2dOut);

private:
    CComPtr<IMFDXGIDeviceManager>   mDeviceManager;
    CComPtr<ID3D11Device>           mDevice;
    
    CComPtr<ID3D11ComputeShader> mCopyRGshader; /**< Compiled compute shaders */
    CComPtr<ID3D11ComputeShader> mCopyRshader;  /**< Compiled compute shaders */

    bool mIsInited;

private:
    HRESULT doTransform(ID3D11Texture2D* pOutLuminance, ID3D11Texture2D* pOutChrominance,
                        ID3D11Texture2D* pInLuminance, ID3D11Texture2D* pInChrominance);
    HRESULT initComputeShaders();
    HRESULT createNv12Planes(ID3D11Texture2D* nv12Texture, ID3D11Texture2D** luminanceAsTexture,
                             ID3D11Texture2D** chrominanceAsTexture);
};
#endif