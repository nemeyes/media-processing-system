
// VmxnetParallelRTSPServerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "VmxnetParallelRTSPServer.h"
#include "VmxnetParallelRTSPServerDlg.h"
#include "afxdialogex.h"

#include <BasicUsageEnvironment.hh>
#include "vmxnet_rtsp_server.h"
#include "log/logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	// 구현입니다.
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


// CVmxnetParallelRTSPServerDlg 대화 상자




CVmxnetParallelRTSPServerDlg::CVmxnetParallelRTSPServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVmxnetParallelRTSPServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVmxnetParallelRTSPServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVmxnetParallelRTSPServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CVmxnetParallelRTSPServerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CVmxnetParallelRTSPServerDlg::OnBnClickedButtonStop)
	ON_MESSAGE(TRAY_NOTIFY, OnTrayIconClick)
	ON_BN_CLICKED(IDC_BUTTON_TO_TRAY, &CVmxnetParallelRTSPServerDlg::OnBnClickedButtonToTray)
END_MESSAGE_MAP()


// CVmxnetParallelRTSPServerDlg 메시지 처리기

BOOL CVmxnetParallelRTSPServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	EnableTray();

	_stop_streaming = false;
	_thread = INVALID_HANDLE_VALUE;
	Start();
	CWnd *wnd = GetDlgItem(IDC_BUTTON_START);
	wnd->EnableWindow(FALSE);
	wnd = GetDlgItem(IDC_BUTTON_STOP);
	wnd->EnableWindow(TRUE);
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CVmxnetParallelRTSPServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CVmxnetParallelRTSPServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CVmxnetParallelRTSPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVmxnetParallelRTSPServerDlg::Start()
{
	if (_thread != INVALID_HANDLE_VALUE)
	{
		_stop_streaming = true;
		::WaitForSingleObjectEx(_thread, INFINITE, FALSE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}

	unsigned int thrd_addr;
	_stop_streaming = false;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, &CVmxnetParallelRTSPServerDlg::ProcessStreaming, this, CREATE_SUSPENDED, &thrd_addr);
	::SetThreadPriority(_thread, THREAD_PRIORITY_HIGHEST);
	::ResumeThread(_thread);
}

void CVmxnetParallelRTSPServerDlg::OnBnClickedButtonStart()
{
	Start();
	CWnd *wnd = GetDlgItem(IDC_BUTTON_START);
	wnd->EnableWindow(FALSE);
	wnd = GetDlgItem(IDC_BUTTON_STOP);
	wnd->EnableWindow(TRUE);
}

void CVmxnetParallelRTSPServerDlg::OnBnClickedButtonStop()
{
	if (_thread != INVALID_HANDLE_VALUE)
	{
		_stop_streaming = true;
		::WaitForSingleObjectEx(_thread, INFINITE, FALSE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}
	CWnd *wnd = GetDlgItem(IDC_BUTTON_START);
	wnd->EnableWindow(TRUE);
	wnd = GetDlgItem(IDC_BUTTON_STOP);
	wnd->EnableWindow(FALSE);
}

void CVmxnetParallelRTSPServerDlg::DoStreaming(void)
{
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	UserAuthenticationDatabase* authDB = NULL;
	/*
	#ifdef ACCESS_CONTROL
	authDB								= new UserAuthenticationDatabase;
	authDB->addUserRecord( "nemeyes", "7224" );
	#endif
	*/

	// Create the RTSP server.  Try first with the default port number (554),
	// and then with the alternative port number (8554):
	RTSPServer* rtspServer;
	portNumBits rtspServerPortNum = 554;
	rtspServer = vmxnet_rtsp_server::createNew(*env, rtspServerPortNum, authDB);
	if (rtspServer == NULL)
	{
		rtspServerPortNum = 8554;
		rtspServer = vmxnet_rtsp_server::createNew(*env, rtspServerPortNum, authDB);
	}
	if (rtspServer == NULL)
	{
		logger::instance().make_system_fatal_log("failed to create RTSP server");
		return;
	}


	char log[100] = { 0 };
	_snprintf(log, sizeof(log), "RTSP server is started with portnumber [%d]", rtspServerPortNum);
	logger::instance().make_system_info_log(log);

	if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080))
	{
		memset(log, 0x00, sizeof(log));
		_snprintf(log, sizeof(log), "port number %d is used for optional RTSP-over-HTTP tunneling", rtspServer->httpServerPortNum());
		logger::instance().make_system_info_log(log);
	}
	else
	{
		logger::instance().make_system_warn_log("RTSP-over-HTTP tunneling is not available.");
	}
	env->taskScheduler().doEventLoop(((char*)&_stop_streaming)); // does not return
}

unsigned __stdcall CVmxnetParallelRTSPServerDlg::ProcessStreaming(void *self)
{
	CVmxnetParallelRTSPServerDlg *rtsp_server = static_cast<CVmxnetParallelRTSPServerDlg*>(self);
	rtsp_server->DoStreaming();
	return 0;
}


void CVmxnetParallelRTSPServerDlg::EnableTray()
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

void CVmxnetParallelRTSPServerDlg::DisableTray()
{
	NOTIFYICONDATA nid;

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = 0;
	Shell_NotifyIcon(NIM_DELETE, &nid);

	ShowWindow(SW_RESTORE);
	SetForegroundWindow();
}

LRESULT CVmxnetParallelRTSPServerDlg::OnTrayIconClick(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:   // 트레이아이콘 왼쪽버튼 더블클릭시 창이 다시 열린다.
	{
		DisableTray();
	}
	break;
	case WM_RBUTTONDOWN:     // 트레이아이콘 오른쪽버튼 클릭 
	{

	}
	break;
	}
	return 0;
}



void CVmxnetParallelRTSPServerDlg::OnBnClickedButtonToTray()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	EnableTray();
}
