#pragma once

#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <evr.h>

class dk_dshow_video_renderer
{
public:
	virtual ~dk_dshow_video_renderer(void) {};
	virtual bool has_video(void) const = 0;
	virtual HRESULT add_to_graph(IGraphBuilder * graph, HWND hwnd) = 0;
	virtual HRESULT finalize_graph(IGraphBuilder * graph) = 0;
	virtual HRESULT update_video_window(HWND hwnd, const LPRECT rect) = 0;
	virtual HRESULT repaint(HWND hwnd, HDC hdc)=0;
	virtual HRESULT on_change_displaymode(void) = 0;
};

class dk_evr_video_renderer : public dk_dshow_video_renderer
{
public:
	dk_evr_video_renderer(void);
	~dk_evr_video_renderer(void);

	bool has_video(void) const;
	HRESULT add_to_graph(IGraphBuilder * graph, HWND hwnd);
	HRESULT finalize_graph(IGraphBuilder * graph);
	HRESULT update_video_window(HWND hwnd, const LPRECT rect);
	HRESULT repaint(HWND hwnd, HDC hdc);
	HRESULT on_change_displaymode(void);

private:
	IBaseFilter * _evr;
	IMFVideoDisplayControl * _video_display;
};
