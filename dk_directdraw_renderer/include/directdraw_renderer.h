#ifndef _DIRECTDRAW_RENDERER_H_
#define _DIRECTDRAW_RENDERER_H_

#include "dk_directdraw_renderer.h"
#include <ddraw.h>

class directdraw_renderer
{
public:
	directdraw_renderer(void);
	~directdraw_renderer(void);

	bool is_initialized(void);

	dk_directdraw_renderer::ERR_CODE initialize_renderer(dk_directdraw_renderer::configuration_t * config);
	dk_directdraw_renderer::ERR_CODE release_renderer(void);
	dk_directdraw_renderer::ERR_CODE render(dk_directdraw_renderer::dk_video_entity_t * decoded);

	dk_directdraw_renderer::ERR_CODE open(void);
	dk_directdraw_renderer::ERR_CODE close(void);

private:
	dk_directdraw_renderer::ERR_CODE create(HWND hwnd, DWORD width, DWORD height, DWORD pos_x, DWORD pos_y, DWORD *rgb_bitcount = 0);
	dk_directdraw_renderer::ERR_CODE destroy(void);


	dk_directdraw_renderer::ERR_CODE enable_time_text(bool enable = true);
	dk_directdraw_renderer::ERR_CODE set_time_text_position(unsigned short x, unsigned short y);
	dk_directdraw_renderer::ERR_CODE set_time_text_font_size(unsigned char size);
	dk_directdraw_renderer::ERR_CODE set_time_text_color(unsigned char red, unsigned char green, unsigned char blue);

	dk_directdraw_renderer::ERR_CODE enable_osd_text(bool enable = true);
	dk_directdraw_renderer::ERR_CODE set_osd_text(wchar_t *osd);
	dk_directdraw_renderer::ERR_CODE set_osd_text_position(unsigned short x, unsigned short y);
	dk_directdraw_renderer::ERR_CODE set_osd_text_font_size(unsigned char size);
	dk_directdraw_renderer::ERR_CODE set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue);

	dk_directdraw_renderer::ERR_CODE set_background_color(unsigned char red, unsigned char green, unsigned char blue);

	dk_directdraw_renderer::ERR_CODE set_normal_screen_handle(HWND hwnd);
	dk_directdraw_renderer::ERR_CODE set_full_screen_handle(HWND hwnd);

	dk_directdraw_renderer::ERR_CODE enable_full_screen(bool enable);
	dk_directdraw_renderer::ERR_CODE enable_stretch(bool enable);

	dk_directdraw_renderer::ERR_CODE set_enable(bool enable);
	bool get_enable(void);

	/*
	HWND			get_parent_window( LPVMS_SUBMEDIA_ELEMENT_T subpe );
	HWND			get_parent_full_screen_window( LPVMS_SUBMEDIA_ELEMENT_T subpe );
	*/

	dk_directdraw_renderer::ERR_CODE make_normal_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);
	dk_directdraw_renderer::ERR_CODE make_full_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);



	dk_directdraw_renderer::configuration_t * _config;

	bool					_enable;
	bool					_is_initialized;
	HINSTANCE				_library;
	HWND					_draw;

	LPDIRECTDRAW7			_pdd;
	LPDIRECTDRAWSURFACE7	_pdd_primary;
	LPDIRECTDRAWSURFACE7	_pdd_video;
	LPDIRECTDRAWSURFACE7	_pdd_rgb;
	LPDIRECTDRAWSURFACE7	_copy_video;

	LPDIRECTDRAWCLIPPER		_pdd_cliper;

	DWORD					_rgb_bitcount;
	DWORD					_out_rgb_bitcount;

	DWORD					_display_width;
	DWORD					_display_height;

	bool					_is_video_stretch;
	bool					_is_video_fullscreen;

	HWND					_noraml_screen_hwnd;
	HWND					_full_screen_hwnd;

	bool _enable_time_text;
	wchar_t _time[100];
	COLORREF _time_text_shadow_color;
	uint16_t _time_text_position_x;
	uint16_t _time_text_position_y;
	uint8_t _time_text_font_size;
	uint8_t _time_text_color_red;
	uint8_t _time_text_color_green;
	uint8_t _time_text_color_blue;

	bool _enable_osd_text;
	wchar_t _osd[100];
	COLORREF _osd_text_shadow_color;
	uint16_t _osd_text_position_x;
	uint16_t _osd_text_position_y;
	uint8_t _osd_text_font_size;
	uint8_t _osd_text_color_red;
	uint8_t _osd_text_color_green;
	uint8_t _osd_text_color_blue;
	uint8_t _background_red;
	uint8_t _background_green;
	uint8_t _background_blue;
};











#endif