
// dk_mpeg2ts_demuxer_testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dk_mpeg2ts_demuxer_test.h"
#include "dk_mpeg2ts_demuxer_testDlg.h"
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


// Cdk_mpeg2ts_demuxer_testDlg dialog



Cdk_mpeg2ts_demuxer_testDlg::Cdk_mpeg2ts_demuxer_testDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cdk_mpeg2ts_demuxer_testDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cdk_mpeg2ts_demuxer_testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cdk_mpeg2ts_demuxer_testDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FILE, &Cdk_mpeg2ts_demuxer_testDlg::OnBnClickedButtonFile)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// Cdk_mpeg2ts_demuxer_testDlg message handlers

BOOL Cdk_mpeg2ts_demuxer_testDlg::OnInitDialog()
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

void Cdk_mpeg2ts_demuxer_testDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cdk_mpeg2ts_demuxer_testDlg::OnPaint()
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
HCURSOR Cdk_mpeg2ts_demuxer_testDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cdk_mpeg2ts_demuxer_testDlg::OnBnClickedButtonFile()
{
	// TODO: Add your control notification handler code here
	wchar_t filter[] = L"TS Files(*.ts)|*.ts||"; //L"All Files(*.*)|*.*||";
	CFileDialog dlg(TRUE, L"mkv", NULL, OFN_HIDEREADONLY, filter);

	if (dlg.DoModal() == IDOK)
	{
		_filename = dlg.GetPathName();
		
		if (_filename.GetLength() > 0)
		{
			unsigned int thrdaddr = 0;
			_ts_demuxing_run = FALSE;
			_ts_demuxing_thread = (HANDLE)_beginthreadex(NULL, 0, Cdk_mpeg2ts_demuxer_testDlg::DemuxingProc, this, 0, &thrdaddr);
		}

		/*

		_ts_file = CreateFile(_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (_ts_file == NULL || _ts_file == INVALID_HANDLE_VALUE)
			return;

		*/
	}
}


void Cdk_mpeg2ts_demuxer_testDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	//_ts_demuxer.release();
}


unsigned Cdk_mpeg2ts_demuxer_testDlg::DemuxingProc(void * param)
{
	Cdk_mpeg2ts_demuxer_testDlg * self = static_cast<Cdk_mpeg2ts_demuxer_testDlg*>(param);
	self->demuxing();
	return 0;
}

void Cdk_mpeg2ts_demuxer_testDlg::demuxing(void)
{
	HANDLE ts_file = CreateFile(_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (ts_file == NULL || ts_file == INVALID_HANDLE_VALUE)
		return;

	_ts_demuxing_run = TRUE;

	DWORD ts_file_buffer_size = 1500;//1024 * 1024 * 2;
	uint8_t * ts_file_buffer = static_cast<uint8_t*>(malloc(ts_file_buffer_size));

	dk_mpeg2ts_demuxer ts_demuxer;
	ts_demuxer.initialize();
	while (_ts_demuxing_run)
	{
		/*
		BOOL WINAPI ReadFile(
		_In_        HANDLE       hFile,
		_Out_       LPVOID       lpBuffer,
		_In_        DWORD        nNumberOfBytesToRead,
		_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
		*/
		DWORD nb = 0;
		BOOL ret = ReadFile(ts_file, ts_file_buffer, ts_file_buffer_size, &nb, NULL);
		if (ret)
		{
			ts_demuxer.demultiplexing(ts_file_buffer, nb);
		}
	}

	if (ts_file_buffer)
	{
		free(ts_file_buffer);
		ts_file_buffer = nullptr;
	}
	ts_demuxer.release();
	CloseHandle(ts_file);
}