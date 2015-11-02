#pragma once
#include <vector>
#include <mfidl.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <evr.h>

class dk_dshow_helper
{
public:
	typedef struct _capture_device_info_t
	{
		wchar_t friendly_name[MAX_PATH];
		wchar_t description[MAX_PATH];
		long wave_id;
		wchar_t device_path[MAX_PATH];
		_capture_device_info_t(void);
		_capture_device_info_t(_capture_device_info_t const & clone);
		_capture_device_info_t operator=(_capture_device_info_t const & clone);
	} capture_device_info_t;

	static HRESULT get_pin(IBaseFilter * filter, PIN_DIRECTION direction, IPin ** pin);
	static HRESULT get_pin(IBaseFilter * filter, LPCWSTR name, IPin ** pin);
	static HRESULT add_to_rot(IUnknown * graph_unknown, DWORD * rot_id);
	static HRESULT remove_from_rot(DWORD * rot_id);
	static HRESULT get_next_filter(IBaseFilter * filter, PIN_DIRECTION pin_direction, IBaseFilter ** next);
	static HRESULT remove_filter_chain(IGraphBuilder * graph, IBaseFilter * filter, IBaseFilter * stop_filter);
	static HRESULT add_filter_by_clsid(IGraphBuilder * graph, LPCWSTR name, const GUID& clsid, IBaseFilter ** filter);
	static HRESULT get_filter_by_clsid(const GUID & clsid, IBaseFilter ** filter);
	static IBaseFilter * create_capture_filter_by_name(const WCHAR * capture_device_name, const GUID & category);
	static HRESULT retreive_capture_device(std::vector<capture_device_info_t> & device, const GUID & category);
	static HRESULT build_capture_graph(IGraphBuilder * graph, const WCHAR * capture_device_name);

	static HRESULT remove_unconnected_renderer(IGraphBuilder * graph, IBaseFilter * renderer, BOOL * removed);

	static HRESULT is_pin_connected(IPin * ppin, BOOL * result);
	static HRESULT is_pin_direction(IPin * ppin, PIN_DIRECTION dir, BOOL * result);
	static HRESULT find_connected_pin(IBaseFilter * filter, PIN_DIRECTION dir, IPin ** pppin);

	static HRESULT initialize_evr(IBaseFilter * evr, HWND hwnd, IMFVideoDisplayControl ** ppwc);
	static HRESULT init_windowless_vmr9(IBaseFilter * vmr, HWND hwnd, IVMRWindowlessControl9 ** ppwc);
	static HRESULT init_windowless_vmr(IBaseFilter * vmr, HWND hwnd, IVMRWindowlessControl ** ppwc);

private:
	dk_dshow_helper(void);
	~dk_dshow_helper(void);
};


template <class T>
void safe_release(T ** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}