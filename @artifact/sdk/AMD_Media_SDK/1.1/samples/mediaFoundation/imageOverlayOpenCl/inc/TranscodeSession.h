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
 * @file <TranscodeSession.h>
 *
 * @brief Defines class for transcode session
 *
 ********************************************************************************
 */
#ifndef TRANSCODESESSION_H_
#define TRANSCODESESSION_H_

#include <mfapi.h>
#include <codecapi.h>
#include <atlbase.h>
#include <mfidl.h>
#include "ImageOverlayConfig.h"
#include "MftUtils.h"
#include "MftDx11Transform.h"
#include "MftDx9Transform.h"
/**
 *   @class CTranscodeSession. Builds and manages transcode topology with DX11 acceleration.
 */
class CTranscodeSession: public IMFAsyncCallback
{
public:

    CTranscodeSession(void);
    ~CTranscodeSession(void);

    /**
     *   @brief instantiateTopology(). Creates required components of transcode topology.
     */
    HRESULT instantiateTopology();

    HRESULT buildAndLoadTopology();

    /**
     *   @brief Run(). Runs transcoding.
     */
    HRESULT Run();

    /**
     *   @brief IMFAsyncCallback::GetParameters().
     */
    virtual HRESULT STDMETHODCALLTYPE GetParameters(DWORD *pdwFlags, DWORD *pdwQueue);

    /**
     *   @brief IIMFAsyncCallback::Invoke().
     */
    virtual HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult *pAsyncResult);

    /**
     *   @brief IUnknown::QueryInterface().
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

    /**
     *   @brief IUnknown::AddRef().
     */
    virtual ULONG STDMETHODCALLTYPE AddRef();

    /**
     *   @brief IUnknown::Release().
     */
    virtual ULONG STDMETHODCALLTYPE Release();

    HRESULT setParameters(int8* inputFileName, int8* outputFileName,
                    ConfigCtrl *pConfigCtrl);

    /**
     *   @fn instantiateVideoStream.
     *   @brief Builds video stream from source to sink.
     */
    HRESULT instantiateVideoStream(DWORD streamNumber,
                    IMFMediaType* sourceMediaType,
                    IMFMediaType* partialEncodedType, bool useDx11);
    /**
     *   @fn createH264VideoType.
     *   @brief Creats H264 media type structure.
     */
    HRESULT createH264VideoType(IMFMediaType** encodedVideoType,
                    IMFMediaType* sourceVideoType, uint32 width, uint32 height);

    HRESULT createOverlayTransform(IMFTransform** transform,
                    ULONG_PTR deviceManagerPtr, bool useDx11);
    HRESULT createOverlayTransformNode(IMFMediaType* targetVideoType,
                    IMFTopologyNode **transformNode,
                    ULONG_PTR deviceManagerPtr, IMFTransform** transform,
                    bool useDx11);

    HRESULT createColorConvertTransform(IMFTransform** transform,
                    ULONG_PTR deviceManagerPtr, bool useDx11);
    HRESULT createCCNode(IMFMediaType* sourceMediaType,
                    IMFTopologyNode** ccNode, ULONG_PTR deviceManagerPtr,
                    const GUID& inputVideoSubType,
                    const GUID& outputVideoSubType, bool useDx11);

    /**
     *   @fn setLogFile.
     *   @brief Sets the log file for dumping logs in verbose mode.
     */
    void setLogFile(FILE *logFile);

private:
    ULONG mRefCount;
    HANDLE mTranscodeEndEvent;
    CComPtr<IMFMediaSession> mMediaSession;
    CComPtr<IMFTopology> mResolvedTopology;
    ConfigCtrl mConfigCtrl;
    WCHAR mInputFileName[MAX_PATH]; /* Place holder for input file name */
    WCHAR mOutputFileName[MAX_PATH]; /*Place holder for output file name */
    FILE *mLogFile; /*Log file neme to dump logs in verbose mode*/
    CComPtr<IMFTopology> mPartialTopology;

    CComPtr<IMFMediaSource> mMediaSource;
    CComPtr<IMFPresentationDescriptor> mSourcePresentationDescriptor;
    CComPtr<IMFActivate> mMediaSinkActivate;
    CComPtr<IMFStreamDescriptor> mSourceStreamDescriptor;
    CComPtr<IMFDXGIDeviceManager> mDxgiDeviceManager;

    CComPtr<IMFTopologyNode> mSourceNode; /* Source node*/
    CComPtr<IMFTopologyNode> mDecoderNode; /* Decoder node*/
    CComPtr<IMFTopologyNode> mScalarNode; /* Scalar node*/
    CComPtr<IMFTopologyNode> mEncoderNode; /* Encoder node*/
    CComPtr<IMFTopologyNode> mSinkNode; /* sink node*/
    CComPtr<IMFTopologyNode> mccNodeNV12ToRGB; /* NV12 to RGB node*/
    CComPtr<IMFTopologyNode> mccNodeRGBToNV12; /* RGB to NV12 node*/
    msdk_CMftBuilder* mMftBuilderObjPtr; /* Pointer for MFT Builder utility class*/
};

#endif
