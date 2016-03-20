#pragma once

#include <string>
#include <atlstr.h>
#pragma comment(lib, "rpcrt4.lib")

class dk_misc_helper
{
public:
	static void retrieve_absolute_module_path(const char * filename, char ** path)
	{
		HINSTANCE module_handle = ::GetModuleHandleA(filename);
		char module_path[260] = { 0 };
		char * module_name = module_path;
		module_name += ::GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
		if (module_name != module_path)
		{
			char * slash = strrchr(module_path, '\\');
			if (slash != NULL)
			{
				module_name = slash + 1;
				_strset_s(module_name, strlen(module_path), 0);
			}
			else
			{
				_strset_s(module_path, strlen(module_path), 0);
			}
		}
		
		size_t length = strlen(module_path) + 1;
		(*path) = static_cast<char*>(malloc(length));
		memset((*path), 0x00, length);
		strncpy((*path), &module_path[0], length);
	}

	static void generate_guid(char ** guid)
	{
		GUID seed;
		ATL::CStringA strGUID;
		CoCreateGuid(&seed);
		RPC_CSTR tmp_guid;
		UuidToStringA(&seed, &tmp_guid);
		strGUID.Format("%s", tmp_guid);
		RpcStringFreeA(&tmp_guid);

		(*guid) = static_cast<char*>(malloc(strlen((LPSTR)(LPCSTR)strGUID) + 1));
		memset((*guid), 0x00, strlen((LPSTR)(LPCSTR)strGUID) + 1);
		strncpy((*guid), (LPSTR)(LPCSTR)strGUID, strlen((LPSTR)(LPCSTR)strGUID) + 1);
	}

	static std::string retrieve_absolute_module_path(const char * filename)
	{
		HINSTANCE module_handle = ::GetModuleHandleA(filename);
		char module_path[260] = { 0 };
		char * module_name = module_path;
		module_name += ::GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
		if (module_name != module_path)
		{
			char * slash = strrchr(module_path, '\\');
			if (slash != NULL)
			{
				module_name = slash + 1;
				_strset_s(module_name, sizeof(module_path), 0);
			}
			else
			{
				_strset_s(module_path, sizeof(module_path), 0);
			}
		}

		return std::string(module_path);
	}

	static std::string generate_guid(void)
	{
		GUID seed;
		ATL::CStringA strGUID;
		CoCreateGuid(&seed);
		RPC_CSTR guid;
		UuidToStringA(&seed, &guid);
		strGUID.Format("%s", guid);
		RpcStringFreeA(&guid);
		return (std::string)(LPSTR)(LPCSTR)strGUID;
	}
};