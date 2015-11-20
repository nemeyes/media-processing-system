#include <windows.h>
#include <dk_dshow_helper.h>
#include "dshow_player_framework.h"
#include <uuids.h>
#include <dk_submedia_type.h>
#include <string>


dshow_player_framework::dshow_player_framework(void)
	: _state(dk_player_framework::STATE_NO_GRAPH)
{

}

dshow_player_framework::~dshow_player_framework(void)
{
	release();
}

dk_player_framework::ERR_CODE dshow_player_framework::initialize(HWND hwnd, bool aspect_ratio, bool use_clock, bool enable_audio)
{
	HRESULT hr;
	hr = release();
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	CComPtr<IGraphBuilder> graph;
	CComPtr<IMediaSeeking> seeking;
	CComPtr<IMediaControl> control;
	CComPtr<IMediaEventEx> event;
	hr = graph.CoCreateInstance(CLSID_FilterGraph);
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	hr = graph->QueryInterface(IID_PPV_ARGS(&control));
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	hr = graph->QueryInterface(IID_PPV_ARGS(&seeking));
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;


	hr = graph->QueryInterface(IID_PPV_ARGS(&event));
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	hr = event->SetNotifyWindow((OAHWND)_hwnd, WM_GRAPH_EVENT, NULL);
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	if (!use_clock)
	{
		CComQIPtr<IMediaFilter> sync_interface(graph);
		if (sync_interface)
			hr = sync_interface->SetSyncSource(NULL);
		if (FAILED(hr))
			return dk_player_framework::ERR_CODE_FAILED;
	}

	_hwnd = hwnd;
	_aspect_ratio = aspect_ratio;
	_use_clock = use_clock;
	_enable_audio = enable_audio;
	_graph = graph;
	_control = control;
	_seeking = seeking;
	_event = event;
	_state = dk_player_framework::STATE_STOPPED;


	return dk_player_framework::ERR_CODE_SUCCESS;
}

dk_player_framework::ERR_CODE dshow_player_framework::release(void)
{
	stop();
	if (_event)
		_event->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
	_state = dk_player_framework::STATE_NO_GRAPH;
	return dk_player_framework::ERR_CODE_SUCCESS;
}

dk_player_framework::STATE dshow_player_framework::state(void) 
{
	return _state;
}

