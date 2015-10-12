/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2008-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include "intel_media_sdk\mfx_config.h"

#include "intel_media_sdk\ms_defs.h"

#include "intel_media_sdk\vpp_ex.h"
#include "intel_media_sdk\vm/atomic_defs.h"

static const mfxU32 MFX_TIME_STAMP_FREQUENCY = 90000;


MFXVideoVPPEx::MFXVideoVPPEx(mfxSession session) :
MFXVideoVPP(session)
#if !defined (USE_VPP_EX)
{};
#else
, m_nCurrentPTS(0)
, m_nIncreaseTime(0)
, m_nInputTimeStamp(0)
, m_nArraySize(0)
{
    memset(&m_VideoParams, 0, sizeof(m_VideoParams));
};

mfxStatus MFXVideoVPPEx::Close(void)
{
    for(std::vector<mfxFrameSurface1*>::iterator it = m_LockedSurfacesList.begin(); it != m_LockedSurfacesList.end(); ++it)
    {
        try {
            msdk_atomic_dec16((volatile mfxU16*)(&(*it)->Data.Locked));
        } catch (...) { // improve robustness by try/catch
        }
    }

    m_LockedSurfacesList.clear();

    return MFXVideoVPP::Close();
};

mfxStatus MFXVideoVPPEx::QueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest request[2])
{
    mfxVideoParam   params;

    if (NULL == par)
    {
        return MFX_ERR_NULL_PTR;
    };

    MSDK_MEMCPY_VAR(params, par, sizeof(mfxVideoParam));

    params.vpp.In.FrameRateExtD = params.vpp.Out.FrameRateExtD;
    params.vpp.In.FrameRateExtN = params.vpp.Out.FrameRateExtN;

    return MFXVideoVPP::QueryIOSurf(&params, request);
};

mfxStatus MFXVideoVPPEx::Query(mfxVideoParam *in, mfxVideoParam *out)
{
    mfxVideoParam   params;

    if (NULL == out)
    {
        return MFX_ERR_NULL_PTR;
    }

    if (in)
    {
        MSDK_MEMCPY_VAR(params, in, sizeof(mfxVideoParam));

        params.vpp.In.FrameRateExtD = params.vpp.Out.FrameRateExtD;
        params.vpp.In.FrameRateExtN = params.vpp.Out.FrameRateExtN;
    }

    return MFXVideoVPP::Query((in) ? &params : NULL, out);
};

mfxStatus MFXVideoVPPEx::Init(mfxVideoParam *par)
{
    mfxStatus sts = MFX_ERR_NONE;

    if (NULL == par)
    {
        return MFX_ERR_NULL_PTR;
    };

    m_nCurrentPTS = 0;
    m_nArraySize = 0;
    m_nInputTimeStamp = 0;

    for(std::vector<mfxFrameSurface1*>::iterator it = m_LockedSurfacesList.begin(); it != m_LockedSurfacesList.end(); ++it)
    {
        msdk_atomic_dec16((volatile mfxU16*)(&(*it)->Data.Locked));
    }

    m_LockedSurfacesList.clear();

    m_nIncreaseTime = (mfxU64)((mfxF64)MFX_TIME_STAMP_FREQUENCY * par->vpp.Out.FrameRateExtD / par->vpp.Out.FrameRateExtN);

    MSDK_MEMCPY_VAR(m_VideoParams, par, sizeof(mfxVideoParam));

    m_VideoParams.vpp.In.FrameRateExtD = m_VideoParams.vpp.Out.FrameRateExtD;
    m_VideoParams.vpp.In.FrameRateExtN = m_VideoParams.vpp.Out.FrameRateExtN;

    sts = MFXVideoVPP::Init(&m_VideoParams);

    m_VideoParams.vpp.In.FrameRateExtD = par->vpp.In.FrameRateExtD;
    m_VideoParams.vpp.In.FrameRateExtN = par->vpp.In.FrameRateExtN;

    return sts;
}

mfxStatus MFXVideoVPPEx::GetVideoParam(mfxVideoParam *par)
{
    mfxStatus sts = MFXVideoVPP::GetVideoParam(par);

    if (MFX_ERR_NONE == sts)
    {
        par->vpp.In.FrameRateExtD = m_VideoParams.vpp.In.FrameRateExtD;
        par->vpp.In.FrameRateExtN = m_VideoParams.vpp.In.FrameRateExtN;

        par->vpp.Out.FrameRateExtD = m_VideoParams.vpp.Out.FrameRateExtD;
        par->vpp.Out.FrameRateExtN = m_VideoParams.vpp.Out.FrameRateExtN;
    };

    return sts;
};

