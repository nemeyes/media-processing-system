#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>

#include "dk_base_filter.h"

class dk_gdcl_mpeg4_demuxer : public dk_base_source_filter
{
public:
	dk_gdcl_mpeg4_demuxer(void);
	virtual ~dk_gdcl_mpeg4_demuxer(void);

	CComPtr<IBaseFilter> get_filter(void);
	CComPtr<IPin> get_video_output_pin(void);
	CComPtr<IPin> get_audio_output_pin(void);

	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file);

private:
	CComPtr<IBaseFilter> _source;
	CComPtr<IBaseFilter> _parser;
};