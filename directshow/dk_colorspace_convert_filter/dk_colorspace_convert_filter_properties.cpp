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

#include "dk_colorspace_convert_filter.h"
#include "dk_colorspace_convert_filter_properties.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dk_colorspace_convert_filter_properties::dk_colorspace_convert_filter_properties(LPUNKNOWN unk)
	:CBasePropertyPage(NAME("dk_colorspace_convert_filter_properties"), unk, IDD_PROPPAGE_MEDIUM, IDS_PROPPAGE_TITLE)
{

}

dk_colorspace_convert_filter_properties::~dk_colorspace_convert_filter_properties(VOID)
{

}

CUnknown* WINAPI dk_colorspace_convert_filter_properties::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	CUnknown *punk = new dk_colorspace_convert_filter_properties(unk);
	if (punk == NULL)
	{
		*hr = E_OUTOFMEMORY;
	}
	return punk;
}

HRESULT dk_colorspace_convert_filter_properties::OnConnect(IUnknown *unk)
{
	//HRESULT hr = unk->QueryInterface(IID_IVmxnetVideoDecodeSetting, (void **)&_setting);
	//if (FAILED(hr))
	//	return hr;
	return S_OK;
}

HRESULT dk_colorspace_convert_filter_properties::OnDisconnect(VOID)
{
	//if (_setting == NULL)
	//	return E_UNEXPECTED;
	//_setting->Release();
	//_setting = NULL;
	return S_OK;
}

HRESULT dk_colorspace_convert_filter_properties::OnActivate(VOID)
{
	return S_OK;
}

HRESULT dk_colorspace_convert_filter_properties::OnDeactivate(VOID)
{
	return S_OK;
}

HRESULT dk_colorspace_convert_filter_properties::OnApplyChanges(VOID)
{
	//if (_setting)
	//{
	//	return S_OK;
	//}
	return E_FAIL;
}

INT_PTR dk_colorspace_convert_filter_properties::OnReceiveMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//HWND hwnd_edit;
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		//hwnd_edit = ::GetDlgItem(hwnd, IDC_COMBO_OUTPUT_MEDIA_TYPE);
		//::EnableWindow(hwnd_edit, TRUE);
		//hwnd_edit = ::GetDlgItem(hwnd, IDC_EDIT_OUTPUT_RESOLUTION_WIDTH);
		//::EnableWindow(hwnd_edit, TRUE);
		//hwnd_edit = ::GetDlgItem(hwnd, IDC_EDIT_OUTPUT_RESOLUTION_HEIGHT);
		//::EnableWindow(hwnd_edit, TRUE);

		//hwnd_edit = ::GetDlgItem(hwnd, IDC_COMBO_OUTPUT_MEDIA_TYPE);
		//if (hwnd_edit)
		//{
		//	::SendMessage(hwnd_edit, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(_T("RGB32")));
		//	::SendMessage(hwnd_edit, CB_INSERTSTRING, 1, reinterpret_cast<LPARAM>(_T("YUY2")));
		//	::SendMessage(hwnd_edit, CB_INSERTSTRING, 2, reinterpret_cast<LPARAM>(_T("YV12")));
		//	if (_setting)
		//	{
		//		VMXNET_SUB_MEDIA_TYPE submedia_type;
		//		_setting->GetMediaType(submedia_type);
		//		if (submedia_type == VMXNET_SUB_MEDIA_TYPE_RGB32)
		//			::SendMessage(hwnd_edit, CB_SETCURSEL, 0, 0);
		//		if (submedia_type == VMXNET_SUB_MEDIA_TYPE_YUY2)
		//			::SendMessage(hwnd_edit, CB_SETCURSEL, 1, 0);
		//		if (submedia_type == VMXNET_SUB_MEDIA_TYPE_YV12)
		//			::SendMessage(hwnd_edit, CB_SETCURSEL, 2, 0);
		//	}
		//}

		//if (_setting)
		//{
		//	USHORT width;
		//	USHORT height;
		//	_setting->GetResolution(width, height);
		//	hwnd_edit = ::GetDlgItem(hwnd, IDC_EDIT_OUTPUT_RESOLUTION_WIDTH);
		//	if (hwnd_edit)
		//	{
		//		TCHAR str_width[10] = { 0 };
		//		_sntprintf(str_width, sizeof(str_width), _T("%d"), width);
		//		if (_tcslen(str_width)>0)
		//			::SetWindowText(hwnd_edit, str_width);
		//	}
		//	hwnd_edit = ::GetDlgItem(hwnd, IDC_EDIT_OUTPUT_RESOLUTION_HEIGHT);
		//	if (hwnd_edit)
		//	{
		//		TCHAR str_height[10] = { 0 };
		//		_sntprintf(str_height, sizeof(str_height), _T("%d"), height);
		//		if (_tcslen(str_height)>0)
		//			::SetWindowText(hwnd_edit, str_height);
		//	}
		//}
	}
	case WM_COMMAND:
	{
		//hwnd_edit = (HWND)lparam;
		//switch (LOWORD(wparam))
		//{
		//case IDC_COMBO_OUTPUT_MEDIA_TYPE:
		//{
		//	switch (HIWORD(wparam))
		//	{
		//	case CBN_SELCHANGE:
		//	{
		//		LRESULT selected_index = SendMessage(hwnd_edit, CB_GETCURSEL, 0, 0);
		//		VMXNET_SUB_MEDIA_TYPE submedia_type = VMXNET_SUB_MEDIA_TYPE_UNKNOWN;
		//		switch (selected_index)
		//		{
		//		case 0:
		//			submedia_type = VMXNET_SUB_MEDIA_TYPE_RGB32;
		//			break;
		//		case 1:
		//			submedia_type = VMXNET_SUB_MEDIA_TYPE_YUY2;
		//			break;
		//		case 2:
		//			submedia_type = VMXNET_SUB_MEDIA_TYPE_YV12;
		//			break;
		//		}
		//		if (_setting)
		//			_setting->SetMediaType(submedia_type);

		//		/*
		//		m_bDirty = TRUE;
		//		if( m_pPageSite )
		//		m_pPageSite->OnStatusChange( PROPPAGESTATUS_DIRTY );
		//		*/
		//		break;
		//	}
		//	}
		//	break;
		//}
		//case IDC_EDIT_OUTPUT_RESOLUTION_WIDTH:
		//{
		//	if (HIWORD(wparam) == EN_CHANGE)
		//	{
		//		TCHAR str_width[MAX_PATH] = { 0 };
		//		GetWindowText(hwnd_edit, str_width, MAX_PATH);
		//		if (_tcslen(str_width)>0)
		//			_width = _ttoi(str_width);
		//		if (_setting)
		//			_setting->SetResolution(_width, _height);

		//		m_bDirty = TRUE;
		//		if (m_pPageSite)
		//			m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
		//	}
		//	break;
		//}
		//case IDC_EDIT_OUTPUT_RESOLUTION_HEIGHT:
		//{
		//	if (HIWORD(wparam) == EN_CHANGE)
		//	{
		//		TCHAR str_height[MAX_PATH] = { 0 };
		//		GetWindowText(hwnd_edit, str_height, MAX_PATH);
		//		if (_tcslen(str_height)>0)
		//			_height = _ttoi(str_height);
		//		if (_setting)
		//			_setting->SetResolution(_width, _height);

		//		m_bDirty = TRUE;
		//		if (m_pPageSite)
		//			m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
		//	}
		//	break;
		//}
		//}
	}
	break;
	}
	return CBasePropertyPage::OnReceiveMessage(hwnd, msg, wparam, lparam);
}