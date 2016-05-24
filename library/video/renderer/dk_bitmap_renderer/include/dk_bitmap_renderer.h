#ifndef _DK_BITMAP_RENDERER_H_
#define _DK_BITMAP_RENDERER_H_

#include <dk_video_base.h>
namespace debuggerking
{
	class bitmap_core;
	class EXP_CLASS bitmap_renderer : public video_renderer
	{
	public:
		bitmap_renderer(void);
		virtual ~bitmap_renderer(void);

		int32_t enable_osd_text(bool enable);
		int32_t set_osd_text(wchar_t * osd);
		int32_t set_osd_text_position(unsigned short x, unsigned short y);
		int32_t set_osd_text_font_size(unsigned char size);
		int32_t set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue);

		int32_t initialize_renderer(void * config);
		int32_t release_renderer(void);
		int32_t render(bitmap_renderer::entity_t * decoded);

	private:
		bitmap_core * _core;
	};
};

#endif