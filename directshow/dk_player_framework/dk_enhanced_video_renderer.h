#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <mfidl.h>	
#include <vmr9.h>
#include <evr.h>

#include "dk_base_filter.h"

/*#define DXVA2_ModeH264_MoComp_NoFGT DXVA2_ModeH264_A
#define DXVA2_ModeH264_MoComp_FGT   DXVA2_ModeH264_B
#define DXVA2_ModeH264_IDCT_NoFGT   DXVA2_ModeH264_C
#define DXVA2_ModeH264_IDCT_FGT     DXVA2_ModeH264_D
#define DXVA2_ModeH264_VLD_NoFGT    DXVA2_ModeH264_E
#define DXVA2_ModeH264_VLD_FGT      DXVA2_ModeH264_F*/

class dk_enhanced_video_renderer : public dk_base_video_render_filter
{
public:
	dk_enhanced_video_renderer(void);
	virtual ~dk_enhanced_video_renderer(void);

	CComPtr<IBaseFilter> get_filter(void);
	CComPtr<IPin> get_output_pin(void);
	CComPtr<IPin> get_input_pin(void);

	void aspect_ratio(bool enable);
	void fullscreen(bool enable);
	void list_dxva2_decoder_guids(std::vector<GUID> * guids);

	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, HWND hwnd, bool aspect_ratio);

	HRESULT update_video_window(HWND hwnd, const LPRECT rect);
	HRESULT repaint(HWND hwnd, HDC hdc);
	HRESULT on_change_displaymode(void);


	//HRESULT list_dxva2_mode(std::vector<ATL::CString> dxva2modes);

private:
	CComPtr<IBaseFilter> _renderer;
	CComPtr<IMFVideoDisplayControl> _display;

	HWND _hwnd;
	RECT _original_rect;
	bool _fullscreen;

	/*CComPtr<IMFGetService> _mf_get_service;
	CComPtr<IDirect3DDeviceManager9> _d3d_device_manager;
	CComPtr<IDirectXVideoDecoderService> _dxva_service;
	DXVA2_VideoDesc _dxva2_description;*/
};