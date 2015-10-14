#pragma once

class dk_colorspace_convert_filter_properties : public CBasePropertyPage
{
public:
	dk_colorspace_convert_filter_properties(LPUNKNOWN pUnk);
	virtual ~dk_colorspace_convert_filter_properties(VOID);
	static CUnknown* WINAPI CreateInstance(LPUNKNOWN unk, HRESULT *hr);

	HRESULT OnConnect(IUnknown *unk);
	HRESULT OnDisconnect(VOID);
	HRESULT OnActivate(VOID);
	HRESULT OnDeactivate(VOID);
	HRESULT OnApplyChanges(VOID);
	INT_PTR OnReceiveMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};