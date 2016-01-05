#pragma once
#include <vector>

class dk_base_filter
{
public:
	virtual CComPtr<IPin> get_output_pin(void) = 0;
	virtual CComPtr<IPin> get_input_pin(void) = 0;
	virtual CComPtr<IBaseFilter> get_filter(void) = 0;
};

class dk_base_source_filter : public dk_base_filter
{
public:
	dk_base_source_filter(void) {};
	virtual ~dk_base_source_filter(void) {};
	virtual HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file) { return E_NOTIMPL; }
	virtual HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * url, wchar_t * id, wchar_t * pwd) { return E_NOTIMPL; }


	CComPtr<IPin> get_output_pin(void) { return NULL; };
	CComPtr<IPin> get_input_pin(void) { return NULL; };

	virtual CComPtr<IPin> get_video_output_pin(void) = 0;
	virtual CComPtr<IPin> get_audio_output_pin(void) = 0;
};

class dk_base_video_decode_filter : public dk_base_filter
{
public:
	dk_base_video_decode_filter(void) {};
	virtual ~dk_base_video_decode_filter(void) {};
	virtual HRESULT add_to_graph(CComPtr<IGraphBuilder> graph) = 0;
};

class dk_base_audio_decode_filter : public dk_base_filter
{
public:
	dk_base_audio_decode_filter(void) {};
	virtual ~dk_base_audio_decode_filter(void) {};
	virtual HRESULT add_to_graph(CComPtr<IGraphBuilder> graph) = 0;
};

class dk_base_video_render_filter : public dk_base_filter
{
public:
	dk_base_video_render_filter(void) {};
	virtual ~dk_base_video_render_filter(void) {};
	virtual void aspect_ratio(bool enable) = 0;
	virtual void fullscreen(bool enable) = 0;
	virtual void list_dxva2_decoder_guids(std::vector<GUID> * guids) = 0;

	virtual HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, HWND hwnd, bool aspect_ratio) = 0;
	virtual HRESULT update_video_window(HWND hwnd, const LPRECT rect) = 0;
	virtual HRESULT repaint(HWND hwnd, HDC hdc) = 0;
	virtual HRESULT on_change_displaymode(void) = 0;
};

class dk_base_audio_render_filter : public dk_base_filter
{
public:
	dk_base_audio_render_filter(void) {};
	virtual ~dk_base_audio_render_filter(void) {};
	virtual HRESULT add_to_graph(CComPtr<IGraphBuilder> graph) = 0;
};