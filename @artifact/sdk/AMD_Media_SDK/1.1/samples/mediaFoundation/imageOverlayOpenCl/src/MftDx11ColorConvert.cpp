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
 * @file <MftDx11ColorConvert.cpp>                          
 *                                       
 * @brief This file contains functions for color converting image using OpenCL
 *         
 ********************************************************************************
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

#include "MftDx11ColorConvert.h"
#include "MftDx11NV12ToRGBA.cl.h"
#include "MftDx11RGBAToNV12.cl.h"

#define OPENCL_RETURNIFFAILED(hr) if ((hr) != CL_SUCCESS) \
{ \
    DebugBreak(); \
    return E_FAIL; \
}

#define INITPFN(x,y) \
    x = (x ## _fn)clGetExtensionFunctionAddressForPlatform(y,#x); \
    if(!x) \
{    \
    std::wostringstream errorStream; \
    errorStream << "INITPFN failed geting " << #x << std::endl; \
    OutputDebugString(errorStream.str().c_str()); \
}

extern HMODULE g_hModule;

/**********************************************************************************
 * NV12 texture from DX11 is represented as a CL_R/CL_UNSIGNED_INT8 OpenCL image   * 
 * RGB texture from DX11 is represented as a CL_BGRA/CL_UNSIGNED_INT8 OpenCL image * 
 ***********************************************************************************/
cl_image_format RawNV12Format = { CL_R, CL_UNSIGNED_INT8 };
cl_image_format RawRGBFormat = { CL_BGRA, CL_UNORM_INT8 };

clGetDeviceIDsFromD3D11KHR_fn clGetDeviceIDsFromD3D11KHR = nullptr;
clCreateFromD3D11Texture2DKHR_fn clCreateFromD3D11Texture2DKHR = nullptr;
clEnqueueAcquireD3D11ObjectsKHR_fn clEnqueueAcquireD3D11ObjectsKHR = nullptr;
clEnqueueReleaseD3D11ObjectsKHR_fn clEnqueueReleaseD3D11ObjectsKHR = nullptr;

/** 
 *******************************************************************************
 *  @fn     texture2Image
 *  @brief  Gets OpenCL image object from D3D11 texture using interop
 *           
 *  @param[in] context              : OpenCL Context
 *  @param[in] texture              : Input texture
 *  @param[in] textureViewIndex     : Texture view index
 *  @param[in] memAccess            : Memory access flags
 *  @param[out] image               : Output OpenCL Image
 *  @param[out] width               : Output OpenCL image's width
 *  @param[out] height              : Output OpenCL image's height
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT texture2Image(cl::Context& context, CComPtr<ID3D11Texture2D> texture,
                UINT textureViewIndex, cl_mem_flags memAccess,
                cl::Image2D& image, UINT32* width, UINT32* height)
{
    if (nullptr == texture)
    {
        return E_INVALIDARG;
    }

    if (nullptr == width || nullptr == height)
    {
        return E_POINTER;
    }

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    *width = desc.Width;
    *height = desc.Height;

    cl_int errNum;
    image = clCreateFromD3D11Texture2DKHR(context(), memAccess, texture,
                    textureViewIndex, &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     ~ColorConvert
 *  @brief  Destructor
 *******************************************************************************
 */
ColorConvert::~ColorConvert()
{
    delete mMftBuilderObjPtr;
}

/** 
 *******************************************************************************
 *  @fn     ColorConvert
 *  @brief  Constructor
 *******************************************************************************
 */
ColorConvert::ColorConvert()
{

    mMftBuilderObjPtr = new msdk_CMftBuilder;
}

/** 
 *******************************************************************************
 *  @fn     Init
 *  @brief  Initializes the color convert object
 *           
 *  @param[in] device     : D3D11 device pointer
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT ColorConvert::init(ID3D11Device* device)
{
    if (nullptr == device)
    {
        return E_INVALIDARG;
    }

    mDevice.Release();

    HRESULT hr;

    CComPtr < ID3D11Device > localDevice;
    hr = device->QueryInterface(&localDevice);
    RETURNIFFAILED(hr);

    cl_int errNum;

    std::vector < cl::Platform > platforms;
    errNum = cl::Platform::get(&platforms);

    if (errNum != CL_SUCCESS || platforms.size() == 0)
    {
        RETURNIFFAILED( E_FAIL);
    }

    cl::Platform platform = platforms[0];

    cl_uint num_devices;
    cl_device_id cdDevices[10];

    INITPFN(clGetDeviceIDsFromD3D11KHR,platform());
    INITPFN(clCreateFromD3D11Texture2DKHR,platform());
    INITPFN(clEnqueueAcquireD3D11ObjectsKHR,platform());
    INITPFN(clEnqueueReleaseD3D11ObjectsKHR,platform());

    if (!clGetDeviceIDsFromD3D11KHR || !clCreateFromD3D11Texture2DKHR
                    || !clEnqueueAcquireD3D11ObjectsKHR
                    || !clEnqueueReleaseD3D11ObjectsKHR)
    {
        RETURNIFFAILED( E_FAIL);
    }
    errNum = clGetDeviceIDsFromD3D11KHR(platform(), CL_D3D11_DEVICE_KHR,
                    device, CL_PREFERRED_DEVICES_FOR_D3D11_KHR, ARRAYSIZE(
                                    cdDevices), cdDevices, &num_devices);
    OPENCL_RETURNIFFAILED(errNum);

    if (0 == num_devices)
    {
        TRACE_MSG("There is no applicable OpenCL devices", 0);
        return E_FAIL;
    }

    /***************************************************************************
     * Use the first device                                                     *
     ***************************************************************************/
    std::vector < cl::Device > devices;
    devices.push_back(cl::Device(cdDevices[0]));

    cl_context_properties contextProperties[] =
                    { CL_CONTEXT_D3D11_DEVICE_KHR, (cl_context_properties)(
                                    (void*) device), CL_CONTEXT_PLATFORM,
                      (cl_context_properties) platform(), 0 };

    mClContext = cl::Context(devices, contextProperties, NULL, NULL, &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    cl::Program::Sources sources;
    sources.push_back(std::make_pair(NV12TORGBA_OPENCL_SCRIPT, ARRAYSIZE(
                    NV12TORGBA_OPENCL_SCRIPT)));
    sources.push_back(std::make_pair(RGBATONV12_OPENCL_SCRIPT, ARRAYSIZE(
                    RGBATONV12_OPENCL_SCRIPT)));

    cl::Program clProgram = cl::Program(mClContext, sources, &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = clProgram.build(devices);
    if (errNum != CL_SUCCESS)
    {
        size_t usedBufferSize;
        errNum = ::clGetProgramBuildInfo(clProgram(), devices[0](),
                        CL_PROGRAM_BUILD_LOG, 0, nullptr, &usedBufferSize);
        OPENCL_RETURNIFFAILED(errNum);

        std::vector<char> buffer(usedBufferSize + 1, 0);
        errNum = ::clGetProgramBuildInfo(clProgram(), devices[0](),
                        CL_PROGRAM_BUILD_LOG, usedBufferSize, &buffer[0],
                        &usedBufferSize);
        OPENCL_RETURNIFFAILED(errNum);

        const char* msg = &buffer[0];

        std::cout << msg << std::endl;

        return E_FAIL;
    }

    mClNV12TORGBAKernel = cl::Kernel(clProgram, NV12TORGBA_PROGRAM_NAME,
                    &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    mClRGBATONV12Kernel = cl::Kernel(clProgram, RGBATONV12_PROGRAM_NAME,
                    &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    mClQueue = cl::CommandQueue(mClContext, devices[0], 0, &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    /***************************************************************************
     * Initialization has finished => save device.                              *
     ***************************************************************************/
    mDevice = device;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     Process
 *  @brief  Process the color convertion operation
 *           
 *  @param[in] deviceManagerPtr     : D3D11 device manager pointer
 *  @param[in] inputMediaType       : Input media type
 *  @param[in] inputSample          : Input media sample
 *  @param[in] outputMediaType      : Output media type
 *  @param[out] outputSample        : Output media sample
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT ColorConvert::process(ULONG_PTR deviceManagerPtr,
                IMFMediaType* inputMediaType, IMFSample* inputSample,
                IMFMediaType* outputMediaType, IMFSample* outputSample,
                bool useInterop)
{
    if (0 == deviceManagerPtr)
    {
        return E_INVALIDARG;
    }

    if (nullptr == inputSample)
    {
        return E_INVALIDARG;
    }

    if (nullptr == outputSample)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    DWORD outputBufferCount;
    hr = outputSample->GetBufferCount(&outputBufferCount);
    RETURNIFFAILED(hr);

    if (outputBufferCount != 0)
    {
        TRACE_MSG(
                        "Unexpected sample. Expected a sample with zero attached buffers",
                        outputBufferCount);
        return E_UNEXPECTED;
    }

    CComPtr < IUnknown > deviceManagerUnknown
                    = reinterpret_cast<IUnknown*> (deviceManagerPtr);

    CComPtr < IMFDXGIDeviceManager > dxgiDeviceManager;
    hr = deviceManagerUnknown->QueryInterface(&dxgiDeviceManager);
    RETURNIFFAILED(hr);

    HANDLE deviceHandle;
    hr = dxgiDeviceManager->OpenDeviceHandle(&deviceHandle);
    RETURNIFFAILED(hr);

    CComPtr < ID3D11Device > device;
    hr
                    = dxgiDeviceManager->GetVideoService(deviceHandle,
                                    IID_PPV_ARGS(&device));
    RETURNIFFAILED(hr);

    if (mDevice != device)
    {
        hr = init(device);
        RETURNIFFAILED(hr);
    }

    UINT32 outputWidth;
    UINT32 outputHeight;
    hr = MFGetAttributeSize(outputMediaType, MF_MT_FRAME_SIZE, &outputWidth,
                    &outputHeight);
    RETURNIFFAILED(hr);

    UINT32 inputWidth;
    UINT32 inputHeight;
    hr = MFGetAttributeSize(inputMediaType, MF_MT_FRAME_SIZE, &inputWidth,
                    &inputHeight);
    RETURNIFFAILED(hr);

    CComPtr < IMFMediaBuffer > inputMediaBuffer;
    hr = inputSample->ConvertToContiguousBuffer(&inputMediaBuffer);
    RETURNIFFAILED(hr);

    cl_int errNum;

    CComPtr < IMFMediaBuffer > outputMediaBuffer;

    GUID inputSubType;
    hr = inputMediaType->GetGUID(MF_MT_SUBTYPE, &inputSubType);
    RETURNIFFAILED(hr);

    GUID outputSubType;
    hr = outputMediaType->GetGUID(MF_MT_SUBTYPE, &outputSubType);
    RETURNIFFAILED(hr);

    if (useInterop)
    {
        CComPtr < ID3D11Texture2D > outputTexture;
        CComPtr < IDXGISurface > outputSurface;
        if (IsEqualGUID(MFVideoFormat_RGB32, outputSubType)) // Output is RGB
        {
            hr
                            = mMftBuilderObjPtr->createTexture(
                                            outputWidth,
                                            outputHeight,
                                            D3D11_BIND_RENDER_TARGET
                                                            | D3D11_BIND_SHADER_RESOURCE,
                                            DXGI_FORMAT_B8G8R8A8_UNORM,
                                            mDevice, &outputTexture);
            RETURNIFFAILED(hr);

            hr = outputTexture->QueryInterface(&outputSurface);
            RETURNIFFAILED(hr);

            hr = MFCreateDXGISurfaceBuffer(IID_ID3D11Texture2D, outputSurface,
                            0, TRUE, &outputMediaBuffer);
            RETURNIFFAILED(hr);
        }
        else
        {
            hr = mMftBuilderObjPtr->createTexture(outputWidth, outputHeight,
                            D3D11_BIND_RENDER_TARGET
                                            | D3D11_BIND_SHADER_RESOURCE,
                            DXGI_FORMAT_NV12, mDevice, &outputTexture);
            RETURNIFFAILED(hr);

            hr = outputTexture->QueryInterface(&outputSurface);
            RETURNIFFAILED(hr);

            hr = MFCreateDXGISurfaceBuffer(IID_ID3D11Texture2D, outputSurface,
                            0, FALSE, &outputMediaBuffer);
            RETURNIFFAILED(hr);
        }

        UINT32 widthOut;
        UINT32 heightOut;
        cl::Image2D clImageOut;
        hr = texture2Image(mClContext, outputTexture, 0, CL_MEM_WRITE_ONLY,
                        clImageOut, &widthOut, &heightOut);
        RETURNIFFAILED(hr);

        UINT inputTextureViewIndex;
        CComPtr < ID3D11Texture2D > inputTexture;

        hr
                        = mMftBuilderObjPtr->mediaBuffer2Texture(mDevice,
                                        inputMediaType, inputMediaBuffer,
                                        &inputTexture, &inputTextureViewIndex);
        RETURNIFFAILED(hr);

        UINT32 widthIn;
        UINT32 heightIn;
        cl::Image2D clImageIn;
        /***************************************************************************
         * Get openCL image fromm input texture using interop                       *
         ***************************************************************************/
        hr = texture2Image(mClContext, inputTexture, inputTextureViewIndex,
                        CL_MEM_READ_ONLY, clImageIn, &widthIn, &heightIn);
        RETURNIFFAILED(hr);

        errNum = clEnqueueAcquireD3D11ObjectsKHR(mClQueue(), 1, &clImageIn(),
                        0, NULL, NULL);
        OPENCL_RETURNIFFAILED(errNum);

        errNum = clEnqueueAcquireD3D11ObjectsKHR(mClQueue(), 1, &clImageOut(),
                        0, NULL, NULL);
        OPENCL_RETURNIFFAILED(errNum);

        /***************************************************************************
         * Do the color conversion                                                        *
         ***************************************************************************/
        if (IsEqualGUID(MFVideoFormat_RGB32, inputSubType)) // Input format is RGB
        {
            hr = rgbatonv12(clImageIn, widthIn, heightIn, clImageOut, widthOut,
                            heightOut);
            RETURNIFFAILED(hr);
        }
        else
        {
            hr = nv12torgba(clImageIn, widthIn, heightIn, clImageOut, widthOut,
                            heightOut);
            RETURNIFFAILED(hr);
        }

        errNum = clEnqueueReleaseD3D11ObjectsKHR(mClQueue(), 1, &clImageIn(),
                        0, NULL, NULL);
        OPENCL_RETURNIFFAILED(errNum);

        errNum = clEnqueueReleaseD3D11ObjectsKHR(mClQueue(), 1, &clImageOut(),
                        0, NULL, NULL);
        OPENCL_RETURNIFFAILED(errNum);

        errNum = mClQueue.finish();
        OPENCL_RETURNIFFAILED(errNum);

        inputTexture.Release();
        outputTexture.Release();
    }
    else
    {
        BYTE* source;
        DWORD maxSourceSize;
        DWORD currentSourceSize;
        hr
                        = inputMediaBuffer->Lock(&source, &maxSourceSize,
                                        &currentSourceSize);
        RETURNIFFAILED(hr);

        UINT32 widthIn = inputWidth;
        UINT32 heightIn = inputHeight;

        cl::Image2D clImageIn;

        cl_image_desc desc;
        memset(&desc, '\0', sizeof(cl_image_desc));

        if (IsEqualGUID(MFVideoFormat_RGB32, inputSubType)) // Input is RGB 
        {
            desc.image_type = CL_MEM_OBJECT_IMAGE2D;
            desc.image_width = widthIn;
            desc.image_height = heightIn;

            clImageIn = clCreateImage(mClContext(), CL_MEM_READ_ONLY
                            | CL_MEM_COPY_HOST_PTR, &RawRGBFormat, &desc,
                            source, &errNum);
            OPENCL_RETURNIFFAILED(errNum);
        }
        else
        {
            desc.image_type = CL_MEM_OBJECT_IMAGE2D;
            desc.image_width = widthIn;
            desc.image_height = heightIn * 3 / 2;

            clImageIn = clCreateImage(mClContext(), CL_MEM_READ_ONLY
                            | CL_MEM_COPY_HOST_PTR, &RawNV12Format, &desc,
                            source, &errNum);
            OPENCL_RETURNIFFAILED(errNum);
        }

        UINT32 widthOut = outputWidth;
        UINT32 heightOut = outputHeight;

        cl::Image2D clImageOut;
        cl_image_desc descOut;
        memset(&descOut, '\0', sizeof(cl_image_desc));

        if (IsEqualGUID(MFVideoFormat_RGB32, outputSubType)) // Output is RGB
        {
            descOut.image_type = CL_MEM_OBJECT_IMAGE2D;
            descOut.image_width = widthOut;
            descOut.image_height = heightOut;

            clImageOut = clCreateImage(mClContext(), CL_MEM_WRITE_ONLY
                            | CL_MEM_HOST_READ_ONLY, &RawRGBFormat, &descOut,
                            nullptr, &errNum);
            OPENCL_RETURNIFFAILED(errNum);
        }
        else
        {
            descOut.image_type = CL_MEM_OBJECT_IMAGE2D;
            descOut.image_width = widthOut;
            descOut.image_height = heightOut * 3 / 2;

            clImageOut = clCreateImage(mClContext(), CL_MEM_WRITE_ONLY
                            | CL_MEM_HOST_READ_ONLY, &RawNV12Format, &descOut,
                            nullptr, &errNum);
            OPENCL_RETURNIFFAILED(errNum);
        }

        /***************************************************************************
         * Do the color conversion                                                        *
         ***************************************************************************/
        if (IsEqualGUID(MFVideoFormat_RGB32, inputSubType)) // Input format is RGB
        {
            hr = rgbatonv12(clImageIn, widthIn, heightIn, clImageOut, widthOut,
                            heightOut);
            RETURNIFFAILED(hr);
        }
        else
        {
            hr = nv12torgba(clImageIn, widthIn, heightIn, clImageOut, widthOut,
                            heightOut);
            RETURNIFFAILED(hr);
        }

        errNum = mClQueue.finish();
        OPENCL_RETURNIFFAILED(errNum);

        if (IsEqualGUID(MFVideoFormat_RGB32, outputSubType)) // Output if RGBA
        {
            hr = MFCreateMemoryBuffer(widthOut * heightOut * 4,
                            &outputMediaBuffer);
            RETURNIFFAILED(hr);
        }
        else
        {
            hr = MFCreateMemoryBuffer(widthOut * heightOut * 3 / 2,
                            &outputMediaBuffer);
            RETURNIFFAILED(hr);
        }

        DWORD maxBufferSize;
        hr = outputMediaBuffer->GetMaxLength(&maxBufferSize);
        RETURNIFFAILED(hr);

        hr = outputMediaBuffer->SetCurrentLength(maxBufferSize);
        RETURNIFFAILED(hr);

        BYTE* destination;
        DWORD maxDestinationSize;
        DWORD currentDestinationSize;
        hr = outputMediaBuffer->Lock(&destination, &maxDestinationSize,
                        &currentDestinationSize);
        RETURNIFFAILED(hr);

        if (IsEqualGUID(MFVideoFormat_RGB32, outputSubType)) // RGBA output 
        {
            size_t origin[] = { 0, 0, 0 };
            size_t region[] = { widthOut, heightOut, 1 };
            errNum = clEnqueueReadImage(mClQueue(), clImageOut(), CL_TRUE,
                            origin, region, 0, 0, destination, 0, nullptr,
                            nullptr);
            OPENCL_RETURNIFFAILED(errNum);

            errNum = mClQueue.finish();
            OPENCL_RETURNIFFAILED(errNum);
        }
        else
        {
            size_t origin[] = { 0, 0, 0 };
            size_t region[] = { widthOut, heightOut * 3 / 2, 1 };
            errNum = clEnqueueReadImage(mClQueue(), clImageOut(), CL_TRUE,
                            origin, region, 0, 0, destination, 0, nullptr,
                            nullptr);
            OPENCL_RETURNIFFAILED(errNum);

            errNum = mClQueue.finish();
            OPENCL_RETURNIFFAILED(errNum);
        }

        hr = inputMediaBuffer->Unlock();
        RETURNIFFAILED(hr);

        hr = outputMediaBuffer->Unlock();
        RETURNIFFAILED(hr);
    }

    hr = outputSample->AddBuffer(outputMediaBuffer);
    RETURNIFFAILED(hr);

    LONGLONG hnsDuration = 0;
    hr = inputSample->GetSampleDuration(&hnsDuration);
    RETURNIFFAILED(hr);

    hr = outputSample->SetSampleDuration(hnsDuration);
    RETURNIFFAILED(hr);

    LONGLONG hnsTime = 0;
    hr = inputSample->GetSampleTime(&hnsTime);
    RETURNIFFAILED(hr);

    hr = outputSample->SetSampleTime(hnsTime);
    RETURNIFFAILED(hr);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     NV12ToRGBA Color Convertion
 *  @brief  OpenCL Color Convertion operation
 *           
 *  @param[in] inputImage       : OpenCL 2D input image
 *  @param[in] inputWidth       : Input image's width
 *  @param[in] inputHeight      : Input images's height
 *  @param[out] outputImage     : OpenCL 2D output image
 *  @param[in] outputWidth      : Output image's width
 *  @param[in] outputHeight     : Output image's height
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */

HRESULT ColorConvert::nv12torgba(cl::Image2D& inputImage, cl_int inputWidth,
                cl_int inputHeight, cl::Image2D& outputImage,
                cl_int outputWidth, cl_int outputHeight)
{
    cl_int errNum;

    errNum = mClNV12TORGBAKernel.setArg<cl_mem> (0, inputImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_int> (1, inputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_int> (2, inputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_mem> (3, outputImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_int> (4, outputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_int> (5, outputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClQueue.enqueueNDRangeKernel(mClNV12TORGBAKernel, cl::NullRange,
                    cl::NDRange(inputWidth / 2, inputHeight / 2));
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClQueue.flush();
    OPENCL_RETURNIFFAILED(errNum);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     RGBAToNV12 Color Convertion
 *  @brief  OpenCL Color Convertion operation
 *           
 *  @param[in] inputImage       : OpenCL 2D input image
 *  @param[in] inputWidth       : Input image's width
 *  @param[in] inputHeight      : Input images's height
 *  @param[out] outputImage     : OpenCL 2D output image
 *  @param[in] outputWidth      : Output image's width
 *  @param[in] outputHeight     : Output image's height
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT ColorConvert::rgbatonv12(cl::Image2D& inputImage, cl_int inputWidth,
                cl_int inputHeight, cl::Image2D& outputImage,
                cl_int outputWidth, cl_int outputHeight)
{
    cl_int errNum;

    errNum = mClRGBATONV12Kernel.setArg<cl_mem> (0, inputImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_int> (1, inputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_int> (2, inputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_mem> (3, outputImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_int> (4, outputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_int> (5, outputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClQueue.enqueueNDRangeKernel(mClRGBATONV12Kernel, cl::NullRange,
                    cl::NDRange(inputWidth / 2, inputHeight / 2));
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClQueue.flush();
    OPENCL_RETURNIFFAILED(errNum);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     Shutdown
 *  @brief  Shutdown operation
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT ColorConvert::shutdown()
{
    mDevice.Release();

    return S_OK;
}

