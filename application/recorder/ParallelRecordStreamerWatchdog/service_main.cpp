#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <process.h>
#include <tlhelp32.h>
#include "userenv.h"
#include "wtsapi32.h"

//wtsapi32.lib; userenv.lib; ws2_32.lib

#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "userenv.lib")

SERVICE_STATUS			g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE	g_StatusHandle = NULL;
HANDLE					g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI				ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI				ServiceCtrlHandler(DWORD);
DWORD WINAPI			ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME	_T("DebuggerKing Parallel Record Streamer Service")

bool IsProcessRunning(const WCHAR *processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_wcsicmp(entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}

BOOL CreateProcessByName(CHAR *processName)
{
	// Get Current VmxnetWatchdog Location
	HINSTANCE self;
	self = ::GetModuleHandleA("ParallelRecordStreamerWatchdog.exe");
	CHAR szModuleName[MAX_PATH] = { 0 };
	CHAR szModuleFindPath[MAX_PATH] = { 0 };
	CHAR szModulePath[MAX_PATH] = { 0 };
	CHAR *pszModuleName = szModulePath;
	CHAR szModuleDirectory[MAX_PATH] = { 0 };
	pszModuleName += GetModuleFileNameA(self, pszModuleName, (sizeof(szModulePath) / sizeof(*szModulePath)) - (pszModuleName - szModulePath));
	if (pszModuleName != szModulePath)
	{
		CHAR *slash = strrchr(szModulePath, '\\');
		if (slash != NULL)
		{
			pszModuleName = slash + 1;
			_strset_s(pszModuleName, strlen(pszModuleName)+1, 0);
		}
		else
		{
			_strset_s(szModulePath, strlen(pszModuleName) + 1, 0);
		}
	}

	strcpy_s(szModuleDirectory, szModulePath);
	_snprintf_s(szModulePath, sizeof(szModulePath), "%s%s", szModulePath, processName);
	OutputDebugStringA(szModulePath);

	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	BOOL bResult = FALSE;
	DWORD dwSessionId, winlogonPid;
	HANDLE hUserToken, hUserTokenDup, hPToken, hProcess;
	DWORD dwCreationFlags;

	// Log the client on to the local computer.
	dwSessionId = WTSGetActiveConsoleSessionId();

	//////////////////////////////////////////
	// Find the winlogon process
	////////////////////////////////////////
	PROCESSENTRY32 procEntry;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnap, &procEntry))
	{
		return FALSE;
	}

	do
	{
		if (_tcsicmp(procEntry.szExeFile, _T("winlogon.exe")) == 0)
		{
			// We found a winlogon process...make sure it's running in the console session
			DWORD winlogonSessId = 0;
			if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId) && winlogonSessId == dwSessionId)
			{
				winlogonPid = procEntry.th32ProcessID;
				break;
			}
		}
	} while (Process32Next(hSnap, &procEntry));

	////////////////////////////////////////////////////////////////////////

	WTSQueryUserToken(dwSessionId, &hUserToken);
	dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = "winsta0\\default";
	ZeroMemory(&pi, sizeof(pi));
	TOKEN_PRIVILEGES tp;
	LUID luid;
	hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogonPid);
	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
		| TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID
		| TOKEN_READ | TOKEN_WRITE, &hPToken))
	{
		int abcd = GetLastError();
		//printf( "Process token open Error: %u\n",GetLastError() ); 
		OutputDebugStringA("Process token open Error");
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		//printf("Lookup Privilege value Error: %u\n",GetLastError());
		OutputDebugStringA("Lookup Privilege value Error");
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hUserTokenDup);
	int dup = GetLastError();

	//Adjust Token privilege
	SetTokenInformation(hUserTokenDup, TokenSessionId, (void*)dwSessionId, sizeof(DWORD));

	if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL))
	{
		int abc = GetLastError();
		//printf("Adjust Privilege value Error: %u\n",GetLastError());
		OutputDebugStringA("Adjust Privilege value Error");
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		printf("Token does not have the provilege\n");
		OutputDebugStringA("Token does not have the provilege");
	}

	LPVOID pEnv = NULL;
	if (CreateEnvironmentBlock(&pEnv, hUserTokenDup, TRUE))
	{
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
	}
	else
		pEnv = NULL;

	// Launch the process in the client's logon session.

	bResult = CreateProcessAsUserA(hUserTokenDup,      // client's access token
		szModulePath,       // file to execute
		NULL,				// command line
		NULL,				// pointer to process SECURITY_ATTRIBUTES
		NULL,				// pointer to thread SECURITY_ATTRIBUTES
		FALSE,				// handles are not inheritable
		dwCreationFlags,	// creation flags
		pEnv,				// pointer to new environment block 
		szModuleDirectory,				// name of current directory 
		&si,				// pointer to STARTUPINFO structure
		&pi					// receives information about new process
		);

	int iResultOfCreateProcessAsUser = GetLastError();

	CloseHandle(hProcess);
	CloseHandle(hUserToken);
	CloseHandle(hUserTokenDup);
	CloseHandle(hPToken);

	return 0;
}

void killProcessByName(const TCHAR *filename)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (_tcscmp(pEntry.szExeFile, filename) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

int _tmain(int argc, TCHAR *argv[])
{
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Returned Error"));
		return GetLastError();
	}
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;

	OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service : ServiceMain: Entry"));

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service : ServiceMain: RegisterServiceCtrlHandler returned error"));
		goto EXIT;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Returned Error"));
	}

	OutputDebugString(_T("Performing Service Start Operations"));

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		OutputDebugString(_T("CreateEvent(g_ServiceStopEvent) returned error"));

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;
		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Returned Warning"));
		}
		goto EXIT;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service SetServiceStatus returned error"));
	}

	// Start the thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Waiting for Worker Thread to complete"));

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);

	OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Worker Thread Stop Event signaled"));


	/*
	* Perform any cleanup tasks
	*/
	OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Performing Cleanup Operations"));
	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service SetServiceStatus returned error"));
	}

EXIT:
	OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Exit"));

	return;
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service : ServiceCtrlHandler: Entry"));
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
	{
		OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service : SERVICE_CONTROL_STOP Request"));

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		* Perform tasks neccesary to stop the service here
		*/


		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service Returned Error"));
		}
		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);
		break;
	}
	default:
	{
		break;
	}
	}
	OutputDebugString(_T("DebuggerKing Parallel Record Streamer Service : ServiceCtrlHandler: Exit"));
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	CreateProcessByName("ParallelRecordStreamer.exe");
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		if (!IsProcessRunning(_T("ParallelRecordStreamer.exe")))
		{
			killProcessByName(_T("ParallelRecordStreamer.exe"));
			CreateProcessByName("ParallelRecordStreamer.exe");
		}
		Sleep(1000);
	}
	killProcessByName(_T("ParallelRecordStreamer.exe"));
	return ERROR_SUCCESS;
}