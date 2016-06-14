
// ParallelRecordClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ParallelRecordClient.h"
#include "ParallelRecordClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CParallelRecordClientDlg dialog

void __stdcall PlayTimeCallback(int index, int year, int month, int day, int hour, int minute, int second)
{
	TRACE(L"index[%d], %.4d-%.2d-%.2d %.2d::%.2d::%.2d\n", index, year, month, day, hour, minute, second);
}

CParallelRecordClientDlg::CParallelRecordClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CParallelRecordClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CParallelRecordClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PARALLEL_RECORDER_ADDRESS, _parallel_recorder_address);
	DDX_Control(pDX, IDC_EDIT_PARALLEL_RECORDER_PORT_NUMBER, _parallel_recorder_port_number);
	DDX_Control(pDX, IDC_EDIT_PARALLEL_RECORDER_USERNAME, _parallel_recorder_username);
	DDX_Control(pDX, IDC_EDIT_PARALLEL_RECORDER_PASSWORD, _parallel_recorder_password);
	DDX_Control(pDX, IDC_EDIT_UUID, _uuid);
	DDX_Control(pDX, IDC_COMBO_RECORDING_YEARS, _recording_years);
	DDX_Control(pDX, IDC_COMBO_RECORDING_MONTHS, _recording_months);
	DDX_Control(pDX, IDC_COMBO_RECORDING_DAYS, _recording_days);
	DDX_Control(pDX, IDC_COMBO_RECORDING_HOURS, _recording_hours);
	DDX_Control(pDX, IDC_COMBO_MINUTES, _recording_minutes);
	DDX_Control(pDX, IDC_COMBO6, _recording_seconds);
	DDX_Control(pDX, IDC_EDIT_YEAR, _manual_recording_year);
	DDX_Control(pDX, IDC_EDIT_MONTH, _manual_recording_month);
	DDX_Control(pDX, IDC_EDIT_DAY, _manual_recording_day);
	DDX_Control(pDX, IDC_EDIT_HOUR, _manual_recording_hour);
	DDX_Control(pDX, IDC_EDIT_MINUTE, _manual_recording_minute);
	DDX_Control(pDX, IDC_EDIT_SECOND, _manual_recording_second);
	DDX_Control(pDX, IDC_EDIT_RTSP_ADDRESS, _rtsp_address);
	DDX_Control(pDX, IDC_EDIT_RTSP_USERNAME, _rtsp_username);
	DDX_Control(pDX, IDC_EDIT9_RTSP_PASSWORD, _rtsp_password);
	DDX_Control(pDX, IDC_EDIT_RECORDING_PLAY_SCALE, _recording_play_scale);
	DDX_Control(pDX, IDC_EDIT_OSD_POSITION_X, _osd_x);
	DDX_Control(pDX, IDC_EDIT_OSD_POSITON_Y, _osd_y);
	DDX_Control(pDX, IDC_EDIT_EXPORT_UUID, _exp_uuid);
	DDX_Control(pDX, IDC_EDIT_EXPORT_BEGIN_YEAR, _exp_begin_year);
	DDX_Control(pDX, IDC_EDIT_EXPORT_BEGIN_MONTH, _exp_begin_month);
	DDX_Control(pDX, IDC_EDIT_EXPORT_BEGIN_DAY, _exp_begin_day);
	DDX_Control(pDX, IDC_EDIT_EXPORT_BEGIN_HOUR, _exp_begin_hour);
	DDX_Control(pDX, IDC_EDIT_EXPORT_BEGIN_MINUTE, _exp_begin_minute);
	DDX_Control(pDX, IDC_EDIT_EXPORT_BEGIN_SECOND, _exp_begin_second);
	DDX_Control(pDX, IDC_EDIT8, _exp_end_year);
	DDX_Control(pDX, IDC_EDIT9, _exp_end_month);
	DDX_Control(pDX, IDC_EDIT10, _exp_end_day);
	DDX_Control(pDX, IDC_EDIT11, _exp_end_hour);
	DDX_Control(pDX, IDC_EDIT12, _exp_end_minute);
	DDX_Control(pDX, IDC_EDIT13, _exp_end_second);
	DDX_Control(pDX, IDC_COMBO_OSD_ENABLE, _osd_enable);
}

