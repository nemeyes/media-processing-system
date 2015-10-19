#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	// {C883B4C4-D933-47E0-92E5-544D538FF591}
	DEFINE_GUID(IID_IRTSPClient, 0xc883b4c4, 0xd933, 
								   0x47e0, 0x92, 0xe5, 0x54, 0x4d, 0x53, 0x8f, 0xf5, 0x91);

	DECLARE_INTERFACE_(IRTSPClient, IUnknown)
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
