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
 * @file <MftResizer.h>                          
 *                                       
 * @brief Defines class for video editing (resizing) by means of C++ AMP.
 *         
 ********************************************************************************
 */

#ifndef MFTRESIZER_H
#define MFTRESIZER_H

#include <d3d11.h>
#include <atlcomcli.h>
#include <mfobjects.h>

#include "MftUtils.h"
/**
 *   @class Resizer. 
 *   @brief Implements video editing (resizing) by means C++ Amp.
 *   Compute shaders are used for conversion NV12 <-> (R8(Y) + R8G8(UV)).
 */
class Resizer
{
public:

    /**
     *   @brief Constructor.
     */
    Resizer();

    /**
     *   @brief Destructor.
     */
    ~Resizer();

    /**
     *   @brief init().
     */
    HRESULT init(IMFDXGIDeviceManager* deviceManager);

    /**
     *   @brief cleanup().
     */
    HRESULT cleanup();

    /**
     *   @brief process().
     */
    HRESULT process(ID3D11Texture2D* d3d11Texture2dIn, UINT srcViewIndex,
                    ID3D11Texture2D* d3d11Texture2dOut);

private:

    /**
     *   @brief doResize(). Does actual work.
     */
    HRESULT doResize(ID3D11Texture2D* pOutLuminance,
                    ID3D11Texture2D* pOutChrominance,
                    ID3D11Texture2D* pInLuminance,
                    ID3D11Texture2D* pInChrominance);

    /**
     *   @brief initComputeShaders().
     */
    HRESULT initComputeShaders();

    /**
     *   @brief createNV12Planes(). Create Y and UV planes as textures
     */
    HRESULT createNV12Planes(ID3D11Texture2D* nv12Texture,
                    ID3D11Texture2D** luminanceAsTexture,
                    ID3D11Texture2D** chrominanceAsTexture);

    /**
     *   @brief mergeYandUVplanesToNV12(). Merges two textures (R8 and R8G8) to single NV12.
     */
    HRESULT mergeYandUVplanesToNV12(ID3D11Texture2D* pInY,
                    ID3D11Texture2D* pInUV, ID3D11Texture2D* pOutText);

    CComPtr<IMFDXGIDeviceManager> mDeviceManager;
    CComPtr<ID3D11Device> mD3d11Device;

    CComPtr<ID3D11ComputeShader> mCopyRGshader;
    CComPtr<ID3D11ComputeShader> mCopyRshader;

    bool mIsInited;
    msdk_CMftBuilder* mMftBuilderObjPtr;
};
#endif
