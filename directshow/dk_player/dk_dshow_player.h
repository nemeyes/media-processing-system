#pragma once

const UINT WM_GRAPH_EVENT = WM_APP + 1;
typedef void (CALLBACK * fn_graph_event)(HWND hwnd, long eventCode, LONG_PTR param1, LONG_PTR param2);

class dk_dshow_video_renderer;
class dk_dshow_player
{
public:
	typedef enum _STATE_T
	{
		STATE_NO_GRAPH,
		STATE_RUNNING,
		STATE_PAUSED,
		STATE_STOPPED,
	} STATE_T;

	dk_dshow_player(HWND hwnd);
	~dk_dshow_player(void);

	
	dk_dshow_player::STATE_T state(void) const
	{
		return _state;
	}

	HRESULT open_file(wchar_t * filename);
	HRESULT play(void);
	HRESULT pause(void);
	HRESULT stop(void);

	bool has_video(void) const;
	HRESULT update_video_windows(const LPRECT rect);
	HRESULT repaint(HDC hdc);
	HRESULT on_change_displaymode(void);
	HRESULT handle_graphevent(fn_graph_event func);

private:
	HRESULT initialize_graph(void);
	void release_graph(void);
	HRESULT create_video_renderer(void);
	HRESULT render_video(IBaseFilter * source);


private:
	dk_dshow_player::STATE_T _state;
	HWND _hwnd;

	IGraphBuilder * _graph;
	IMediaControl * _control;
	IMediaEventEx * _event;

	dk_dshow_video_renderer * _video_renderer;
};