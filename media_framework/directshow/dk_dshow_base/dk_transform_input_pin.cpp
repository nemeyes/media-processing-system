#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>


#include <css_auth.h>
#include <css_scramble.h>

#include "dk_transform_input_pin.h"

dk_transform_input_pin::dk_transform_input_pin(TCHAR * objname, CTransformFilter * filter, HRESULT * hr, LPWSTR name)
	: CTransformInputPin(objname, filter, hr, name)
{

}

void dk_transform_input_pin::decrypt(IMediaSample * sample)
{
	long len = sample->GetActualDataLength();

	BYTE* p = nullptr;
	if (SUCCEEDED(sample->GetPointer(&p)) && len > 0) 
	{
		if (m_mt.majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && len == 2048 && (p[0x14] & 0x30)) 
		{
			css_descramble(p, m_TitleKey);
			p[0x14] &= ~0x30;

			IMediaSample2 * ms2 = nullptr;
			if (SUCCEEDED(sample->QueryInterface(&ms2)) && ms2) 
			{
				AM_SAMPLE2_PROPERTIES props;
				memset(&props, 0, sizeof(props));
				if (SUCCEEDED(ms2->GetProperties(sizeof(props), (BYTE*)&props)) && (props.dwTypeSpecificFlags & AM_UseNewCSSKey)) 
				{
					props.dwTypeSpecificFlags &= ~AM_UseNewCSSKey;
					ms2->SetProperties(sizeof(props), (BYTE*)&props);
				}
				ms2->Release();
			}
		}
	}
}

void dk_transform_input_pin::strip_packet(BYTE *& p, long & len)
{
	GUID majortype = m_mt.majortype;
	GUID subtype = m_mt.subtype;

	if (majortype == MEDIATYPE_MPEG2_PACK || majortype == MEDIATYPE_DVD_ENCRYPTED_PACK) 
	{
		if (len > 0 && *(DWORD*)p == 0xba010000) // MEDIATYPE_*_PACK
		{ 
			len -= 14;
			p += 14;
			if (int stuffing = (p[-1] & 7)) 
			{
				len -= stuffing;
				p += stuffing;
			}
			majortype = MEDIATYPE_MPEG2_PES;
		}
	}

	if (majortype == MEDIATYPE_MPEG2_PES) 
	{
		if (len > 0 && *(DWORD*)p == 0xbb010000) 
		{
			len -= 4;
			p += 4;
			int hdrlen = ((p[0] << 8) | p[1]) + 2;
			len -= hdrlen;
			p += hdrlen;
		}

		if ((len>4) && ((*(DWORD*)p & 0xf0ffffff) == 0xe0010000 || (*(DWORD*)p & 0xe0ffffff) == 0xc0010000 || (*(DWORD*)p & 0xbdffffff) == 0xbd010000)) // PES
		{ 
			bool ps1 = (*(DWORD*)p & 0xbdffffff) == 0xbd010000;
			len -= 4;
			p += 4;
			size_t expected = ((p[0] << 8) | p[1]);
			len -= 2;
			p += 2;
			BYTE* p0 = p;

			for (int i = 0; i < 16 && *p == 0xff; i++, len--, p++) {
				;
			}

			if ((*p & 0xc0) == 0x80) 
			{ // mpeg2
				len -= 2;
				p += 2;
				len -= *p + 1;
				p += *p + 1;
			}
			else { // mpeg1
				if ((*p & 0xc0) == 0x40) 
				{
					len -= 2;
					p += 2;
				}

				if ((*p & 0x30) == 0x30 || (*p & 0x30) == 0x20) 
				{
					bool pts = !!(*p & 0x20), dts = !!(*p & 0x10);
					if (pts) 
					{
						len -= 5;
					}
					p += 5;
					if (dts) 
					{
						ASSERT((*p & 0xf0) == 0x10);
						len -= 5;
						p += 5;
					}
				}
				else 
				{
					len--;
					p++;
				}
			}

			if (ps1) 
			{
				len--;
				p++;
				if (subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) 
				{
					len -= 6;
					p += 6;
				}
				else if (subtype == MEDIASUBTYPE_DOLBY_AC3 || subtype == FOURCCMap(0x2000) || subtype == MEDIASUBTYPE_DTS || subtype == FOURCCMap(0x2001)) 
				{
					len -= 3;
					p += 3;
				}
			}

			if (expected > 0) 
			{
				expected -= (p - p0);
				len = min((long)expected, len);
			}
		}

		if (len < 0) 
		{
			ASSERT(0);
			len = 0;
		}
	}
}

STDMETHODIMP dk_transform_input_pin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IKsPropertySet))
	{
		return GetInterface(static_cast<IKsPropertySet*>(this), ppv);
	}
	else
	{
		return CTransformInputPin::NonDelegatingQueryInterface(riid, ppv);
	}
}

// IMemInputPin
STDMETHODIMP dk_transform_input_pin::Receive(IMediaSample * ms)
{
	decrypt(ms);
	return CTransformInputPin::Receive(ms);
}

HRESULT dk_transform_input_pin::SetMediaType(const CMediaType * mt)
{
	set_css_media_type(mt);
	return CTransformInputPin::SetMediaType(mt);
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

		css_key2(m_varient, m_Challenge, &m_Key[5]);

		css_buskey(m_varient, m_Key, m_KeyCheck);
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

				css_disckey(DiscKey, g_PlayerKeys[j]);

				BYTE Hash[6];
				for (int i = 0; i < 5; i++) {
					Hash[i] = pDiscKey->DiscKey[i] ^ m_KeyCheck[4 - i];
				}
				Hash[5] = 0;

				css_disckey(Hash, DiscKey);

				if (!memcmp(Hash, DiscKey, 6)) 
				{
					memcpy(m_DiscKey, DiscKey, 6);
					j = g_nPlayerKeys;
					fSuccess = true;
					break;
				}
			}
		}

		if (!fSuccess) 
		{
			return E_FAIL;
		}
	}
	break;
	case AM_PROPERTY_DVDCOPY_DVD_KEY1:
	{ // 2. auth: receive our drive-encrypted nonce word and decrypt it for verification
		AM_DVDCOPY_BUSKEY* pKey1 = (AM_DVDCOPY_BUSKEY*)property_data;
		for (int i = 0; i < 5; i++) 
		{
			m_Key[i] = pKey1->BusKey[4 - i];
		}

		m_varient = -1;

		for (int i = 31; i >= 0; i--) 
		{
			css_key1(i, m_Challenge, m_KeyCheck);

			if (memcmp(m_KeyCheck, &m_Key[0], 5) == 0) 
			{
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
		css_titlekey(m_TitleKey, m_DiscKey);
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

