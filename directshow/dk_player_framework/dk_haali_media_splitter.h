#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>

#include "dk_base_filter.h"

class dk_haali_media_splitter : public dk_base_source_filter
{
public:
	dk_haali_media_splitter(void);
	virtual ~dk_haali_media_splitter(void);

	CComPtr<IBaseFilter> get_filter(void);
	CComPtr<IPin> get_video_output_pin(void);
	CComPtr<IPin> get_audio_output_pin(void);

	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file);

private:
	CComPtr<IBaseFilter> _source;
};