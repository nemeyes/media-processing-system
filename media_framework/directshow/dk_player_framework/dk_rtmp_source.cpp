#include "dk_rtmp_source.h"
#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <dk_media_source.h>
#include <codecapi.h>

dk_rtmp_source::dk_rtmp_source(void)
{

}

dk_rtmp_source::~dk_rtmp_source(void)
{
	//safe_release(&_source);
}


CComPtr<IBaseFilter> dk_rtmp_source::get_filter(void)
{
	return _source;
}

CComPtr<IPin> dk_rtmp_source::get_video_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_source, PINDIR_OUTPUT, MEDIATYPE_Video, &outpin);
	return outpin;
}

CComPtr<IPin> dk_rtmp_source::get_audio_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_source, PINDIR_OUTPUT, MEDIATYPE_Audio, &outpin);
	return outpin;
}

HRESULT dk_rtmp_source::add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * url, wchar_t * id, wchar_t * pwd, bool enable_video, bool enable_audio)
{
	CComPtr<IBaseFilter> source;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Source", CLSID_DK_RTMP_SOURCE_FILTER, &source);
	if (FAILED(hr))
		return hr;

	CComPtr<IRTMPClient> rtmp_source;
	source->QueryInterface(IID_IRTMPClient, (void**)&rtmp_source);
	rtmp_source->SetUrl(url);
	rtmp_source->SetUsername(id);
	rtmp_source->SetPassword(pwd);
	rtmp_source->SetRepeat(TRUE);

	USHORT recv_option = DK_RECV_NONE;
	if (enable_video)
		recv_option |= DK_RECV_VIDEO;
	if (enable_audio)
		recv_option |= DK_RECV_AUDIO;
	rtmp_source->SetRecvOption(recv_option);

	CComQIPtr<IFileSourceFilter> file_source(source);
	hr = file_source->Load(url, NULL);
	if (FAILED(hr))
		return hr;

	_source = source;
	return hr;
}

/*HRESULT dk_rtmp_source::set_url(wchar_t * url)
{
	CComQIPtr<IFileSourceFilter> rtmp_source(_source);
	//HRESULT hr = 
}

HRESULT dk_rtmp_source::set_username(wchar_t * username)
{

}

HRESULT dk_rtmp_source::set_password(wchar_t * password)
{

}

HRESULT dk_rtmp_source::set_recv_option(wchar_t * password)
{

}

HRESULT dk_rtmp_source::set_recv_timeout(uint64_t * timeout)
{

}

HRESULT dk_rtmp_source::set_connection_timeout(uint64_t * timeout)
{

}

HRESULT dk_rtmp_source::set_repeat(bool repeat)
{

}*/