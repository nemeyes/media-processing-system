
// CloudMediaEdgeFileEncoderDlg.h : header file
//

#pragma once

#include "media_edge_file_encoder.h"

// CCloudMediaEdgeFileEncoderDlg dialog
class CCloudMediaEdgeFileEncoderDlg : public CDialogEx
{
// Construction
public:
	CCloudMediaEdgeFileEncoderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CLOUDMEDIAEDGEFILEENCODER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	media_edge_file_encoder _encoder;


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
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
