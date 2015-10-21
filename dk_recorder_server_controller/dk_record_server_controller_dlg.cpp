
// record_serverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dk_record_server_controller.h"
#include "dk_record_server_controller_dlg.h"
#include "afxdialogex.h"
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


// Crecord_serverDlg dialog



dk_record_server_controller_dlg::dk_record_server_controller_dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(dk_record_server_controller_dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void dk_record_server_controller_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_URL, _url);
	DDX_Control(pDX, IDC_EDIT_USERNAME, _username);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, _password);
	DDX_Control(pDX, IDC_COMBO_STREAMING_PROTOCOL, _cmb_streaming_protocol);
	DDX_Control(pDX, IDC_COMBO_TRANSPORT_TYPE, _cmb_transport_type);
	DDX_Control(pDX, IDC_COMBO_RECV_OPTION, _cmb_recv_option);
	DDX_Control(pDX, IDC_COMBO_RETRY_CONNECTION, _cmb_retry_connection);
}

BEGIN_MESSAGE_MAP(dk_record_server_controller_dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_RECORD, &dk_record_server_controller_dlg::OnBnClickedButtonStartRecord)
	ON_BN_CLICKED(IDC_BUTTON_STOP_RECORD, &dk_record_server_controller_dlg::OnBnClickedButtonStopRecord)
	ON_BN_CLICKED(IDC_BUTTON_START_PREVIEW, &dk_record_server_controller_dlg::OnBnClickedButtonStartPreview)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PREVIEW, &dk_record_server_controller_dlg::OnBnClickedButtonStopPreview)
END_MESSAGE_MAP()


// Crecord_serverDlg message handlers

BOOL dk_record_server_controller_dlg::OnInitDialog()
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
	_cmb_streaming_protocol.SetCurSel(0);
	_cmb_transport_type.InsertString(dk_rtsp_client::RTP_OVER_UDP, L"RTP OVER TCP");
	_cmb_transport_type.InsertString(dk_rtsp_client::RTP_OVER_TCP, L"RTP OVER UDP");
	_cmb_transport_type.InsertString(dk_rtsp_client::RTP_OVER_HTTP, L"RTP OVER HTTP");
	_cmb_transport_type.SetCurSel(0);
	_cmb_recv_option.InsertString(dk_rtsp_client::RECV_AUDIO_VIDEO, L"Recv Audio & Video");
	_cmb_recv_option.InsertString(dk_rtsp_client::RECV_VIDEO, L"Recv Video");
	_cmb_recv_option.InsertString(dk_rtsp_client::RECV_AUDIO, L"Recv Audio");
	_cmb_recv_option.SetCurSel(0);
	_cmb_retry_connection.SetCurSel(0);

	//_url.SetWindowTextW(_T("rtsp://127.0.0.1:8554/01.264"));
	_url.SetWindowTextW(_T("rtsp://10.202.140.138/01.264"));
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void dk_record_server_controller_dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void dk_record_server_controller_dlg::OnPaint()
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
HCURSOR dk_record_server_controller_dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void dk_record_server_controller_dlg::OnBnClickedButtonStartPreview()
{
	// TODO: Add your control notification handler code here
}

void dk_record_server_controller_dlg::OnBnClickedButtonStopPreview()
{
	// TODO: Add your control notification handler code here
}

void dk_record_server_controller_dlg::OnBnClickedButtonStartRecord()
{
	// TODO: Add your control notification handler code here
	CString str_url;
	CString str_username;
	CString str_password;
	_url.GetWindowTextW(str_url);
	_username.GetWindowTextW(str_username);
	_password.GetWindowTextW(str_password);

	char * url = 0;
	char * username = 0;
	char * password = 0;

	dk_string_helper::convert_wide2multibyte((LPTSTR)(LPCTSTR)str_url, &url);
	dk_string_helper::convert_wide2multibyte((LPTSTR)(LPCTSTR)str_username, &username);
	dk_string_helper::convert_wide2multibyte((LPTSTR)(LPCTSTR)str_password, &password);

	_rtsp_receiver.start_recording(url, username, password, _cmb_transport_type.GetCurSel(), _cmb_recv_option.GetCurSel());

	if (url)
		free(url);
	if (username)
		free(username);
	if (password)
		free(password);
}

void dk_record_server_controller_dlg::OnBnClickedButtonStopRecord()
{
	// TODO: Add your control notification handler code here
	_rtsp_receiver.stop_recording();
}



