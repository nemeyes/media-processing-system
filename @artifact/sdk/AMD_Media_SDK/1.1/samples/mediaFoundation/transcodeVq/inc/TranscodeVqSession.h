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
 * @file <TranscodeVqSession.h>                          
 *                                       
 * @brief Defines class for transcode sessoin 
 *         
 ********************************************************************************
 */
#ifndef TRANSCODEVQSESSION_H_
#define TRANSCODEVQSESSION_H_

#include <mfapi.h>
#include <Ks.h>
#include <codecapi.h>
#include <atlbase.h>
#include <mfidl.h>
#include "TranscodeVqConfig.h"
#include "MftUtils.h"
#include "Typedef.h"
#include "VqMft.h"

class CTranscodeVqSession: public IMFAsyncCallback
{
public:

    CTranscodeVqSession(void);
    ~CTranscodeVqSession(void);

    /**
     *   @brief instantiateTopology(). Creates required components of transcode topology.
     */
    HRESULT createH264VideoType(IMFMediaType** encodedVideoType,
                    IMFMediaType* sourceVideoType);
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

    void setLogFile(FILE *logFile);

    /**
     *   @fn BuildVideoStream.
     *   @brief Builds video stream from source to sink.
     */
    HRESULT instantiateVideoStream(DWORD streamNumber,
                    IMFMediaType* sourceMediaType,
                    IMFMediaType* partialEncodedType, bool useDx11);

private:
    FILE *mLogFile;
    ConfigCtrl mPConfigCtrl;
    WCHAR mInputFileName[MAX_PATH]; /**< place holder for input file name */
    WCHAR mOutputFileName[MAX_PATH]; /**< place holder for output file name */
    HANDLE mTranscodeEndEvent;
    CComPtr<IMFMediaSession> mMediaSession;
    ULONG mRefCount;

    msdk_CMftBuilder *mMftBuilderObjPtr; /* Pointer for MFT Builder utility class*/
    CComPtr<IMFTopology> mPartialTopology;
    CComPtr<IMFMediaSource> mMediaSource;
    CComPtr<IMFPresentationDescriptor> mSourcePresentationDescriptor;
    CComPtr<IMFActivate> mMediaSinkActivate;
    CComPtr<IMFStreamDescriptor> mSourceStreamDescriptor;
    CComPtr<IMFDXGIDeviceManager> mDxgiDeviceManager;
    CComPtr<IMFTopology> mTopology;

    //Nodes
    CComPtr<IMFTopologyNode> mSourceNode; /* Source node*/
    CComPtr<IMFTopologyNode> mDecoderNode; /* Decoder node*/
    CComPtr<IMFTopologyNode> mVqNode; /* Video Quality filter node*/
    CComPtr<IMFTopologyNode> mEncoderNode; /* Encoder node*/
    CComPtr<IMFTopologyNode> mSinkNode; /* sink node*/

    HRESULT setVqAttributes(CComPtr<IMFAttributes> vqAttributes,
                    ULONG_PTR deviceManagerPtr, IMFMediaType* sourceMediaType,
                    AMFCMRequestType requestType);
    HRESULT checkVqParams();
    bool mIsVqSupported;
};

#endif
