#ifndef _DK_DDRAW_VIDEO_RENDERER_H_
#define _DK_DDRAW_VIDEO_RENDERER_H_

#if defined(WIN32)
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#define EXP_CLASS
#endif

#include <cstdint>
#include <ddraw.h>

typedef struct _dk_render_entity_t
{
	uint8_t * data;
	size_t data_size;
	int32_t year;
	int32_t month;
	int32_t day;
	int32_t hour;
	int32_t minute;
	int32_t second;
	int64_t timestamp;
} dk_render_entity_t;

class EXP_CLASS dk_ddraw_video_renderer
{
public:

	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;


	typedef struct EXP_CLASS _configuration_t
	{
		int32_t width;
		int32_t height;
		HWND full_hwnd;
		HWND normal_hwnd;
		_configuration_t(void)
			: width(0)
			, height(0)
			, full_hwnd(NULL)
			, normal_hwnd(NULL)
		{}

		_configuration_t(const _configuration_t & clone)
		{
			width = clone.width;
			height = clone.height;
			full_hwnd = clone.full_hwnd;
			normal_hwnd = clone.normal_hwnd;
		}

		_configuration_t & operator=(const _configuration_t & clone)
		{
			width = clone.width;
			height = clone.height;
			full_hwnd = clone.full_hwnd;
			normal_hwnd = clone.normal_hwnd;
			return (*this);
		}
	} configuration_t;

	dk_ddraw_video_renderer(void);
	~dk_ddraw_video_renderer(void);

	bool is_initialized(void);

	ERR_CODE initialize_renderer(configuration_t * config);
	ERR_CODE release_renderer(void);
	ERR_CODE render(dk_render_entity_t * p);

	ERR_CODE open(void);
	ERR_CODE close(void);

private:
	ERR_CODE create(HWND hwnd, DWORD width, DWORD height, DWORD pos_x, DWORD pos_y, DWORD *rgb_bitcount = 0);
	ERR_CODE destroy(void);


	ERR_CODE enable_time_text(bool enable = true);
	ERR_CODE set_time_text_position(unsigned short x, unsigned short y);
	ERR_CODE set_time_text_font_size(unsigned char size);
	ERR_CODE set_time_text_color(unsigned char red, unsigned char green, unsigned char blue);

	ERR_CODE enable_osd_text(bool enable = true);
	ERR_CODE set_osd_text(wchar_t *osd);
	ERR_CODE set_osd_text_position(unsigned short x, unsigned short y);
	ERR_CODE set_osd_text_font_size(unsigned char size);
	ERR_CODE set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue);

	ERR_CODE set_background_color(unsigned char red, unsigned char green, unsigned char blue);

	ERR_CODE set_normal_screen_handle(HWND hwnd);
	ERR_CODE set_full_screen_handle(HWND hwnd);

	ERR_CODE enable_full_screen(bool enable);
	ERR_CODE enable_stretch(bool enable);

	ERR_CODE set_enable(bool enable);
	bool get_enable(void);

	/*
	HWND			get_parent_window( LPVMS_SUBMEDIA_ELEMENT_T subpe );
	HWND			get_parent_full_screen_window( LPVMS_SUBMEDIA_ELEMENT_T subpe );
	*/

	ERR_CODE make_normal_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);
	ERR_CODE make_full_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);



	configuration_t * _config;

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