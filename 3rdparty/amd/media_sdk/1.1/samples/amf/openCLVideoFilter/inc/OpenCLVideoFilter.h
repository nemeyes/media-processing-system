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
 * @file <OpenCLVideoFilter.h>
 *
 * @brief VideoFilter implementation using OpenCL
 *
 ********************************************************************************
 */

#pragma once

#include <fstream>
#include <CL/cl.h>
#include <CL/cl.hpp>
#include <CL/opencl.h>
#include "AMFPlatform.h"
#include "Context.h"

#define KERNEL_FILE_NAME  "SobelFilterLuma.cl"
#define KERNEL_LUMA       "SobelFilterLuma"
#define KERNEL_CHROMA     "ConstantChroma"

class OpenCLVideoFilter
{
public:
    OpenCLVideoFilter()
    {
    };
    ~OpenCLVideoFilter();
    AMF_RESULT ConvertToString(const char *filename, std::string& s);

    virtual AMF_RESULT              Init(amf::AMFContext* context, amf::AMF_MEMORY_TYPE memType, int width, int height);
    virtual AMF_RESULT              Terminate();
    virtual AMF_RESULT              Process(amf::AMFData* pData, amf::AMFData** ppDataOut);

    amf::AMFContext*                m_pContext;
    amf::AMF_MEMORY_TYPE            m_memType;
    amf_int32                       m_width;
    amf_int32                       m_height;
    cl_kernel                       m_kernelYFilter;
    cl_kernel                       m_kernelUVFilter;
};

typedef std::shared_ptr<OpenCLVideoFilter> OpenCLVideoFilterPtr;
