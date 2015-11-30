/**********************************************************************
Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/


#ifndef SIMPLEDX9_H_
#define SIMPLEDX9_H_




/**
* Header Files
*/
#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "CL\OpenCL.h"
#include "CL\cl_dx9_media_sharing.h"
#include "CLUtil.hpp"
#include <SDKBitMap.hpp>
#include <d3d9.h>
#include "DXGI.h"

#define SAMPLE_VERSION "AMD-APP-SDK-v3.0.253.2"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

#define WINDOW_CLASS_NAME   L"OpenCL DX9-Interop"
#define CAPTION_NAME        L"OpenCL SimpleDX9"

using namespace appsdk;


clGetDeviceIDsFromDX9MediaAdapterKHR_fn pfn_clGetDeviceIDsFromDX9MediaAdapterKHR
    = NULL;
clCreateFromDX9MediaSurfaceKHR_fn pfn_clCreateFromDX9MediaSurfaceKHR = NULL;
clEnqueueAcquireDX9MediaSurfacesKHR_fn pfn_clEnqueueAcquireDX9MediaSurfacesKHR =
    NULL;
clEnqueueReleaseDX9MediaSurfacesKHR_fn pfn_clEnqueueReleaseDX9MediaSurfacesKHR =
    NULL;

class SimpleDX9
{
        cl_uint g_dwImgWidth;                  /**< Input bitmap width */
        cl_uint g_dwImgHeight;                 /**< Input bitmap height */
        cl_uint framesize;                     /**< the size of the input framedata width */
        cl_uint outFrameSize;                  /**< the size of the output frame */
        char *g_sInName;                       /**< the name of the input image */
        HRESULT hr;


        cl_bool bD3D9Share;                    /**< flag about whether the driver support DX9 and openCL interOp or not */


        SDKBitMap inputBitmap;      /**< Bitmap class object */
        uchar4* pixelData;          /**< the data of the input image */
        cl_uchar4* inputImageData;             /**< Input bitmap data to device */
        cl_uchar4* outputImageData;            /**< data of output image */

        cl_mem clRGBbuf;                       /**< The input buffer of the kernel */
        cl_mem clShareBufIn;                   /**< The image created from the d3d9 offscreen surface */

        unsigned char*
        verifyOutput;                          /**< The CPU result when sampleArgs->verify */

        size_t globalThreads[2];               /**< global threads */
        size_t localThreads[2];                /**< local threads */

        cl_int numFrames;                      /**< The number of reference frames when compute the FPS */

        cl_double setupTime;                   /**< The setup time */
        cl_double kernelTime;                  /**< The kernel time */

        cl_platform_id platform;               /**< CL platform */

        cl_context context;                    /**< CL context */
        cl_device_id device;                   /**< CL device */
        cl_device_id *devices;                 /**< CL device list */
        cl_int deviceCount;
        cl_command_queue commandQueue;         /**< CL commandQueue */
        cl_program program;                    /**< CL program */
        cl_kernel mirrokernel;                 /**< CL kernel */

        SDKDeviceInfo deviceInfo;        /**< Structure to store device information */
        KernelWorkGroupInfo
        kernelInfo;      /**< Structure to store kernel related info */
        SDKTimer *sampleTimer;

    public:
        CLCommandArgs * sampleArgs;
        /**
        * Constructor
        * Initialize member variables
        * @param name name of sample (string)
        */
        SimpleDX9()
            :
            g_dwImgWidth(640),
            g_dwImgHeight(360),
            g_sInName("SimpleDX9_input.bmp"),
            hr(FALSE),
            inputImageData(NULL),
            outputImageData(NULL),
            numFrames(50),
            devices(NULL),
            setupTime(0),
            kernelTime(0)
        {
            sampleArgs =  new CLCommandArgs();
            sampleTimer = new SDKTimer();
            sampleArgs->sampleVerStr = SAMPLE_VERSION;
        }

        ~SimpleDX9()
        {

        }

        /**
        * Allocate and initialize required host memory with appropriate values
        *  @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        // int setupSimpleDX9();
        /**
        * Override from SDKSample, Generate binary image of given kernel
        * and exit application
        *  @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int genBinaryImage();

        /**
        * OpenCL related initialisations.
        * Set up Context, Device list, Command Queue, Memory buffers
        * Build CL kernel program executable
        *  @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int setupCL();

        /**
        * Set values for kernels' arguments, enqueue calls to the kernels
        * on to the command queue, wait till end of kernel execution.
        * Get kernel start and end time if timing is enabled
        *  @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int GPUtrans();

        /**
        * Override from SDKSample. Print sample stats.
        */
        void printStats();

        /**
        * Override from SDKSample. Initialize
        * command line parser, add custom options
        *  @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int initialize();

        /**
        * Override from SDKSample, adjust width and height
        * of execution domain, perform all sample setup
        * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int setup();

        /**
        * Override from SDKSample
        * Run OpenCL DwtHaar1D
        * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int run();

        /**
        * Override from SDKSample
        * Cleanup memory allocations
        *  @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int cleanup();

        /**
        * Override from SDKSample
        * Verify against reference implementation
        * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int verifyResults();

    private:

        /**
        * Create D3D9 device and Create OpenCL interOp devices
        * and Create OpenCL DX9 interOp devices
        * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int SetupD3D9();

        /**
        * Read input bmp image to the buffer
        * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int readInputImage(std::string inputImageName);

        /**
        * roung up
        * @return the value afer yrounding up
        */
        size_t RoundUp(int group_size, int global_size);

        /**
        * Using D3D to render a picture
        * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
        */
        int Direct3DRenderScene();



};

/**
 * clean operation
 * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
 */
void D3D9Cleanup();

/**
 * Create D3D9 Texture
 * and Vetex buffer and set arguments
 * @return returns SDK_SUCCESS on success and SDK_FAILURE otherwise
 */
int CreateScence();
#endif