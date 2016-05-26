#include <initguid.h>
#include "common.h"
#include "activate.h"
#include "class_factory.h"
#include "media_sink.h"

HMODULE g_module = NULL;
volatile long debuggerking::mf_base::_object_count = 0;
volatile long debuggerking::class_factory::_lock_count = 0;

HRESULT create_object_keyname(const GUID& guid, _Out_writes_(cchMax) TCHAR* pszName, DWORD cchMax)
{
    pszName[0] = _T('\0');

    // convert CLSID to string
    OLECHAR pszCLSID[CHARS_IN_GUID];
    HRESULT hr = StringFromGUID2(guid, pszCLSID, CHARS_IN_GUID);
    if (SUCCEEDED(hr))
    {
        // create a string of the form "CLSID\{clsid}"
        hr = StringCchPrintf(pszName, cchMax - 1, TEXT("CLSID\\%ls"), pszCLSID);
    }
    return hr;
}

HRESULT set_key_value(HKEY hKey, const TCHAR* pszName, const TCHAR* pszValue)
{
    size_t cch = 0;
    DWORD cbData = 0;
    HRESULT hr = StringCchLength(pszValue, MAXLONG, &cch);
    if (SUCCEEDED(hr))
    {
        cbData = (DWORD) (sizeof(TCHAR) * (cch + 1)); // add 1 for the NULL character
        hr = __HRESULT_FROM_WIN32(RegSetValueEx(hKey, pszName, 0, REG_SZ, reinterpret_cast<const BYTE*>(pszValue), cbData));
    }
    return hr;
}

HRESULT register_object(GUID guid, const TCHAR* pszDescription, const TCHAR* pszThreadingModel)
{
    HRESULT hr = S_OK;
    TCHAR pszTemp[MAX_PATH];
    HKEY hKey = NULL;
    HKEY hSubkey = NULL;
    DWORD dwRet = 0;

    do
    {
        hr = create_object_keyname(guid, pszTemp, MAX_PATH);
        if (FAILED(hr))
        {
            break;
        }

        hr = __HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CLASSES_ROOT, pszTemp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL));
        if (FAILED(hr))
        {
            break;
        }

        hr = set_key_value(hKey, NULL, pszDescription);
        if (FAILED(hr))
        {
            break;
        }

        hr = __HRESULT_FROM_WIN32(RegCreateKeyEx(hKey, L"InprocServer32", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubkey, NULL));
        if (FAILED(hr))
        {
            break;
        }

		dwRet = GetModuleFileName(g_module, pszTemp, MAX_PATH);
        if (dwRet == 0)
        {
            hr = __HRESULT_FROM_WIN32(GetLastError());
            break;
        }
        if (dwRet == MAX_PATH)
        {
            hr = E_FAIL; // buffer too small
            break;
        }

		hr = set_key_value(hSubkey, NULL, pszTemp);
        if (FAILED(hr))
        {
            break;
        }

		hr = set_key_value(hSubkey, L"ThreadingModel", pszThreadingModel);
    }
    while (FALSE);

    if (hSubkey != NULL)
    {
        RegCloseKey(hSubkey);
    }

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return hr;
}

HRESULT unregister_object(GUID guid)
{
    HRESULT hr = S_OK;
    TCHAR pszTemp[MAX_PATH];

    do
    {
		hr = create_object_keyname(guid, pszTemp, MAX_PATH);
        if (FAILED(hr))
        {
            break;
        }

        hr = __HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, pszTemp));
    }
    while (FALSE);

    return hr;
}

// DLL Exports

STDAPI create_d3d11_video_renderer(REFIID riid, void ** ppvobj)
{
	return debuggerking::media_sink::CreateInstance(riid, ppvobj);
}

STDAPI create_d3d11_video_renderer_activate(HWND hwnd, IMFActivate ** ppact)
{
	return debuggerking::activate::CreateInstance(hwnd, ppact);
}

STDAPI DllCanUnloadNow(void)
{
    return (debuggerking::mf_base::get_object_count() == 0 && debuggerking::class_factory::is_locked() == FALSE) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(_In_ REFCLSID clsid, _In_ REFIID riid, _Outptr_ void ** ppvobj)
{
	if (clsid != CLSID_D3D11VideoRenderer)
        return CLASS_E_CLASSNOTAVAILABLE;

	debuggerking::class_factory * factory = new debuggerking::class_factory();
	
	if (factory == NULL)
    {
        return E_OUTOFMEMORY;
    }
	factory->AddRef();
    HRESULT hr = factory->QueryInterface(riid, ppvobj);
    safe_release(factory);
    return hr;
}

BOOL APIENTRY DllMain(HANDLE module, DWORD  ul_reason_for_call, LPVOID reserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_module = static_cast<HMODULE>(module);
            break;
        }
        default:
        {
            break;
        }
    }

    return TRUE;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = S_OK;

    do
    {
		hr = register_object(CLSID_D3D11VideoRenderer, L"DebuggerKing D3D11 Video Renderer", L"Both");
        if (FAILED(hr))
        {
            break;
        }

		hr = register_object(CLSID_D3D11VideoRendererActivate, L"DebuggerKing D3D11 Video Renderer Activate", L"Both");
        if (FAILED(hr))
        {
            break;
        }

        hr = MFTRegister(
			CLSID_D3D11VideoRenderer,    // CLSID
            MFT_CATEGORY_OTHER,         // Category
            L"DebuggerKing D3D11 Video Renderer",     // Friendly name
            0,                          // Reserved, must be zero.
            0,
            NULL,
            0,
            NULL,
            NULL
            );
    }
    while (FALSE);

    return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = unregister_object(CLSID_D3D11VideoRenderer);
	HRESULT hrTemp = MFTUnregister(CLSID_D3D11VideoRenderer);
    if (SUCCEEDED(hr))
        hr = hrTemp;

    return hr;
}
