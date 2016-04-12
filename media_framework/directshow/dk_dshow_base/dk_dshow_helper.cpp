#include <atlbase.h>
#include <initguid.h>
#include <atlconv.h>
#include <dshow.h>
#include <tchar.h>
#include "dk_dshow_helper.h"

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4005) 
#pragma warning(pop)     

dk_dshow_helper::_capture_device_info_t::_capture_device_info_t(void)
{
	memset(friendly_name, 0x00, sizeof(WCHAR)*MAX_PATH);
	memset(description, 0x00, sizeof(WCHAR)*MAX_PATH);
	wave_id = 0;
	memset(device_path, 0x00, sizeof(WCHAR)*MAX_PATH);
}

dk_dshow_helper::_capture_device_info_t::_capture_device_info_t(_capture_device_info_t const & clone)
{
	wcscpy_s(friendly_name, clone.friendly_name);
	wcscpy_s(description, clone.description);
	wcscpy_s(device_path, clone.device_path);
	wave_id = clone.wave_id;
}

dk_dshow_helper::_capture_device_info_t dk_dshow_helper::_capture_device_info_t::operator = (_capture_device_info_t const & clone)
{
	wcscpy_s(friendly_name, clone.friendly_name);
	wcscpy_s(description, clone.description);
	wcscpy_s(device_path, clone.device_path);
	wave_id = clone.wave_id;
	return (*this);
}

HRESULT dk_dshow_helper::get_pin(IBaseFilter * filter, PIN_DIRECTION direction, IPin ** pin)
{
	HRESULT		hr;
	IEnumPins  *pin_enum;
	IPin		*ppin;
	// Create a pin enumerator
	if (FAILED(filter->EnumPins(&pin_enum)))
		return E_FAIL;
	hr = pin_enum->Next(1, &ppin, 0);
	while (hr == S_OK)
	{
		PIN_DIRECTION my_direction;
		hr = ppin->QueryDirection(&my_direction);
		if (hr == S_OK && (direction == my_direction))
		{
			*pin = ppin;
			break;
		}
		safe_release(&ppin);
	}
	safe_release(&pin_enum);
	return hr;
}

HRESULT dk_dshow_helper::get_pin(IBaseFilter * filter, LPCWSTR name, IPin ** pin)
{
	HRESULT		hr = E_FAIL;
	IEnumPins*	pin_enum = NULL;
	IPin*		ppin = NULL;
	DWORD		fetched = 0;
	PIN_INFO	pin_info = { 0 };

	// Create a pin enumerator
	if (FAILED(filter->EnumPins(&pin_enum)))
		return E_FAIL;

	// Get the first instance
	hr = pin_enum->Next(1, &ppin, &fetched);
	while (hr == S_OK)
	{
		ppin->QueryPinInfo(&pin_info);
		if (!wcscmp(name, pin_info.achName))
		{
			// pin names match so use this one and exit
			*pin = ppin;
			break;
		}
		safe_release(&pin_info.pFilter);
		safe_release(&ppin);

		hr = pin_enum->Next(1, &ppin, &fetched);
	}

	safe_release(&pin_info.pFilter);
	safe_release(&pin_enum);

	// if the pPin address is null we didnt find a pin with the wanted name
	if (&*ppin == NULL)
		hr = VFW_E_NOT_FOUND;
	return hr;
}


HRESULT dk_dshow_helper::get_pin(IBaseFilter * filter, PIN_DIRECTION direction, GUID major_type, IPin ** pin)
{
	HRESULT		hr = E_FAIL;
	IEnumPins*	pin_enum = NULL;
	IPin*		ppin = NULL;
	DWORD		fetched = 0;
	PIN_INFO	pin_info = { 0 };

	// Create a pin enumerator
	if (FAILED(filter->EnumPins(&pin_enum)))
		return E_FAIL;

	// Get the first instance
	*pin = NULL;
	hr = pin_enum->Next(1, &ppin, &fetched);
	while (hr == S_OK)
	{
		ppin->QueryPinInfo(&pin_info);
		if (pin_info.dir == direction)
		{
			IEnumMediaTypes * media_enum = 0;
			hr = ppin->EnumMediaTypes(&media_enum);
			AM_MEDIA_TYPE * mt;
			while (S_OK == media_enum->Next(1, &mt, NULL))
			{
				if (IsEqualGUID(mt->majortype, major_type))
				{
					*pin = ppin;
					break;
				}
			}
			safe_release(&media_enum);

			if (*pin)
				break;
		}
		safe_release(&pin_info.pFilter);
		safe_release(&ppin);

		hr = pin_enum->Next(1, &ppin, &fetched);
	}

	safe_release(&pin_info.pFilter);
	safe_release(&pin_enum);

	// if the pPin address is null we didnt find a pin with the wanted name
	if (&*ppin == NULL)
		hr = VFW_E_NOT_FOUND;
	return hr;
}

