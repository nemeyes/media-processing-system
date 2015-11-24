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
 * @file <OpenCLVideoFilter.cpp>
 *
 * @brief VideoFilter implementation using OpenCL
 *
 ********************************************************************************
 */

#include "OpenCLVideoFilter.h"

OpenCLVideoFilter::~OpenCLVideoFilter()
{
    Terminate();
}

AMF_RESULT OpenCLVideoFilter::Init(amf::AMFContext* context,
                                   amf::AMF_MEMORY_TYPE memType,
                                   int width, int height)
{
    m_pContext = context;
    m_memType  = memType;
    m_width    = width;
    m_height   = height;

    cl_int status;

    // Get opencl context, opencl command queue and opencl device id
    cl_context opencl_context = (cl_context)m_pContext->GetOpenCLContext();
    cl_command_queue opencl_queue = (cl_command_queue)m_pContext->GetOpenCLCommandQueue();
    cl_device_id opencl_device = (cl_device_id)m_pContext->GetOpenCLDeviceID();

    // create a CL program using the kernel source
    const char *filename = KERNEL_FILE_NAME;
    std::string sourceStr;
    status = ConvertToString(filename, sourceStr);
    if (status != 0)
    {
        printf("Error reading kernel file %s.\n", filename);
        return AMF_FAIL;
    }

    const char *source = sourceStr.c_str();
    size_t sourceSize[] ={strlen(source)};

    // create opencl program
    cl_program opencl_program = clCreateProgramWithSource(opencl_context, 1, &source, sourceSize, &status);
    if (status != CL_SUCCESS)
    {
        printf("Error: clCreateProgramWithSource failed, status = %d\n", status);
        return AMF_FAIL;
    }

    // build opencl program
    status = clBuildProgram(opencl_program, 1, &opencl_device, NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        printf("clBuildProgram failed with %d\n", status);

        size_t length;
        int size = 163844;
        char *buffer = (char *) malloc(size);
        clGetProgramBuildInfo(opencl_program, opencl_device, CL_PROGRAM_BUILD_LOG, size, buffer, &length);

        FILE *fp = fopen("buildlog.txt", "w");
        fprintf(fp, "%s\n", buffer);
        fclose(fp);
        free(buffer);
        clReleaseContext(opencl_context);
        return AMF_FAIL;
    }

    // create opencl kernel for filtering luma
    m_kernelYFilter = clCreateKernel(opencl_program, KERNEL_LUMA, &status);
    if (status != CL_SUCCESS)
    {
        printf("Error: Creating kernel failed, status = %d\n", status);
        return AMF_FAIL;
    }

    // create opencl kernel for filling chroma with constant
    m_kernelUVFilter = clCreateKernel(opencl_program, KERNEL_CHROMA, &status);
    if (status != CL_SUCCESS)
    {
        printf("Error: Creating kernel failed, status = %d\n", status);
        return AMF_FAIL;
    }

    return AMF_OK;
}

AMF_RESULT OpenCLVideoFilter::Terminate()
{
    return AMF_OK;
}