dk_player_framework::ERR_CODE dshow_player_framework::open_file(wchar_t * path)
{
	HRESULT hr;
	if (!path || wcslen(path) < 1)
		return dk_player_framework::ERR_CODE_FAILED;

	std::wstring str_path(path);
	std::wstring str_ext = str_path.substr(str_path.find_last_of(L".") + 1);

	//TODO check file format not by file extension
	hr = stop();
	if (!wcscmp(str_ext.c_str(), L"avi"))
	{
		_source = new dk_avi_splitter();
	}
	else if(!wcscmp(str_ext.c_str(), L"mkv"))
	{
		_source = new dk_haali_media_splitter();
	}
	else if (!wcscmp(str_ext.c_str(), L"mp4"))
	{
		_source = new dk_gdcl_mpeg4_demuxer();
	}
	hr = _source->add_to_graph(_graph, path);

	CComPtr<IPin> vopin = _source->get_video_output_pin();
	IEnumMediaTypes * venum = 0;
	hr = vopin->EnumMediaTypes(&venum);
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;
	AM_MEDIA_TYPE * vmt;
	while (S_OK == venum->Next(1, &vmt, NULL))
	{
		if (IsEqualGUID(vmt->subtype, MEDIASUBTYPE_H264) ||
			IsEqualGUID(vmt->subtype, MEDIASUBTYPE_AVC1) ||
			IsEqualGUID(vmt->subtype, MEDIASUBTYPE_avc1) ||
			IsEqualGUID(vmt->subtype, MEDIASUBTYPE_h264) ||
			IsEqualGUID(vmt->subtype, MEDIASUBTYPE_X264) ||
			IsEqualGUID(vmt->subtype, MEDIASUBTYPE_x264))
		{
			_video_decoder = new dk_microsoft_video_decoder();
			break;
		}
		else if (IsEqualGUID(vmt->subtype, MEDIASUBTYPE_MP4V) ||
				 IsEqualGUID(vmt->subtype, MEDIASUBTYPE_XVID) ||
				 IsEqualGUID(vmt->subtype, MEDIASUBTYPE_xvid))
		{
			_video_decoder = new dk_dmo_mpeg4s_decoder();
			break;
		}
	}
	safe_release(&venum);

	_video_renderer = new dk_enhanced_video_renderer();

	hr = _video_decoder->add_to_graph(_graph);
	hr = _video_renderer->add_to_graph(_graph, _hwnd, _aspect_ratio);
	hr = _graph->Render(_source->get_video_output_pin());

	if (_enable_audio)
	{
		CComPtr<IPin> aopin = _source->get_audio_output_pin();
		if (aopin)
		{
			IEnumMediaTypes * aenum = 0;
			hr = aopin->EnumMediaTypes(&aenum);
			if (FAILED(hr))
				return dk_player_framework::ERR_CODE_FAILED;
			AM_MEDIA_TYPE * amt;
			while (S_OK == aenum->Next(1, &amt, NULL))
			{
				if (IsEqualGUID(amt->subtype, MEDIASUBTYPE_AAC) || 
					IsEqualGUID(amt->subtype, MEDIASUBTYPE_AAC1) ||
					IsEqualGUID(amt->subtype, MEDIASUBTYPE_AAC2) ||
					IsEqualGUID(amt->subtype, MEDIASUBTYPE_AAC3) ||
					IsEqualGUID(amt->subtype, MEDIASUBTYPE_DOLBY_AC3))
				{
					_audio_decoder = new dk_microsoft_audio_decoder();
					break;
				}
				else if (IsEqualGUID(amt->subtype, MEDIASUBTYPE_MP3))
				{
					_audio_decoder = new dk_dmo_mp3_decoder();
					break;
				}
			}
			safe_release(&aenum);

			_audio_renderer = new dk_default_direct_sound_renderer();

			hr = _audio_decoder->add_to_graph(_graph);
			hr = _audio_renderer->add_to_graph(_graph);
			hr = _graph->Render(_source->get_audio_output_pin());
		}
	}

	//1 second is equal to 1000 millisecond, or 1000000000 nanosecond
	//1 millisecond is equal to 1000000 nanosecond
	long long duration_100nanosec = 0;
	_seeking->GetDuration(&duration_100nanosec);
	_total_duration = duration_100nanosec / 10000000;
	_duration_step = (float)1000 / _total_duration;

	if (SUCCEEDED(hr))
		return dk_player_framework::ERR_CODE_SUCCESS;
	else
		return dk_player_framework::ERR_CODE_FAILED;
}

//dk_player_framework::ERR_CODE open_rtsp(wchar_t * url);
dk_player_framework::ERR_CODE dshow_player_framework::play(void)
{
	if (_state != dk_player_framework::STATE_PAUSED && _state != dk_player_framework::STATE_STOPPED)
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}
	HRESULT hr = _control->Run();
	if (SUCCEEDED(hr))
	{
		RECT rect;
		::GetClientRect(_hwnd, &rect);
		update_video_windows(&rect);
		_state = dk_player_framework::STATE_RUNNING;
		return dk_player_framework::ERR_CODE_SUCCESS;
	}
	else
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}
}

dk_player_framework::ERR_CODE dshow_player_framework::pause(void)
{
	if (_state != dk_player_framework::STATE_RUNNING)
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}

	HRESULT hr = _control->Pause();
	if (SUCCEEDED(hr))
	{
		RECT rect;
		::GetClientRect(_hwnd, &rect);
		update_video_windows(&rect);
		_state = dk_player_framework::STATE_PAUSED;
		return dk_player_framework::ERR_CODE_SUCCESS;
	}
	else
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}
}

