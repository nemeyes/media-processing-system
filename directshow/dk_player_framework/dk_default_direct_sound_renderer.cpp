#include "dk_default_direct_sound_renderer.h"
#include <atlstr.h>
#include <dk_dshow_helper.h>
#include <uuids.h>
#include <initguid.h>

dk_default_direct_sound_renderer::dk_default_direct_sound_renderer(void)
{

}

dk_default_direct_sound_renderer::~dk_default_direct_sound_renderer(void)
{
	//safe_release(&_renderer);
}

CComPtr<IBaseFilter> dk_default_direct_sound_renderer::get_filter(void)
{
	return _renderer;
}

CComPtr<IPin> dk_default_direct_sound_renderer::get_output_pin(void)
{
	return NULL;
}

CComPtr<IPin> dk_default_direct_sound_renderer::get_input_pin(void)
{
	CComPtr<IPin> inpin;
	dk_dshow_helper::get_pin(_renderer, L"Audio Input pin (rendered)", &inpin);
	return inpin;
}

HRESULT dk_default_direct_sound_renderer::add_to_graph(CComPtr<IGraphBuilder> graph)
{
	CComPtr<IBaseFilter> renderer;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Default Direct Sound Renderer", CLSID_DSoundRender, &renderer);
	if (FAILED(hr))
		return hr;
	return hr;
}