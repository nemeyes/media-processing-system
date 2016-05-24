#ifndef _BITMAP_RENDERER_H_
#define _BITMAP_RENDERER_H_

#include "dk_bitmap_renderer.h"

namespace debuggerking
{
	class bitmap_core
	{
	public:
		bitmap_core(bitmap_renderer * front);
		~bitmap_core(void);

		int32_t initialize_renderer(bitmap_renderer::configuration_t * config);
		int32_t release_renderer(void);
		int32_t render(bitmap_renderer::entity_t * decoded);

		int32_t enable_osd_text(bool enable = true);
		int32_t set_osd_text(wchar_t *osd);
		int32_t set_osd_text_position(unsigned short x, unsigned short y);
		int32_t set_osd_text_font_size(unsigned char size);
		int32_t set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue);


	private:
		int32_t make_normal_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);
		int32_t make_full_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);

	private:
		bitmap_renderer * _front;
		bitmap_renderer::configuration_t * _config;

		uint8_t * _buffer;
		BITMAPINFO * _bitmap_info;
		BITMAPINFOHEADER * _bitmap_info_hdr;
		uint8_t * _bitmap_buffer;

		bool _enable_osd_text;
		wchar_t _osd[100];
		COLORREF _osd_text_shadow_color;
		uint16_t _osd_text_position_x;
		uint16_t _osd_text_position_y;
		uint8_t _osd_text_font_size;
		uint8_t _osd_text_color_red;
		uint8_t _osd_text_color_green;
		uint8_t _osd_text_color_blue;


	};
};

#endif