#pragma once
#include "dk_player_framework.h"
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>
#include <uuids.h>
#include <initguid.h>

#include "dk_haali_media_splitter.h"
#include "dk_avi_splitter.h"
#include "dk_gdcl_mpeg4_demuxer.h"
#include "dk_wmv_splitter.h"

#include "dk_microsoft_video_decoder.h"
#include "dk_dmo_mpeg4_decoder.h"
#include "dk_dmo_mpeg4s_decoder.h"
#include "dk_dmo_mpeg43_decoder.h"
#include "dk_dmo_wmvideo_decoder.h"

#include "dk_microsoft_audio_decoder.h"
#include "dk_dmo_mp3_decoder.h"
#include "dk_dmo_wmaudio_decoder.h"

#include "dk_enhanced_video_renderer.h"
#include "dk_default_direct_sound_renderer.h"

class dshow_player_framework
{
public:
	dshow_player_framework(void);
	~dshow_player_framework(void);

	dk_player_framework::ERR_CODE initialize(HWND hwnd, bool aspect_ratio, bool use_clock, bool enable_audio);
	dk_player_framework::ERR_CODE release(void);

	dk_player_framework::STATE state(void);
	bool seekable(void);
	int seek_resolution(void);
	int current_seek_position(void);
	long long current_media_time(void);

	long long get_total_duration(void);
	float get_step_duration(void);

	dk_player_framework::ERR_CODE seek(int position);
	dk_player_framework::ERR_CODE slowfoward_rate(double rate);
	dk_player_framework::ERR_CODE fastforward_rate(double rate);

	dk_player_framework::ERR_CODE open_file(wchar_t * file);
	dk_player_framework::ERR_CODE play(void);
	dk_player_framework::ERR_CODE pause(void);
	dk_player_framework::ERR_CODE stop(void);

	void aspect_ratio(bool enable);
	void fullscreen(bool enalbe);
	void list_dxva2_decoder_guids(std::vector<GUID> * guids);

	HRESULT update_video_windows(const LPRECT rect);
	HRESULT repaint(HDC hdc);
	HRESULT on_change_displaymode(void);
	HRESULT handle_graphevent(fn_graph_event func);

private:
	HWND _hwnd;
	bool _aspect_ratio;
	bool _use_clock;
	bool _enable_audio;

	dk_player_framework::STATE _state;

	CComPtr<IGraphBuilder> _graph;
	CComPtr<IMediaControl> _control;
	CComPtr<IMediaSeeking> _seeking;
	CComPtr<IMediaEventEx> _event;

	dk_base_source_filter * _source;
	dk_base_video_decode_filter * _video_decoder;
	dk_base_video_render_filter * _video_renderer;

	dk_base_audio_decode_filter * _audio_decoder;
	dk_base_audio_render_filter * _audio_renderer;


	long long _total_duration;
	float _step_duration;
	int _seek_resolution;
};