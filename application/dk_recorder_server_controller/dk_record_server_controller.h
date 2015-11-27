
// record_server.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Crecord_serverApp:
// See record_server.cpp for the implementation of this class
//

class dk_record_server_controller : public CWinApp
{
public:
	dk_record_server_controller();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern dk_record_server_controller theApp;