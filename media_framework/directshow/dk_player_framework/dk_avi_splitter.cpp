#include "dk_avi_splitter.h"
#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>

dk_avi_splitter::dk_avi_splitter(void)
{

}

dk_avi_splitter::~dk_avi_splitter(void)
{
	safe_release(&_source);
	safe_release(&_parser);
}


CComPtr<IBaseFilter> dk_avi_splitter::get_filter(void)
{
	return _source;
}

CComPtr<IPin> dk_avi_splitter::get_video_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_parser, PINDIR_OUTPUT, MEDIATYPE_Video, &outpin);
	return outpin;
}

CComPtr<IPin> dk_avi_splitter::get_audio_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_parser, PINDIR_OUTPUT, MEDIATYPE_Audio, &outpin);
	return outpin;
}

HRESULT dk_avi_splitter::add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file)
{
	CComPtr<IBaseFilter> source;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Source", CLSID_AsyncReader, &source);
	if (FAILED(hr))
		return hr;

	CComQIPtr<IFileSourceFilter> file_source(source);
	hr = file_source->Load(file, NULL);
	if (FAILED(hr))
		return hr;

	//
	CComPtr<IBaseFilter> parser;
	hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Parser", CLSID_AviSplitter, &parser);
	if (FAILED(hr))
		return hr;

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