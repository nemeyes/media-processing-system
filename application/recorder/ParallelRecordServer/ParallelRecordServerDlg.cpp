
// ParallelRecordServerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ParallelRecordServer.h"
#include "ParallelRecordServerDlg.h"
#include "afxdialogex.h"

#include "dk_recorder_service.h"

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
public:
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


// CParallelRecordServerDlg 대화 상자



CParallelRecordServerDlg::CParallelRecordServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CParallelRecordServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CParallelRecordServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CParallelRecordServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_RECORD, &CParallelRecordServerDlg::OnBnClickedButtonStartRecord)
	ON_BN_CLICKED(IDC_BUTTON_STOP_RECORD, &CParallelRecordServerDlg::OnBnClickedButtonStopRecord)
	ON_BN_CLICKED(IDC_BUTTON_TO_TRAY, &CParallelRecordServerDlg::OnBnClickedButtonToTray)
	ON_MESSAGE(TRAY_NOTIFY, OnTrayIconClick)
	ON_COMMAND(ID_TRAY_START_RECORDING, &CParallelRecordServerDlg::OnTrayStartRecording)
	ON_COMMAND(ID_TRAY_STOP_RECORDING, &CParallelRecordServerDlg::OnTrayStopRecording)
	ON_COMMAND(ID_TRAY_EXIT, &CParallelRecordServerDlg::OnTrayExit)
	ON_UPDATE_COMMAND_UI(ID_TRAY_START_RECORDING, &CParallelRecordServerDlg::OnUpdateTrayStartRecording)
	ON_UPDATE_COMMAND_UI(ID_TRAY_STOP_RECORDING, &CParallelRecordServerDlg::OnUpdateTrayStopRecording)
	ON_COMMAND(ID_TRAY_SHUTDOWN, &CParallelRecordServerDlg::OnTrayShutdown)
END_MESSAGE_MAP()


// CParallelRecordServerDlg 메시지 처리기

BOOL CParallelRecordServerDlg::OnInitDialog()
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	Position2Center();
	_is_recording = FALSE;
	EnableTray(TRUE);
	StartRecording();
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

BOOL CParallelRecordServerDlg::DestroyWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	StopRecording();
	return CDialogEx::DestroyWindow();
}

void CParallelRecordServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CParallelRecordServerDlg::OnPaint()
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
HCURSOR CParallelRecordServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CParallelRecordServerDlg::Position2Center(void)
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

void CParallelRecordServerDlg::StartRecording(void)
{
	if (!_is_recording)
	{
		debuggerking::recorder_service::instance().start_recording();
		CWnd *wnd = GetDlgItem(IDC_BUTTON_START_RECORD);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_BUTTON_STOP_RECORD);
		wnd->EnableWindow(TRUE);

		_is_recording = TRUE;
	}
}

void CParallelRecordServerDlg::StopRecording(void)
{
	if (_is_recording)
	{
		debuggerking::recorder_service::instance().stop_recording();
		CWnd *wnd = GetDlgItem(IDC_BUTTON_START_RECORD);
		wnd->EnableWindow(TRUE);
		wnd = GetDlgItem(IDC_BUTTON_STOP_RECORD);
		wnd->EnableWindow(FALSE);

		_is_recording = FALSE;
	}
}

void CParallelRecordServerDlg::OnBnClickedButtonStartRecord()
{
	// TODO: Add your control notification handler code here
	StartRecording();
}


void CParallelRecordServerDlg::OnBnClickedButtonStopRecord()
{
	// TODO: Add your control notification handler code here
	StopRecording();
}

void CParallelRecordServerDlg::OnBnClickedButtonToTray()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	EnableTray(TRUE);
}

void CParallelRecordServerDlg::EnableTray(BOOL enable)
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

LRESULT CParallelRecordServerDlg::OnTrayIconClick(WPARAM wParam, LPARAM lParam)
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
			if (_is_recording)
			{
				Menu.EnableMenuItem(ID_TRAY_START_RECORDING, MF_DISABLED | MF_GRAYED);
				Menu.EnableMenuItem(ID_TRAY_STOP_RECORDING, MF_ENABLED);
			}
			else
			{
				Menu.EnableMenuItem(ID_TRAY_START_RECORDING, MF_ENABLED);
				Menu.EnableMenuItem(ID_TRAY_STOP_RECORDING, MF_DISABLED | MF_GRAYED);
			}
			pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, AfxGetMainWnd());
			break;
		}
	}
	return 0;
}

void CParallelRecordServerDlg::OnTrayStartRecording()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	StartRecording();
}


void CParallelRecordServerDlg::OnTrayStopRecording()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	StopRecording();
}

void CParallelRecordServerDlg::OnTrayShutdown()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	EndDialog(IDOK);
}

void CParallelRecordServerDlg::OnTrayExit()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}


void CParallelRecordServerDlg::OnUpdateTrayStartRecording(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	if (_is_recording)
	{
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->Enable(TRUE);
	}
}


void CParallelRecordServerDlg::OnUpdateTrayStopRecording(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	if (_is_recording)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}
