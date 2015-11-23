#include "dk_haali_media_splitter.h"
#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>

typedef HRESULT(STDAPICALLTYPE * eapHaaliMediaSpliterDllRegisterServer)();

dk_haali_media_splitter::dk_haali_media_splitter(void)
{

}

dk_haali_media_splitter::~dk_haali_media_splitter(void)
{
	safe_release(&_source);
}


CComPtr<IBaseFilter> dk_haali_media_splitter::get_filter(void)
{
	return _source;
}

CComPtr<IPin> dk_haali_media_splitter::get_video_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_source, L"Video", &outpin);
	return outpin;
}

CComPtr<IPin> dk_haali_media_splitter::get_audio_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_source, L"Audio", &outpin);
	return outpin;
}

HRESULT dk_haali_media_splitter::add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file)
{
	HRESULT hr = E_FAIL;
	CComPtr<IBaseFilter> source;
	do
	{
		hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Source", CLSID_HaaliMediaSplitter, &source);
		if (FAILED(hr)/*hr == REGDB_E_CLASSNOTREG || hr == ERROR_MOD_NOT_FOUND*/)
		{
			wchar_t exe_path[MAX_PATH] = { 0 };
			wchar_t module_path[MAX_PATH] = { 0 };
			wchar_t * module_name = L"splitter.ax";
			HMODULE exe = GetModuleHandle(NULL);
			if (exe != NULL)
			{
				// When passing NULL to GetModuleHandle, it returns handle of exe itself
				GetModuleFileName(exe, exe_path, (sizeof(exe_path)));
				PathRemoveFileSpec(exe_path);
				_snwprintf_s(module_path, sizeof(module_path), L"%s\\%s", exe_path, module_name);
				HMODULE module = ::LoadLibrary(module_path);
				if (module != INVALID_HANDLE_VALUE)
				{
					eapHaaliMediaSpliterDllRegisterServer fnDllRegisterServer = (eapHaaliMediaSpliterDllRegisterServer)::GetProcAddress(module, "DllRegisterServer");
					hr = fnDllRegisterServer();
					::FreeLibrary(module);
				}
			}
		}
		else
		{
			break;
		}
	} while (1);

	CComQIPtr<IFileSourceFilter> file_source = source;
	hr = file_source->Load(file, NULL);
	if (FAILED(hr))
		return hr;
	_source = source;
	return hr;
}