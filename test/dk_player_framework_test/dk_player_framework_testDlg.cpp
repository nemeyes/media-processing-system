
// dk_player_framework_testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dk_player_framework_test.h"
#include "dk_player_framework_testDlg.h"
#include "afxdialogex.h"
#include <uuids.h>
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>

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
public:
	afx_msg void OnPaint();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// Cdk_player_framework_testDlg dialog



Cdk_player_framework_testDlg::Cdk_player_framework_testDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cdk_player_framework_testDlg::IDD, pParent)
	, _fullscreen(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cdk_player_framework_testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DXVA2_DECODER_GUIDS, _dxva2_decoder_guids);
}

BEGIN_MESSAGE_MAP(Cdk_player_framework_testDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &Cdk_player_framework_testDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &Cdk_player_framework_testDlg::OnBnClickedButtonStop)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &Cdk_player_framework_testDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_CHECK_ASPECT_RATIO, &Cdk_player_framework_testDlg::OnBnClickedCheckAspectRatio)
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_BUTTON_OPEN_FILE, &Cdk_player_framework_testDlg::OnBnClickedButtonOpenFile)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_RTSP, &Cdk_player_framework_testDlg::OnBnClickedButtonOpenRtsp)
	ON_BN_CLICKED(IDC_BUTTON_HSL, &Cdk_player_framework_testDlg::OnBnClickedButtonHsl)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_RTMP, &Cdk_player_framework_testDlg::OnBnClickedButtonOpenRtmp)
END_MESSAGE_MAP()


// Cdk_player_framework_testDlg message handlers

BOOL Cdk_player_framework_testDlg::OnInitDialog()
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
	CheckDlgButton(IDC_CHECK_ASPECT_RATIO, TRUE);
	CheckDlgButton(IDC_CHECK_USE_CLOCK, TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cdk_player_framework_testDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cdk_player_framework_testDlg::OnPaint()
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

		PAINTSTRUCT ps;
		HDC hdc;
		hdc = ::BeginPaint(::GetDlgItem(this->m_hWnd, IDC_STATIC_VIDEO_VIEW), &ps);
		if (_player.state() != dk_player_framework::STATE_NO_GRAPH)
		{
			// The player has video, so ask the player to repaint. 
			_player.repaint(hdc);
		}
		else
		{
			FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}
		::EndPaint(::GetDlgItem(this->m_hWnd, IDC_STATIC_VIDEO_VIEW), &ps);
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cdk_player_framework_testDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cdk_player_framework_testDlg::OnBnClickedButtonPlay()
{
	// TODO: Add your control notification handler code here
	if (_player.state() == dk_player_framework::STATE_PAUSED)
	{
		_player.play();
	}
}

void Cdk_player_framework_testDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	_player.stop();
}

void Cdk_player_framework_testDlg::OnBnClickedButtonPause()
{
	// TODO: Add your control notification handler code here
	_player.pause();
}

void CAboutDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages
}

void Cdk_player_framework_testDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	RECT rc;
	::GetClientRect(::GetDlgItem(this->m_hWnd, IDC_STATIC_VIDEO_VIEW), &rc);
	_player.update_video_windows(&rc);
}


void Cdk_player_framework_testDlg::OnBnClickedCheckAspectRatio()
{
	// TODO: Add your control notification handler code here
	BOOL ar_checked = IsDlgButtonChecked(IDC_CHECK_ASPECT_RATIO);
	if (ar_checked)
		_player.aspect_ratio(true);
	else
		_player.aspect_ratio(false);
}

