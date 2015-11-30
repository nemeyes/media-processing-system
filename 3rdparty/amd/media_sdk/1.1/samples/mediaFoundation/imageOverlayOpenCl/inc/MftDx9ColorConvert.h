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
 ******************************************************************************/
/**  
 ********************************************************************************
 * @file <MftDx9ColorConvert.h>                          
 *                                       
 * @brief Defines class for Color Converting by means OpenCL.
 *         
 ********************************************************************************
 */

#ifndef MFTCOLORCONVERT_H
#define MFTCOLORCONVERT_H

#include <atlbase.h>
#include <mfobjects.h>
#include <CL/cl.hpp>
#include <CL/cl_ext.h>

#include "MftUtils.h"
#include "VideoEffect.h"

#if !defined(D3DFORMAT_NV12)
#define D3DFORMAT_NV12 (D3DFORMAT)MAKEFOURCC('N','V','1','2')
#endif

/**
 *   @class ColorConvert. 
 *   @brief Implements Color Conversion by means OpenCL.
 */
class ColorConvert: public VideoEffect
{
public:

    /**
     *   @brief Constructor.
     */
    ColorConvert();

    /**
     *   @brief Destructor.
     */
    ~ColorConvert();

    /**
     *   @brief VideoEffect::isD3DAware().
     */
    virtual bool isD3DAware()
    {
        return true;
    }
    ;

    /**
     *   @brief VideoEffect::isD3D11Aware().
     */
    virtual bool isD3D11Aware()
    {
        return false;
    }
    ;

    /**
     *   @brief VideoEffect::process().
     */
    virtual HRESULT process(ULONG_PTR deviceManagerPtr,
                    IMFMediaType* inputMediaType, IMFSample* inputSample,
                    IMFMediaType* outputMediaType, IMFSample* outputSample,
                    bool useInterop);
    /**
     *   @brief VideoEffect::shutdown().
     */

    virtual HRESULT shutdown();

private:

    HRESULT init(IDirect3DDevice9Ex* device);

    HRESULT rgbatonv12(cl::Image2D& inputRGBImage, UINT32 widthIn,
                    UINT32 heightIn, cl::Image2D& outputLumaImage,
                    cl::Image2D& outputChromaImage);

    HRESULT nv12torgba(cl::Image2D& inputLumaImage,
                    cl::Image2D& inputChromaImage, UINT32 inputWidth,
                    UINT32 inputHeight, cl::Image2D& outputRGBImage);
    CComPtr<IDirect3DDevice9Ex> mDevice;

    cl::Context mClContext;
    cl::Kernel mClRGBATONV12Kernel;
    cl::Kernel mClNV12TORGBAKernel;

    cl::CommandQueue mClQueue;
};

template<class D, class M>
class DeviceLock
{
public:
    DeviceLock(M* manager) :
        mHandle(INVALID_HANDLE_VALUE), mManager(manager)
    {
    }
    ~DeviceLock()
    {
        if (nullptr == mManager)
        {
            return;
        }

        HRESULT hr = mManager->UnlockDevice(mHandle, FALSE);
    }

    HRESULT Lock(D** device)
    {
        if (nullptr == mManager)
        {
            return E_INVALIDARG;
        }

        HRESULT hr;

        hr = mManager->OpenDeviceHandle(&mHandle);
        RETURNIFFAILED(hr);

        CComPtr<D> localDevice;
        hr = mManager->LockDevice(mHandle, &localDevice, TRUE);
        RETURNIFFAILED(hr);

        *device = localDevice.Detach();

        return S_OK;
    }

private:
    CComPtr<M> mManager;
    HANDLE mHandle;
};
#endif
