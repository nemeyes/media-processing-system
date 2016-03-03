
// CloudMediaClientDlg.h : header file
//

#pragma once
#include "afxwin.h"

namespace ic
{
	class media_edge_client;
}
// CCloudMediaClientDlg dialog
class CCloudMediaClientDlg : public CDialogEx
{
// Construction
public:
	CCloudMediaClientDlg(CWnd* pParent = NULL);	// standard constructor
	void EnableConnectButton(BOOL enable);
	void EnableDisconnectButton(BOOL enable);

// Dialog Data
	enum { IDD = IDD_CLOUDMEDIACLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	CEdit _server_address;
	CEdit _server_port_number;

	ic::media_edge_client * _client;


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
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	virtual BOOL DestroyWindow();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