BEGIN_MESSAGE_MAP(CParallelRecordClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PARALLEL_RECORDER_CONNECT, &CParallelRecordClientDlg::OnBnClickedButtonParallelRecorderConnect)
	ON_BN_CLICKED(IDC_BUTTON_PARALLEL_RECORDER_DISCONNECT, &CParallelRecordClientDlg::OnBnClickedButtonParallelRecorderDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_GET_RECORDING_YEAR, &CParallelRecordClientDlg::OnBnClickedButtonGetRecordingYears)
	ON_BN_CLICKED(IDC_BUTTON_START_PLAYBACK, &CParallelRecordClientDlg::OnBnClickedButtonStartPlayback)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PLAYBACK, &CParallelRecordClientDlg::OnBnClickedButtonStopPlayback)
	ON_CBN_SELCHANGE(IDC_COMBO_RECORDING_YEARS, &CParallelRecordClientDlg::OnCbnSelchangeComboRecordingYears)
	ON_CBN_SELCHANGE(IDC_COMBO_RECORDING_MONTHS, &CParallelRecordClientDlg::OnCbnSelchangeComboRecordingMonths)
	ON_CBN_SELCHANGE(IDC_COMBO_RECORDING_DAYS, &CParallelRecordClientDlg::OnCbnSelchangeComboRecordingDays)
	ON_CBN_SELCHANGE(IDC_COMBO_RECORDING_HOURS, &CParallelRecordClientDlg::OnCbnSelchangeComboRecordingHours)
	ON_CBN_SELCHANGE(IDC_COMBO_MINUTES, &CParallelRecordClientDlg::OnCbnSelchangeComboMinutes)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_START_PLAYBACK, &CParallelRecordClientDlg::OnBnClickedButtonManualStartPlayback)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_STOP_PLAYBACK, &CParallelRecordClientDlg::OnBnClickedButtonManualStopPlayback)
	ON_BN_CLICKED(IDC_BUTTON_RTSP_PLAY, &CParallelRecordClientDlg::OnBnClickedButtonRtspPlay)
	ON_BN_CLICKED(IDC_BUTTON_RTSP_STOP, &CParallelRecordClientDlg::OnBnClickedButtonRtspStop)
	ON_BN_CLICKED(IDC_BUTTON_OSD_ENABLE, &CParallelRecordClientDlg::OnBnClickedButtonOsdEnable)
	ON_BN_CLICKED(IDC_BUTTON_SET_OSD_POSITION, &CParallelRecordClientDlg::OnBnClickedButtonSetOsdPosition)
	ON_BN_CLICKED(IDC_BUTTON_START_EXPORT, &CParallelRecordClientDlg::OnBnClickedButtonStartExport)
	ON_BN_CLICKED(IDC_BUTTON_STOP_EXPORT, &CParallelRecordClientDlg::OnBnClickedButtonStopExport)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE_PLAYBACK, &CParallelRecordClientDlg::OnBnClickedButtonPausePlayback)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_PAUSE_PLAYBACK, &CParallelRecordClientDlg::OnBnClickedButtonManualPausePlayback)
END_MESSAGE_MAP()


// CParallelRecordClientDlg message handlers

