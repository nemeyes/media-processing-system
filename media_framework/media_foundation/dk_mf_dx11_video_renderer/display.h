#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define AMDDRAWMONITORINFO_PRIMARY_MONITOR	0x0001

namespace debuggerking
{
	typedef struct _cam_ddraw_monitor_info_t
	{
		UINT device_id;
		HMONITOR monitor;
		TCHAR device[32];
		LARGE_INTEGER driver_version;
		DWORD vendor_id;
		DWORD device_id;
		DWORD subsys_id;
		DWORD revision;
		SIZE phys_monitor_dimension;
		DWORD refresh_rate;
		IUnknown * pdd;
	} cam_ddraw_monitor_info_t;

	#define EVR_MAX_MONITORS 16

	class monitor_array
	{
	public:
		monitor_array(void);
		virtual ~monitor_array(void);

		virtual HRESULT				initialize_display_system(_In_ HWND hwnd);
		virtual HRESULT				initialize_xcl_mode_display_system(_In_ IUnknown * pdd, _Out_ UINT * adapter_id) { return E_NOTIMPL; }
		virtual void				terminate_display_system(void);
		cam_ddraw_monitor_info_t *  find_monitor(_In_ HMONITOR monitor);
		HRESULT						match_guid(UINT device_id, _Out_ DWORD * match_id);


		cam_ddraw_monitor_info_t &  operator[](int i) { return _dd_monitor[i]; }
		DWORD						count(void) const { return _nmonitors; }

		static BOOL CALLBACK		monitor_enum_proc(_In_ HMONITOR monitor, _In_opt_ HDC hdc, _In_ LPRECT rect, LPARAM data);
		virtual BOOL				init_monitor(_In_ HMONITOR monitor, BOOL fxclmode);
	protected:
		BOOL						get_am_ddraw_monitor_info(UINT device_id, _Out_ cam_ddraw_monitor_info_t * lpmi, _In_ HMONITOR hm);
		virtual void				term_ddraw_monitor_info(_Inout_ cam_ddraw_monitor_info_t * pmi);
		DWORD						_nmonitors;
		cam_ddraw_monitor_info_t	_dd_monitor[EVR_MAX_MONITORS];
	};

	
	typedef struct _monitor_enum_proc_info_t
	{
		HWND hwnd;
		monitor_array * mon_array;
	} monitor_enum_proc_info_t;
};

#endif // !defined(__DISPLAY_H__)

