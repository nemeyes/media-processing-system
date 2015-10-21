#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>
#include <streams.h>

#include "dk_transform_input_pin.h"

dk_transform_input_pin::dk_transform_input_pin(TCHAR * objname, CTransformFilter * filter, HRESULT * hr, LPWSTR name)
	: CTransformInputPin(objname, filter, hr, name)
{

}

STDMETHODIMP dk_transform_input_pin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{

}

// IMemInputPin
STDMETHODIMP dk_transform_input_pin::Receive(IMediaSample * sample)
{

}

HRESULT dk_transform_input_pin::SetMediaType(const CMediaType * mt)
{

}

// IKsPropertySet
STDMETHODIMP dk_transform_input_pin::Set(REFGUID prop_set, ULONG id, LPVOID instance_data, ULONG instance_length, LPVOID property_data, ULONG data_length)
{
	if (prop_set != AM_KSPROPSETID_CopyProt) 
	{
		return E_NOTIMPL;
	}

	switch (id) 
	{
	case AM_PROPERTY_COPY_MACROVISION:
		break;
	case AM_PROPERTY_DVDCOPY_CHLG_KEY:
	{ // 3. auth: receive drive nonce word, also store and encrypt the buskey made up of the two nonce words
		AM_DVDCOPY_CHLGKEY* pChlgKey = (AM_DVDCOPY_CHLGKEY*)property_data;
		for (int i = 0; i < 10; i++) {
			m_Challenge[i] = pChlgKey->ChlgKey[9 - i];
		}

		CSSkey2(m_varient, m_Challenge, &m_Key[5]);

		CSSbuskey(m_varient, m_Key, m_KeyCheck);
	}
	break;
	case AM_PROPERTY_DVDCOPY_DISC_KEY:
	{ // 5. receive the disckey
		AM_DVDCOPY_DISCKEY* pDiscKey = (AM_DVDCOPY_DISCKEY*)property_data; // pDiscKey->DiscKey holds the disckey encrypted with itself and the 408 disckeys encrypted with the playerkeys

		bool fSuccess = false;

		for (int j = 0; j < g_nPlayerKeys; j++) {
			for (int k = 1; k < 409; k++) {
				BYTE DiscKey[6];
				for (int i = 0; i < 5; i++) {
					DiscKey[i] = pDiscKey->DiscKey[k * 5 + i] ^ m_KeyCheck[4 - i];
				}
				DiscKey[5] = 0;

				CSSdisckey(DiscKey, g_PlayerKeys[j]);

				BYTE Hash[6];
				for (int i = 0; i < 5; i++) {
					Hash[i] = pDiscKey->DiscKey[i] ^ m_KeyCheck[4 - i];
				}
				Hash[5] = 0;

				CSSdisckey(Hash, DiscKey);

				if (!memcmp(Hash, DiscKey, 6)) {
					memcpy(m_DiscKey, DiscKey, 6);
					j = g_nPlayerKeys;
					fSuccess = true;
					break;
				}
			}
		}

		if (!fSuccess) {
			return E_FAIL;
		}
	}
	break;
	case AM_PROPERTY_DVDCOPY_DVD_KEY1:
	{ // 2. auth: receive our drive-encrypted nonce word and decrypt it for verification
		AM_DVDCOPY_BUSKEY* pKey1 = (AM_DVDCOPY_BUSKEY*)property_data;
		for (int i = 0; i < 5; i++) {
			m_Key[i] = pKey1->BusKey[4 - i];
		}

		m_varient = -1;

		for (int i = 31; i >= 0; i--) {
			CSSkey1(i, m_Challenge, m_KeyCheck);

			if (memcmp(m_KeyCheck, &m_Key[0], 5) == 0) {
				m_varient = i;
			}
		}
	}
	break;
	case AM_PROPERTY_DVDCOPY_REGION:
		break;
	case AM_PROPERTY_DVDCOPY_SET_COPY_STATE:
		break;
	case AM_PROPERTY_DVDCOPY_TITLE_KEY:
	{ // 6. receive the title key and decrypt it with the disc key
		AM_DVDCOPY_TITLEKEY* pTitleKey = (AM_DVDCOPY_TITLEKEY*)property_data;
		for (int i = 0; i < 5; i++) {
			m_TitleKey[i] = pTitleKey->TitleKey[i] ^ m_KeyCheck[4 - i];
		}
		m_TitleKey[5] = 0;
		CSStitlekey(m_TitleKey, m_DiscKey);
	}
	break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

STDMETHODIMP dk_transform_input_pin::Get(REFGUID prop_set, ULONG id, LPVOID instance_data, ULONG instance_length, LPVOID property_data, ULONG data_length, ULONG * bytes_raeturned)
{
	if (prop_set != AM_KSPROPSETID_CopyProt) 
	{
		return E_NOTIMPL;
	}

	switch (id) 
	{
	case AM_PROPERTY_DVDCOPY_CHLG_KEY:
	{ // 1. auth: send our nonce word
		AM_DVDCOPY_CHLGKEY* pChlgKey = (AM_DVDCOPY_CHLGKEY*)property_data;
		for (int i = 0; i < 10; i++) {
			pChlgKey->ChlgKey[i] = 9 - (m_Challenge[i] = i);
		}
		*bytes_raeturned = sizeof(AM_DVDCOPY_CHLGKEY);
	}
	break;
	case AM_PROPERTY_DVDCOPY_DEC_KEY2:
	{ // 4. auth: send back the encrypted drive nonce word to finish the authentication
		AM_DVDCOPY_BUSKEY* pKey2 = (AM_DVDCOPY_BUSKEY*)property_data;
		for (int i = 0; i < 5; i++) {
			pKey2->BusKey[4 - i] = m_Key[5 + i];
		}
		*bytes_raeturned = sizeof(AM_DVDCOPY_BUSKEY);
	}
	break;
	case AM_PROPERTY_DVDCOPY_REGION:
	{
		DVD_REGION* pRegion = (DVD_REGION*)property_data;
		pRegion->RegionData = 0;
		pRegion->SystemRegion = 0;
		*bytes_raeturned = sizeof(DVD_REGION);
	}
	break;
	case AM_PROPERTY_DVDCOPY_SET_COPY_STATE:
	{
		AM_DVDCOPY_SET_COPY_STATE* pState = (AM_DVDCOPY_SET_COPY_STATE*)property_data;
		pState->DVDCopyState = AM_DVDCOPYSTATE_AUTHENTICATION_REQUIRED;
		*bytes_raeturned = sizeof(AM_DVDCOPY_SET_COPY_STATE);
	}
	break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

STDMETHODIMP dk_transform_input_pin::QuerySupported(REFGUID prop_set, ULONG id, ULONG * type_support)
{
	if (prop_set != AM_KSPROPSETID_CopyProt) 
	{
		return E_NOTIMPL;
	}

	switch (id)
	{
	case AM_PROPERTY_COPY_MACROVISION:
		*type_support = KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_CHLG_KEY:
		*type_support = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_DEC_KEY2:
		*type_support = KSPROPERTY_SUPPORT_GET;
		break;
	case AM_PROPERTY_DVDCOPY_DISC_KEY:
		*type_support = KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_DVD_KEY1:
		*type_support = KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_REGION:
		*type_support = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_SET_COPY_STATE:
		*type_support = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_TITLE_KEY:
		*type_support = KSPROPERTY_SUPPORT_SET;
		break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