BOOL CParallelRecordClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	_paused = FALSE;
	_parallel_recorder_address.SetWindowText(L"127.0.0.1");
	_parallel_recorder_port_number.SetWindowText(L"15000");
	_parallel_recorder_username.SetWindowTextW(L"root");
	_parallel_recorder_password.SetWindowTextW(L"pass");

	_rtsp_address.SetWindowTextW(L"rtsp://1.217.25.234:5054/8/stream1");

	_uuid.SetWindowTextW(L"CH1");
	_recording_play_scale.SetWindowTextW(L"1");

	_osd_enable.SetCurSel(0);
	_osd_x.SetWindowTextW(L"20");
	_osd_y.SetWindowTextW(L"20");


	_exp_uuid.SetWindowTextW(L"CH1");
	_exp_begin_year.SetWindowTextW(L"2016");
	_exp_begin_month.SetWindowTextW(L"5");
	_exp_begin_day.SetWindowTextW(L"20");
	_exp_begin_hour.SetWindowTextW(L"16");
	_exp_begin_minute.SetWindowTextW(L"42");
	_exp_begin_second.SetWindowTextW(L"40");

	_exp_end_year.SetWindowTextW(L"2016");
	_exp_end_month.SetWindowTextW(L"5");
	_exp_end_day.SetWindowTextW(L"20");
	_exp_end_hour.SetWindowTextW(L"16");
	_exp_end_minute.SetWindowTextW(L"44");
	_exp_end_second.SetWindowTextW(L"40");



	PRMC_Initialize(GetSafeHwnd());
	PRMC_index = -1;
	PRMC_RTSP_index = -1;


	CWnd * wnd = GetDlgItem(IDC_BUTTON_PARALLEL_RECORDER_CONNECT);
	wnd->EnableWindow(TRUE);
	wnd = GetDlgItem(IDC_BUTTON_PARALLEL_RECORDER_DISCONNECT);
	wnd->EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CParallelRecordClientDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	/*
	if (PRMC_index >= 0)
	{
		PRMC_Stop(index_1);
		PRMC_Remove(index_1);
	}
	*/
	PRMC_Release();

	return CDialogEx::DestroyWindow();
}

void CParallelRecordClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CParallelRecordClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CBrush brush;
		brush.CreateSolidBrush(RGB(0, 0, 0));

		CRect rect;
		CWnd * video_view = GetDlgItem(IDC_STATIC_VIDEO1);
		CDC * video_view_dc = video_view->GetDC();
		video_view->GetClientRect(rect);
		video_view_dc->FillRect(rect, &brush);
		brush.DeleteObject();
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CParallelRecordClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CParallelRecordClientDlg::OnBnClickedButtonParallelRecorderConnect()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	CString recorder_port_number;
	CString recorder_username;
	CString recorder_password;
	_parallel_recorder_address.GetWindowText(recorder_address);
	_parallel_recorder_port_number.GetWindowText(recorder_port_number);
	_parallel_recorder_username.GetWindowText(recorder_username);
	_parallel_recorder_password.GetWindowText(recorder_password);
	int result = PRMC_Connect((LPCWSTR)recorder_address, _wtoi(recorder_port_number), (LPCWSTR)recorder_username, (LPCWSTR)recorder_password);
	if (result == PRMC_SUCCESS)
	{
		CWnd * wnd = GetDlgItem(IDC_BUTTON_PARALLEL_RECORDER_CONNECT);
		wnd->EnableWindow(FALSE);

		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_ADDRESS);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_PORT_NUMBER);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_USERNAME);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_PASSWORD);
		wnd->EnableWindow(FALSE);

		wnd = GetDlgItem(IDC_BUTTON_PARALLEL_RECORDER_DISCONNECT);
		wnd->EnableWindow(TRUE);
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonParallelRecorderDisconnect()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	int result = PRMC_Disconnect((LPCWSTR)recorder_address);
	if (result == PRMC_SUCCESS)
	{
		CWnd * wnd = GetDlgItem(IDC_BUTTON_PARALLEL_RECORDER_CONNECT);
		wnd->EnableWindow(TRUE);

		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_ADDRESS);
		wnd->EnableWindow(TRUE);
		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_PORT_NUMBER);
		wnd->EnableWindow(TRUE);
		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_USERNAME);
		wnd->EnableWindow(TRUE);
		wnd = GetDlgItem(IDC_EDIT_PARALLEL_RECORDER_PASSWORD);
		wnd->EnableWindow(TRUE);

		wnd = GetDlgItem(IDC_BUTTON_PARALLEL_RECORDER_DISCONNECT);
		wnd->EnableWindow(FALSE);
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonGetRecordingYears()
{
	// TODO: Add your control notification handler code here
	_recording_years.ResetContent();
	_recording_months.ResetContent();
	_recording_days.ResetContent();
	_recording_hours.ResetContent();
	_recording_minutes.ResetContent();
	_recording_seconds.ResetContent();

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (recorder_address.GetLength()>0 && uuid.GetLength() > 0)
	{
		int years[10] = { 0 };
		int size = 0;
		PRMC_GetYears((LPCWSTR)recorder_address, (LPCWSTR)uuid, years, sizeof(years) / sizeof(int), size);

		_recording_years.InsertString(0, L"select");
		for (int index = 0; index < size; index++)
		{
			wchar_t str_year[10] = { 0 };
			_sntprintf_s(str_year, sizeof(str_year) / sizeof(wchar_t), L"%d", years[index]);
			_recording_years.InsertString(index + 1, str_year);
		}
		_recording_years.SetCurSel(0);
	}
}

