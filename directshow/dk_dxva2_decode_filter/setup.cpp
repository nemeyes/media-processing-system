#include <stdio.h>
#include <tchar.h>
#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312) 
#include <streams.h>
#pragma warning(pop)     
#include "dk_dxva2_decode_filter_properties.h"
#include "dk_dxva2_decode_filter.h"

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{ 
	&MEDIATYPE_NULL,	// Major CLSID
	&MEDIASUBTYPE_NULL	// Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
	{
		L"Output",		// Pin's string name
		FALSE,			// Is it rendered
		TRUE,			// Is it an output
		FALSE,			// Allowed none
		FALSE,			// Allowed many
		&CLSID_NULL,	// Connects to filter
		L"Input",		// Connects to pin
		1,				// Number of types
		&sudPinTypes	// Pin type information
	},
};

const AMOVIESETUP_FILTER sudFilter =
{
	&CLSID_DK_DXVA2_DECODE_FILTER,		// CLSID of filter
	g_szFilterName,						// Filter's name
	MERIT_DO_NOT_USE,					// Filter merit
	1,									// Number of pins
	psudPins							// Pin information
};

CFactoryTemplate g_Templates[] =
{
	{
		g_szFilterName,
		&CLSID_DK_DXVA2_DECODE_FILTER,
		dk_dxva2_decode_filter::CreateInstance,
		NULL,
		&sudFilter
	},
	{
		L"dk_msdk_decode_filter_properties",
		&CLSID_DK_DXVA2_DECODE_FILTER_PROPERTIES,
		dk_dxva2_decode_filter::CreateInstance,
		NULL,
		NULL
	},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer(VOID)
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer(VOID)
{
	return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}