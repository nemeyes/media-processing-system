#include "dk_haali_media_splitter.h"
#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>

dk_haali_media_splitter::dk_haali_media_splitter(void)
{

}

dk_haali_media_splitter::~dk_haali_media_splitter(void)
{

}


CComPtr<IBaseFilter> dk_haali_media_splitter::get_filter(void)
{
	return _source;
}

CComPtr<IPin> dk_haali_media_splitter::get_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_source, L"Video", &outpin);
	return outpin;
}

CComPtr<IPin> dk_haali_media_splitter::get_input_pin(void)
{
	return NULL;
}

HRESULT dk_haali_media_splitter::add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file)
{
	CComPtr<IBaseFilter> source;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Source", CLSID_HaaliMediaSplitter, &source);
	if (FAILED(hr))
		return hr;

	CComQIPtr<IFileSourceFilter> file_source = source;
	hr = file_source->Load(file, NULL);
	if (FAILED(hr))
		return hr;
	_source = source;
	return hr;
}