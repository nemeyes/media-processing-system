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
 * @file <MftDx9Resizer.cpp>                          
 *                                       
 * @brief This file contains functions for resizing textures using OpenCL
 *         
 ********************************************************************************
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <CL/cl.hpp>
#include <CL/cl_ext.h>
#include <CL/cl_dx9_media_sharing.h>

#include "MftDx9Resize.cl.h"
#include "MftDx9Resizer.h"

extern HMODULE g_hModule;

#define OPENCL_RETURNIFFAILED(hr) if ((hr) != CL_SUCCESS) \
{ \
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

clGetDeviceIDsFromDX9MediaAdapterKHR_fn clGetDeviceIDsFromDX9MediaAdapterKHR =
                nullptr;
clCreateFromDX9MediaSurfaceKHR_fn clCreateFromDX9MediaSurfaceKHR = nullptr;
clEnqueueAcquireDX9MediaSurfacesKHR_fn clEnqueueAcquireDX9MediaSurfacesKHR =
                nullptr;
clEnqueueReleaseDX9MediaSurfacesKHR_fn clEnqueueReleaseDX9MediaSurfacesKHR =
                nullptr;

cl_image_format LuminancePlaneFormat = { CL_R, CL_UNORM_INT8 };
cl_image_format ChrominancePlaneFormat = { CL_RG, CL_UNORM_INT8 };

/** 
 *******************************************************************************
 *  @fn     nV12Surface2OpenCLImages
 *  @brief  Gets openCL image from D3D9 surface
 *           
 *  @param[in] context              : OpebCL contect
 *  @param[in] surface              : Input D3D9 surfaceMedia type
 *  @param[in] memAccess            : Memory access flags
 *  @param[out] luminanceImage      : Output openCL image for luminance
 *  @param[out] chrominanceImage    : Output openCL image for chrominance
 *  @param[out] width               : Output width
 *  @param[out] height              : Output height
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT nV12Surface2OpenCLImages(cl::Context& context,
                IDirect3DSurface9* surface, cl_mem_flags memAccess,
                cl::Image2D& luminanceImage, cl::Image2D& chrominanceImage,
                UINT32* width = nullptr, UINT32* height = nullptr)
{
    if (nullptr == surface)
    {
        return E_INVALIDARG;
    }

    if (nullptr == width || nullptr == height)
    {
        return E_POINTER;
    }

    HRESULT hr;

    D3DSURFACE_DESC desc;
    hr = surface->GetDesc(&desc);
    RETURNIFFAILED(hr);

    *width = desc.Width;
    *height = desc.Height;

    _cl_dx9_surface_info_khr surfaceInfo = { surface, 0 };

    cl_int errNum;

    const cl_uint luminancePlainIdx = 0;

    luminanceImage = clCreateFromDX9MediaSurfaceKHR(context(), memAccess,
                    CL_ADAPTER_D3D9EX_KHR, &surfaceInfo, luminancePlainIdx,
                    &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    const cl_uint chrominancePlainIdx = 1;

    chrominanceImage = clCreateFromDX9MediaSurfaceKHR(context(), memAccess,
                    CL_ADAPTER_D3D9EX_KHR, &surfaceInfo, chrominancePlainIdx,
                    &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     mediaBuffer2Surface
 *  @brief  Gets D3D9 surface object from media buffer
 *           
 *  @param[in] device           : D3D9  device pointer
 *  @param[in] mediaType        : Media type
 *  @param[in] mediaBuffer      : Input media buffer
 *  @param[out] surface         : Output D3D9 surface pointer
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT mediaBuffer2Surface(IDirect3DDevice9* device, IMFMediaType* mediaType,
                IMFMediaBuffer* mediaBuffer, IDirect3DSurface9** surface)
{
    if (nullptr == mediaBuffer || nullptr == device || nullptr == mediaType)
    {
        return E_INVALIDARG;
    }

    if (nullptr == surface)
    {
        return E_POINTER;
    }

    HRESULT hr;

    GUID mediaSybtype;
    hr = mediaType->GetGUID(MF_MT_SUBTYPE, &mediaSybtype);
    RETURNIFFAILED(hr);

    if (!IsEqualGUID(mediaSybtype, MFVideoFormat_NV12))
    {
        return E_INVALIDARG;
    }

    hr = MFGetService(mediaBuffer, MR_BUFFER_SERVICE, IID_PPV_ARGS(surface));
    if (SUCCEEDED(hr))
    {
        return S_OK;
    }

    /***************************************************************************
     * Otherwise  create a new DX9 surface and copy the buffer to it            *
     ***************************************************************************/
    UINT32 width;
    UINT32 height;
    hr = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
    RETURNIFFAILED(hr);

    UINT32 stride;
    hr = mediaType->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride);
    if (FAILED(hr))
    {
        LONG longStride;
        hr = MFGetStrideForBitmapInfoHeader(mediaSybtype.Data1, width,
                        &longStride);
        RETURNIFFAILED(hr);

        stride = static_cast<UINT32> (longStride);
    }

    CComPtr < IDirect3DSurface9 > bufferSurface;
    hr = device->CreateOffscreenPlainSurface(width, height, D3DFORMAT_NV12,
                    D3DPOOL_DEFAULT, &bufferSurface, nullptr);
    RETURNIFFAILED(hr);

    BYTE* bufferData;
    DWORD bufferMaxSize;
    DWORD bufferCurrentSize;

    hr = mediaBuffer->Lock(&bufferData, &bufferMaxSize, &bufferCurrentSize);
    RETURNIFFAILED(hr);

    D3DLOCKED_RECT lockedRect;
    hr = bufferSurface->LockRect(&lockedRect, nullptr, D3DLOCK_NO_DIRTY_UPDATE);
    RETURNIFFAILED(hr);

    BYTE* surfaceBits = reinterpret_cast<BYTE*> (lockedRect.pBits);

    const UINT32 nv12RawHeight = height * 3 / 2;

    for (UINT32 rowIdx = 0; rowIdx < nv12RawHeight; rowIdx++)
    {
        memcpy(surfaceBits, bufferData, width);

        surfaceBits += lockedRect.Pitch;
        bufferData += stride;
    }

    hr = bufferSurface->UnlockRect();

    hr = mediaBuffer->Unlock();
    RETURNIFFAILED(hr);

    *surface = bufferSurface.Detach();

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     ~Resizer
 *  @brief  Destructor
 *******************************************************************************
 */
