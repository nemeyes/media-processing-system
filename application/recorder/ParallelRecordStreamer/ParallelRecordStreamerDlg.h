
// ParallelRecordStreamerDlg.h : header file
//

#pragma once

#include <dk_vod_rtsp_server.h>

// CParallelRecordStreamerDlg dialog
#define TRAY_NOTIFY        (WM_APP + 100)
class CParallelRecordStreamerDlg : public CDialogEx
{
// Construction
public:
	CParallelRecordStreamerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PARALLELRECORDSTREAMER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	LRESULT OnTrayIconClick(WPARAM wParam, LPARAM lParam);
private:
	dk_vod_rtsp_server _server;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonTray();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
};