void Cdk_player_framework_testDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (_player.state() == dk_player_framework::STATE_NO_GRAPH)
		return;
		
	if (_fullscreen)
	{
		SetWindowPos(&wndNoTopMost, _original_rect.left, _original_rect.top, _original_rect.Width(), _original_rect.Height(), SWP_HIDEWINDOW);
		_player.fullscreen(false);
		ShowWindow(SW_SHOW);
		_fullscreen = FALSE;
	}
	else
	{
		GetWindowRect(&_original_rect);
		HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		if (!monitor)
			return;

		MONITORINFO info;
		info.cbSize = sizeof(info);
		if (!GetMonitorInfo(monitor, &info))
			return;

		CRect rect(info.rcMonitor);
		SetWindowPos(&wndTopMost, rect.left, rect.top, rect.Width(), rect.Height(), SWP_HIDEWINDOW);
		_player.fullscreen(true);
		ShowWindow(SW_SHOW);
		_fullscreen = TRUE;
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void Cdk_player_framework_testDlg::OnBnClickedButtonOpenFile()
{
	// TODO: Add your control notification handler code here
	BOOL ar_checked = IsDlgButtonChecked(IDC_CHECK_ASPECT_RATIO);
	BOOL uc_checked = IsDlgButtonChecked(IDC_CHECK_USE_CLOCK);

	wchar_t filter[] = L"MKV Media Files(*.mkv)|*.mkv||"; //L"All Files(*.*)|*.*||";
	CFileDialog dlg(TRUE, L"mkv", NULL, OFN_HIDEREADONLY, filter);

	if (dlg.DoModal() == IDOK)
	{
		_filename = dlg.GetPathName();

		if (_player.state() == dk_player_framework::STATE_RUNNING || _player.state() == dk_player_framework::STATE_PAUSED)
		{
			_player.stop();
			_player.release();
		}
		_player.initialize(::GetDlgItem(this->m_hWnd, IDC_STATIC_VIDEO_VIEW), ar_checked ? true : false, uc_checked ? true : false);
		_player.open_file((LPWSTR)(LPCWSTR)_filename);


		_dxva2_decoder_guids.Clear();
		std::vector<GUID>::iterator iter;
		std::vector<GUID> dxva2_decoder_guids;
		_player.list_dxva2_decoder_guids(&dxva2_decoder_guids);
		for (iter = dxva2_decoder_guids.begin(); iter != dxva2_decoder_guids.end(); iter++)
		{
			if(IsEqualGUID(DXVA2_ModeMPEG2_MoComp, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG2_MoComp");
			else if (IsEqualGUID(DXVA2_ModeMPEG2_IDCT, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG2_IDCT");
			else if (IsEqualGUID(DXVA2_ModeMPEG2_VLD, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG2_VLD");
			else if (IsEqualGUID(DXVA2_ModeMPEG1_VLD, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG1_VLD");
			else if (IsEqualGUID(DXVA2_ModeMPEG2and1_VLD, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG2and1_VLD");
			else if (IsEqualGUID(DXVA2_ModeH264_A, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_A");
			else if (IsEqualGUID(DXVA2_ModeH264_B, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_B");
			else if (IsEqualGUID(DXVA2_ModeH264_C, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_C");
			else if (IsEqualGUID(DXVA2_ModeH264_D, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_D");
			else if (IsEqualGUID(DXVA2_ModeH264_E, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_E");
			else if (IsEqualGUID(DXVA2_ModeH264_F, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_F");
			else if (IsEqualGUID(DXVA2_ModeH264_VLD_Stereo_Progressive_NoFGT, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_VLD_Stereo_Progressive_NoFGT");
			else if (IsEqualGUID(DXVA2_ModeH264_VLD_Stereo_NoFGT, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_VLD_Stereo_NoFGT");
			else if (IsEqualGUID(DXVA2_ModeH264_VLD_Multiview_NoFGT, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeH264_VLD_Multiview_NoFGT");
			else if (IsEqualGUID(DXVA2_ModeWMV8_A, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeWMV8_A");
			else if (IsEqualGUID(DXVA2_ModeWMV8_B, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeWMV8_B");
			else if (IsEqualGUID(DXVA2_ModeWMV9_A, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeWMV9_A");
			else if (IsEqualGUID(DXVA2_ModeWMV9_B, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeWMV9_B");
			else if (IsEqualGUID(DXVA2_ModeWMV9_C, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeWMV9_C");
			else if (IsEqualGUID(DXVA2_ModeVC1_A, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeVC1_A");
			else if (IsEqualGUID(DXVA2_ModeVC1_B, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeVC1_B");
			else if (IsEqualGUID(DXVA2_ModeVC1_C, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeVC1_C");
			else if (IsEqualGUID(DXVA2_ModeVC1_D, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeVC1_D");
			else if (IsEqualGUID(DXVA2_ModeVC1_D2010, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeVC1_D2010");
			else if (IsEqualGUID(DXVA2_NoEncrypt, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_NoEncrypt");
			else if (IsEqualGUID(DXVA2_VideoProcProgressiveDevice, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_VideoProcProgressiveDevice");
			else if (IsEqualGUID(DXVA2_VideoProcBobDevice, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_VideoProcBobDevice");
			else if (IsEqualGUID(DXVA2_VideoProcSoftwareDevice, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_VideoProcSoftwareDevice");
			else if (IsEqualGUID(DXVA2_ModeMPEG4pt2_VLD_Simple, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG4pt2_VLD_Simple");
			else if (IsEqualGUID(DXVA2_ModeMPEG4pt2_VLD_AdvSimple_NoGMC, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG4pt2_VLD_AdvSimple_NoGMC");
			else if (IsEqualGUID(DXVA2_ModeMPEG4pt2_VLD_AdvSimple_GMC, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeMPEG4pt2_VLD_AdvSimple_GMC");
			else if (IsEqualGUID(DXVA2_ModeHEVC_VLD_Main, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeHEVC_VLD_Main");
			else if (IsEqualGUID(DXVA2_ModeHEVC_VLD_Main10, (*iter)))
				_dxva2_decoder_guids.AddString(L"DXVA2_ModeHEVC_VLD_Main10");
			else
			{
				OLECHAR wszGuid[40] = { 0 };
				CoCreateGuid(&(*iter));
				StringFromGUID2((*iter), wszGuid, _countof(wszGuid));
				_dxva2_decoder_guids.AddString(wszGuid);
			}
		}

		_player.play();
	}
}


void Cdk_player_framework_testDlg::OnBnClickedButtonOpenRtsp()
{
	// TODO: Add your control notification handler code here
}


void Cdk_player_framework_testDlg::OnBnClickedButtonHsl()
{
	// TODO: Add your control notification handler code here
}


void Cdk_player_framework_testDlg::OnBnClickedButtonOpenRtmp()
{
	// TODO: Add your control notification handler code here
}