void CParallelRecordClientDlg::OnCbnSelchangeComboRecordingYears()
{
	// TODO: Add your control notification handler code here
	_recording_months.ResetContent();
	_recording_days.ResetContent();
	_recording_hours.ResetContent();
	_recording_minutes.ResetContent();
	_recording_seconds.ResetContent();

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	CString uuid;
	_uuid.GetWindowText(uuid);

	if (recorder_address.GetLength()>0 && uuid.GetLength()>0)
	{
		CString str_year;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		if (wcscmp(L"select", str_year))
		{
			int year = _ttoi(str_year);
			int months[12] = { 0 };
			int size = 0;
			PRMC_GetMonths((LPCWSTR)recorder_address, (LPCWSTR)uuid, year, months, sizeof(months) / sizeof(int), size);

			_recording_months.InsertString(0, L"select");
			for (int index = 0; index < size; index++)
			{
				wchar_t str_month[10] = { 0 };
				_sntprintf_s(str_month, sizeof(str_month) / sizeof(wchar_t), L"%d", months[index]);
				_recording_months.InsertString(index + 1, str_month);
			}
			_recording_months.SetCurSel(0);
		}
	}
}

void CParallelRecordClientDlg::OnCbnSelchangeComboRecordingMonths()
{
	// TODO: Add your control notification handler code here
	_recording_days.ResetContent();
	_recording_hours.ResetContent();
	_recording_minutes.ResetContent();
	_recording_seconds.ResetContent();

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (recorder_address.GetLength()>0 && uuid.GetLength()>0)
	{
		CString str_year, str_month;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		_recording_months.GetLBText(_recording_months.GetCurSel(), str_month);
		if (wcscmp(L"select", str_year) && wcscmp(L"select", str_month))
		{
			int year = _ttoi(str_year);
			int month = _ttoi(str_month);
			int days[31] = { 0 };
			int size = 0;
			PRMC_GetDays((LPCWSTR)recorder_address, (LPCWSTR)uuid, year, month, days, sizeof(days) / sizeof(int), size);

			_recording_days.InsertString(0, L"select");
			for (int index = 0; index < size; index++)
			{
				wchar_t str_day[10] = { 0 };
				_sntprintf_s(str_day, sizeof(str_day) / sizeof(wchar_t), L"%d", days[index]);
				_recording_days.InsertString(index + 1, str_day);
			}
			_recording_days.SetCurSel(0);
		}
	}
}