/*CComPtr<IPin> vopin = _source->get_video_output_pin();
IEnumMediaTypes * venum = 0;
hr = vopin->EnumMediaTypes(&venum);
if (FAILED(hr))
return dk_player_framework::ERR_CODE_FAILED;
AM_MEDIA_TYPE * vmt;
while (S_OK == venum->Next(1, &vmt, NULL))
{
	if (IsEqualGUID(vmt->subtype, MEDIASUBTYPE_H264) ||
		IsEqualGUID(vmt->subtype, MEDIASUBTYPE_AVC1) ||
		IsEqualGUID(vmt->subtype, MEDIASUBTYPE_avc1) ||
		IsEqualGUID(vmt->subtype, MEDIASUBTYPE_h264) ||
		IsEqualGUID(vmt->subtype, MEDIASUBTYPE_X264) ||
		IsEqualGUID(vmt->subtype, MEDIASUBTYPE_x264))
	{
		_video_decoder = new dk_microsoft_video_decoder();
		break;
	}
	else if (IsEqualGUID(vmt->subtype, MEDIASUBTYPE_MP4V) ||
		IsEqualGUID(vmt->subtype, MEDIASUBTYPE_XVID))
	{
		_video_decoder = new dk_dmo_mpeg4s_decoder();
		break;
	}
}
safe_release(&venum);*/


HRESULT dk_dshow_helper::add_to_rot(IUnknown * graph_unknown, DWORD * rot_id)
{
	if (!graph_unknown || !rot_id)
		return E_INVALIDARG;

	IMoniker * pmoniker;
	IRunningObjectTable *prot;
	if (FAILED(GetRunningObjectTable(0, &prot)))
		return E_FAIL;

	WCHAR wsz[256];
	wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)graph_unknown, GetCurrentProcessId());
	HRESULT hr = CreateItemMoniker(L"!", wsz, &pmoniker);
	if (SUCCEEDED(hr))
	{
		hr = prot->Register(0, graph_unknown, pmoniker, rot_id);
		safe_release(&pmoniker);
	}
	safe_release(&prot);
	return hr;
}

HRESULT dk_dshow_helper::remove_from_rot(DWORD *rot_id)
{
	if (!rot_id)
		return E_INVALIDARG;
	if ((*rot_id) != -1)
	{
		IRunningObjectTable *prot;
		if (SUCCEEDED(GetRunningObjectTable(0, &prot)))
		{
			prot->Revoke((*rot_id));
			(*rot_id) = -1;
			safe_release(&prot);
		}
		return S_OK;
	}
	return E_FAIL;
}

HRESULT dk_dshow_helper::get_next_filter(IBaseFilter * filter, PIN_DIRECTION pin_direction, IBaseFilter ** next)
{
	if (!filter || !next)
		return E_POINTER;

	IEnumPins *pin_enum = 0;
	IPin *ppin = 0;
	HRESULT hr = filter->EnumPins(&pin_enum);
	if (FAILED(hr))
		return hr;
	while (S_OK == pin_enum->Next(1, &ppin, 0))
	{
		// See if this pin matches the specified direction.
		PIN_DIRECTION my_pin_direction;
		hr = ppin->QueryDirection(&my_pin_direction);
		if (FAILED(hr))
		{
			// Something strange happened.
			hr = E_UNEXPECTED;
			ppin->Release();
			break;
		}
		if (my_pin_direction == pin_direction)
		{
			// Check if the pin is connected to another pin.
			IPin *ppin_next = 0;
			hr = ppin->ConnectedTo(&ppin_next);
			if (SUCCEEDED(hr))
			{
				// Get the filter that owns that pin.
				PIN_INFO pin_info;
				hr = ppin_next->QueryPinInfo(&pin_info);
				safe_release(&ppin_next);
				safe_release(&ppin);
				safe_release(&pin_enum);
				if (FAILED(hr) || (pin_info.pFilter == NULL))
				{
					// Something strange happened.
					return E_UNEXPECTED;
				}
				// This is the filter we're looking for.
				*next = pin_info.pFilter; // Client must release.
				return S_OK;
			}
		}
		safe_release(&ppin);
	}
	safe_release(&pin_enum);
	// Did not find a matching filter.
	return E_FAIL;
}

