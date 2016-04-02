
// ParallelRecordStreamerDlg.h : header file
//

#pragma once

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

private:
	void Position2Center(void);
	void StartStreaming(void);
	void StopStreaming(void);

	LRESULT OnTrayIconClick(WPARAM wParam, LPARAM lParam);
	void EnableTray(BOOL enable);

private:
	BOOL _is_streaming;
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
	afx_msg void OnTrayStartStreaming();
	afx_msg void OnTrayStopStreaming();
	afx_msg void OnTrayExit();
	virtual BOOL DestroyWindow();
};