void CParallelRecordClientDlg::OnCbnSelchangeComboRecordingDays()
{
	// TODO: Add your control notification handler code here
	_recording_hours.ResetContent();
	_recording_minutes.ResetContent();
	_recording_seconds.ResetContent();

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (recorder_address.GetLength()>0 && uuid.GetLength()>0)
	{
		CString str_year, str_month, str_day;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		_recording_months.GetLBText(_recording_months.GetCurSel(), str_month);
		_recording_days.GetLBText(_recording_days.GetCurSel(), str_day);
		if (wcscmp(L"select", str_year) && wcscmp(L"select", str_month) && wcscmp(L"select", str_day))
		{
			int year = _ttoi(str_year);
			int month = _ttoi(str_month);
			int day = _ttoi(str_day);
			int hours[12] = { 0 };
			int size = 0;
			PRMC_GetHours((LPCWSTR)recorder_address, (LPCWSTR)uuid, year, month, day, hours, sizeof(hours) / sizeof(int), size);

			_recording_hours.InsertString(0, L"select");
			for (int index = 0; index < size; index++)
			{
				wchar_t str_hour[10] = { 0 };
				_sntprintf_s(str_hour, sizeof(str_hour) / sizeof(wchar_t), L"%d", hours[index]);
				_recording_hours.InsertString(index + 1, str_hour);
			}
			_recording_hours.SetCurSel(0);
		}
	}
}

void CParallelRecordClientDlg::OnCbnSelchangeComboRecordingHours()
{
	// TODO: Add your control notification handler code here
	_recording_minutes.ResetContent();
	_recording_seconds.ResetContent();

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (recorder_address.GetLength()>0 && uuid.GetLength()>0)
	{
		CString str_year, str_month, str_day, str_hour;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		_recording_months.GetLBText(_recording_months.GetCurSel(), str_month);
		_recording_days.GetLBText(_recording_days.GetCurSel(), str_day);
		_recording_hours.GetLBText(_recording_hours.GetCurSel(), str_hour);
		if (wcscmp(L"select", str_year) && wcscmp(L"select", str_month) && wcscmp(L"select", str_day) && wcscmp(L"select", str_hour))
		{
			int year = _ttoi(str_year);
			int month = _ttoi(str_month);
			int day = _ttoi(str_day);
			int hour = _ttoi(str_hour);
			int minutes[60] = { 0 };
			int size = 0;
			PRMC_GetMinutes((LPCWSTR)recorder_address, (LPCWSTR)uuid, year, month, day, hour, minutes, sizeof(minutes) / sizeof(int), size);

			_recording_minutes.InsertString(0, L"select");
			for (int index = 0; index < size; index++)
			{
				wchar_t str_minute[10] = { 0 };
				_sntprintf_s(str_minute, sizeof(str_minute) / sizeof(wchar_t), L"%d", minutes[index]);
				_recording_minutes.InsertString(index + 1, str_minute);
			}
			_recording_minutes.SetCurSel(0);
		}
	}
}

void CParallelRecordClientDlg::OnCbnSelchangeComboMinutes()
{
	// TODO: Add your control notification handler code here
	_recording_seconds.ResetContent();

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (recorder_address.GetLength()>0 && uuid.GetLength()>0)
	{
		CString str_year, str_month, str_day, str_hour, str_minute;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		_recording_months.GetLBText(_recording_months.GetCurSel(), str_month);
		_recording_days.GetLBText(_recording_days.GetCurSel(), str_day);
		_recording_hours.GetLBText(_recording_hours.GetCurSel(), str_hour);
		_recording_minutes.GetLBText(_recording_minutes.GetCurSel(), str_minute);
		if (wcscmp(L"select", str_year) && wcscmp(L"select", str_month) && wcscmp(L"select", str_day) && wcscmp(L"select", str_hour) && wcscmp(L"select", str_minute))
		{
			int year = _ttoi(str_year);
			int month = _ttoi(str_month);
			int day = _ttoi(str_day);
			int hour = _ttoi(str_hour);
			int minute = _ttoi(str_minute);
			int seconds[60] = { 0 };
			int size = 0;
			PRMC_GetSeconds((LPCWSTR)recorder_address, (LPCWSTR)uuid, year, month, day, hour, minute, seconds, sizeof(seconds) / sizeof(int), size);

			_recording_seconds.InsertString(0, L"select");
			for (int index = 0; index < size; index++)
			{
				wchar_t str_second[10] = { 0 };
				_sntprintf_s(str_second, sizeof(str_second) / sizeof(wchar_t), L"%d", seconds[index]);
				_recording_seconds.InsertString(index + 1, str_second);
			}
			_recording_seconds.SetCurSel(0);
		}
	}
}

