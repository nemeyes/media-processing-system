
// dk_player_framework_test.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cdk_player_framework_testApp:
// See dk_player_framework_test.cpp for the implementation of this class
//

class Cdk_player_framework_testApp : public CWinApp
{
public:
	Cdk_player_framework_testApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cdk_player_framework_testApp theApp;