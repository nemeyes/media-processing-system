#pragma once

class dk_yuvsource_filter_properties : public CBasePropertyPage
{
public:
	dk_yuvsource_filter_properties(LPUNKNOWN unk);
	virtual ~dk_yuvsource_filter_properties(void);
	static CUnknown* WINAPI CreateInstance(LPUNKNOWN unk, HRESULT * hr);

	HRESULT OnConnect(IUnknown * unk);
	HRESULT OnDisconnect(void);
	HRESULT OnActivate(void);
	HRESULT OnDeactivate(void);
	HRESULT OnApplyChanges(void);
	INT_PTR OnReceiveMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};