void CParallelRecordClientDlg::OnBnClickedButtonStartPlayback()
{
	// TODO: Add your control notification handler code here
	int status = PRMC_FAIL;

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	CString uuid;
	_uuid.GetWindowText(uuid);

	if (!_paused)
	{
		CString play_scale;
		float scale = 1.0f;
		_recording_play_scale.GetWindowText(play_scale);
		scale = _ttof(play_scale);

		CString str_year, str_month, str_day, str_hour, str_minute, str_second;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		_recording_months.GetLBText(_recording_months.GetCurSel(), str_month);
		_recording_days.GetLBText(_recording_days.GetCurSel(), str_day);
		_recording_hours.GetLBText(_recording_hours.GetCurSel(), str_hour);
		_recording_minutes.GetLBText(_recording_minutes.GetCurSel(), str_minute);
		_recording_seconds.GetLBText(_recording_seconds.GetCurSel(), str_second);

		if (recorder_address.GetLength() > 0 &&
			wcscmp(L"select", str_year) && wcscmp(L"select", str_month) && wcscmp(L"select", str_day) &&
			wcscmp(L"select", str_hour) && wcscmp(L"select", str_minute) && wcscmp(L"select", str_second))
		{
			int year = _ttoi(str_year);
			int month = _ttoi(str_month);
			int day = _ttoi(str_day);
			int hour = _ttoi(str_hour);
			int minute = _ttoi(str_minute);
			int second = _ttoi(str_second);

			HWND hwnd = NULL;
			hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO1);
			PRMC_index = PRMC_Add((LPCWSTR)recorder_address, (LPCWSTR)uuid, hwnd, PlayTimeCallback);
			if (PRMC_index != PRMC_FAIL)
			{
				//status = PRMC_StartExport((LPCWSTR)recorder_address, PRMC_index, year, month, day, hour, minute, second, year, month, day, hour, minute, second);
				status = PRMC_Play((LPCWSTR)recorder_address, PRMC_index, year, month, day, hour, minute, second, scale, false);
			}
		}
	}
	else
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_Resume((LPCWSTR)recorder_address, PRMC_index);
		if (status == PRMC_SUCCESS)
			_paused = FALSE;
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonStopPlayback()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	int status = PRMC_FAIL;
	if (recorder_address.GetLength()>0)
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_Stop((LPCWSTR)recorder_address, PRMC_index);
		if (status == PRMC_SUCCESS)
			status = PRMC_Remove((LPCWSTR)recorder_address, PRMC_index);
	}

	CBrush brush;
	brush.CreateSolidBrush(RGB(0, 0, 0));
	CRect rect;
	CWnd * video_view = GetDlgItem(IDC_STATIC_VIDEO1);
	CDC * video_view_dc = video_view->GetDC();
	video_view->GetClientRect(rect);
	video_view_dc->FillRect(rect, &brush);
	brush.DeleteObject();

	//InvalidateRect(rect);
}

