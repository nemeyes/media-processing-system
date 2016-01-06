#ifndef _DK_DIRECTDRAW_RENDERER_H_
#define _DK_DIRECTDRAW_RENDERER_H_

#include <dk_video_base.h>

class directdraw_renderer;
class EXP_CLASS dk_directdraw_renderer : public dk_video_renderer
{
public:
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

	dk_directdraw_renderer(void);
	~dk_directdraw_renderer(void);

	dk_directdraw_renderer::ERR_CODE initialize_renderer(void * config);
	dk_directdraw_renderer::ERR_CODE release_renderer(void);
	dk_directdraw_renderer::ERR_CODE render(dk_video_entity_t * decoded);
	
private:
	directdraw_renderer * _core;

};


#endif