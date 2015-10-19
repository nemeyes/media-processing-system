#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	// {04F86F76-9E29-479E-A7B7-E1FBE10D0756}
	DEFINE_GUID(IID_IImageSynthesizer,
		0x4f86f76, 0x9e29, 0x479e, 0xa7, 0xb7, 0xe1, 0xfb, 0xe1, 0xd, 0x7, 0x56);

	DECLARE_INTERFACE_(IImageSynthesizer, IUnknown)
	{
		STDMETHOD(RegisterImage)(BSTR url, UINT x, UINT y, UINT width, UINT height, SHORT alpha, USHORT & index) PURE;
		STDMETHOD(UnregisterImage)(USHORT index) PURE;
		//STDMETHOD(ResizeImage)(USHORT index) PURE;
	};

#ifdef __cplusplus
}
#endif
