#include "bitmap_renderer.h"

#define banollim(x,dig) (floor(float(x)*pow(10.0f,float(dig))+0.5f)/pow(10.0f,float(dig)))

debuggerking::bitmap_core::bitmap_core(bitmap_renderer * front)
	: _front(front)
	, _bitmap_buffer(nullptr)
	, _enable_osd_text(false)
	, _osd_text_position_x(0)
	, _osd_text_position_y(0)
	, _osd_text_font_size(9)
	, _osd_text_color_red(0xFF)
	, _osd_text_color_blue(0xFF)
	, _osd_text_color_green(0xFF)
{
	wcscpy_s(_osd, L"What the Fuck!!");
}

debuggerking::bitmap_core::~bitmap_core(void)
{

}

int32_t debuggerking::bitmap_core::enable_osd_text(bool enable)
{
	_enable_osd_text = enable;
	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::set_osd_text(wchar_t *osd)
{
	//wcscpy_s(_osd, osd);
	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::set_osd_text_position(unsigned short x, unsigned short y)
{
	_osd_text_position_x = x;
	_osd_text_position_y = y;
	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::set_osd_text_font_size(unsigned char size)
{
	_osd_text_font_size = size;
	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue)
{
	_osd_text_color_red = red;
	_osd_text_color_green = green;
	_osd_text_color_blue = blue;
	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::initialize_renderer(bitmap_renderer::configuration_t * config)
{
	_config = config;

	_bitmap_buffer = new uint8_t[sizeof(BITMAPINFOHEADER) + 1024];
	_bitmap_info = (BITMAPINFO*)_bitmap_buffer;
	_bitmap_info_hdr = &(_bitmap_info->bmiHeader);
	memset(_bitmap_info_hdr, 0, sizeof(BITMAPINFOHEADER));
	_bitmap_info_hdr->biSize = sizeof(BITMAPINFOHEADER);
	_bitmap_info_hdr->biWidth = _config->width;
	_bitmap_info_hdr->biHeight = -abs(_config->height);
	_bitmap_info_hdr->biPlanes = 1;
	_bitmap_info_hdr->biBitCount = 32;
	_bitmap_info_hdr->biCompression = BI_RGB;
	_bitmap_info_hdr->biYPelsPerMeter = 0;
	_bitmap_info_hdr->biXPelsPerMeter = 0;
	_bitmap_info_hdr->biClrUsed = 0;
	_bitmap_info_hdr->biClrImportant = 0;

	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::release_renderer(void)
{
	if (_bitmap_buffer)
		delete _bitmap_buffer;
	_bitmap_buffer = nullptr;
	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::render(bitmap_renderer::entity_t * decoded)
{
	int32_t value = bitmap_renderer::err_code_t::fail;
	int32_t display_width = 0, display_height = 0, display_x = 0, display_y = 0;

	if ((_config->height > 0) && (_config->width > 0))
	{
		HDC dc = NULL;
		HFONT font = NULL;
		if (_config->hwnd_full)
		{
			value = make_full_screen_display_size(display_width, display_height, display_x, display_y);
			dc = ::GetDC(_config->hwnd_full);

		}
		else
		{
			value = make_normal_screen_display_size(display_width, display_height, display_x, display_y);
			dc = ::GetDC(_config->hwnd);
		}

		if (value == bitmap_renderer::err_code_t::success)
		{
			::SetStretchBltMode(dc, COLORONCOLOR);
			::StretchDIBits(dc, display_x, display_y, display_width, display_height, 0, 0, _config->width, _config->height, decoded->data, _bitmap_info, DIB_RGB_COLORS, SRCCOPY);
			::SetBkMode(dc, TRANSPARENT);

			DWORD colorref = 0;
			::SetTextColor(dc, colorref);
			font = ::CreateFont(_osd_text_font_size * 2, _osd_text_font_size, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, L"Times New Roman");
			::SelectObject(dc, font);
			::TextOut(dc, _osd_text_position_x+1, _osd_text_position_y+1, _osd, wcslen(_osd));

			colorref |= (_osd_text_color_red) << 16;
			colorref |= (_osd_text_color_blue) << 8;
			colorref |= (_osd_text_color_green);
			::SetTextColor(dc, colorref);
			font = ::CreateFont(_osd_text_font_size * 2, _osd_text_font_size, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, L"Times New Roman");
			::SelectObject(dc, font);
			::TextOut(dc, _osd_text_position_x, _osd_text_position_y, _osd, wcslen(_osd));
		}
		::DeleteObject(font);
		::ReleaseDC(_config->hwnd, dc);

	}
	return value;
}

int32_t debuggerking::bitmap_core::make_normal_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y)
{
	RECT dst_rect = { 0 };
	if (_config->hwnd)
		::GetClientRect(_config->hwnd, &dst_rect);
	else
		return bitmap_renderer::err_code_t::fail;


	unsigned int iwidth = _config->width;
	unsigned int iheight = _config->height;
	unsigned int dwidth = dst_rect.right - dst_rect.left;
	unsigned int dheight = dst_rect.bottom - dst_rect.top;

	if (_config->stretch)
	{
		display_width = dwidth;
		display_height = dheight;
		display_x = 0;
		display_y = 0;
	}
	else
	{
		float orgVideoRatio = (float)iwidth / (float)iheight;
		float disVideoRatio = (float)dwidth / (float)dheight;
		if (orgVideoRatio>disVideoRatio) // width larger than height
		{
			display_width = dwidth;
			display_height = static_cast<unsigned short>(banollim(dwidth / orgVideoRatio, 0));
			display_y = static_cast<unsigned short>(banollim((dheight - display_height) / 2., 0));
			display_x = 0;
		}
		else if (orgVideoRatio<disVideoRatio) //원본보다 Width가 넓은 경우
		{
			display_width = static_cast<unsigned short>(banollim(orgVideoRatio*dheight, 0));
			display_height = dheight;
			display_x = static_cast<unsigned short>(banollim((dwidth - display_width) / 2., 0));
			display_y = 0;
		}
		else
		{
			display_width = dwidth;
			display_height = dheight;
			display_x = 0;
			display_y = 0;
		}
	}
	return bitmap_renderer::err_code_t::success;
}

int32_t debuggerking::bitmap_core::make_full_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y)
{
	RECT dst_rect;
	::GetClientRect(_config->hwnd_full, &dst_rect);

	unsigned int iwidth = _config->width;
	unsigned int iheight = _config->height;
	unsigned int dwidth = dst_rect.right - dst_rect.left;
	unsigned int dheight = dst_rect.bottom - dst_rect.top;

	if (_config->stretch)
	{
		display_width = dwidth;
		display_height = dheight;
		display_x = 0;
		display_y = 0;
	}
	else
	{
		float orgVideoRatio = (float)iwidth / (float)iheight;
		float disVideoRatio = (float)dwidth / (float)dheight;
		if (orgVideoRatio>disVideoRatio) // width larger than height
		{
			display_width = dwidth;
			display_height = static_cast<unsigned short>(banollim(dwidth / orgVideoRatio, 0));
			display_y = static_cast<unsigned short>(banollim((dheight - display_height) / 2., 0));
			display_x = 0;
		}
		else if (orgVideoRatio<disVideoRatio) //원본보다 Width가 넓은 경우
		{
			display_width = static_cast<unsigned short>(banollim(orgVideoRatio*dheight, 0));
			display_height = dheight;
			display_x = static_cast<unsigned short>(banollim((dwidth - display_width) / 2., 0));
			display_y = 0;
		}
		else
		{
			display_width = dwidth;
			display_height = dheight;
			display_x = 0;
			display_y = 0;
		}
	}
	return bitmap_renderer::err_code_t::success;
}