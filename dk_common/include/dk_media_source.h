#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	// {C883B4C4-D933-47E0-92E5-544D538FF591}
	DEFINE_GUID(IID_IRTSPClient, 0xc883b4c4, 0xd933, 
								   0x47e0, 0x92, 0xe5, 0x54, 0x4d, 0x53, 0x8f, 0xf5, 0x91);

	DECLARE_INTERFACE_IID_(IRTSPClient, IUnknown, "C883B4C4-D933-47E0-92E5-544D538FF591")
	{
		STDMETHOD(SetUrl)(BSTR url) PURE;
		STDMETHOD(SetUsername)(BSTR username) PURE;
		STDMETHOD(SetPassword)(BSTR password) PURE;
		STDMETHOD(SetTransportOption)(USHORT option) PURE;
		STDMETHOD(SetRecvOption)(USHORT option) PURE;
		STDMETHOD(SetRecvTimeout)(ULONGLONG timeout) PURE;
		STDMETHOD(SetConnectionTimeout)(ULONGLONG timeout) PURE;
		STDMETHOD(SetSessionTimeout)(ULONGLONG timeout) PURE;
		STDMETHOD(SetParseSDP)(BOOL sdp) PURE;
		STDMETHOD(SetFocus)(USHORT focus) PURE;
		STDMETHOD(SetRepeat)(BOOL repeat) PURE;
		STDMETHOD(GetUrl)(BSTR * url) PURE;
		STDMETHOD(GetUsername)(BSTR * username) PURE;
		STDMETHOD(GetPassword)(BSTR * password) PURE;
		STDMETHOD(GetTransportOption)(USHORT & option) PURE;
		STDMETHOD(GetRecvOption)(USHORT & option) PURE;
		STDMETHOD(GetRecvTimeout)(ULONGLONG & timeout) PURE;
		STDMETHOD(GetConnectionTimeout)(ULONGLONG & timeout) PURE;
		STDMETHOD(GetSessionTimeout)( ULONGLONG & timeout) PURE;
		STDMETHOD(GetParseSDP)(BOOL & sdp) PURE;
		STDMETHOD(GetFocus)(USHORT & focus) PURE;
		STDMETHOD(GetRepeat)(BOOL & repeat) PURE;
	};

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
	// {56C0869F-6575-4F46-8DD9-B3A9BDF39F21}
	DEFINE_GUID(IID_IRTMPClient,
		0x56c0869f, 0x6575, 0x4f46, 0x8d, 0xd9, 0xb3, 0xa9, 0xbd, 0xf3, 0x9f, 0x21);


	DECLARE_INTERFACE_IID_(IRTMPClient, IUnknown, "56C0869F-6575-4F46-8DD9-B3A9BDF39F21")
	{
		STDMETHOD(SetUrl)(BSTR url) PURE;
		STDMETHOD(SetUsername)(BSTR username) PURE;
		STDMETHOD(SetPassword)(BSTR password) PURE;
		STDMETHOD(SetRecvOption)(USHORT option) PURE;
		STDMETHOD(SetRecvTimeout)(ULONGLONG timeout) PURE;
		STDMETHOD(SetConnectionTimeout)(ULONGLONG timeout) PURE;
		STDMETHOD(SetRepeat)(BOOL repeat) PURE;
		STDMETHOD(GetUrl)(BSTR * url) PURE;
		STDMETHOD(GetUsername)(BSTR * username) PURE;
		STDMETHOD(GetPassword)(BSTR * password) PURE;
		STDMETHOD(GetRecvOption)(USHORT & option) PURE;
		STDMETHOD(GetRecvTimeout)(ULONGLONG & timeout) PURE;
		STDMETHOD(GetConnectionTimeout)(ULONGLONG & timeout) PURE;
		STDMETHOD(GetRepeat)(BOOL & repeat) PURE;
	};

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

	// {263768C3-5933-4D6B-B20C-2FD1FBE62AA7}
	DEFINE_GUID(IID_IYUVSource,
		0x263768c3, 0x5933, 0x4d6b, 0xb2, 0xc, 0x2f, 0xd1, 0xfb, 0xe6, 0x2a, 0xa7);

	DECLARE_INTERFACE_IID_(IYUVSource, IUnknown, "263768C3-5933-4D6B-B20C-2FD1FBE62AA7")
	{
		STDMETHOD(SetWidth)(UINT option) PURE;
		STDMETHOD(SetHeight)(UINT option) PURE;
		STDMETHOD(SetFPS)(UINT option) PURE;
	};

#ifdef __cplusplus
}
#endif