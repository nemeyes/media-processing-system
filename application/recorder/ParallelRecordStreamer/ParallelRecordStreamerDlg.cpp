
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

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
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
	_server.start();
}


void CParallelRecordStreamerDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	_server.stop();
}

LRESULT CParallelRecordStreamerDlg::OnTrayIconClick(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:   // 트레이아이콘 왼쪽버튼 더블클릭시 창이 다시 열린다.
	{
		NOTIFYICONDATA nid;

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = this->m_hWnd;
		nid.uID = 0;
		Shell_NotifyIcon(NIM_DELETE, &nid);

		ShowWindow(SW_RESTORE);
		SetForegroundWindow();
	}
	break;
	case WM_RBUTTONDOWN:     // 트레이아이콘 오른쪽버튼 클릭 
	{

	}
	break;
	}
	return 0;
}
