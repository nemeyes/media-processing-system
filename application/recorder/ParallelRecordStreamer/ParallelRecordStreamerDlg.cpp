
// ParallelRecordStreamerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ParallelRecordStreamer.h"
#include "ParallelRecordStreamerDlg.h"
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


// CParallelRecordStreamerDlg dialog



CParallelRecordStreamerDlg::CParallelRecordStreamerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CParallelRecordStreamerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CParallelRecordStreamerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CParallelRecordStreamerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_TRAY, &CParallelRecordStreamerDlg::OnBnClickedButtonTray)
	ON_BN_CLICKED(IDC_BUTTON_START, &CParallelRecordStreamerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CParallelRecordStreamerDlg::OnBnClickedButtonStop)
	ON_MESSAGE(TRAY_NOTIFY, OnTrayIconClick)
	ON_COMMAND(ID_TRAY_START_STREAMING, &CParallelRecordStreamerDlg::OnTrayStartStreaming)
	ON_COMMAND(ID_TRAY_STOP_STREAMING, &CParallelRecordStreamerDlg::OnTrayStopStreaming)
	ON_COMMAND(ID_TRAY_EXIT, &CParallelRecordStreamerDlg::OnTrayExit)
	ON_COMMAND(ID_TRAY_SHUTDOWN, &CParallelRecordStreamerDlg::OnTrayShutdown)
END_MESSAGE_MAP()


// CParallelRecordStreamerDlg message handlers

BOOL CParallelRecordStreamerDlg::OnInitDialog()
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

	Position2Center();
	_is_streaming = FALSE;
	EnableTray(TRUE);
	StartStreaming();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CParallelRecordStreamerDlg::DestroyWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	StopStreaming();

	return CDialogEx::DestroyWindow();
}


void CParallelRecordStreamerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CParallelRecordStreamerDlg::OnPaint()
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
HCURSOR CParallelRecordStreamerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CParallelRecordStreamerDlg::Position2Center(void)
{
	//Getting the desktop hadle and rectangule
	RECT screen;
	GetDesktopWindow()->GetWindowRect(&screen);

	//Set windows size(see the width problem)

	// Get the current width and height of the console
	RECT client;
	GetWindowRect(&client);
	int width = client.right - client.left;
	int height = client.bottom - client.top;

	//caculate the window console to center of the screen	
	int x;
	int y;
	x = ((screen.right - screen.left) / 2 - width / 2);
	y = ((screen.bottom - screen.top) / 2 - height / 2);
	SetWindowPos(NULL, x, y, width, height, SWP_SHOWWINDOW || SWP_NOSIZE);
}

void CParallelRecordStreamerDlg::StartStreaming(void)
{
	if (!_is_streaming)
	{
		_streamer.start();

		CWnd *wnd = GetDlgItem(IDC_BUTTON_START);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_BUTTON_STOP);
		wnd->EnableWindow(TRUE);

		_is_streaming = TRUE;
	}
}

void CParallelRecordStreamerDlg::StopStreaming(void)
{
	if (_is_streaming)
	{
		_streamer.stop();

		CWnd *wnd = GetDlgItem(IDC_BUTTON_START);
		wnd->EnableWindow(TRUE);
		wnd = GetDlgItem(IDC_BUTTON_STOP);
		wnd->EnableWindow(FALSE);

		_is_streaming = FALSE;
	}
}

void CParallelRecordStreamerDlg::OnBnClickedButtonTray()
{
	// TODO: Add your control notification handler code here
	NOTIFYICONDATA nid;

	ShowWindow(SW_SHOWMINIMIZED);
	PostMessage(WM_SHOWWINDOW, FALSE, SW_OTHERUNZOOM);

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = 0;
	Shell_NotifyIcon(NIM_DELETE, &nid);

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	nid.hWnd = this->m_hWnd;
	CString str;
	GetWindowText(str);
	StrCpyW(nid.szTip, str.GetBuffer(str.GetLength() + 1));
	str.ReleaseBuffer();
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = TRAY_NOTIFY;
	Shell_NotifyIcon(NIM_ADD, &nid);
}


void CParallelRecordStreamerDlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	StartStreaming();
}


void CParallelRecordStreamerDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	StopStreaming();
}

void CParallelRecordStreamerDlg::EnableTray(BOOL enable)
{
	if (enable)
	{
		NOTIFYICONDATA nid;

		ShowWindow(SW_SHOWMINIMIZED);
		PostMessage(WM_SHOWWINDOW, FALSE, SW_OTHERUNZOOM);

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = this->m_hWnd;
		nid.uID = 0;
		Shell_NotifyIcon(NIM_DELETE, &nid);

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		nid.hWnd = this->m_hWnd;
		CString str;
		GetWindowText(str);
		StrCpyW(nid.szTip, str.GetBuffer(str.GetLength() + 1));
		str.ReleaseBuffer();
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = TRAY_NOTIFY;
		Shell_NotifyIcon(NIM_ADD, &nid);
	}
	else
	{
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = this->m_hWnd;
		nid.uID = 0;
		Shell_NotifyIcon(NIM_DELETE, &nid);

		ShowWindow(SW_RESTORE);
		SetForegroundWindow();
	}
}

LRESULT CParallelRecordStreamerDlg::OnTrayIconClick(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:   // 트레이아이콘 왼쪽버튼 더블클릭시 창이 다시 열린다.
	{
		EnableTray(FALSE);
		break;
	}
	case WM_RBUTTONDOWN:     // 트레이아이콘 오른쪽버튼 클릭 
	{
		CPoint ptMouse;
		::GetCursorPos(&ptMouse);

		CMenu Menu;
		Menu.LoadMenu(IDR_MENU_TRAY);
		CMenu *  pMenu = Menu.GetSubMenu(0);

		/*
		if( val == 1 )  // val은 전역변수
		{
		pMenu->EnableMenuItem( ID_MENUITEM32771, MF_GRAYED | MF_DISABLED);
		}
		else if( val == 2 )
		{
		pMenu->EnableMenuItem( ID_MENUITEM32771, MF_ENABLED);
		*/
		if (_is_streaming)
		{
			Menu.EnableMenuItem(ID_TRAY_START_STREAMING, MF_DISABLED | MF_GRAYED);
			Menu.EnableMenuItem(ID_TRAY_STOP_STREAMING, MF_ENABLED);
		}
		else
		{
			Menu.EnableMenuItem(ID_TRAY_START_STREAMING, MF_ENABLED);
			Menu.EnableMenuItem(ID_TRAY_STOP_STREAMING, MF_DISABLED | MF_GRAYED);
		}
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, AfxGetMainWnd());
		break;
	}
	}
	return 0;
}

void CParallelRecordStreamerDlg::OnTrayStartStreaming()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	StartStreaming();
}


void CParallelRecordStreamerDlg::OnTrayStopStreaming()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	StopStreaming();
}

void CParallelRecordStreamerDlg::OnTrayShutdown()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	EndDialog(IDOK);
}

void CParallelRecordStreamerDlg::OnTrayExit()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}
