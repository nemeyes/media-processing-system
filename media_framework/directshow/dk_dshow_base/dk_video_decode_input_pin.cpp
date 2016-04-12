#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>

#include "dk_video_decode_input_pin.h"

dk_video_decode_input_pin::dk_video_decode_input_pin(TCHAR * objname, CTransformFilter * filter, HRESULT * hr, LPWSTR name)
	: dk_transform_input_pin(objname, filter, hr, name)
{
}

// IKsPropertySet
STDMETHODIMP dk_video_decode_input_pin::Set(REFGUID PropSet, ULONG Id, LPVOID pInstanceData, ULONG InstanceLength, LPVOID pPropertyData, ULONG DataLength)
{
	if (PropSet != AM_KSPROPSETID_TSRateChange) 
	{
		return __super::Set(PropSet, Id, pInstanceData, InstanceLength, pPropertyData, DataLength);
	}

	switch (Id) {
	case AM_RATE_SimpleRateChange: {
		AM_SimpleRateChange* p = (AM_SimpleRateChange*)pPropertyData;
		if (!m_CorrectTS) {
			return E_PROP_ID_UNSUPPORTED;
		}
		CAutoLock cAutoLock(&m_csRateLock);
		m_ratechange = *p;
	}
								   break;
	case AM_RATE_UseRateVersion: {
		WORD* p = (WORD*)pPropertyData;
		if (*p > 0x0101) {
			return E_PROP_ID_UNSUPPORTED;
		}
	}
								 break;
	case AM_RATE_CorrectTS: {
		LONG* p = (LONG*)pPropertyData;
		m_CorrectTS = *p;
	}
							break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

STDMETHODIMP dk_video_decode_input_pin::Get(REFGUID PropSet, ULONG Id, LPVOID pInstanceData, ULONG InstanceLength, LPVOID pPropertyData, ULONG DataLength, ULONG* pBytesReturned)
{
	if (PropSet != AM_KSPROPSETID_TSRateChange) 
	{
		return __super::Get(PropSet, Id, pInstanceData, InstanceLength, pPropertyData, DataLength, pBytesReturned);
	}

	switch (Id) {
	case AM_RATE_SimpleRateChange: {
		AM_SimpleRateChange* p = (AM_SimpleRateChange*)pPropertyData;
		CAutoLock cAutoLock(&m_csRateLock);
		*p = m_ratechange;
		*pBytesReturned = sizeof(AM_SimpleRateChange);
	}
								   break;
	case AM_RATE_MaxFullDataRate: {
		AM_MaxFullDataRate* p = (AM_MaxFullDataRate*)pPropertyData;
		*p = 2 * 10000;
		*pBytesReturned = sizeof(AM_MaxFullDataRate);
	}
								  break;
	case AM_RATE_QueryFullFrameRate: {
		AM_QueryRate* p = (AM_QueryRate*)pPropertyData;
		p->lMaxForwardFullFrame = 2 * 10000;
		p->lMaxReverseFullFrame = 0;
		*pBytesReturned = sizeof(AM_QueryRate);
	}
									 break;
	case AM_RATE_QueryLastRateSegPTS: {
		//REFERENCE_TIME* p = (REFERENCE_TIME*)pPropertyData;
		return E_PROP_ID_UNSUPPORTED;
	}
									  break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

STDMETHODIMP dk_video_decode_input_pin::QuerySupported(REFGUID PropSet, ULONG Id, ULONG* pTypeSupport)
{
	if (PropSet != AM_KSPROPSETID_TSRateChange) {
		return __super::QuerySupported(PropSet, Id, pTypeSupport);
	}

	switch (Id) {
	case AM_RATE_SimpleRateChange:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
		break;
	case AM_RATE_MaxFullDataRate:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;
		break;
	case AM_RATE_UseRateVersion:
		*pTypeSupport = KSPROPERTY_SUPPORT_SET;
		break;
	case AM_RATE_QueryFullFrameRate:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;
		break;
	case AM_RATE_QueryLastRateSegPTS:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;
		break;
	case AM_RATE_CorrectTS:
		*pTypeSupport = KSPROPERTY_SUPPORT_SET;
		break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}
