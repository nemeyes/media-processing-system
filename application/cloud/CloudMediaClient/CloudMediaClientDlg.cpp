
// CloudMediaClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CloudMediaClient.h"
#include "CloudMediaClientDlg.h"
#include "afxdialogex.h"

#include "media_edge_client.h"
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


// CCloudMediaClientDlg dialog



CCloudMediaClientDlg::CCloudMediaClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCloudMediaClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCloudMediaClientDlg::EnableConnectButton(BOOL enable)
{
	CWnd * wnd = GetDlgItem(IDC_BUTTON_CONNECT);
	wnd->EnableWindow(enable);
}

void CCloudMediaClientDlg::EnableDisconnectButton(BOOL enable)
{
	CWnd * wnd = GetDlgItem(IDC_BUTTON_DISCONNECT);
	wnd->EnableWindow(enable);
}

void CCloudMediaClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ADDRESS, _server_address);
	DDX_Control(pDX, IDC_EDIT_PORT_NUMBER, _server_port_number);
}

BEGIN_MESSAGE_MAP(CCloudMediaClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CCloudMediaClientDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CCloudMediaClientDlg::OnBnClickedButtonDisconnect)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CCloudMediaClientDlg message handlers

BOOL CCloudMediaClientDlg::OnInitDialog()
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
	EnableConnectButton(TRUE);
	EnableDisconnectButton(FALSE);

	_server_address.SetWindowText(L"127.0.0.1");
	_server_port_number.SetWindowTextW(L"15000");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCloudMediaClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCloudMediaClientDlg::OnPaint()
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

BOOL CCloudMediaClientDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	if (_client)
	{
		_client->disconnect();
		delete _client;
		_client = nullptr;
	}

	return CDialogEx::DestroyWindow();
}

BOOL CCloudMediaClientDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CDialogEx::OnEraseBkgnd(pDC);
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCloudMediaClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCloudMediaClientDlg::OnBnClickedButtonConnect()
{
	// TODO: Add your control notification handler code here
	if (_client)
	{
		_client->disconnect();
		delete _client;
		_client = nullptr;
	}

	char * server_address = nullptr;
	int32_t server_port_number = 0;

	CString str_server_address;
	_server_address.GetWindowTextW(str_server_address);
	dk_string_helper::convert_wide2multibyte((LPWSTR)(LPCWSTR)str_server_address, &server_address);
	if (!server_address || strlen(server_address) < 1)
		return;

	CString str_server_port_number;
	_server_port_number.GetWindowTextW(str_server_port_number);
	server_port_number = _ttoi(str_server_port_number);

	_client = new ic::media_edge_client(this);
	bool result = _client->connect(server_address, server_port_number);

	if (server_address)
		free(server_address);
}

void CCloudMediaClientDlg::OnBnClickedButtonDisconnect()
{
	// TODO: Add your control notification handler code here
	if (_client)
	{
		bool result = _client->disconnect();
		delete _client;
		_client = nullptr;
	}
}
