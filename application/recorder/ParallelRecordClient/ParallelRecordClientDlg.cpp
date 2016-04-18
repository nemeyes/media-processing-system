
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
	_parallel_recorder_address.SetWindowText(L"127.0.0.1");
	_parallel_recorder_port_number.SetWindowText(L"15000");
	_parallel_recorder_username.SetWindowTextW(L"root");
	_parallel_recorder_password.SetWindowTextW(L"pass");

	_uuid.SetWindowTextW(L"CH1");

	PRMC_Initialize(GetSafeHwnd());
	index_1 = -1;
	//index_2 = -1;
	//index_3 = -1;
	//index_4 = -1;

	//HWND hwnd = NULL;
	//hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO1);
	//index_1 = PRMC_Add(L"rtsp://127.0.0.1/CH1/20160416031400", 0, 0, false, hwnd);
	//PRMC_Play(index_1);
	
	//hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO2);
	//index_2 = PRMC_Add(L"rtsp://now.iptime.org/2/stream1", 0, 0, false, hwnd);
	//PRMC_Play(index_2);
 
	//hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO3);
	//index_3 = PRMC_Add(L"rtsp://root:pass@basetec.iptime.org:554/axis-media/media.amp?camera=1", 0, 0, false, hwnd);
	//PRMC_Play(index_3);

	//hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO4);
	//index_4 = PRMC_Add(L"rtsp://root:pass@basetec.iptime.org:554/axis-media/media.amp?camera=2", 0, 0, false, hwnd);
	//PRMC_Play(index_4);

	//hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO3);
	//index_3 = PRMC_Add(L"rtmp://10.202.140.37/vod/00.flv", 0, 0, false, hwnd);
	//PRMC_Play(index_3);

	//hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO4);
	//index_4 = PRMC_Add(L"rtmp://10.202.140.37/vod/01.flv", 0, 0, false, hwnd);
	//PRMC_Play(index_4);

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
	if (index_1 >= 0)
	{
		PRMC_Stop(index_1);
		PRMC_Remove(index_1);
	}
	if (index_2 >= 0)
	{
		PRMC_Stop(index_2);
		PRMC_Remove(index_2);
	}
	if (index_3 >= 0)
	{
		PRMC_Stop(index_3);
		PRMC_Remove(index_3);
	}
	if (index_4 >= 0)
	{
		PRMC_Stop(index_4);
		PRMC_Remove(index_4);
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

		HWND hwnd = NULL;
		hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO1);
		index_1 = PRMC_Add(L"127.0.0.1", L"CH1", hwnd);
		if (index_1!=PRMC_FAIL)
			PRMC_Play(L"127.0.0.1", 554, index_1, 2016, 4, 16, 3, 14, 0, false);
	}
}


void CParallelRecordClientDlg::OnBnClickedButtonParallelRecorderDisconnect()
{
	// TODO: Add your control notification handler code here
	if (index_1 != PRMC_FAIL)
		index_1 = PRMC_Stop(L"127.0.0.1", index_1);
	if (index_1 != PRMC_FAIL)
		index_1 = PRMC_Remove(L"127.0.0.1", index_1);

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

	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (uuid.GetLength() > 0)
	{
		int years[10] = { 0 };
		int size = 0;
		PRMC_GetYears(L"127.0.0.1", (LPCWSTR)uuid, years, sizeof(years) / sizeof(int), size);

		_recording_years.InsertString(0, L"Select");
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
	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (uuid.GetLength()>0)
	{
		CString str_year;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		if (wcscmp(L"Select", str_year))
		{
			int year = _ttoi(str_year);
			int months[12] = { 0 };
			int size = 0;
			PRMC_GetMonths(L"127.0.0.1", (LPCWSTR)uuid, year, months, sizeof(months) / sizeof(int), size);

			_recording_months.InsertString(0, L"Select");
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
	CString uuid;
	_uuid.GetWindowTextW(uuid);

	if (uuid.GetLength()>0)
	{
		CString str_year, str_month;
		_recording_years.GetLBText(_recording_years.GetCurSel(), str_year);
		_recording_months.GetLBText(_recording_months.GetCurSel(), str_month);
		if (wcscmp(L"Select", str_year) && wcscmp(L"Select", str_month))
		{
			int year = _ttoi(str_year);
			int month = _ttoi(str_month);
			int days[31] = { 0 };
			int size = 0;
			PRMC_GetDays(L"127.0.0.1", (LPCWSTR)uuid, year, month, days, sizeof(days) / sizeof(int), size);

			_recording_days.InsertString(0, L"Select");
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
}

void CParallelRecordClientDlg::OnCbnSelchangeComboRecordingHours()
{
	// TODO: Add your control notification handler code here
}

void CParallelRecordClientDlg::OnCbnSelchangeComboMinutes()
{
	// TODO: Add your control notification handler code here
}

void CParallelRecordClientDlg::OnBnClickedButtonStartPlayback()
{
	// TODO: Add your control notification handler code here
}


void CParallelRecordClientDlg::OnBnClickedButtonStopPlayback()
{
	// TODO: Add your control notification handler code here
}