dk_player_framework::ERR_CODE dshow_player_framework::stop(void)
{
	if ((_state != dk_player_framework::STATE_RUNNING) && (_state != dk_player_framework::STATE_PAUSED))
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}

	HRESULT hr = _control->Stop();
	if (SUCCEEDED(hr))
	{
		CComPtr<IBaseFilter> sf = _source->get_filter();
		CComPtr<IBaseFilter> tf = _video_decoder->get_filter();
		CComPtr<IBaseFilter> rf = _video_renderer->get_filter();

		hr = dk_dshow_helper::remove_filter_chain(_graph, sf, rf);
		if (SUCCEEDED(hr))
		{
			if (_source)
			{
				delete _source;
				_source = nullptr;
			}
			if (_video_decoder)
			{
				delete _video_decoder;
				_video_decoder = nullptr;
			}
			if (_video_renderer)
			{
				delete _video_renderer;
				_video_renderer = nullptr;
			}

			if (_audio_decoder)
			{
				delete _audio_decoder;
				_audio_decoder = nullptr;
			}
			if (_audio_renderer)
			{
				delete _audio_renderer;
				_audio_renderer = nullptr;
			}
		}

		_state = dk_player_framework::STATE_STOPPED;
		return dk_player_framework::ERR_CODE_SUCCESS;
	}
	else
	{
		if (_source)
		{
			delete _source;
			_source = nullptr;
		}
		if (_video_decoder)
		{
			delete _video_decoder;
			_video_decoder = nullptr;
		}
		if (_video_renderer)
		{
			delete _video_renderer;
			_video_renderer = nullptr;
		}

		if (_audio_decoder)
		{
			delete _audio_decoder;
			_audio_decoder = nullptr;
		}
		if (_audio_renderer)
		{
			delete _audio_renderer;
			_audio_renderer = nullptr;
		}
		return dk_player_framework::ERR_CODE_FAILED;
	}
}

void dshow_player_framework::aspect_ratio(bool enable)
{
	if (_video_renderer)
		_video_renderer->aspect_ratio(enable);
}

void dshow_player_framework::fullscreen(bool enable)
{
	if (_video_renderer)
		_video_renderer->fullscreen(enable);
}

void dshow_player_framework::list_dxva2_decoder_guids(std::vector<GUID> * guids)
{
	if (_video_renderer)
		_video_renderer->list_dxva2_decoder_guids(guids);
}

HRESULT dshow_player_framework::update_video_windows(const LPRECT rect)
{
	if (_video_renderer)
	{
		return _video_renderer->update_video_window(_hwnd, rect);
	}
	else
	{
		return S_OK;
	}
}

HRESULT dshow_player_framework::repaint(HDC hdc)
{
	if (_video_renderer)
	{
		return _video_renderer->repaint(_hwnd, hdc);
	}
	else
	{
		return S_OK;
	}
}

HRESULT dshow_player_framework::on_change_displaymode(void)
{
	if (_video_renderer)
	{
		return _video_renderer->on_change_displaymode();
	}
	else
	{
		return S_OK;
	}
}

HRESULT dshow_player_framework::handle_graphevent(fn_graph_event func)
{
	if (!_event)
		return E_UNEXPECTED;

	long code = 0;
	LONG_PTR param1 = 0, param2 = 0;

	HRESULT hr = S_OK;
	while (SUCCEEDED(_event->GetEvent(&code, &param1, &param2, 0)))
	{
		func(_hwnd, code, param1, param2);

		hr = _event->FreeEventParams(code, param1, param2);
		if (FAILED(hr))
		{
			break;
		}
	}
	return hr;
}

long long dshow_player_framework::get_total_duration(void)
{
	return _total_duration;
}

float dshow_player_framework::get_duration_step(void)
{
	return _duration_step;
}