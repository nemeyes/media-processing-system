#pragma once

class dk_video_decode_output_pin : public CTransformOutputPin
{
public:
	dk_video_decode_output_pin(TCHAR * objname, IBaseFilter * filter, HRESULT * hr, LPCWSTR name);
	virtual ~dk_video_decode_output_pin();

	HRESULT InitAllocator(IMemAllocator **ppAlloc);
};
