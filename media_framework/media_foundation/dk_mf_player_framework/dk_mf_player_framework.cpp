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

void debuggerking::mf_player_framework::retreieve_gpus(std::vector<mf_player_framework::gpu_desc_t> & adapters)
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
		string_helper::convert_wide2multibyte(desc.Description, &description);
		mf_player_framework::gpu_desc_t gpu_desc;
		strncpy_s(gpu_desc.description, description, sizeof(gpu_desc.description));
		gpu_desc.adaptor_index = i;
		gpu_desc.vendor_id = desc.VendorId;
		gpu_desc.subsys_id = desc.SubSysId;
		gpu_desc.device_id = desc.DeviceId;
		gpu_desc.revision = desc.Revision;

		ATL::CComPtr<IDXGIOutput> output = NULL;
		if (DXGI_ERROR_NOT_FOUND != dxgi_adapter->EnumOutputs(0, &output))
		{
			DXGI_OUTPUT_DESC output_desc;
			HRESULT hr = output->GetDesc(&output_desc);
			gpu_desc.coord_left = output_desc.DesktopCoordinates.left;
			gpu_desc.coord_top = output_desc.DesktopCoordinates.top;
			gpu_desc.coord_right = output_desc.DesktopCoordinates.right;
			gpu_desc.coord_bottom = output_desc.DesktopCoordinates.bottom;
		}

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


debuggerking::mf_player_framework::mf_player_framework(void)
{
	_core = new (std::nothrow) mf_player_core();
}

debuggerking::mf_player_framework::~mf_player_framework(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

int32_t debuggerking::mf_player_framework::open_file(mf_player_framework::configuration_t * config)
{
	return _core->open_file(config);
}

int32_t debuggerking::mf_player_framework::play(void)
{
	return _core->play();
}

int32_t debuggerking::mf_player_framework::pause(void)
{
	return _core->pause();
}

int32_t debuggerking::mf_player_framework::stop(void)
{
	return _core->stop();
}

debuggerking::mf_player_framework::player_state debuggerking::mf_player_framework::state(void) const
{
	return _core->state();
}

void debuggerking::mf_player_framework::on_keydown_right(void)
{
	_core->on_keydown_right();
}

void debuggerking::mf_player_framework::on_keyup_right(void)
{
	_core->on_keyup_right();
}

void debuggerking::mf_player_framework::on_keydown_left(void)
{
	_core->on_keydown_left();
}

void debuggerking::mf_player_framework::on_keyup_left(void)
{
	_core->on_keyup_left();
}

void debuggerking::mf_player_framework::on_keydown_up(void)
{
	_core->on_keydown_up();
}

void debuggerking::mf_player_framework::on_keyup_up(void)
{
	_core->on_keyup_up();
}

void debuggerking::mf_player_framework::on_keydown_down(void)
{
	_core->on_keydown_down();
}

void debuggerking::mf_player_framework::on_keyup_down(void)
{
	_core->on_keyup_down();
}
