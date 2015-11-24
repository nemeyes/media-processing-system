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
 * @file <MftDx11Resizer.h>                          
 *                                       
 * @brief Defines class for video editing (resizing) by means OpenCL.
 *         
 ********************************************************************************
 */

#ifndef MFTRESIZER_H
#define MFTRESIZER_H

#include <atlbase.h>
#include <mfobjects.h>
#include <CL/cl.hpp>
#include <CL/cl_ext.h>
#include <CL/cl_d3d11.h>

#include "MftUtils.h"
#include "VideoEffect.h"

/**
 *   @class Resizer. 
 *   @brief Implements video editing (resizing) by means OpenCL.
 */
class Resizer: public VideoEffect
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
     *   @brief VideoEffect::isD3DAware().
     */
    virtual bool isD3DAware()
    {
        return false;
    }
    ;

    /**
     *   @brief VideoEffect::IsD3D11Aware().
     */
    virtual bool isD3D11Aware()
    {
        return true;
    }
    ;

    /**
     *   @brief VideoEffect::process().
     */
    virtual HRESULT process(ULONG_PTR deviceManagerPtr,
                    IMFMediaType* inputMediaType, IMFSample* inputSample,
                    IMFMediaType* outputMediaType, IMFSample* outputSample);
    /**
     *   @brief VideoEffect::shutdown().
     */
    virtual HRESULT shutdown();

private:

    HRESULT init(ID3D11Device* device);

    HRESULT resize(cl::Image2D& clTexture2DIn, cl_int widthIn, cl_int heightIn,
                    cl::Image2D& clTexture2DOut, cl_int widthOut,
                    cl_int heightOut);

    CComPtr<ID3D11Device> mDevice;

    cl::Context mClContext;
    cl::Kernel mClResizeKernel;

    cl::CommandQueue mClQueue;

    msdk_CMftBuilder* mMftBuilderObjPtr;
};
#endif