AMF_RESULT OpenCLVideoFilter::Process(amf::AMFData* pData, amf::AMFData** ppDataOut)
{
    AMF_RESULT res = AMF_OK;
    cl_int status;

    // Get opencl command queue
    cl_command_queue opencl_queue = (cl_command_queue)m_pContext->GetOpenCLCommandQueue();

    // Input surface: decoder output NV12 surface
    // Convert surface memory type from "Dx9/Dx11" to "OpenCL"
    amf::AMFSurfacePtr pSurfaceIn(pData);
    res = pSurfaceIn->Convert(amf::AMF_MEMORY_OPENCL);
    if(res != AMF_OK)
    {
        printf("AMFSurfrace::Convert(amf::AMF_MEMORY_OPENCL) failed\n");
        return res;
    }

    // Output surface: Create a NV12 surface of Dx9/Dx11 type
    // Convert surface memory type from "Dx9/Dx11" to "OpenCL"
    amf::AMFSurfacePtr pSurfaceOut;
    res = m_pContext->AllocSurface(m_memType, amf::AMF_SURFACE_NV12, m_width, m_height, &pSurfaceOut);
    if(res != AMF_OK)
    {
        printf("AMFSurfrace::AllocSurface() failed\n");
        return res;
    }
    res = pSurfaceOut->Convert(amf::AMF_MEMORY_OPENCL);
    if(res != AMF_OK)
    {
        printf("AMFSurfrace::Convert(amf::AMF_MEMORY_OPENCL) failed\n");
        return res;
    }

    amf::AMFContext::AMFOpenCLLocker locker(m_pContext);

    // Input cl memory buffers from the surface
    cl_mem inLumaBuf = (cl_mem)pSurfaceIn->GetPlane(amf::AMF_PLANE_Y)->GetNative();

    // Output cl memory buffers from the surface
    cl_mem outLumaBuf  = (cl_mem)pSurfaceOut->GetPlane(amf::AMF_PLANE_Y)->GetNative();
    cl_mem outChromaBuf = (cl_mem)pSurfaceOut->GetPlane(amf::AMF_PLANE_UV)->GetNative();

    cl_uint width = pSurfaceIn->GetPlaneAt(0)->GetWidth();
    cl_uint height = pSurfaceIn->GetPlaneAt(0)->GetHeight();

    // Process on Luma
    // set kernel args.
    status = clSetKernelArg(m_kernelYFilter, 0, sizeof(cl_mem), &inLumaBuf);
    if (status != CL_SUCCESS)
    {
        printf("Error: Error in setting kernel argument 0, status = %d\n", status);
        return AMF_FAIL;
    }
    status = clSetKernelArg(m_kernelYFilter, 1, sizeof(cl_mem), &outLumaBuf);
    if (status != CL_SUCCESS)
    {
        printf("Error: Error in setting kernel argument 1, status = %d\n", status);
        return AMF_FAIL;
    }

    // Enqueue a kernel run call.
    size_t global_threads[] = {width, height};

    status = clEnqueueNDRangeKernel(
                 opencl_queue,
                 m_kernelYFilter,
                 2,
                 NULL,
                 global_threads,
                 NULL,
                 0,
                 NULL,
                 NULL);
    if(status != CL_SUCCESS)
    {
        printf("Error: Enqueue kernel onto command queue failed, status = %d\n", status);
        return AMF_FAIL;
    }


    // Process on Chroma
    //set kernel args.
    status = clSetKernelArg(m_kernelUVFilter, 0, sizeof(cl_mem), &outChromaBuf);
    if (status != CL_SUCCESS)
    {
        printf("Error: Error in setting kernel argument, status = %d\n", status);
        return AMF_FAIL;
    }

    // Enqueue a kernel run call.
    size_t global_threads2[] = {width/2, height/2};

    status = clEnqueueNDRangeKernel(
                 opencl_queue,
                 m_kernelUVFilter,
                 2,
                 NULL,
                 global_threads2,
                 NULL,
                 0,
                 NULL,
                 NULL);
    if (status != CL_SUCCESS)
    {
        printf("Error: Enqueue kernel onto command queue failed, status = %d\n", status);
        return AMF_FAIL;
    }

    status = clFinish(opencl_queue);

    if(pSurfaceOut)
    {
        *ppDataOut = pSurfaceOut.Detach();
    }

    pSurfaceIn = NULL;

    return res;
}

/* convert the kernel file into a string */
AMF_RESULT OpenCLVideoFilter::ConvertToString(const char *filename, std::string& s)
{
    size_t size;
    char*  str;
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size+1];
        if(!str)
        {
            f.close();
            return AMF_OK;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
        s = str;
        delete[] str;
        return AMF_OK;
    }
    printf("Error: failed to open file %s\n", filename);
    return AMF_FAIL;
}