HRESULT dk_dshow_helper::remove_filter_chain(IGraphBuilder * graph, IBaseFilter * filter, IBaseFilter * stop_filter)
{
	if (!graph || !filter || !stop_filter)
		return E_INVALIDARG;

	IBaseFilter* filter_found = NULL;
	HRESULT hr = get_next_filter(filter, PINDIR_OUTPUT, &filter_found);
	if (SUCCEEDED(hr) && filter_found != stop_filter)
	{
		hr = remove_filter_chain(graph, filter_found, stop_filter);
		safe_release(&filter_found);
	}
	hr = graph->RemoveFilter(filter);
	return hr;
}

HRESULT dk_dshow_helper::add_filter_by_clsid(IGraphBuilder * graph, LPCWSTR name, const GUID & clsid, IBaseFilter ** filter)
{
	if (!graph || !name || wcslen(name)<1)
		return E_INVALIDARG;

	*filter = NULL;
	HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)filter);
	if (SUCCEEDED(hr))
	{
		hr = graph->AddFilter((*filter), name);
	}
	return hr;
}

HRESULT dk_dshow_helper::get_filter_by_clsid(const GUID & clsid, IBaseFilter ** filter)
{
	*filter = NULL;
	HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)filter);
	return hr;
}

//filter category  : https://msdn.microsoft.com/ko-kr/library/windows/desktop/dd375655(v=vs.85).aspx
IBaseFilter * dk_dshow_helper::create_capture_filter_by_name(const WCHAR * capture_device_name, const GUID & category)
{
	USES_CONVERSION;
	IEnumMoniker * enum_moniker = NULL;
	ICreateDevEnum * dev_enum = NULL;
	IBaseFilter * capture_filter = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dev_enum));
	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = dev_enum->CreateClassEnumerator(category, &enum_moniker, 0);
		if (hr == S_OK)
		{
			IMoniker * moniker = NULL;
			ULONG fetched;
			while (enum_moniker->Next(1, &moniker, &fetched) == S_OK)
			{
				IPropertyBag * prop_bag;
				HRESULT hr = moniker->BindToStorage(0, 0, IID_PPV_ARGS(&prop_bag));
				if (SUCCEEDED(hr))
				{
					VARIANT var_name;
					VariantInit(&var_name);

					if (wcscmp(capture_device_name, var_name.bstrVal) == 0)
					{
						hr = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&capture_filter);
						if (FAILED(hr))
							capture_filter = NULL;
					}
					VariantClear(&var_name);
				}
				moniker->Release();
			}
		}
		dev_enum->Release();
	}
	return capture_filter;
}

//filter category  : https://msdn.microsoft.com/ko-kr/library/windows/desktop/dd375655(v=vs.85).aspx
HRESULT dk_dshow_helper::retreive_capture_device(std::vector<capture_device_info_t> & device, const GUID & category)
{
	USES_CONVERSION;
	IEnumMoniker * enum_moniker = NULL;
	ICreateDevEnum * dev_enum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dev_enum));
	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = dev_enum->CreateClassEnumerator(category, &enum_moniker, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		dev_enum->Release();
	}


	IMoniker * moniker = NULL;
	while (enum_moniker->Next(1, &moniker, NULL) == S_OK)
	{
		capture_device_info_t dev_info;

		IPropertyBag *prop_bag;
		HRESULT hr = moniker->BindToStorage(0, 0, IID_PPV_ARGS(&prop_bag));
		if (FAILED(hr))
		{
			moniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = prop_bag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = prop_bag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			int length = ::SysStringLen(var.bstrVal);
			if (length>0)
			{
				wcscpy_s(dev_info.friendly_name, var.bstrVal);
			}
			VariantClear(&var);
		}

		//hr = prop_bag->Write(L"FriendlyName", &var);

		// WaveInID applies only to audio capture devices.
		hr = prop_bag->Read(L"WaveInID", &var, 0);
		if (SUCCEEDED(hr))
		{
			dev_info.wave_id = var.lVal;
			//wprintf(L"WaveIn ID: %d\n", var.lVal);
			VariantClear(&var);
		}

		hr = prop_bag->Read(L"DevicePath", &var, 0);
		if (SUCCEEDED(hr))
		{
			// The device path is not intended for display.
			//wprintf(L"Device path: %s\n", var.bstrVal);
			int length = ::SysStringLen(var.bstrVal);
			if (length>0)
			{
				wcscpy_s(dev_info.device_path, var.bstrVal);
			}
			VariantClear(&var);
		}
		prop_bag->Release();
		moniker->Release();

		device.push_back(dev_info);
	}
	return hr;
}


