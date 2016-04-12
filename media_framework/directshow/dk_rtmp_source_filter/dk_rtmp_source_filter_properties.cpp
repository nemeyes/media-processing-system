#include <winsock2.h>
#include <windows.h>

#include <tchar.h>
#include <dshow.h>
#include <initguid.h> 
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <atlbase.h>
#include <string.h>
#include <stdlib.h>
#include <streams.h>
#include <dvdmedia.h>

#include <dk_media_source.h>

#include "dk_rtmp_source_filter.h"
#include "dk_rtmp_video_source_stream.h"
#include "dk_rtmp_source_filter_properties.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dk_rtmp_source_filter_properties::dk_rtmp_source_filter_properties(LPUNKNOWN unk)
	:CBasePropertyPage(NAME("dk_rtmp_source_filter_properties"), unk, IDD_PROPPAGE_MEDIUM, IDS_PROPPAGE_TITLE)
	, _setting(NULL)
{

}

dk_rtmp_source_filter_properties::~dk_rtmp_source_filter_properties(VOID)
{

}

CUnknown* WINAPI dk_rtmp_source_filter_properties::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	CUnknown *punk = new dk_rtmp_source_filter_properties(unk);
	if (punk == NULL)
	{
		*hr = E_OUTOFMEMORY;
	}
	return punk;
}

HRESULT dk_rtmp_source_filter_properties::OnConnect(IUnknown *unk)
{
	HRESULT hr = unk->QueryInterface(IID_IRTMPClient, (void **)&_setting);
	if (FAILED(hr))
		return hr;
	hr = unk->QueryInterface(IID_IFileSourceFilter, (void **)&_file_source_filter);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

HRESULT dk_rtmp_source_filter_properties::OnDisconnect(VOID)
{
	if (_setting == NULL)
		return E_UNEXPECTED;
	_setting->Release();
	_setting = NULL;
	return S_OK;
}

HRESULT dk_rtmp_source_filter_properties::OnActivate(VOID)
{
	return S_OK;
}

HRESULT dk_rtmp_source_filter_properties::OnDeactivate(VOID)
{
	return S_OK;
}

HRESULT dk_rtmp_source_filter_properties::OnApplyChanges(VOID)
{
	if (_setting && _file_source_filter)
	{
		_file_source_filter->Load(NULL, NULL);
		return S_OK;
	}
	return E_FAIL;
}

INT_PTR dk_rtmp_source_filter_properties::OnReceiveMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	HWND hwnd_edit;
	switch (msg)
	{
	/*case WM_INITDIALOG:
	{
		hwnd_edit = ::GetDlgItem(hwnd, IDC_EDIT_MEDIA_URL);
		if (hwnd_edit)
		{
			if (_setting)
			{
				BSTR url;
				_setting->get_media_url(&url);
				if (SysStringLen(url) > 0)
				{
					::SetWindowText(hwnd_edit, url);
				}
			}
		}

		hwnd_edit = ::GetDlgItem(hwnd, IDC_EDIT_ACCESS_ID);
		if (hwnd_edit)
		{
			if (_setting)
			{
				BSTR admin;
				_setting->get_access_id(&admin);
				if (SysStringLen(admin) > 0)
				{
					::SetWindowText(hwnd_edit, admin);
				}
			}
		}

		hwnd_edit = ::GetDlgItem(hwnd, IDC_EDIT_ACCESS_PWD);
		if (hwnd_edit)
		{
			if (_setting)
			{
				BSTR password;
				_setting->get_access_password(&password);
				if (SysStringLen(password) > 0)
				{
					::SetWindowText(hwnd_edit, password);
				}
			}
		}
	}
	case WM_COMMAND:
	{
		hwnd_edit = (HWND)lparam;
		switch (LOWORD(wparam))
		{
		case IDC_EDIT_MEDIA_URL:
		{
			if (HIWORD(wparam) == EN_CHANGE)
			{
				TCHAR url[MAX_PATH] = { 0 };
				GetWindowText(hwnd_edit, url, MAX_PATH);
				if (_setting)
					_setting->set_media_url(url);
				m_bDirty = TRUE;
				if (m_pPageSite)
					m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
			}
			break;
		}
		case IDC_EDIT_ACCESS_ID:
		{
			if (HIWORD(wparam) == EN_CHANGE)
			{
				TCHAR access_id[MAX_PATH] = { 0 };
				GetWindowText(hwnd_edit, access_id, MAX_PATH);
				if (_setting)
					_setting->set_access_id(access_id);
				m_bDirty = TRUE;
				if (m_pPageSite)
					m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
			}
			break;
		}
		case IDC_EDIT_ACCESS_PWD:
		{
			if (HIWORD(wparam) == EN_CHANGE)
			{
				TCHAR access_password[MAX_PATH] = { 0 };
				GetWindowText(hwnd_edit, access_password, MAX_PATH);
				if (_setting)
					_setting->set_access_password(access_password);
				m_bDirty = TRUE;
				if (m_pPageSite)
					m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
			}
			break;
		}
		}
		break;
	}*/
	}
	return CBasePropertyPage::OnReceiveMessage(hwnd, msg, wparam, lparam);
}