mfxStatus MFXVideoVPPEx::RunFrameVPPAsync(mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp)
{
    mfxStatus sts = MFX_ERR_NONE;

    if (NULL == out || NULL == syncp)
    {
        return MFX_ERR_NULL_PTR;
    };

    if (!in)
    {
        if (!m_LockedSurfacesList.empty())
        {
            // subtract 1 to handle minimal difference between input and expected timestamps
            if (m_nCurrentPTS - 1 <= m_LockedSurfacesList[0]->Data.TimeStamp)
            {
                mfxU64 nPTS = m_LockedSurfacesList[0]->Data.TimeStamp;
                m_LockedSurfacesList[0]->Data.TimeStamp = m_nCurrentPTS;

                sts = MFXVideoVPP::RunFrameVPPAsync(m_LockedSurfacesList[0], out, aux, syncp);

                m_LockedSurfacesList[0]->Data.TimeStamp = nPTS;

                if (MFX_WRN_DEVICE_BUSY != sts)
                {
                    m_nCurrentPTS += m_nIncreaseTime;
                }
            }
            else
            {
                for(std::vector<mfxFrameSurface1*>::iterator it = m_LockedSurfacesList.begin(); it != m_LockedSurfacesList.end(); ++it)
                {
                    msdk_atomic_dec16((volatile mfxU16*)(&(*it)->Data.Locked));
                }

                m_LockedSurfacesList.clear();

                return MFXVideoVPP::RunFrameVPPAsync(in, out, aux, syncp);
            }
        }
        else
        {
            return MFXVideoVPP::RunFrameVPPAsync(in, out, aux, syncp);
        }
    }
    else
    {
        m_nArraySize = m_LockedSurfacesList.size();

        if (!m_nArraySize)
        {
            m_nCurrentPTS = (mfxU64)in->Data.TimeStamp;

            msdk_atomic_inc16((volatile mfxU16*)&in->Data.Locked);
            m_LockedSurfacesList.push_back(in);

            return MFX_ERR_MORE_DATA;
        }

        if (1 == m_nArraySize)
        {
            if (in->Data.TimeStamp < m_nCurrentPTS)
            {
                return MFX_ERR_MORE_DATA;
            }

            if (in->Data.TimeStamp > m_LockedSurfacesList[0]->Data.TimeStamp && (in->Data.TimeStamp < m_nCurrentPTS + m_nIncreaseTime/2))
            {
                return MFX_ERR_MORE_DATA;
            }
        }

        {
            mfxStatus stsRunFrame = MFX_ERR_NONE;

            m_nInputTimeStamp = m_LockedSurfacesList[0]->Data.TimeStamp;

            if (m_nCurrentPTS <= m_LockedSurfacesList[0]->Data.TimeStamp ||
                m_nCurrentPTS < (in->Data.TimeStamp - (mfxF64)m_nIncreaseTime/2))
            {
                m_nInputTimeStamp = m_LockedSurfacesList[0]->Data.TimeStamp;
                m_LockedSurfacesList[0]->Data.TimeStamp = m_nCurrentPTS;

                stsRunFrame = sts = MFXVideoVPP::RunFrameVPPAsync(m_LockedSurfacesList[0], out, aux, syncp);

                m_LockedSurfacesList[0]->Data.TimeStamp = m_nInputTimeStamp;

                if (MFX_WRN_DEVICE_BUSY != stsRunFrame)
                {
                    m_nCurrentPTS += m_nIncreaseTime;
                }

                if (MFX_ERR_NONE == stsRunFrame)
                {
                    sts = MFX_ERR_MORE_SURFACE;
                }
            }

            if (MFX_WRN_DEVICE_BUSY != stsRunFrame)
            {
                if (1 == m_nArraySize)
                {
                    msdk_atomic_inc16((volatile mfxU16*)&in->Data.Locked);
                    m_LockedSurfacesList.push_back(in);
                }

                if (m_nCurrentPTS > m_LockedSurfacesList[0]->Data.TimeStamp &&
                    m_nCurrentPTS >= (m_LockedSurfacesList[1]->Data.TimeStamp - (mfxF64)m_nIncreaseTime/2))
                {
                    msdk_atomic_dec16((volatile mfxU16*)&m_LockedSurfacesList[0]->Data.Locked);
                    m_LockedSurfacesList.erase(m_LockedSurfacesList.begin());

                    if (MFX_ERR_NONE == stsRunFrame)
                    {
                        if (stsRunFrame != sts)
                        {
                            sts = MFX_ERR_NONE;
                        }
                        else
                        {
                            sts = MFX_ERR_MORE_DATA;
                        }
                    }
                }
            }
        }
    }

    return sts;
};

#endif