HRESULT dk_dshow_helper::build_capture_graph(IGraphBuilder * graph, const WCHAR * capture_device_name)
{
	HRESULT hr = S_OK;

	ICaptureGraphBuilder2 * capture_graph = NULL;

	//// Create the Capture Graph Builder.
	//HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
	//	CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);

	return S_OK;
}

HRESULT dk_dshow_helper::remove_unconnected_renderer(IGraphBuilder * graph, IBaseFilter * renderer, BOOL * removed)
{
	IPin *ppin = NULL;
	*removed = FALSE;

	// Look for a connected input pin on the renderer.

	HRESULT hr = find_connected_pin(renderer, PINDIR_INPUT, &ppin);
	safe_release(&ppin);

	// If this function succeeds, the renderer is connected, so we don't remove it.
	// If it fails, it means the renderer is not connected to anything, so
	// we remove it.

	if (FAILED(hr))
	{
		hr = graph->RemoveFilter(renderer);
		*removed = TRUE;
	}
	return hr;
}

HRESULT dk_dshow_helper::is_pin_connected(IPin * ppin, BOOL * result)
{
	IPin * ptmp = NULL;
	HRESULT hr = ppin->ConnectedTo(&ptmp);
	if (SUCCEEDED(hr))
	{
		*result = TRUE;
	}
	else if (hr == VFW_E_NOT_CONNECTED)
	{
		// The pin is not connected. This is not an error for our purposes.
		*result = FALSE;
		hr = S_OK;
	}

	safe_release(&ptmp);
	return hr;
}

HRESULT dk_dshow_helper::is_pin_direction(IPin * ppin, PIN_DIRECTION dir, BOOL * result)
{
	PIN_DIRECTION pin_direction;
	HRESULT hr = ppin->QueryDirection(&pin_direction);
	if (SUCCEEDED(hr))
	{
		*result = (pin_direction == dir);
	}
	return hr;
}

HRESULT dk_dshow_helper::find_connected_pin(IBaseFilter * filter, PIN_DIRECTION dir, IPin ** pppin)
{
	*pppin = NULL;

	IEnumPins * penum = NULL;
	IPin * ppin = NULL;

	HRESULT hr = filter->EnumPins(&penum);
	if (FAILED(hr))
	{
		return hr;
	}

	BOOL bfound = FALSE;
	while (S_OK == penum->Next(1, &ppin, NULL))
	{
		BOOL bconnected;
		hr = is_pin_connected(ppin, &bconnected);
		if (SUCCEEDED(hr))
		{
			if (bconnected)
			{
				hr = is_pin_direction(ppin, dir, &bfound);
			}
		}

		if (FAILED(hr))
		{
			ppin->Release();
			break;
		}
		if (bfound)
		{
			*pppin = ppin;
			break;
		}
		ppin->Release();
	}

	penum->Release();

	if (!bfound)
	{
		hr = VFW_E_NOT_FOUND;
	}
	return hr;
}

HRESULT dk_dshow_helper::initialize_evr(IBaseFilter * evr, HWND hwnd, IMFVideoDisplayControl ** ppwc)
{
	IMFGetService *pgs = NULL;
	IMFVideoDisplayControl *pdisplay = NULL;

	HRESULT hr = evr->QueryInterface(__uuidof(IMFGetService), (void**)&pgs);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pgs->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)&pdisplay);
	if (FAILED(hr))
	{
		goto done;
	}

	// Set the clipping window.
	hr = pdisplay->SetVideoWindow(hwnd);
	if (FAILED(hr))
	{
		goto done;
	}

	// Preserve aspect ratio by letter-boxing
	hr = pdisplay->SetAspectRatioMode(MFVideoARMode_PreservePicture);
	if (FAILED(hr))
	{
		goto done;
	}

	// Return the IMFVideoDisplayControl pointer to the caller.
	*ppwc = pdisplay;
	(*ppwc)->AddRef();

done:
	safe_release(&pgs);
	safe_release(&pdisplay);
	return hr;
}

HRESULT dk_dshow_helper::init_windowless_vmr9(IBaseFilter * vmr, HWND hwnd, IVMRWindowlessControl9 ** ppwc)
{
	return S_OK;
}

HRESULT dk_dshow_helper::init_windowless_vmr(IBaseFilter * vmr, HWND hwnd, IVMRWindowlessControl ** ppwc)
{
	return S_OK;
}
