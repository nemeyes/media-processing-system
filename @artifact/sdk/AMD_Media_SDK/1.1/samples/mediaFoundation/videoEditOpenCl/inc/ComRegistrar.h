/**
 * @file <ComRegistrar.h>
 * @brief COM register/unregister functions
 */

#pragma once

#include <Shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <strmif.h>
#include <initguid.h>

/**
 *   @fn CreateObjectKeyName.
 */
HRESULT CreateObjectKeyName(const GUID& guid, TCHAR *sName, DWORD cchMax);

/**
 *   @fn CreateRegKeyAndValue.
 */
HRESULT CreateRegKeyAndValue(HKEY hKey, PCWSTR pszSubKeyName,
                PCWSTR pszValueName, PCWSTR pszData, PHKEY phkResult);

/**
 *   @fn RegisterObject.
 */
HRESULT RegisterObject(HMODULE hModule, const GUID& guid,
                const TCHAR *pszDescription, const TCHAR *pszThreadingModel);

/**
 *   @fn UnregisterObject.
 */
HRESULT UnregisterObject(const GUID& guid);
