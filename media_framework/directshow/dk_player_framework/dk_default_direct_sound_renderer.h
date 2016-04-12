#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>

#include "dk_base_filter.h"

class dk_default_direct_sound_renderer : public dk_base_audio_render_filter
{
public:
	dk_default_direct_sound_renderer(void);
	virtual ~dk_default_direct_sound_renderer(void);

	CComPtr<IBaseFilter> get_filter(void);
	CComPtr<IPin> get_output_pin(void);
	CComPtr<IPin> get_input_pin(void);

	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph);

private:
	CComPtr<IBaseFilter> _renderer;
};