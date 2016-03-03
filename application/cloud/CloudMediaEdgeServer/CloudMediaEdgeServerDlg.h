
// CloudMediaEdgeServerDlg.h : header file
//

#pragma once
#include "afxwin.h"

namespace ic
{
	class media_edge_server;
}
// CCloudMediaEdgeServerDlg dialog
class CCloudMediaEdgeServerDlg : public CDialogEx
{
// Construction
public:
	CCloudMediaEdgeServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CLOUDMEDIAEDGESERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	ic::media_edge_server * _server;

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
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
private:
	CEdit _server_uuid;
	CEdit _server_port_number;
public:
	virtual BOOL DestroyWindow();
};