Resizer::~Resizer()
{
}

/** 
 *******************************************************************************
 *  @fn     Resizer
 *  @brief  Constructor
 *******************************************************************************
 */
Resizer::Resizer()
{

}

/** 
 *******************************************************************************
 *  @fn     Init
 *  @brief  Initializes the resizer object
 *           
 *  @param[in] device     : D3D9 device pointer
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT Resizer::init(IDirect3DDevice9Ex* device)
{
    if (nullptr == device)
    {
        return E_INVALIDARG;
    }
    uint32 numAdapters = 0;
    mDevice.Release();

    cl_int errNum;
    IDXGIFactory* factory;
    CreateDXGIFactory(__uuidof(IDXGIFactory), (void**) &factory);
    IDXGIAdapter * pAdapter = 0;
    UINT i = 0;
    /*************************************************************************
     *  Get number of adapters                                                *
     *************************************************************************/
    while (factory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        ++numAdapters;
        i++;
    }
    factory->Release();

    std::vector < cl::Platform > platforms;
    errNum = cl::Platform::get(&platforms);

    if (errNum != CL_SUCCESS || platforms.size() == 0)
    {
        RETURNIFFAILED( E_FAIL);
    }

    cl::Platform platform = platforms[0];

    cl_uint num_devices;
    cl_device_id *cdDevices = new cl_device_id[numAdapters];
    cl_dx9_media_adapter_type_khr *mediaAdapterTypes =
                    new cl_dx9_media_adapter_type_khr[numAdapters];

    for (uint32 adapterIndex = 0; adapterIndex < numAdapters; adapterIndex++)
    {
        mediaAdapterTypes[adapterIndex] = CL_ADAPTER_D3D9EX_KHR;
    }

    INITPFN(clGetDeviceIDsFromDX9MediaAdapterKHR,platform());
    INITPFN(clCreateFromDX9MediaSurfaceKHR,platform());
    INITPFN(clEnqueueAcquireDX9MediaSurfacesKHR,platform());
    INITPFN(clEnqueueReleaseDX9MediaSurfacesKHR,platform());

    if (!clGetDeviceIDsFromDX9MediaAdapterKHR
                    || !clCreateFromDX9MediaSurfaceKHR
                    || !clEnqueueAcquireDX9MediaSurfacesKHR
                    || !clEnqueueReleaseDX9MediaSurfacesKHR)
    {
        RETURNIFFAILED( E_FAIL);
    }
    errNum = clGetDeviceIDsFromDX9MediaAdapterKHR(platform(), numAdapters,
                    mediaAdapterTypes, &device,
                    CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR,
                    numAdapters, cdDevices, &num_devices);
    OPENCL_RETURNIFFAILED(errNum);

    std::vector < cl::Device > devices(num_devices);
    for (cl_uint i = 0; i < num_devices; ++i)
    {
        devices[i] = cl::Device(cdDevices[i]);
    }

    cl_context_properties contextProperties[] =
                    { CL_CONTEXT_ADAPTER_D3D9_KHR, (cl_context_properties)(
                                    (void*) device), CL_CONTEXT_PLATFORM,
                      (cl_context_properties) platform(),
                      CL_CONTEXT_INTEROP_USER_SYNC, CL_TRUE, 0 };

    mClContext = cl::Context(devices, contextProperties, nullptr, nullptr,
                    &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    cl::Program::Sources sources;
    sources.push_back(std::make_pair(RESIZER_OPENCL_SCRIPT, ARRAYSIZE(
                    RESIZER_OPENCL_SCRIPT)));

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

    mClResizeKernel = cl::Kernel(clProgram, RESIZER_PROGRAM_NAME, &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    mClQueue = cl::CommandQueue(mClContext, devices[0], 0, &errNum);
    OPENCL_RETURNIFFAILED(errNum);
    /***************************************************************************
     * Initialization has finished => save device                               *
     ***************************************************************************/
    mDevice = device;

    delete mediaAdapterTypes;
    delete cdDevices;
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     Process
 *  @brief  Process the resize operation
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
HRESULT Resizer::process(ULONG_PTR deviceManagerPtr,
                IMFMediaType* inputMediaType, IMFSample* inputSample,
                IMFMediaType* outputMediaType, IMFSample* outputSample)
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

    CComPtr < IUnknown > deviceManagerUnknown
                    = reinterpret_cast<IUnknown*> (deviceManagerPtr);

    HRESULT hr;

    CComPtr < IDirect3DDeviceManager9 > deviceManager;
    hr = deviceManagerUnknown->QueryInterface(&deviceManager);
    RETURNIFFAILED(hr);

    DeviceLock < IDirect3DDevice9, IDirect3DDeviceManager9 > deviceLock(
                    deviceManager);

    CComPtr < IDirect3DDevice9 > device;
    hr = deviceLock.Lock(&device);
    RETURNIFFAILED(hr);

    CComPtr < IDirect3DDevice9Ex > deviceEx;
    hr = device.QueryInterface(&deviceEx);
    RETURNIFFAILED(hr);

    if (mDevice != device)
    {
        hr = init(deviceEx);
        RETURNIFFAILED(hr);
    }

    DWORD outputBufferCount;
    hr = outputSample->GetBufferCount(&outputBufferCount);
    RETURNIFFAILED(hr);

    if (outputBufferCount != 0)
    {
        TRACE_MSG(
                        "Unexpected media sample. Expected a sample with zero attached buffers",
                        outputBufferCount);
        return E_UNEXPECTED;
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

    CComPtr < IMFMediaBuffer > outputMediaBuffer;

    cl_int errNum;

    CComPtr < IDirect3DSurface9 > inputSurface;
    hr = mediaBuffer2Surface(mDevice, inputMediaType, inputMediaBuffer,
                    &inputSurface);
    RETURNIFFAILED(hr);

    CComPtr < IDirect3DSurface9 > outputSurface;
    hr = device->CreateOffscreenPlainSurface(outputWidth, outputHeight,
                    D3DFORMAT_NV12, D3DPOOL_DEFAULT, &outputSurface, nullptr);
    RETURNIFFAILED(hr);

    hr = MFCreateDXSurfaceBuffer(IID_IDirect3DSurface9, outputSurface, FALSE,
                    &outputMediaBuffer);
    RETURNIFFAILED(hr);

    UINT32 outputLuminanceWidth;
    UINT32 outputLuminanceHeight;
    cl::Image2D outputLuminanceImage;
    cl::Image2D outputChrominanceImage;
    hr = nV12Surface2OpenCLImages(mClContext, outputSurface, CL_MEM_WRITE_ONLY,
                    outputLuminanceImage, outputChrominanceImage,
                    &outputLuminanceWidth, &outputLuminanceHeight);
    RETURNIFFAILED(hr);

    UINT32 inputLuminanceWidth;
    UINT32 inputLuminanceHeight;
    cl::Image2D inputLuminanceImage;
    cl::Image2D inputChrominanceImage;
    hr = nV12Surface2OpenCLImages(mClContext, inputSurface, CL_MEM_READ_ONLY,
                    inputLuminanceImage, inputChrominanceImage,
                    &inputLuminanceWidth, &inputLuminanceHeight);
    RETURNIFFAILED(hr);

    /***************************************************************************
     * Process luminance                                                        *
     ***************************************************************************/
    errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                    &inputLuminanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                    &outputLuminanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    hr = resize(inputLuminanceImage, inputLuminanceWidth, inputLuminanceHeight,
                    outputLuminanceImage, outputLuminanceWidth,
                    outputLuminanceHeight);
    RETURNIFFAILED(hr);

    errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                    &inputLuminanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                    &outputLuminanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClQueue.finish();
    OPENCL_RETURNIFFAILED(errNum);

    /***************************************************************************
     * Process chrominance                                                      *
     ***************************************************************************/
    errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                    &inputChrominanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                    &outputChrominanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    hr = resize(inputChrominanceImage, inputLuminanceWidth / 2,
                    inputLuminanceHeight / 2, outputChrominanceImage,
                    outputLuminanceWidth / 2, outputLuminanceHeight / 2);
    RETURNIFFAILED(hr);

    errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                    &inputChrominanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                    &outputChrominanceImage(), 0, nullptr, nullptr);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClQueue.finish();
    OPENCL_RETURNIFFAILED(errNum);

    inputSurface.Release();
    outputSurface.Release();

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
 *  @fn     Resize
 *  @brief  OpenCL resize operation
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
HRESULT Resizer::resize(cl::Image2D& inputImage, UINT32 inputWidth,
                UINT32 inputHeight, cl::Image2D& outputImage,
                UINT32 outputWidth, UINT32 outputHeight)
{
    cl_int errNum;

    errNum = mClResizeKernel.setArg<cl_mem> (0, inputImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClResizeKernel.setArg<cl_mem> (1, outputImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClResizeKernel.setArg<cl_int> (2, inputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClResizeKernel.setArg<cl_int> (3, inputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClResizeKernel.setArg<cl_int> (4, outputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClResizeKernel.setArg<cl_int> (5, outputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClQueue.enqueueNDRangeKernel(mClResizeKernel, cl::NullRange,
                    cl::NDRange(outputWidth, outputHeight));
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
HRESULT Resizer::shutdown()
{
    return S_OK;
}