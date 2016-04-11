#include "platform.h"
#include "ptz_module_mapper.h"
#include <windows.h>
#include "ptz_controller.h"

typedef base_ptz_controller* (*pfnCreatePTZController)();
typedef void (*pfnDestroyPTZController)( base_ptz_controller **conroller );

ptz_module_mapper & ptz_module_mapper::instance( void )
{
	static ptz_module_mapper _instance;
	return _instance;
}

ptz_module_mapper::ptz_module_mapper( void )
	: _is_mapped(false)
{
	pthread_mutex_init( &_mutex, 0 );
}

ptz_module_mapper::~ptz_module_mapper( void )
{
	pthread_mutex_destroy( &_mutex );
}

unsigned short ptz_module_mapper::get_module_vendor_information( int ***vendor_ids, char ***vendor_names, int *length )
{
	load();
	std::map<int,std::string> vendor_ids_names;
	std::map<unsigned int,vendor>::iterator vendor_iter;
	for( vendor_iter=_ptz_vendors.begin(); vendor_iter!=_ptz_vendors.end(); vendor_iter++ )
	{
		int	   single_vendor_id = (*vendor_iter).first;
		vendor single_vendor	= (*vendor_iter).second;
		vendor_ids_names.insert( std::make_pair(single_vendor_id, single_vendor.name) );
	}
	(*length) = vendor_ids_names.size();
	if( (*length)>0 )
	{
		(*vendor_ids)	= static_cast<int**>( calloc(sizeof(int*), (*length)) );
		(*vendor_names)	= static_cast<char**>( calloc(sizeof(char*), (*length)) );
		std::map<int,std::string>::iterator iter;
		int index=0;
		for( iter=vendor_ids_names.begin(); iter!=vendor_ids_names.end(); iter++, index++ )
		{
			(*vendor_ids)[index]	= static_cast<int*>( malloc(sizeof(int)) );
			(*(*vendor_ids)[index])	= (*iter).first;
			(*vendor_names)[index]	= strdup( ((*iter).second).c_str() );
		}
	}
	else
	{
		(*vendor_ids)	= 0;
		(*vendor_names)	= 0;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short ptz_module_mapper::get_module_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length )
{
	load();
	std::map<int,std::string> vendor_device_ids_names;
	std::map<unsigned int,vendor>::iterator vendor_iter;
	vendor_iter = _ptz_vendors.find( vendor_id );
	if( vendor_iter!=_ptz_vendors.end() )
	{
		vendor single_vendor = (*vendor_iter).second;
		std::map<unsigned int,device>::iterator device_iter;
		for( device_iter=single_vendor.device_list.begin(); device_iter!=single_vendor.device_list.end(); device_iter++ )
		{
			int	   single_device_id = (*device_iter).first;
			device single_device	= (*device_iter).second;
			vendor_device_ids_names.insert( std::make_pair(single_device_id, single_device.name) );
		}
	}
	(*length) = vendor_device_ids_names.size();
	if( (*length)>0 )
	{
		(*vendor_device_ids)	= static_cast<int**>( calloc(sizeof(int*), (*length)) );
		(*vendor_device_names)	= static_cast<char**>( calloc(sizeof(char*), (*length)) );
		std::map<int,std::string>::iterator iter;
		int index=0;
		for( iter=vendor_device_ids_names.begin(); iter!=vendor_device_ids_names.end(); iter++, index++ )
		{
			(*vendor_device_ids)[index]		= static_cast<int*>( malloc(sizeof(int)) );
			(*(*vendor_device_ids)[index])	= (*iter).first;
			(*vendor_device_names)[index]	= strdup( ((*iter).second).c_str() );
		}
	}
	else
	{
		(*vendor_device_ids)	= 0;
		(*vendor_device_names)	= 0;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short ptz_module_mapper::get_module_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length )
{
	load();
	std::map<int,std::string> vendor_device_protocol_ids_names;
	std::map<unsigned int,vendor>::iterator vendor_iter;
	vendor_iter = _ptz_vendors.find( vendor_id );
	if( vendor_iter!=_ptz_vendors.end() )
	{
		vendor single_vendor = (*vendor_iter).second;
		std::map<unsigned int,device>::iterator device_iter;
		device_iter = single_vendor.device_list.find( vendor_device_id );
		if( device_iter!=single_vendor.device_list.end() )
		{
			device single_device = (*device_iter).second;
			std::map<unsigned int,protocol>::iterator protocol_iter;
			for( protocol_iter=single_device.protocol_list.begin(); protocol_iter!=single_device.protocol_list.end(); protocol_iter++ )
			{
				int			single_protocol_id	= (*protocol_iter).first;
				protocol	single_protocol		= (*protocol_iter).second;
				vendor_device_protocol_ids_names.insert( std::make_pair(single_protocol_id, single_protocol.name) );
			}
		}
	}
	(*length) = vendor_device_protocol_ids_names.size();
	if( (*length)>0 )
	{
		(*vendor_device_protocol_ids)	= static_cast<int**>( calloc(sizeof(int*), (*length)) );
		(*vendor_device_protocol_names)	= static_cast<char**>( calloc(sizeof(char*), (*length)) );
		std::map<int,std::string>::iterator iter;
		int index=0;
		for( iter=vendor_device_protocol_ids_names.begin(); iter!=vendor_device_protocol_ids_names.end(); iter++, index++ )
		{
			(*vendor_device_protocol_ids)[index]	= static_cast<int*>( malloc(sizeof(int)) );
			(*(*vendor_device_protocol_ids)[index])	= (*iter).first;
			(*vendor_device_protocol_names)[index]	= strdup( ((*iter).second).c_str() );
		}
	}
	else
	{
		(*vendor_device_protocol_ids)	= 0;
		(*vendor_device_protocol_names) = 0;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short ptz_module_mapper::get_module_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length )
{
	std::map<int,std::string> vendor_device_version_ids_names;
	std::map<unsigned int,vendor>::iterator vendor_iter;
	vendor_iter = _ptz_vendors.find( vendor_id );
	if( vendor_iter!=_ptz_vendors.end() )
	{
		vendor single_vendor = (*vendor_iter).second;
		std::map<unsigned int,device>::iterator device_iter;
		device_iter = single_vendor.device_list.find( vendor_device_id );
		if( device_iter!=single_vendor.device_list.end() )
		{
			device single_device = (*device_iter).second;
			std::map<unsigned int,protocol>::iterator protocol_iter;
			protocol_iter = single_device.protocol_list.find( vendor_device_protocol_id );
			if( protocol_iter!=single_device.protocol_list.end() )
			{
				protocol single_protocol = (*protocol_iter).second;
				std::map<unsigned int,firmware_version>::iterator version_iter;
				for( version_iter=single_protocol.firmware_version_list.begin(); version_iter!=single_protocol.firmware_version_list.end(); version_iter++ )
				{
					int					single_version_id	= (*version_iter).first;
					firmware_version	single_version		= (*version_iter).second;
					vendor_device_version_ids_names.insert( std::make_pair(single_version_id, single_version.name) );
				}
			}
		}
	}

	(*length) = vendor_device_version_ids_names.size();
	if( (*length)>0 )
	{
		(*vendor_device_version_ids)	= static_cast<int**>( calloc(sizeof(int*), (*length)) );
		(*vendor_device_version_names)	= static_cast<char**>( calloc(sizeof(char*), (*length)) );
		std::map<int,std::string>::iterator iter;
		int index=0;
		for( iter=vendor_device_version_ids_names.begin(); iter!=vendor_device_version_ids_names.end(); iter++, index++ )
		{
			(*vendor_device_version_ids)[index]		= static_cast<int*>( malloc(sizeof(int)) );
			(*(*vendor_device_version_ids)[index])	= (*iter).first;
			(*vendor_device_version_names)[index]	= strdup( ((*iter).second).c_str() );
		}
	}
	else
	{
		(*vendor_device_version_ids)	= 0;
		(*vendor_device_version_names)	= 0;
	}
	return VMS_PTZ_SUCCESS;
}

void ptz_module_mapper::get_module_name( unsigned int vendor_id, unsigned int vendor_device_id, unsigned int protocol_id, unsigned int firmware_version_id, char *module_name )
{
	load();
	std::map<unsigned int,vendor>::iterator vendors_iter;
	vendors_iter = _ptz_vendors.find( vendor_id );
	if( vendors_iter!=_ptz_vendors.end() )
	{
		vendor single_vendor = (*vendors_iter).second;
		std::map<unsigned int,device>::iterator devices_iter;
		devices_iter = single_vendor.device_list.find( vendor_device_id );
		if( devices_iter!=single_vendor.device_list.end() )
		{
			device single_device = (*devices_iter).second;
			std::map<unsigned int,protocol>::iterator protocols_iter;
			protocols_iter = single_device.protocol_list.find( protocol_id );
			if( protocols_iter!=single_device.protocol_list.end() )
			{
				protocol single_protocol = (*protocols_iter).second;
				std::map<unsigned int,firmware_version>::iterator versions_iter;
				versions_iter = single_protocol.firmware_version_list.find( firmware_version_id );
				if( versions_iter!=single_protocol.firmware_version_list.end() )
					strncpy( module_name, (*versions_iter).second.module.c_str() ,MAX_PATH );
			}
		}
	}
}

void ptz_module_mapper::load( void )
{
	pthread_mutex_lock( &_mutex );
	if( !_is_mapped )
	{
		HINSTANCE self;
#if defined(_DEBUG)
		self = ::GetModuleHandleA( "ptz_controllerd.dll" );
#else
		self = ::GetModuleHandleA( "ptz_controller.dll" );
#endif
		CHAR szModuleName[MAX_PATH] = {0};
		CHAR szModuleFindPath[FILENAME_MAX] = {0};
		CHAR szModulePath[FILENAME_MAX] = {0};
		CHAR *pszModuleName = szModulePath;
		pszModuleName += GetModuleFileNameA( self, pszModuleName, (sizeof(szModulePath)/sizeof(*szModulePath))-(pszModuleName-szModulePath) );
		if( pszModuleName!=szModulePath )
		{ 
			CHAR *slash = strrchr( szModulePath, '\\' );
			if( slash!=NULL )
			{
				pszModuleName = slash+1;
				strset( pszModuleName, 0 );
			}
			else
			{
				strset( szModulePath, 0 );
			}
		}

		{
			snprintf( szModulePath, sizeof(szModulePath), "%s%s", szModulePath, "ptz_implements" );
			snprintf( szModuleFindPath, sizeof(szModuleFindPath), "%s%s", szModulePath, "\\*.dll" );
		
			BOOL				result = TRUE;
			HANDLE				search;
			WIN32_FIND_DATAA	wfd;

			search = FindFirstFileA( szModuleFindPath, &wfd );
			if( search==INVALID_HANDLE_VALUE )
				return;

			HINSTANCE instance;
			base_ptz_controller	*controller;
			while( result )
			{
				if( (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY )
				{
					strcpy( szModuleName, wfd.cFileName );
					if( strlen(szModulePath)>0 )
					{
						SetDllDirectoryA( szModulePath );
						instance = LoadLibraryA( szModuleName );
						if( instance )
						{
							pfnCreatePTZController pfnCreate = (pfnCreatePTZController)::GetProcAddress( instance, "create" );
							if( pfnCreate )
							{
								controller = (pfnCreate)();
								if( controller )
								{
									std::map<unsigned int,vendor>::iterator vendors_iter;
									vendors_iter = _ptz_vendors.find( controller->get_vendor_id() );
									if( vendors_iter!=_ptz_vendors.end() )
									{
										vendor single_vendor = (*vendors_iter).second;
										std::map<unsigned int,device>::iterator devices_iter;
										devices_iter = single_vendor.device_list.find( controller->get_vendor_device_id() );
										if( devices_iter!=single_vendor.device_list.end() )
										{
											device single_device = (*devices_iter).second;
											std::map<unsigned int,protocol>::iterator protocols_iter;
											protocols_iter = single_device.protocol_list.find( controller->get_vendor_device_protocol_id() );
											if( protocols_iter!=single_device.protocol_list.end() )
											{
												firmware_version single_firmware_version;
												single_firmware_version.module	= szModuleName;
												single_firmware_version.name	= controller->get_vendor_device_version_name();

												protocol single_protocol = (*protocols_iter).second;
												single_protocol.firmware_version_list.insert( std::make_pair(controller->get_vendor_device_version_id(), single_firmware_version) );

												single_device.protocol_list.erase( controller->get_vendor_device_protocol_id() );
												single_device.protocol_list.insert( std::make_pair(controller->get_vendor_device_protocol_id(), single_protocol) );
												single_vendor.device_list.erase( controller->get_vendor_device_id() );
												single_vendor.device_list.insert( std::make_pair(controller->get_vendor_device_id(), single_device) );
												_ptz_vendors.erase( controller->get_vendor_id() );
												_ptz_vendors.insert( std::make_pair(controller->get_vendor_id(), single_vendor) );
											}
											else
											{
												firmware_version single_firmware_version;
												single_firmware_version.module	= szModuleName;
												single_firmware_version.name	= controller->get_vendor_device_version_name();

												protocol single_protocol;
												single_protocol.name			= controller->get_vendor_device_protocol_name();
												single_protocol.firmware_version_list.insert( std::make_pair(controller->get_vendor_device_version_id(), single_firmware_version) );

												single_device.protocol_list.insert( std::make_pair(controller->get_vendor_device_protocol_id(), single_protocol) );
												
												single_vendor.device_list.erase( controller->get_vendor_device_id() );
												single_vendor.device_list.insert( std::make_pair(controller->get_vendor_device_id(), single_device) );
												_ptz_vendors.erase( controller->get_vendor_id() );
												_ptz_vendors.insert( std::make_pair(controller->get_vendor_id(), single_vendor) );
											}
										}
										else
										{
											firmware_version single_firmware_version;
											single_firmware_version.module	= szModuleName;
											single_firmware_version.name	= controller->get_vendor_device_version_name();

											protocol single_protocol;
											single_protocol.name			= controller->get_vendor_device_protocol_name();
											single_protocol.firmware_version_list.insert( std::make_pair(controller->get_vendor_device_version_id(), single_firmware_version) );

											device single_device;
											single_device.name				= controller->get_vendor_device_name();
											single_device.protocol_list.insert( std::make_pair(controller->get_vendor_device_protocol_id(), single_protocol) );

											single_vendor.device_list.insert( std::make_pair(controller->get_vendor_device_id(), single_device) );

											_ptz_vendors.erase( controller->get_vendor_id() );
											_ptz_vendors.insert( std::make_pair(controller->get_vendor_id(), single_vendor) );
										}
									}
									else
									{
										firmware_version single_firmware_version;
										single_firmware_version.module	= szModuleName;
										single_firmware_version.name	= controller->get_vendor_device_version_name();

										protocol single_protocol;
										single_protocol.name			= controller->get_vendor_device_protocol_name();
										single_protocol.firmware_version_list.insert( std::make_pair(controller->get_vendor_device_version_id(), single_firmware_version) );

										device single_device;
										single_device.name				= controller->get_vendor_device_name();
										single_device.protocol_list.insert( std::make_pair(controller->get_vendor_device_protocol_id(), single_protocol) );

										vendor single_vendor;
										single_vendor.name				= controller->get_vendor_name();
										single_vendor.device_list.insert( std::make_pair(controller->get_vendor_device_id(), single_device) );

										_ptz_vendors.insert( std::make_pair(controller->get_vendor_id(), single_vendor) );
									}

									pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController)::GetProcAddress( instance, "destroy" );
									if( controller )
									{
										(pfnDestroy)( &controller );
										controller = 0;
									}
									result = FindNextFileA( search, &wfd );
								}
								else
								{
									FreeLibrary( instance );
									result = FindNextFileA( search, &wfd );
								}
							}
							else
							{
								FreeLibrary( instance );
								result = FindNextFileA( search, &wfd );
							}
						}
						else
							result = FindNextFileA( search, &wfd );
					}
					else
						result = FindNextFileA( search, &wfd );

				}
				else
					result = FindNextFileA( search, &wfd );
			}
		}
		_is_mapped = true;
	}
	pthread_mutex_unlock( &_mutex );
}