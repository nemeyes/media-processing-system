
// dk_fft_test.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// Cdk_fft_testApp:
// �� Ŭ������ ������ ���ؼ��� dk_fft_test.cpp�� �����Ͻʽÿ�.
//

class Cdk_fft_testApp : public CWinApp
{
public:
	Cdk_fft_testApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern Cdk_fft_testApp theApp;