/*******************************************************************************
 Copyright �2014 Advanced Micro Devices, Inc. All rights reserved.

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
 * @file <MftDx9ColorConvert.cpp>                          
 *                                       
 * @brief This file contains functions for color converting image using OpenCL
 *         
 ********************************************************************************
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <CL/cl.hpp>
#include <CL/cl_ext.h>

#include "MftDx9ColorConvert.h"
#include "MftDx9NV12ToRGBA.cl.h"
#include "MftDx9RGBAToNV12.cl.h"
#include "cl_dx9_media_sharing.h"

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

cl_image_format lumaPlaneFormat = { CL_R, CL_UNORM_INT8 };
cl_image_format chromaPlaneFormat = { CL_RG, CL_UNORM_INT8 };
cl_image_format RawRGBFormat = { CL_BGRA, CL_UNORM_INT8 };

/** 
 *******************************************************************************
 *  @fn     surface2OpenCLImages
 *  @brief  Gets openCL image from D3D9 surface
 *           
 *  @param[in] context              : OpebCL contect
 *  @param[in] surface              : Input D3D9 surfaceMedia type
 *  @param[in] memAccess            : Memory access flags
 *  @param[out] image               : Output openCL image
 *  @param[out] width               : Output width
 *  @param[out] height              : Output height
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT surface2OpenCLImages(cl::Context& context, IDirect3DSurface9* surface,
                cl_mem_flags memAccess, cl::Image2D& image, UINT32* width =
                                nullptr, UINT32* height = nullptr)
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

    const cl_uint surfaceIdx = 0;

    image = clCreateFromDX9MediaSurfaceKHR(context(), memAccess,
                    CL_ADAPTER_D3D9EX_KHR, &surfaceInfo, surfaceIdx, &errNum);
    OPENCL_RETURNIFFAILED(errNum);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     NV12Surface2OpenCLImages
 *  @brief  Gets openCL image from D3D9 surface
 *           
 *  @param[in] context              : OpebCL contect
 *  @param[in] surface              : Input D3D9 surfaceMedia type
 *  @param[in] memAccess            : Memory access flags
 *  @param[out] luminanceImage      : Output Luminance openCL image
 *  @param[out] Chrominance Image   : Output Chroma openCL image
 *  @param[out] width               : Output width
 *  @param[out] height              : Output height
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT NV12Surface2OpenCLImages(cl::Context& context,
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

    if ((!IsEqualGUID(mediaSybtype, MFVideoFormat_NV12)) && (!IsEqualGUID(
                    mediaSybtype, MFVideoFormat_RGB32)))
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

    INT32 stride;
    stride = MFGetAttributeUINT32(mediaType, MF_MT_DEFAULT_STRIDE, width);

    GUID subtype;
    mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (IsEqualGUID(subtype, MFVideoFormat_NV12))
    {
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
        hr = bufferSurface->LockRect(&lockedRect, nullptr,
                        D3DLOCK_NO_DIRTY_UPDATE);
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
    }

    if (IsEqualGUID(subtype, MFVideoFormat_RGB32))
    {
        CComPtr < IDirect3DSurface9 > bufferSurface;
        hr = device->CreateOffscreenPlainSurface(width, height,
                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &bufferSurface,
                        nullptr);
        RETURNIFFAILED(hr);

        BYTE* bufferData;
        DWORD bufferMaxSize;
        DWORD bufferCurrentSize;

        hr = mediaBuffer->Lock(&bufferData, &bufferMaxSize, &bufferCurrentSize);
        RETURNIFFAILED(hr);

        D3DLOCKED_RECT lockedRect;
        hr = bufferSurface->LockRect(&lockedRect, nullptr,
                        D3DLOCK_NO_DIRTY_UPDATE);
        RETURNIFFAILED(hr);

        BYTE* surfaceBits = reinterpret_cast<BYTE*> (lockedRect.pBits);

        const UINT32 rgbRawHeight = height;

        for (UINT32 rowIdx = 0; rowIdx < rgbRawHeight; rowIdx++)
        {
            memcpy(surfaceBits, bufferData, width * 4);

            surfaceBits += lockedRect.Pitch;
            bufferData -= stride;
        }

        hr = bufferSurface->UnlockRect();

        hr = mediaBuffer->Unlock();
        RETURNIFFAILED(hr);

        *surface = bufferSurface.Detach();
    }

    return S_OK;
}

HRESULT mediaBuffer2SystemBuffer(IMFMediaType *mediaType,
                IMFMediaBuffer *mediaBuffer, BYTE *systemBuffer)
{
    if (nullptr == mediaBuffer || nullptr == mediaType)
    {
        return E_INVALIDARG;
    }

    if (nullptr == systemBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr;

    GUID mediaSybtype;
    hr = mediaType->GetGUID(MF_MT_SUBTYPE, &mediaSybtype);
    RETURNIFFAILED(hr);

    if ((!IsEqualGUID(mediaSybtype, MFVideoFormat_NV12)) && (!IsEqualGUID(
                    mediaSybtype, MFVideoFormat_RGB32)))
    {
        return E_INVALIDARG;
    }

    CComPtr < IDirect3DSurface9 > surface;
    hr = MFGetService(mediaBuffer, MR_BUFFER_SERVICE, IID_PPV_ARGS(&surface));
    RETURNIFFAILED(hr);

    UINT32 width;
    UINT32 height;
    hr = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
    RETURNIFFAILED(hr);

    INT32 stride;
    stride = MFGetAttributeUINT32(mediaType, MF_MT_DEFAULT_STRIDE, width);

    GUID subtype;
    mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (IsEqualGUID(subtype, MFVideoFormat_NV12))
    {
        const UINT32 nv12RawHeight = height * 3 / 2;

        DWORD bufferMaxSize = width * nv12RawHeight;

        D3DLOCKED_RECT lockedRect;
        hr = surface->LockRect(&lockedRect, nullptr, D3DLOCK_NO_DIRTY_UPDATE);
        RETURNIFFAILED(hr);

        BYTE* surfaceBits = reinterpret_cast<BYTE*> (lockedRect.pBits);

        UINT32 offset = 0;

        for (UINT32 rowIdx = 0; rowIdx < nv12RawHeight; rowIdx++)
        {
            memcpy(systemBuffer + offset, surfaceBits, width);

            surfaceBits += lockedRect.Pitch;
            offset += stride;
        }

        hr = surface->UnlockRect();
        RETURNIFFAILED(hr);
    }

    if (IsEqualGUID(subtype, MFVideoFormat_RGB32))
    {
        const UINT32 rgbRawHeight = height;

        DWORD bufferMaxSize = width * rgbRawHeight;

        D3DLOCKED_RECT lockedRect;
        hr = surface->LockRect(&lockedRect, nullptr, D3DLOCK_NO_DIRTY_UPDATE);
        RETURNIFFAILED(hr);

        BYTE* surfaceBits = reinterpret_cast<BYTE*> (lockedRect.pBits);

        UINT32 offset = 0;

        for (UINT32 rowIdx = 0; rowIdx < rgbRawHeight; rowIdx++)
        {
            memcpy(systemBuffer + offset, surfaceBits, width * 4);

            surfaceBits += lockedRect.Pitch;
            offset -= stride;
        }

        hr = surface->UnlockRect();
        RETURNIFFAILED(hr);
    }

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
}

/** 
 *******************************************************************************
 *  @fn     ColorConvert
 *  @brief  Constructor
 *******************************************************************************
 */
