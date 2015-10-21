#pragma once

class dk_transform_input_pin : public CTransformInputPin, public IKsPropertySet
{
public:
	dk_transform_input_pin(TCHAR * objname, CTransformFilter * filter, HRESULT * hr, LPWSTR name);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// IMemInputPin
	STDMETHODIMP Receive(IMediaSample * sample);
	HRESULT SetMediaType(const CMediaType * mt);

	// IKsPropertySet
	STDMETHODIMP Set(REFGUID prop_set, ULONG id, LPVOID instance_data, ULONG instance_length, LPVOID property_data, ULONG data_length);
	STDMETHODIMP Get(REFGUID prop_set, ULONG id, LPVOID instance_data, ULONG instance_length, LPVOID property_data, ULONG data_length, ULONG * bytes_raeturned);
	STDMETHODIMP QuerySupported(REFGUID prop_set, ULONG id, ULONG * type_support);

private:
	int m_varient;
	BYTE m_Challenge[10], m_KeyCheck[5], m_Key[10];
	BYTE m_DiscKey[6], m_TitleKey[6];
};