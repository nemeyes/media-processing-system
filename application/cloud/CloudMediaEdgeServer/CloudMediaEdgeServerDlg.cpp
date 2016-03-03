
// CloudMediaEdgeServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CloudMediaEdgeServer.h"
#include "CloudMediaEdgeServerDlg.h"
#include "afxdialogex.h"

#include "media_edge_server.h"
#include <dk_string_helper.h>

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


// CCloudMediaEdgeServerDlg dialog



CCloudMediaEdgeServerDlg::CCloudMediaEdgeServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCloudMediaEdgeServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCloudMediaEdgeServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_UUID, _server_uuid);
	DDX_Control(pDX, IDC_EDIT_PORT_NUMBER, _server_port_number);
}

BEGIN_MESSAGE_MAP(CCloudMediaEdgeServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CCloudMediaEdgeServerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CCloudMediaEdgeServerDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CCloudMediaEdgeServerDlg message handlers

BOOL CCloudMediaEdgeServerDlg::OnInitDialog()
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
	CWnd * wnd = NULL;
	wnd = GetDlgItem(IDC_BUTTON_START);
	wnd->EnableWindow(TRUE);
	wnd = GetDlgItem(IDC_BUTTON_STOP);
	wnd->EnableWindow(FALSE);

	_server_uuid.SetWindowText(L"9701AE27-7655-41CA-ADEA-F8AEA3BD645C");
	_server_port_number.SetWindowTextW(L"15000");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCloudMediaEdgeServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCloudMediaEdgeServerDlg::OnPaint()
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
HCURSOR CCloudMediaEdgeServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCloudMediaEdgeServerDlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	if (_server)
	{
		_server->stop();
		delete _server;
		_server = nullptr;
	}

	char * server_uuid = nullptr;
	int32_t server_port_number = 0;

	CString str_server_uuid;
	_server_uuid.GetWindowTextW(str_server_uuid);
	dk_string_helper::convert_wide2multibyte((LPWSTR)(LPCWSTR)str_server_uuid, &server_uuid);
	if (!server_uuid || strlen(server_uuid) < 1)
		return;

	CString str_server_port_number;
	_server_port_number.GetWindowTextW(str_server_port_number);
	server_port_number = _ttoi(str_server_port_number);

	_server = new ic::media_edge_server("9701AE27-7655-41CA-ADEA-F8AEA3BD645C");
	bool result = _server->start(0, server_port_number);

	if (server_uuid)
		free(server_uuid);

	if (result)
	{
		CWnd * wnd = NULL;
		wnd = GetDlgItem(IDC_BUTTON_START);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_BUTTON_STOP);
		wnd->EnableWindow(TRUE);
	}
}


void CCloudMediaEdgeServerDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	if (_server)
	{
		bool result = _server->stop();
		delete _server;
		_server = nullptr;

		if (result)
		{
			CWnd * wnd = NULL;
			wnd = GetDlgItem(IDC_BUTTON_START);
			wnd->EnableWindow(TRUE);
			wnd = GetDlgItem(IDC_BUTTON_STOP);
			wnd->EnableWindow(FALSE);
		}
	}
}


BOOL CCloudMediaEdgeServerDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	if (_server)
	{
		_server->stop();
		delete _server;
		_server = nullptr;
	}
	return CDialogEx::DestroyWindow();
}
