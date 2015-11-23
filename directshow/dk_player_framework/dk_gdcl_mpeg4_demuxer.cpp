#include "dk_gdcl_mpeg4_demuxer.h"
#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>

typedef HRESULT(STDAPICALLTYPE * eapGDCLMpeg4DemuxerDllRegisterServer)();

dk_gdcl_mpeg4_demuxer::dk_gdcl_mpeg4_demuxer(void)
{

}

dk_gdcl_mpeg4_demuxer::~dk_gdcl_mpeg4_demuxer(void)
{
	safe_release(&_source);
	safe_release(&_parser);
}


CComPtr<IBaseFilter> dk_gdcl_mpeg4_demuxer::get_filter(void)
{
	return _source;
}

CComPtr<IPin> dk_gdcl_mpeg4_demuxer::get_video_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_parser, PINDIR_OUTPUT, MEDIATYPE_Video, &outpin);
	return outpin;
}

CComPtr<IPin> dk_gdcl_mpeg4_demuxer::get_audio_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_parser, PINDIR_OUTPUT, MEDIATYPE_Audio, &outpin);
	return outpin;
}

HRESULT dk_gdcl_mpeg4_demuxer::add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file)
{
	CComPtr<IBaseFilter> source;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Source", CLSID_AsyncReader, &source);
	if (FAILED(hr))
		return hr;

	CComQIPtr<IFileSourceFilter> file_source(source);
	hr = file_source->Load(file, NULL);
	if (FAILED(hr))
		return hr;

	CComPtr<IBaseFilter> parser;
	do
	{
		hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Parser", CLSID_GDCLMpeg4Demuxer, &parser);
		if (FAILED(hr)/*hr==REGDB_E_CLASSNOTREG || hr==ERROR_MOD_NOT_FOUND*/)
		{
			wchar_t exe_path[MAX_PATH] = { 0 };
			wchar_t module_path[MAX_PATH] = { 0 };
			wchar_t * module_name = L"mp4demux.dll";
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
					eapGDCLMpeg4DemuxerDllRegisterServer fnDllRegisterServer = (eapGDCLMpeg4DemuxerDllRegisterServer)::GetProcAddress(module, "DllRegisterServer");
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

	CComPtr<IPin> outpin;
	hr = dk_dshow_helper::get_pin(source, PINDIR_OUTPUT, &outpin);
	if (FAILED(hr))
		return hr;

	CComPtr<IPin> inpin;
	hr = dk_dshow_helper::get_pin(parser, PINDIR_INPUT, &inpin);
	if (FAILED(hr))
		return hr;

	hr = graph->ConnectDirect(outpin, inpin, NULL);
	if (FAILED(hr))
		return hr;

	_source = source;
	_parser = parser;
	return hr;
}