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
 *******************************************************************************
 * @file <PlaybackPipeline.h>
 *
 * @brief Header file for the Playback Pipeline
 *
 *******************************************************************************
 */

#pragma once

#include "Context.h"
#include "Component.h"
#include "VideoDecoderUVD.h"
#include "VideoConverter.h"
#include "BitStreamParser.h"
#include "VideoPresenter.h"
#include "ParametersStorage.h"
#include "Pipeline.h"
#include "DeviceDX9.h"
#include "DeviceDX11.h"

class PlaybackPipeline: public Pipeline, public ParametersStorage
{
    class PipelineElementAMFComponent;
public:
    PlaybackPipeline();
    virtual ~PlaybackPipeline();
public:
    static const wchar_t* PARAM_NAME_INPUT;
    static const wchar_t* PARAM_NAME_PRESENTER;

    AMF_RESULT Init(HWND hwnd);
    AMF_RESULT Play();
    AMF_RESULT Pause();
    AMF_RESULT Step();
    AMF_RESULT Stop();

    void Terminate();

    double GetProgressSize();
    double GetProgressPosition();

private:
    AMFDataStreamPtr m_pStream;
    amf::AMFContextPtr m_pContext;

    BitStreamParserPtr m_pParser;
    amf::AMFComponentPtr m_pDecoder;
    amf::AMFComponentPtr m_pConverter;
    VideoPresenterPtr m_pPresenter;

    DeviceDX9 m_deviceDX9;
    DeviceDX11 m_deviceDX11;
};