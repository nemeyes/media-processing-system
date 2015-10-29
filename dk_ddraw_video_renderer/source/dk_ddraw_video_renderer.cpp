#include <tchar.h>
#include <cmath>
#include "dk_ddraw_video_renderer.h"

#define DDSAFE_RELEASE(x) if(x) { x->Release(); x=0; }
typedef HRESULT(WINAPI * DIRECTDRAWCREATE)(GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT(WINAPI * DIRECTDRAWCREATEEX)(GUID*, void**, REFIID, IUnknown*);

#define banollim(x,dig) (floor(float(x)*pow(10.0f,float(dig))+0.5f)/pow(10.0f,float(dig)))
#define _RGB16BIT565(r,g,b) ((b%32) + ((g%64) << 6) + ((r%32) << 11)) 

dk_ddraw_video_renderer::dk_ddraw_video_renderer(void)
	: _draw(0)
	, _display_width(0)
	, _display_height(0)
	, _pdd(0)
	, _pdd_primary(0)
	, _pdd_video(0)
	, _pdd_cliper(0)
	, _pdd_rgb(0)
	, _is_video_stretch(true)
	, _is_video_fullscreen(false)
	, _library(0)
	, _enable_time_text(true)
	, _time_text_position_x(5)
	, _time_text_position_y(0)
	, _time_text_font_size(15)
	, _time_text_color_red(0)
	, _time_text_color_green(0)
	, _time_text_color_blue(0)
	, _background_red(0)
	, _background_green(0)
	, _background_blue(0)
	, _enable_osd_text(true)
	, _osd_text_position_x(0)
	, _osd_text_position_y(20)
	, _osd_text_font_size(20)
	, _osd_text_color_red(0)
	, _osd_text_color_green(0)
	, _osd_text_color_blue(0)
	, _enable(false)
	, _is_initialized(false)
{
	_time_text_shadow_color = RGB(0, 0, 0);
	_osd_text_shadow_color = RGB(0, 0, 0);
	wcscpy_s(_osd, _T(""));
}

dk_ddraw_video_renderer::~dk_ddraw_video_renderer(void)
{

}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::enable_time_text(bool enable)
{
	_enable_time_text = enable;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_time_text_position(unsigned short x, unsigned short y)
{
	_time_text_position_x = x;
	_time_text_position_y = y;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_time_text_font_size(unsigned char size)
{
	_time_text_font_size = size;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_time_text_color(unsigned char red, unsigned char green, unsigned char blue)
{
	_time_text_color_red = red;
	_time_text_color_green = green;
	_time_text_color_blue = blue;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::enable_osd_text(bool enable)
{
	_enable_osd_text = enable;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_osd_text(wchar_t * osd)
{
	wcscpy_s(_osd, osd);
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_osd_text_position(unsigned short x, unsigned short y)
{
	_osd_text_position_x = x;
	_osd_text_position_y = y;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_osd_text_font_size(unsigned char size)
{
	_osd_text_font_size = size;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue)
{
	_osd_text_color_red = red;
	_osd_text_color_green = green;
	_osd_text_color_blue = blue;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_background_color(unsigned char red, unsigned char green, unsigned char blue)
{
	_background_red = red;
	_background_green = green;
	_background_blue = blue;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_normal_screen_handle(HWND hwnd)
{
	_noraml_screen_hwnd = hwnd;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_full_screen_handle(HWND hwnd)
{
	_full_screen_hwnd = hwnd;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::enable_full_screen(bool enable)
{
	_is_video_fullscreen = enable;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::enable_stretch(bool enable)
{
	_is_video_stretch = enable;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::set_enable(bool enable)
{
	_enable = enable;
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

bool dk_ddraw_video_renderer::get_enable(void)
{
	return _enable;
}

bool dk_ddraw_video_renderer::is_initialized(void)
{
	return _is_initialized;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::initialize_renderer(configuration_t * config)
{
	_config = config;
	_full_screen_hwnd = _config->full_hwnd;
	_noraml_screen_hwnd = _config->normal_hwnd;

	dk_ddraw_video_renderer::ERR_CODE value = dk_ddraw_video_renderer::ERR_CODE_FAILED;
	HWND hwnd = 0;

	if (_config->height>0 && _config->width>0)
	{
		if (_is_video_fullscreen)
			hwnd = _full_screen_hwnd;
		else
			hwnd = _noraml_screen_hwnd;
		if (hwnd)
			value = open();
	}
	_is_initialized = true;
	return value;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::release_renderer(void)
{
	return close();
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::open(void)
{
	HRESULT hr = NO_ERROR;
	dk_ddraw_video_renderer::ERR_CODE value = dk_ddraw_video_renderer::ERR_CODE_FAILED;
	HWND hwnd = 0;


	DDSURFACEDESC2 ddsd;
	//DDPIXELFORMAT ddpfBltFormat = { sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y', 'U', 'Y', '2'), 0, 0, 0, 0, 0 }; // YUY2
	DDPIXELFORMAT ddpfFormats = { sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0 };
	//close( subpe );
	if (_library == 0)
	{
		_library = ::LoadLibrary(_T("DDRAW.DLL"));
	}

	if (_is_video_fullscreen)
		hwnd = _full_screen_hwnd;
	else
		hwnd = _noraml_screen_hwnd;

	if (hwnd)
	{
		DIRECTDRAWCREATEEX DirectDrawCreateEx = (DIRECTDRAWCREATEEX)GetProcAddress(_library, "DirectDrawCreateEx");
		if (DirectDrawCreateEx(0, (void**)&_pdd, IID_IDirectDraw7, 0) == DD_OK)
		{

			if ((hr = _pdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL)) == DD_OK)
			{
				ZeroMemory(&ddsd, sizeof(ddsd));
				ddsd.dwSize = sizeof(ddsd);
				ddsd.dwFlags = DDSD_CAPS;
				ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

				if (_pdd->CreateSurface(&ddsd, &_pdd_primary, 0) == DD_OK)
				{
					_pdd->CreateClipper(0, &_pdd_cliper, 0);
					_pdd_cliper->SetHWnd(0, hwnd);
					_pdd_primary->SetClipper(_pdd_cliper);

					ZeroMemory(&ddsd, sizeof(ddsd));
					ddsd.dwSize = sizeof(ddsd);
					ddsd.dwFlags = DDSD_CAPS;
					ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
					_pdd->GetDisplayMode(&ddsd);

					// YUV모드 생성시도에는 16비트로 설정함.
					_rgb_bitcount = ddsd.ddpfPixelFormat.dwRGBBitCount;
					_out_rgb_bitcount = _rgb_bitcount;

					// rgb555 포맷일 경우에는 pRGBBitCount의 값을 15로 리턴하도록 하자..
					if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
					{
						if ((ddsd.ddpfPixelFormat.dwRBitMask == 0x07c0) &&
							(ddsd.ddpfPixelFormat.dwGBitMask == 0x03e0) &&
							(ddsd.ddpfPixelFormat.dwBBitMask == 0x001f))
						{
							_out_rgb_bitcount = _rgb_bitcount;
						}
					}

					// create offscreen
					ZeroMemory(&ddsd, sizeof(ddsd));
					ddsd.dwSize = sizeof(ddsd);
					ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
					ddsd.dwWidth = _config->width;
					ddsd.dwHeight = _config->height;
					ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
					memcpy(&ddsd.ddpfPixelFormat, &ddpfFormats, sizeof(DDPIXELFORMAT));


					// 비디오 메모리에 offscreen만들기 시도, 실패하면 시스템 메모리에 만든다...
					// 단, yuv는 비디오 메모리에만 만들어진다...
					hr = _pdd->CreateSurface(&ddsd, &_pdd_video, 0);
					if (hr == DDERR_OUTOFVIDEOMEMORY)
					{
						ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
						hr = _pdd->CreateSurface(&ddsd, &_pdd_video, 0);
					}
					else
					{
						_draw = hwnd;
						value = dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
					}
				}
			}
		}
	}
	if (value != dk_ddraw_video_renderer::ERR_CODE_SUCCESS)
	{
		close();
	}
	return value;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::close(void)
{
	DDSAFE_RELEASE(_pdd_rgb)
	DDSAFE_RELEASE(_pdd_video)
	DDSAFE_RELEASE(_pdd_cliper)
	DDSAFE_RELEASE(_pdd_primary)
	DDSAFE_RELEASE(_pdd)
	_display_width = 0;
	_display_height = 0;

	if (_library)
	{
		::FreeLibrary(_library);
		_library = 0;
	}
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::render(dk_render_entity_t * p)
{
	dk_ddraw_video_renderer::ERR_CODE value = dk_ddraw_video_renderer::ERR_CODE_FAILED;
	int32_t display_width = 0, display_height = 0, display_x = 0, display_y = 0;

	if ((_pdd) && (_config->height>0) && (_config->width>0))
	{
		/*
		if( _is_video_fullscreen!=subpe->is_video_fullscreen )
		{
		close( subpe );
		open( subpe );
		if( subpe->device_context )
		{
		render_video_device_interface *device_context = static_cast<render_video_device_interface*>( subpe->device_context );

		if( subpe->is_video_fullscreen )
		device_context->active_fullscreen();
		else
		device_context->deactive_fullscreen();
		}
		}
		*/
		if (_is_video_fullscreen)
			value = make_full_screen_display_size(display_width, display_height, display_x, display_y);
		else
			value = make_normal_screen_display_size(display_width, display_height, display_x, display_y);

		if (value == dk_ddraw_video_renderer::ERR_CODE_SUCCESS)
		{
			if ((!_pdd_rgb) && ((_display_width != display_width) || (_display_height != display_height)))
			{
				DDSAFE_RELEASE(_pdd_rgb);

				HRESULT			hr;
				DDSURFACEDESC2	ddsd;
				ZeroMemory(&ddsd, sizeof(ddsd));

				ddsd.dwSize = sizeof(ddsd);
				ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
				ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
				ddsd.dwWidth = display_width;
				ddsd.dwHeight = display_height;
				hr = _pdd->CreateSurface(&ddsd, &_pdd_rgb, 0);

				if (hr == DDERR_OUTOFVIDEOMEMORY)
				{
					ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
					hr = _pdd->CreateSurface(&ddsd, &_pdd_rgb, 0);
				}

				if (hr != DD_OK) 
					return dk_ddraw_video_renderer::ERR_CODE_FAILED;

				_display_width = display_width;
				_display_height = display_height;
			}

			DDSURFACEDESC2 ddsd;
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);

			RECT rtDisplay = { display_x, display_y, display_width + display_x, display_height + display_y };

			POINT pt = { 0, 0 };
			::ClientToScreen(_draw, &pt);
			OffsetRect(&rtDisplay, pt.x, pt.y); //현재 창의 스크린좌표로 이동

			uint8_t *src, *dst;
			src = p->data;

			if (_pdd_video->Lock(0, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, 0) != DD_OK)
			{
				_pdd_video->Restore();
				_pdd_rgb->Restore();
				_pdd_primary->Restore();

				_pdd_video->Unlock(0);
				value = dk_ddraw_video_renderer::ERR_CODE_FAILED;
			}
			else
			{
				dst = (unsigned char*)ddsd.lpSurface;

				size_t nStep;
				if (_rgb_bitcount == 16)		
					nStep = _config->width << 1;
				else if (_rgb_bitcount == 24)	
					nStep = _config->width * 3;
				else if (_rgb_bitcount == 32)	
					nStep = _config->width << 2;

				__try
				{
					for (unsigned int i = 0; i<_config->height; i++)
					{
						CopyMemory(dst, src, nStep);
						src += nStep;
						dst += ddsd.lPitch;
					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					_pdd_video->Unlock(0);
					return dk_ddraw_video_renderer::ERR_CODE_FAILED;
				}

				_pdd_video->Unlock(0);
				HRESULT hr;
				if (!_pdd_primary)
					return dk_ddraw_video_renderer::ERR_CODE_FAILED;
				if (!_pdd_rgb)
					return dk_ddraw_video_renderer::ERR_CODE_FAILED;
				if (!_pdd_video)
					return dk_ddraw_video_renderer::ERR_CODE_FAILED;

				while (_pdd_rgb->Blt(0, _pdd_video, 0, DDBLT_WAIT, 0) == DDERR_SURFACEBUSY)
					Sleep(0);

				if (_enable_time_text)
				{
					HDC hdc;
					RECT rtRGB = { _time_text_position_x, _time_text_position_y, display_width, display_height };
					RECT rtShadow = { _time_text_position_x + 1, _time_text_position_y + 1, display_width, display_height };
					if (SUCCEEDED(_pdd_rgb->GetDC(&hdc)))
					{
						::SetBkMode(hdc, TRANSPARENT);

						::SetTextColor(hdc, _time_text_shadow_color);
						_sntprintf_s(_time, sizeof(_time), _T("%.4d/%.2d/%.2d %.2d:%.2d:%.2d"),
							p->year,
							p->month,
							p->day,
							p->hour,
							p->minute,
							p->second);
						HFONT oFont;
						HFONT time_text_font = CreateFont(_time_text_font_size, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, TEXT("Times New Roman"));
						oFont = (HFONT)SelectObject(hdc, time_text_font);

						/*
						if( _is_video_fullscreen )
						oFont = (HFONT)SelectObject( hdc, _hl_font );
						else
						oFont = (HFONT)SelectObject( hdc, _hs_font );
						*/

						::DrawText(hdc, _time, _tcslen(_time), &rtShadow, DT_LEFT | DT_EDITCONTROL);
						::SetTextColor(hdc, RGB(_time_text_color_red, _time_text_color_red, _time_text_color_blue));

						::DrawText(hdc, _time, _tcslen(_time), &rtRGB, DT_LEFT | DT_EDITCONTROL);
						(HFONT)SelectObject(hdc, oFont);

						if (time_text_font)
							DeleteObject(time_text_font);

						_pdd_rgb->ReleaseDC(hdc);
					}
				}

				if (_enable_osd_text)
				{
					HDC hdc;
					RECT rtRGB = { _osd_text_position_x, _osd_text_position_y, display_width, display_height };
					RECT rtShadow = { _osd_text_position_x + 1, _osd_text_position_y + 1, display_width, display_height };
					if (SUCCEEDED(_pdd_rgb->GetDC(&hdc)))
					{
						::SetBkMode(hdc, TRANSPARENT);
						::SetTextColor(hdc, _osd_text_shadow_color);

						HFONT oFont;
						HFONT osd_text_font = CreateFont(_osd_text_font_size, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, TEXT("Times New Roman"));
						oFont = (HFONT)SelectObject(hdc, osd_text_font);

						::DrawText(hdc, _osd, _tcslen(_osd), &rtShadow, DT_LEFT | DT_EDITCONTROL);
						::SetTextColor(hdc, RGB(_osd_text_color_red, _osd_text_color_red, _osd_text_color_blue));

						::DrawText(hdc, _osd, _tcslen(_osd), &rtRGB, DT_LEFT | DT_EDITCONTROL);
						(HFONT)SelectObject(hdc, oFont);

						if (osd_text_font)
							DeleteObject(osd_text_font);

						_pdd_rgb->ReleaseDC(hdc);
					}
				}

				do
				{
					DDBLTFX  ddbltfx;
					ddbltfx.dwSize = sizeof(ddbltfx);
					ddbltfx.dwFillColor = _RGB16BIT565(_background_red, _background_green, _background_blue);
					hr = _pdd_primary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);
					if (hr == DDERR_SURFACELOST)
					{
						hr = _pdd_video->Restore();
						hr = _pdd_rgb->Restore();
						hr = _pdd_primary->Restore();
						return dk_ddraw_video_renderer::ERR_CODE_FAILED;
					}
				} while (hr == DDERR_SURFACEBUSY);

				do
				{
					hr = _pdd_primary->Blt(&rtDisplay, _pdd_rgb, 0, DDBLT_WAIT, 0);
					if (hr == DDERR_SURFACELOST)
					{
						hr = _pdd_video->Restore();
						hr = _pdd_rgb->Restore();
						hr = _pdd_primary->Restore();

						return dk_ddraw_video_renderer::ERR_CODE_FAILED;
					}

				} while (hr == DDERR_SURFACEBUSY);

				value = dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
			}
		}
	}
	return value;
}


dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::make_normal_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y)
{
	RECT dst_rect = { 0 };
	if (_noraml_screen_hwnd)
		::GetClientRect(_noraml_screen_hwnd, &dst_rect);
	else
		return dk_ddraw_video_renderer::ERR_CODE_FAILED;


	unsigned int iwidth = _config->width;
	unsigned int iheight = _config->height;
	unsigned int dwidth = dst_rect.right - dst_rect.left;
	unsigned int dheight = dst_rect.bottom - dst_rect.top;

	if (_is_video_stretch)
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
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}

dk_ddraw_video_renderer::ERR_CODE dk_ddraw_video_renderer::make_full_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y)
{
	RECT dst_rect;
	::GetClientRect(_full_screen_hwnd, &dst_rect);

	unsigned int iwidth = _config->width;
	unsigned int iheight = _config->height;
	unsigned int dwidth = dst_rect.right - dst_rect.left;
	unsigned int dheight = dst_rect.bottom - dst_rect.top;

	if (_is_video_stretch)
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
	return dk_ddraw_video_renderer::ERR_CODE_SUCCESS;
}