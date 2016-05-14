
// ParallelRecordServerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "ParallelRecordServer.h"
#include "ParallelRecordServerDlg.h"
#include "afxdialogex.h"

#include "dk_recorder_service.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CParallelRecordServerDlg ��ȭ ����



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


// CParallelRecordServerDlg �޽��� ó����

BOOL CParallelRecordServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	Position2Center();
	_is_recording = FALSE;
	EnableTray(TRUE);
	StartRecording();
	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

BOOL CParallelRecordServerDlg::DestroyWindow()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CParallelRecordServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
		case WM_LBUTTONDBLCLK:   // Ʈ���̾����� ���ʹ�ư ����Ŭ���� â�� �ٽ� ������.
		{
			EnableTray(FALSE);
			break;
		}
		case WM_RBUTTONDOWN:     // Ʈ���̾����� �����ʹ�ư Ŭ�� 
		{
			CPoint ptMouse;
			::GetCursorPos(&ptMouse);

			CMenu Menu;
			Menu.LoadMenu(IDR_MENU_TRAY);
			CMenu *  pMenu = Menu.GetSubMenu(0);

			/*
		      if( val == 1 )  // val�� ��������
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
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	StartRecording();
}


void CParallelRecordServerDlg::OnTrayStopRecording()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	StopRecording();
}

void CParallelRecordServerDlg::OnTrayShutdown()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	EndDialog(IDOK);
}

void CParallelRecordServerDlg::OnTrayExit()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
}


void CParallelRecordServerDlg::OnUpdateTrayStartRecording(CCmdUI *pCmdUI)
{
	// TODO: ���⿡ ��� ������Ʈ UI ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��� ������Ʈ UI ó���� �ڵ带 �߰��մϴ�.
	if (_is_recording)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}
