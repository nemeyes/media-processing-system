#include <windows.h>
#include <dk_string_helper.h>
#include "dk_mf_player_framework.h"
#include "mf_player_framework.h"
#include <dxgi1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <rpcdce.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "rpcrt4.lib")

dk_mf_player_framework::dk_mf_player_framework(void)
{
	_core = new (std::nothrow) mf_player_framework();
}

dk_mf_player_framework::~dk_mf_player_framework(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

void dk_mf_player_framework::retreieve_gpus(std::vector<dk_mf_player_framework::gpu_desc_t> & adapters)
{
	IDXGIFactory3 * dxgi_factory = NULL;
	IDXGIAdapter1 * dxgi_adapter = NULL;
	HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&dxgi_factory);
	if (FAILED(result))
		return;

	for (int i = 0; dxgi_factory->EnumAdapters1(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		dxgi_adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		char * description = nullptr;
		dk_string_helper::convert_wide2multibyte(desc.Description, &description);
		//char * luid = 0;
		//cap_string_helper::convert_wide2multibyte(UuidToString(desc.AdapterLuid, &luid);
		dk_mf_player_framework::gpu_desc_t gpu_desc;
		strncpy_s(gpu_desc.description, description, sizeof(gpu_desc.description));
		gpu_desc.vendor_id = desc.VendorId;
		gpu_desc.subsys_id = desc.SubSysId;
		gpu_desc.device_id = desc.DeviceId;
		gpu_desc.revision = desc.Revision;

		adapters.push_back(gpu_desc);

		if (description)
			free(description);
		description = nullptr;

		if (dxgi_adapter)
			dxgi_adapter->Release();
		dxgi_adapter = NULL;
	}

	if (dxgi_factory)
		dxgi_factory->Release();
	dxgi_factory = NULL;
}


dk_mf_player_framework::err_code dk_mf_player_framework::open_file(const wchar_t * file, uint32_t gpu_index, HWND hwnd)
{
	return _core->open_file(file, gpu_index, hwnd);
}

dk_mf_player_framework::err_code dk_mf_player_framework::play(void)
{
	return _core->play();
}

dk_mf_player_framework::err_code dk_mf_player_framework::pause(void)
{
	return _core->pause();
}

dk_mf_player_framework::err_code dk_mf_player_framework::stop(void)
{
	return _core->stop();
}

dk_mf_player_framework::player_state dk_mf_player_framework::state(void) const
{
	return _core->state();
}