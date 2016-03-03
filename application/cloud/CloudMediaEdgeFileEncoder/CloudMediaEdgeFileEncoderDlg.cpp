
// CloudMediaEdgeFileEncoderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CloudMediaEdgeFileEncoder.h"
#include "CloudMediaEdgeFileEncoderDlg.h"
#include "afxdialogex.h"

#include "dk_string_helper.h"

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


// CCloudMediaEdgeFileEncoderDlg dialog



CCloudMediaEdgeFileEncoderDlg::CCloudMediaEdgeFileEncoderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCloudMediaEdgeFileEncoderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCloudMediaEdgeFileEncoderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCloudMediaEdgeFileEncoderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CCloudMediaEdgeFileEncoderDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CCloudMediaEdgeFileEncoderDlg::OnBnClickedButtonStop)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CCloudMediaEdgeFileEncoderDlg message handlers

BOOL CCloudMediaEdgeFileEncoderDlg::OnInitDialog()
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

void CCloudMediaEdgeFileEncoderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCloudMediaEdgeFileEncoderDlg::OnPaint()
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
		CBrush brush;
		brush.CreateSolidBrush(RGB(0, 0, 0));

		CRect rect;
		CWnd * video_view = GetDlgItem(IDC_STATIC_VIDEO_VIEW);
		CDC * video_view_dc = video_view->GetDC();
		video_view->GetClientRect(rect);
		video_view_dc->FillRect(rect, &brush);
		brush.DeleteObject();

		CDialogEx::OnPaint();
	}
}

BOOL CCloudMediaEdgeFileEncoderDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return CDialogEx::OnEraseBkgnd(pDC);
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCloudMediaEdgeFileEncoderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCloudMediaEdgeFileEncoderDlg::OnBnClickedButtonPlay()
{
	// TODO: Add your control notification handler code here
	wchar_t filter[] = L"Media Files(*.mkv, *.avi, *.mp4, *.wmv)|*.mkv;*.avi;*.mp4;*.wmv||"; //L"All Files(*.*)|*.*||";
	CFileDialog dlg(TRUE, L"mkv", NULL, OFN_HIDEREADONLY, filter);

	if (dlg.DoModal() == IDOK)
	{
		CString filepath = dlg.GetPathName();
		if (filepath.GetLength()>0)
			_encoder.play((LPCWSTR)filepath, ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO_VIEW));
	}
}


void CCloudMediaEdgeFileEncoderDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	_encoder.stop();
}