void CParallelRecordClientDlg::OnBnClickedButtonPausePlayback()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	int status = PRMC_FAIL;
	if (recorder_address.GetLength()>0)
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_Pause((LPCWSTR)recorder_address, PRMC_index);
		if (status == PRMC_SUCCESS)
			_paused = TRUE;
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonManualStartPlayback()
{
	// TODO: Add your control notification handler code here
	int status = PRMC_FAIL;
	 
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	CString uuid;
	_uuid.GetWindowText(uuid);

	if (!_paused)
	{
		CString play_scale;
		float scale = 1.0f;
		_recording_play_scale.GetWindowText(play_scale);
		scale = _ttof(play_scale);

		CString str_year, str_month, str_day, str_hour, str_minute, str_second;
		_manual_recording_year.GetWindowTextW(str_year);
		_manual_recording_month.GetWindowTextW(str_month);
		_manual_recording_day.GetWindowTextW(str_day);
		_manual_recording_hour.GetWindowTextW(str_hour);
		_manual_recording_minute.GetWindowTextW(str_minute);
		_manual_recording_second.GetWindowTextW(str_second);

		if (recorder_address.GetLength() > 0 &&
			wcscmp(L"select", str_year) && wcscmp(L"select", str_month) && wcscmp(L"select", str_day) &&
			wcscmp(L"select", str_hour) && wcscmp(L"select", str_minute) && wcscmp(L"select", str_second))
		{
			int year = _ttoi(str_year);
			int month = _ttoi(str_month);
			int day = _ttoi(str_day);
			int hour = _ttoi(str_hour);
			int minute = _ttoi(str_minute);
			int second = _ttoi(str_second);

			HWND hwnd = NULL;
			hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO1);
			PRMC_index = PRMC_Add((LPCWSTR)recorder_address, (LPCWSTR)uuid, hwnd, PlayTimeCallback);
			if (PRMC_index != PRMC_FAIL)
			{
				status = PRMC_Play((LPCWSTR)recorder_address, PRMC_index, year, month, day, hour, minute, second, scale, false);
			}
		}
	}
	else
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_Resume((LPCWSTR)recorder_address, PRMC_index);
		if (status == PRMC_SUCCESS)
			_paused = FALSE;
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonManualStopPlayback()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	int status = PRMC_FAIL;
	if (recorder_address.GetLength()>0)
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_Stop((LPCWSTR)recorder_address, PRMC_index);
		if (status == PRMC_SUCCESS)
			status = PRMC_Remove((LPCWSTR)recorder_address, PRMC_index);
	}
	PRMC_index = -1;

	CBrush brush;
	brush.CreateSolidBrush(RGB(0, 0, 0));
	CRect rect;
	CWnd * video_view = GetDlgItem(IDC_STATIC_VIDEO1);
	CDC * video_view_dc = video_view->GetDC();
	video_view->GetClientRect(rect);
	video_view_dc->FillRect(rect, &brush);
	brush.DeleteObject();
}

void CParallelRecordClientDlg::OnBnClickedButtonManualPausePlayback()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	int status = PRMC_FAIL;
	if (recorder_address.GetLength()>0)
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_Pause((LPCWSTR)recorder_address, PRMC_index);
		if (status == PRMC_SUCCESS)
			_paused = TRUE;
	}
}

void CParallelRecordClientDlg::OnBnClickedButtonStartExport()
{
	// TODO: Add your control notification handler code here
	int status = PRMC_FAIL;

	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	CString uuid;
	_exp_uuid.GetWindowText(uuid);

	CString str_begin_year, str_begin_month, str_begin_day, str_begin_hour, str_begin_minute, str_begin_second;
	_exp_begin_year.GetWindowTextW(str_begin_year);
	_exp_begin_month.GetWindowTextW(str_begin_month);
	_exp_begin_day.GetWindowTextW(str_begin_day);
	_exp_begin_hour.GetWindowTextW(str_begin_hour);
	_exp_begin_minute.GetWindowTextW(str_begin_minute);
	_exp_begin_second.GetWindowTextW(str_begin_second);

	CString str_end_year, str_end_month, str_end_day, str_end_hour, str_end_minute, str_end_second;
	_exp_end_year.GetWindowTextW(str_end_year);
	_exp_end_month.GetWindowTextW(str_end_month);
	_exp_end_day.GetWindowTextW(str_end_day);
	_exp_end_hour.GetWindowTextW(str_end_hour);
	_exp_end_minute.GetWindowTextW(str_end_minute);
	_exp_end_second.GetWindowTextW(str_end_second);

	if (recorder_address.GetLength()>0)
	{
		int begin_year = _ttoi(str_begin_year);
		int begin_month = _ttoi(str_begin_month);
		int begin_day = _ttoi(str_begin_day);
		int begin_hour = _ttoi(str_begin_hour);
		int begin_minute = _ttoi(str_begin_minute);
		int begin_second = _ttoi(str_begin_second);
		int end_year = _ttoi(str_end_year);
		int end_month = _ttoi(str_end_month);
		int end_day = _ttoi(str_end_day);
		int end_hour = _ttoi(str_end_hour);
		int end_minute = _ttoi(str_end_minute);
		int end_second = _ttoi(str_end_second);


		WCHAR current_dir[MAX_PATH];
		memset(current_dir, 0, MAX_PATH);
		::GetModuleFileName(NULL, current_dir, sizeof(current_dir));
		CString str_folder = current_dir;
		str_folder = str_folder.Left(str_folder.ReverseFind(_T('\\')) + 1);
		if (str_folder.GetLength() > 0)
		{
			WCHAR destionation_file_path[MAX_PATH] = { 0 };
			_snwprintf_s(destionation_file_path, sizeof(destionation_file_path) / sizeof(wchar_t), L"%s%s", (LPCWSTR)str_folder, L"test.ts");
			PRMC_index = PRMC_AddExport((LPCWSTR)recorder_address, (LPCWSTR)uuid, (LPCWSTR)destionation_file_path);
			if (PRMC_index != PRMC_FAIL)
			{
				status = PRMC_PlayExport((LPCWSTR)recorder_address, PRMC_index,
					begin_year, begin_month, begin_day, begin_hour, begin_minute, begin_second,
					end_year, end_month, end_day, end_hour, end_minute, end_second);
			}
		}
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonStopExport()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);

	int status = PRMC_FAIL;
	if (recorder_address.GetLength()>0)
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_StopExport((LPCWSTR)recorder_address, PRMC_index);
		if (status == PRMC_SUCCESS)
			status = PRMC_RemoveExport((LPCWSTR)recorder_address, PRMC_index);
	}
	PRMC_index = -1;
}

