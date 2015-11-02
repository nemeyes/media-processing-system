
#include "dk_transform_input_pin.h"

#include <cstdint>

#define AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))

class dk_video_decode_input_pin : public dk_transform_input_pin
{
public:
	dk_video_decode_input_pin(TCHAR * objname, CTransformFilter * filter, HRESULT * hr, LPWSTR name);

	// IKsPropertySet
	STDMETHODIMP Set(REFGUID prop_set, ULONG id, LPVOID instance_data, ULONG instance_length, LPVOID property_data, ULONG data_length);
	STDMETHODIMP Get(REFGUID prop_set, ULONG id, LPVOID instance_data, ULONG instance_length, LPVOID property_data, ULONG data_length, ULONG * bytes_raeturned);
	STDMETHODIMP QuerySupported(REFGUID prop_set, ULONG id, ULONG * type_support);

	AM_SimpleRateChange GetDVDRateChange() { CAutoLock cAutoLock(&m_csRateLock); return m_ratechange; }
private:
	//CLAVVideo *m_pLAVVideo = nullptr;
	CCritSec m_csRateLock;

	int m_CorrectTS = 0;
	AM_SimpleRateChange m_ratechange = AM_SimpleRateChange{ AV_NOPTS_VALUE, 10000 };
};