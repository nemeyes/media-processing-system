
// record_serverDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "dk_rtsp_receiver.h"

// Crecord_serverDlg dialog
class dk_record_server_controller_dlg : public CDialogEx
{
// Construction
public:
	dk_record_server_controller_dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RECORD_SERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


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
	afx_msg void OnBnClickedButtonStartRecord();
	afx_msg void OnBnClickedButtonStopRecord();
private:
	CEdit _url;
	CEdit _username;
	CEdit _password;

	dk_rtsp_receiver _rtsp_receiver;
	CComboBox _cmb_streaming_protocol;
	CComboBox _cmb_transport_type;
	CComboBox _cmb_recv_option;
public:
	afx_msg void OnBnClickedButtonStartPreview();
	afx_msg void OnBnClickedButtonStopPreview();
private:
	CComboBox _cmb_retry_connection;
};