void CParallelRecordClientDlg::OnBnClickedButtonOsdEnable()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	int index = _osd_enable.GetCurSel();

	int status = PRMC_FAIL;
	if (recorder_address.GetLength()>0)
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_EnableOSD((LPCWSTR)recorder_address, PRMC_index, index==0?true:false);
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonSetOsdPosition()
{
	// TODO: Add your control notification handler code here
	CString recorder_address;
	_parallel_recorder_address.GetWindowText(recorder_address);
	CString osd_x, osd_y;
	_osd_x.GetWindowText(osd_x);
	_osd_y.GetWindowText(osd_y);

	int status = PRMC_FAIL;
	if (recorder_address.GetLength()>0)
	{
		if (PRMC_index != PRMC_FAIL)
			status = PRMC_SetOSDPosition((LPCWSTR)recorder_address, PRMC_index, _ttoi(osd_x), _ttoi(osd_y));
	}
}

void CParallelRecordClientDlg::OnBnClickedButtonRtspPlay()
{
	// TODO: Add your control notification handler code here
	CString str_url, str_username, str_password;
	_rtsp_address.GetWindowTextW(str_url);
	_rtsp_username.GetWindowTextW(str_username);
	_rtsp_password.GetWindowTextW(str_password);

	int stauts = PRMC_FAIL;
	HWND hwnd = NULL;
	hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO1);
	PRMC_RTSP_index = PRMC_RTSP_Add((LPCWSTR)str_url, 554, (LPCWSTR)str_username, (LPCWSTR)str_password, hwnd);
	if (PRMC_RTSP_index != PRMC_FAIL)
		stauts = PRMC_RTSP_Play(PRMC_RTSP_index, true);
}


void CParallelRecordClientDlg::OnBnClickedButtonRtspStop()
{
	// TODO: Add your control notification handler code here
	int stauts = PRMC_FAIL;
	stauts = PRMC_RTSP_Stop(PRMC_RTSP_index);
	if (stauts != PRMC_FAIL)
		stauts = PRMC_RTSP_Remove(PRMC_RTSP_index);

	PRMC_RTSP_index = -1;

	CBrush brush;
	brush.CreateSolidBrush(RGB(0, 0, 0));
	CRect rect;
	CWnd * video_view = GetDlgItem(IDC_STATIC_VIDEO1);
	CDC * video_view_dc = video_view->GetDC();
	video_view->GetClientRect(rect);
	video_view_dc->FillRect(rect, &brush);
	brush.DeleteObject();
}
