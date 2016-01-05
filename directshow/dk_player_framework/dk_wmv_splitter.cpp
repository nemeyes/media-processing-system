#include "dk_wmv_splitter.h"
#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>

dk_wmv_splitter::dk_wmv_splitter(void)
{

}

dk_wmv_splitter::~dk_wmv_splitter(void)
{
	//safe_release(&_source);
}


CComPtr<IBaseFilter> dk_wmv_splitter::get_filter(void)
{
	return _source;
}

CComPtr<IPin> dk_wmv_splitter::get_video_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_source, PINDIR_OUTPUT, MEDIATYPE_Video, &outpin);
	return outpin;
}

CComPtr<IPin> dk_wmv_splitter::get_audio_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_source, PINDIR_OUTPUT, MEDIATYPE_Audio, &outpin);
	return outpin;
}

HRESULT dk_wmv_splitter::add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file)
{
	CComPtr<IBaseFilter> source;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Source", CLSID_WMAsfReader, &source);
	if (FAILED(hr))
		return hr;

	CComQIPtr<IFileSourceFilter> file_source = source;
	hr = file_source->Load(file, NULL);
	if (FAILED(hr))
		return hr;
	_source = source;
	return hr;
}