ColorConvert::ColorConvert()
{

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
HRESULT ColorConvert::init(IDirect3DDevice9Ex* device)
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
                    CL_ALL_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR, numAdapters,
                    cdDevices, &num_devices);

    OPENCL_RETURNIFFAILED(errNum);

    std::vector < cl::Device > devices;
    devices.push_back(cl::Device(cdDevices[0]));

    cl_context_properties contextProperties[] =
                    { CL_CONTEXT_ADAPTER_D3D9_KHR, (cl_context_properties)(
                                    (void*) device), CL_CONTEXT_PLATFORM,
                      (cl_context_properties) platform(),
                      CL_CONTEXT_INTEROP_USER_SYNC, CL_TRUE, 0 };

    mClContext = cl::Context(devices, contextProperties, nullptr, nullptr,
                    &errNum);
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

    delete mediaAdapterTypes;
    delete cdDevices;

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

    GUID inputSubType;
    hr = inputMediaType->GetGUID(MF_MT_SUBTYPE, &inputSubType);
    RETURNIFFAILED(hr);

    GUID outputSubType;
    hr = outputMediaType->GetGUID(MF_MT_SUBTYPE, &outputSubType);
    RETURNIFFAILED(hr);

    cl_int errNum;

    if (useInterop)
    {
        CComPtr < IDirect3DSurface9 > inputSurface;
        hr = mediaBuffer2Surface(mDevice, inputMediaType, inputMediaBuffer,
                        &inputSurface);
        RETURNIFFAILED(hr);

        CComPtr < IDirect3DSurface9 > outputSurface;
        if (IsEqualGUID(MFVideoFormat_RGB32, outputSubType))
        {
            // Output is RGB 
            hr = device->CreateOffscreenPlainSurface(outputWidth, outputHeight,
                            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &outputSurface,
                            nullptr);
            RETURNIFFAILED(hr);

            hr = MFCreateDXSurfaceBuffer(IID_IDirect3DSurface9, outputSurface,
                            TRUE, &outputMediaBuffer);
            RETURNIFFAILED(hr);

            UINT32 widthOut;
            UINT32 heightOut;
            cl::Image2D clImageOut;
            hr = surface2OpenCLImages(mClContext, outputSurface,
                            CL_MEM_WRITE_ONLY, clImageOut, &widthOut,
                            &heightOut);
            RETURNIFFAILED(hr);

            // Input is NV12. Hence 2 planes.  One for Luma and the other for Chroma.
            UINT32 inputLuminanceWidth;
            UINT32 inputLuminanceHeight;
            cl::Image2D inputLuminanceImage;
            cl::Image2D inputChrominanceImage;
            hr = NV12Surface2OpenCLImages(mClContext, inputSurface,
                            CL_MEM_READ_ONLY, inputLuminanceImage,
                            inputChrominanceImage, &inputLuminanceWidth,
                            &inputLuminanceHeight);
            RETURNIFFAILED(hr);

            errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                            &inputLuminanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                            &inputChrominanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                            &clImageOut(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);

            /***************************************************************************
             * Do the color conversion                                                  *
             ***************************************************************************/
            hr = nv12torgba(inputLuminanceImage, inputChrominanceImage,
                            inputLuminanceWidth, inputLuminanceHeight,
                            clImageOut);
            RETURNIFFAILED(hr);

            errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                            &inputLuminanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                            &inputChrominanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                            &clImageOut(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
        }
        else
        {
            // Output is NV12
            hr = device->CreateOffscreenPlainSurface(outputWidth, outputHeight,
                            D3DFORMAT_NV12, D3DPOOL_DEFAULT, &outputSurface,
                            nullptr);
            RETURNIFFAILED(hr);

            hr = MFCreateDXSurfaceBuffer(IID_IDirect3DSurface9, outputSurface,
                            FALSE, &outputMediaBuffer);
            RETURNIFFAILED(hr);

            UINT32 outputLuminanceWidth;
            UINT32 outputLuminanceHeight;
            cl::Image2D outputLuminanceImage;
            cl::Image2D outputChrominanceImage;
            hr = NV12Surface2OpenCLImages(mClContext, outputSurface,
                            CL_MEM_WRITE_ONLY, outputLuminanceImage,
                            outputChrominanceImage, &outputLuminanceWidth,
                            &outputLuminanceHeight);
            RETURNIFFAILED(hr);

            // Input is RGB
            UINT32 widthIn;
            UINT32 heightIn;
            cl::Image2D clImageIn;
            hr = surface2OpenCLImages(mClContext, inputSurface,
                            CL_MEM_READ_ONLY, clImageIn, &widthIn, &heightIn);
            RETURNIFFAILED(hr);

            errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                            &clImageIn(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                            &outputLuminanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueAcquireDX9MediaSurfacesKHR(mClQueue(), 1,
                            &outputChrominanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);

            /***************************************************************************
             * Do the color conversion                                                  *
             ***************************************************************************/
            hr = rgbatonv12(clImageIn, widthIn, heightIn, outputLuminanceImage,
                            outputChrominanceImage);
            RETURNIFFAILED(hr);

            errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                            &clImageIn(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                            &outputLuminanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
            errNum = clEnqueueReleaseDX9MediaSurfacesKHR(mClQueue(), 1,
                            &outputChrominanceImage(), 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
        }

        errNum = mClQueue.finish();
        OPENCL_RETURNIFFAILED(errNum);

        inputSurface.Release();
        outputSurface.Release();
    }
    else
    {
        std::vector < BYTE > systemBuffer;

        BYTE* source;
        DWORD maxSourceSize;
        DWORD currentSourceSize;

        bool unlockRequired = false;
        hr
                        = inputMediaBuffer->Lock(&source, &maxSourceSize,
                                        &currentSourceSize);
        if (E_NOTIMPL == hr)
        {
            if (IsEqualGUID(MFVideoFormat_RGB32, inputSubType)) // Input is RGB 
            {
                systemBuffer.reserve(inputWidth * inputHeight * 4);
            }
            else
            {
                systemBuffer.reserve(inputWidth * inputHeight * 3 / 2);
            }
            source = systemBuffer.data();

            hr = mediaBuffer2SystemBuffer(inputMediaType, inputMediaBuffer,
                            source);
        }
        else
        {
            unlockRequired = true;
        }
        RETURNIFFAILED(hr);

        UINT32 widthIn = inputWidth;
        UINT32 heightIn = inputHeight;
        UINT32 widthOut = outputWidth;
        UINT32 heightOut = outputHeight;

        cl_image_desc desc;
        memset(&desc, '\0', sizeof(cl_image_desc));

        cl_image_desc descOut;
        memset(&descOut, '\0', sizeof(cl_image_desc));

        cl::Image2D clImageOut;
        cl::Image2D outputImageLuma;
        cl::Image2D outputImageChroma;

        if (IsEqualGUID(MFVideoFormat_RGB32, inputSubType))
        {
            //Input is RGB
            desc.image_type = CL_MEM_OBJECT_IMAGE2D;
            desc.image_width = widthIn;
            desc.image_height = heightIn;

            cl::Image2D clImageIn;
            clImageIn = clCreateImage(mClContext(), CL_MEM_READ_ONLY
                            | CL_MEM_COPY_HOST_PTR, &RawRGBFormat, &desc,
                            source, &errNum);
            OPENCL_RETURNIFFAILED(errNum);

            //Output is NV12
            descOut.image_type = CL_MEM_OBJECT_IMAGE2D;
            descOut.image_width = widthOut;
            descOut.image_height = heightOut;

            outputImageLuma = clCreateImage(mClContext(), CL_MEM_WRITE_ONLY
                            | CL_MEM_HOST_READ_ONLY, &lumaPlaneFormat,
                            &descOut, nullptr, &errNum);
            OPENCL_RETURNIFFAILED(errNum);

            descOut.image_width = widthOut / 2;
            descOut.image_height = heightOut / 2;
            outputImageChroma = clCreateImage(mClContext(), CL_MEM_WRITE_ONLY
                            | CL_MEM_HOST_READ_ONLY, &chromaPlaneFormat,
                            &descOut, nullptr, &errNum);
            OPENCL_RETURNIFFAILED(errNum);

            /***************************************************************************
             * Do the color conversion                                                  *
             ***************************************************************************/
            hr = rgbatonv12(clImageIn, widthIn, heightIn, outputImageLuma,
                            outputImageChroma);
            RETURNIFFAILED(hr);
        }
        else
        {
            //Input is NV12
            desc.image_type = CL_MEM_OBJECT_IMAGE2D;
            desc.image_width = widthIn;
            desc.image_height = heightIn;

            BYTE* luminance = source;

            cl::Image2D inputImageLuma;
            cl::Image2D inputImageChroma;

            inputImageLuma = clCreateImage(mClContext(), CL_MEM_READ_ONLY
                            | CL_MEM_COPY_HOST_PTR, &lumaPlaneFormat, &desc,
                            luminance, &errNum);
            OPENCL_RETURNIFFAILED(errNum);

            desc.image_width = widthIn / 2;
            desc.image_height = heightIn / 2;

            inputImageChroma = clCreateImage(mClContext(), CL_MEM_READ_ONLY
                            | CL_MEM_COPY_HOST_PTR, &chromaPlaneFormat, &desc,
                            (luminance + (widthIn * heightIn)), &errNum);
            OPENCL_RETURNIFFAILED(errNum);

            //Output is RGB
            descOut.image_type = CL_MEM_OBJECT_IMAGE2D;
            descOut.image_width = widthOut;
            descOut.image_height = heightOut;

            clImageOut = clCreateImage(mClContext(), CL_MEM_WRITE_ONLY
                            | CL_MEM_HOST_READ_ONLY, &RawRGBFormat, &descOut,
                            nullptr, &errNum);
            OPENCL_RETURNIFFAILED(errNum);

            /***************************************************************************
             * Do the color conversion                                                  *
             ***************************************************************************/
            hr = nv12torgba(inputImageLuma, inputImageChroma, widthIn,
                            heightIn, clImageOut);
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

        if (IsEqualGUID(MFVideoFormat_RGB32, outputSubType))
        {
            // RGB Output
            size_t origin[] = { 0, 0, 0 };
            size_t region[] = { widthOut, heightOut, 1 };
            errNum = clEnqueueReadImage(mClQueue(), clImageOut(), CL_TRUE,
                            origin, region, 0, 0, destination, 0, nullptr,
                            nullptr);
            OPENCL_RETURNIFFAILED(errNum);
        }
        else
        {
            // NV12 Output
            BYTE* luminanceDestinationPlane = destination;
            BYTE* chrominanceDestinationPlane = destination + (outputWidth
                            * outputHeight);
            size_t origin[] = { 0, 0, 0 };
            size_t luminanceRegion[] = { outputWidth, outputHeight, 1 };
            errNum = clEnqueueReadImage(mClQueue(), outputImageLuma(), CL_TRUE,
                            origin, luminanceRegion, 0, 0,
                            luminanceDestinationPlane, 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);

            size_t chrominanceRegion[] =
                            { outputWidth / 2, outputHeight / 2, 1 };
            errNum = clEnqueueReadImage(mClQueue(), outputImageChroma(),
                            CL_TRUE, origin, chrominanceRegion, 0, 0,
                            chrominanceDestinationPlane, 0, nullptr, nullptr);
            OPENCL_RETURNIFFAILED(errNum);
        }

        errNum = mClQueue.finish();
        OPENCL_RETURNIFFAILED(errNum);

        if (unlockRequired)
        {
            hr = inputMediaBuffer->Unlock();
            RETURNIFFAILED(hr);
        }

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
 *  @param[in] inputLumaImage   : OpenCL 2D input image
 *  @param[in] inputChromaImage : OpenCL 2D input image
 *  @param[in] inputWidth       : Input image's width
 *  @param[in] inputHeight      : Input images's height
 *  @param[out] outputImage     : OpenCL 2D output image
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */

HRESULT ColorConvert::nv12torgba(cl::Image2D& inputLumaImage,
                cl::Image2D& inputChromaImage, UINT32 inputWidth,
                UINT32 inputHeight, cl::Image2D& outputRGBImage)
{
    cl_int errNum;

    errNum = mClNV12TORGBAKernel.setArg<cl_mem> (0, inputLumaImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_mem> (1, inputChromaImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_int> (2, inputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_int> (3, inputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClNV12TORGBAKernel.setArg<cl_mem> (4, outputRGBImage());
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
 *  @param[in] inputImage			: OpenCL 2D input image
 *  @param[in] inputWidth			: Input image's width
 *  @param[in] inputHeight			: Input images's height
 *  @param[out] outputLumaImage		: OpenCL 2D output image
 *  @param[out] outputChromaImage	: OpenCL 2D output image
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT ColorConvert::rgbatonv12(cl::Image2D& inputRGBImage, UINT32 inputWidth,
                UINT32 inputHeight, cl::Image2D& outputLumaImage,
                cl::Image2D& outputChromaImage)
{
    cl_int errNum;

    errNum = mClRGBATONV12Kernel.setArg<cl_mem> (0, inputRGBImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_int> (1, inputWidth);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_int> (2, inputHeight);
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_mem> (3, outputLumaImage());
    OPENCL_RETURNIFFAILED(errNum);

    errNum = mClRGBATONV12Kernel.setArg<cl_mem> (4, outputChromaImage());
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
    return S_OK;
}

