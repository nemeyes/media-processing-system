
// VmxnetParallelRTSPServerDlg.h : ��� ����
//

#pragma once


// CVmxnetParallelRTSPServerDlg ��ȭ ����
#define TRAY_NOTIFY        (WM_APP + 100)
class CVmxnetParallelRTSPServerDlg : public CDialogEx
{
	// �����Դϴ�.
public:
	CVmxnetParallelRTSPServerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

	// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_VMXNETPARALLELRTSPSERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

	void EnableTray();
	void DisableTray();
	void Start();

	LRESULT OnTrayIconClick(WPARAM wParam, LPARAM lParam);
	// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	DECLARE_MESSAGE_MAP()

private:
	static unsigned __stdcall	ProcessStreaming(void *self);
	void						DoStreaming(void);

private:
	bool						_stop_streaming;
	HANDLE						_thread;
public:
	afx_msg void OnBnClickedButtonToTray();
};
