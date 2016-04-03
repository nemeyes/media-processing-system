
// ParallelRecordServerDlg.h : ��� ����
//

#pragma once

// CParallelRecordServerDlg ��ȭ ����
#define TRAY_NOTIFY        (WM_APP + 100)
class CParallelRecordServerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CParallelRecordServerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_PARALLELRECORDSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

private:
	void Position2Center(void);
	void StartRecording(void);
	void StopRecording(void);

	LRESULT OnTrayIconClick(WPARAM wParam, LPARAM lParam);
	void EnableTray(BOOL enable);


	BOOL _is_recording;
// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStartRecord();
	afx_msg void OnBnClickedButtonStopRecord();
	afx_msg void OnBnClickedButtonToTray();
	afx_msg void OnTrayStartRecording();
	afx_msg void OnTrayStopRecording();
	afx_msg void OnTrayExit();
	virtual BOOL DestroyWindow();
	afx_msg void OnUpdateTrayStartRecording(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTrayStopRecording(CCmdUI *pCmdUI);
	afx_msg void OnTrayShutdown();
};
