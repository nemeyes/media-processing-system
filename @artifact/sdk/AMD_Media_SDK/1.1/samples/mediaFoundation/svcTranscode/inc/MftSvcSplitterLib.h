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
 * @file <MftSvcSplitterLib.h>                          
 *                                       
 * @brief This file contains class definition for svc splitter
 *         
 ********************************************************************************
 */
#ifndef MFTSVCSPLITTERLIB_H_
#define MFTSVCSPLITTERLIB_H_

/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/

#include <Windows.h>
#include <Evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <atlbase.h>
#include <assert.h>
#include <new>
#include <shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <strmif.h>
#include <initguid.h>

//#include <amp.h>
//#include <amp_graphics.h>
//#include <amp_math.h>

#include <iostream>
class SvcSplitter
{
public:
    SvcSplitter() :
        m_temporalId(0)
    {
    }

    virtual ~SvcSplitter()
    {
    }

    BYTE getTemporalId()
    {
        return m_temporalId;
    }

    bool setNal(BYTE* buf, size_t len)
    {
        buf += 4;

        bool isAvc = true;
        NaluHeader* pHdr = reinterpret_cast<NaluHeader*> (buf);
        BYTE naluType = pHdr->nal_unit_type;

        switch (naluType)
        {
        case NALU_TYPE_SPS:
            break;
        case NALU_TYPE_PPS:
            break;
        case NALU_TYPE_PREFIX:
        case NALU_TYPE_SLC_EXT:
            decodePrefix(buf, len);
            isAvc = false;
            break;
        case NALU_TYPE_IDR:
            break;
        case NALU_TYPE_SLICE:
            break;
        case NALU_TYPE_SEI:
            break;
        case NALU_TYPE_FILL:
            break;
        case NALU_TYPE_AUD:
            break;
        case NALU_TYPE_EOSEQ:
            break;
        case NALU_TYPE_EOSTREAM:
            break;
        default:
            isAvc = false;
            break;
        }

        return isAvc;
    }

protected:
    enum NALU_TYPE
    {
        NALU_TYPE_SLICE = 1,
        NALU_TYPE_DPA = 2,
        NALU_TYPE_DPB = 3,
        NALU_TYPE_DPC = 4,
        NALU_TYPE_IDR = 5,
        NALU_TYPE_SEI = 6,
        NALU_TYPE_SPS = 7,
        NALU_TYPE_PPS = 8,
        NALU_TYPE_AUD = 9,
        NALU_TYPE_EOSEQ = 10,
        NALU_TYPE_EOSTREAM = 11,
        NALU_TYPE_FILL = 12,
        NALU_TYPE_PREFIX = 14,
        NALU_TYPE_SUB_SPS = 15,
        NALU_TYPE_SLC_EXT = 20,
        NALU_TYPE_VDRD = 24
    };

    union NaluHeader
    {
        struct
        {
            BYTE nal_unit_type :5;
            BYTE nal_ref_idc :2;
            BYTE forbidden_zero :1;
        };
        BYTE value;
    };

    union NaluHeaderExt
    {
        struct
        {
            /*******************************************************************
             * byte #0                                                          *
             *******************************************************************/
            BYTE priority_id :6;
            BYTE idr_flag :1;
            BYTE svc_extension_flag :1;
            /*******************************************************************
             * byte #1                                                          *
             *******************************************************************/
            BYTE quality_id :4;
            BYTE dependency_id :3;
            BYTE no_inter_layer_pred_flag :1;
            /*******************************************************************
             * byte #2                                                          *
             *******************************************************************/
            BYTE reserved_three_2bits :2;
            BYTE output_flag :1;
            BYTE discardable_flag :1;
            BYTE use_ref_base_pic_flag :1;
            BYTE temporal_id :3;
        };
        BYTE values[3];
    };

    /*
     * JVT AF11r2 03/2009
     * clause G.7.3.1.1
     */
    void decodePrefix(BYTE* buf, size_t /*len*/)
    {
        m_temporalId = 0;

        NaluHeaderExt* pHdrExt = reinterpret_cast<NaluHeaderExt*> (buf + 1);
        if (pHdrExt->svc_extension_flag)
        {
            if (pHdrExt->reserved_three_2bits != 0x3)
            {
                printf("Fail: wrong Prefix syntax!\n");
                return;
            }
            m_temporalId = pHdrExt->temporal_id;
        }
    }

private:
    BYTE m_temporalId;
};

#endif//MFTSVCSPLITTERLIB_H_
