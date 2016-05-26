#include <windows.h>
#include <unknwn.h>
#include "display.h"
#include <strsafe.h>

#ifndef DEFAULT_DENSITY_LIMIT
#define DEFAULT_DENSITY_LIMIT       60
#endif
#ifndef WIDTH
#define WIDTH(x) ((x)->right - (x)->left)
#endif
#ifndef HEIGHT
#define HEIGHT(x) ((x)->bottom - (x)->top)
#endif

namespace debuggerking
{
	typedef struct _ddraw_info_t
	{
		DWORD count;
		DWORD pmi_size;
		HRESULT hr_callback;
		const GUID * guid;
		cam_ddraw_monitor_info_t * pmi;
		HWND hwnd;
	} ddraw_info_t;
};


void debuggerking::monitor_array::term_ddraw_monitor_info(_Inout_ cam_ddraw_monitor_info_t * pmi)
{
    ZeroMemory(pmi, sizeof(cam_ddraw_monitor_info_t));
}

BOOL debuggerking::monitor_array::get_am_ddraw_monitor_info(UINT device_id, _Out_ cam_ddraw_monitor_info_t * lpmi, _In_ HMONITOR hm)
{
    MONITORINFOEX miInfoEx;
    miInfoEx.cbSize = sizeof(miInfoEx);

    lpmi->monitor = NULL;
    lpmi->device_id = 0;
	lpmi->phys_monitor_dimension.cx = 0;
	lpmi->phys_monitor_dimension.cy = 0;
    lpmi->refresh_rate = DEFAULT_DENSITY_LIMIT;

    if (GetMonitorInfo(hm, &miInfoEx))
    {
        HRESULT hr = StringCchCopy(lpmi->device, sizeof(lpmi->device)/sizeof(lpmi->device[0]), miInfoEx.szDevice);

        if ( FAILED( hr ) )
        {
            return FALSE;
        }

        lpmi->monitor = hm;
        lpmi->device_id = device_id;
        lpmi->phys_monitor_dimension.cx = WIDTH(&miInfoEx.rcMonitor);
		lpmi->phys_monitor_dimension.cy = HEIGHT(&miInfoEx.rcMonitor);

        int j = 0;
        DISPLAY_DEVICE dd_monitor;

		dd_monitor.cb = sizeof(dd_monitor);
		while (EnumDisplayDevices(lpmi->device, j, &dd_monitor, 0))
        {
			if (dd_monitor.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
            {
                DEVMODE     dm;

                ZeroMemory(&dm, sizeof(dm));
                dm.dmSize = sizeof(dm);
                if (EnumDisplaySettings(lpmi->device, ENUM_CURRENT_SETTINGS, &dm))
                {
					lpmi->refresh_rate = dm.dmDisplayFrequency == 0 ? lpmi->refresh_rate : dm.dmDisplayFrequency;
                }

                // Remove registry snooping for monitor dimensions, as this is not supported by LDDM.
                // if (!FindMonitorDimensions(ddMonitor.DeviceID, &lpmi->physMonDim.cx, &lpmi->physMonDim.cy))
                {
					lpmi->phys_monitor_dimension.cx = WIDTH(&miInfoEx.rcMonitor);
					lpmi->phys_monitor_dimension.cy = HEIGHT(&miInfoEx.rcMonitor);
                }
            }
            j++;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL debuggerking::monitor_array::init_monitor( _In_ HMONITOR monitor, BOOL fxclmode)
{
	if (get_am_ddraw_monitor_info(_nmonitors, &_dd_monitor[_nmonitors], monitor))
    {
        _dd_monitor[_nmonitors].pdd = (IUnknown*)1; // make checks for pDD succeed.
        _nmonitors++;
    }

	if (EVR_MAX_MONITORS >= _nmonitors)
    {
        // don't exceed array bounds
        return TRUE;
    }
    return FALSE;
}

BOOL CALLBACK debuggerking::monitor_array::monitor_enum_proc(_In_ HMONITOR monitor, _In_opt_ HDC hdc, _In_ LPRECT rect, LPARAM data)
{
	monitor_enum_proc_info_t * info = (monitor_enum_proc_info_t*)data;

    if (!info)
        return TRUE;

    return info->mon_array->init_monitor(monitor, FALSE);
}

HRESULT debuggerking::monitor_array::initialize_display_system(_In_ HWND hwnd)
{
    HRESULT hr = S_OK;
    monitor_enum_proc_info_t info;

    info.hwnd = hwnd;
    info.mon_array = this;

    EnumDisplayMonitors(NULL, NULL, &monitor_enum_proc, (LPARAM)&info);

    if (_nmonitors == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return(hr);
    }

    return(hr);
}

debuggerking::cam_ddraw_monitor_info_t * debuggerking::monitor_array::find_monitor(_In_ HMONITOR monitor)
{
    for (DWORD i = 0; i < _nmonitors; i++)
    {
        if (monitor == _dd_monitor[i].monitor)
			return &_dd_monitor[i];
    }
    return NULL;
}

HRESULT debuggerking::monitor_array::match_guid(UINT device_id, _Out_ DWORD * match_id)
{
    HRESULT hr = S_OK;
	*match_id = 0;
    for (DWORD i = 0; i < _nmonitors; i++)
    {
        UINT monitor_device_id = _dd_monitor[i].device_id;
		if (device_id == monitor_device_id)
        {
            *match_id = i;
            hr = S_OK;
            return( hr );
        }
    }
    hr = S_FALSE;
    return( hr );
}

void debuggerking::monitor_array::terminate_display_system(void)
{
    for (DWORD i = 0; i < _nmonitors; i++)
    {
        term_ddraw_monitor_info(&_dd_monitor[i]);
    }
    _nmonitors = 0;
}

debuggerking::monitor_array::monitor_array(void)
    : _nmonitors(0)
{
    ZeroMemory(_dd_monitor, sizeof(_dd_monitor));
}

debuggerking::monitor_array::~monitor_array(void)
{
    terminate_display_system();
}

