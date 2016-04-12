#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>

#include <cstdint>
#include "dk_base_filter.h"

class dk_rtmp_source : public dk_base_source_filter
{
public:
	dk_rtmp_source(void);
	virtual ~dk_rtmp_source(void);

	CComPtr<IBaseFilter> get_filter(void);
	CComPtr<IPin> get_video_output_pin(void);
	CComPtr<IPin> get_audio_output_pin(void);

	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * url, wchar_t * id, wchar_t * pwd, bool enable_video, bool enable_audio);

	/*HRESULT set_url(wchar_t * url);
	HRESULT set_username(wchar_t * username);
	HRESULT set_password(wchar_t * password);
	HRESULT set_recv_option(wchar_t * password);
	HRESULT set_recv_timeout(uint64_t * timeout);
	HRESULT set_connection_timeout(uint64_t * timeout);
	HRESULT set_repeat(bool repeat);*/

private:
	CComPtr<IBaseFilter> _source;
};