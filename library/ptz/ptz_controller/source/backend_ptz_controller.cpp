#if defined(WIN32)
#include <tchar.h>
#include <windows.h>
#include <direct.h>
#else

#endif
#include <map>
#include <string>
#include "platform.h"
#include "ptz_device_info.h"
#include "backend_ptz_controller.h"
#include "ptz_module_mapper.h"

typedef base_ptz_controller* (*pfnCreatePTZController)();
typedef void (*pfnDestroyPTZController)( base_ptz_controller **conroller );

backend_ptz_controller::backend_ptz_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version, bool block_mode )
	: _controller(0)
	, _instance(0)
	, _block_mode(block_mode)
	, _command_info(0)
	, _is_run(false)
	, _is_created(false)
	, _is_entered(false)
{
	if( !_block_mode )
		pthread_mutex_init( &_mutex, 0 );
#if defined(WIN32)
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
	snprintf( szModulePath, sizeof(szModulePath), "%s%s", szModulePath, "ptz_implements" );
	ptz_module_mapper::instance().get_module_name( vendor, vendor_device, protocol, firmware_version, szModuleName );
	if( strlen(szModulePath)>0 && strlen(szModuleName)>0 )
	{
		SetDllDirectoryA( szModulePath );
		_instance = LoadLibraryA( szModuleName );
		if( _instance )
		{
			pfnCreatePTZController pfnCreate = (pfnCreatePTZController)::GetProcAddress( _instance, "create" );
			if( pfnCreate )
			{
				_controller = (pfnCreate)();
				if( _controller )
				{
					if( !_block_mode )
					{
						_is_created = true;
						_is_run		= true;
						pthread_create( &_tid, NULL, (void*(*)(void*))backend_ptz_controller::run, (void*)this ); 
					}
				}
				else
				{
					FreeLibrary( _instance );
				}
			}
			else
			{
				FreeLibrary( _instance );
			}
		}
	}
#elif defined(UBUNTU) || defined(ARM)
	std::string dir = std::string("../../lib/Debug/ptz_implements");
	std::vector<std::string> files = std::vector<std::string>();

	getdir(dir, files);

	for( unsigned int i=0; i < files.size(); i++ ) {
		std::cout << files[i] << std::endl;

		_instance = dlopen( const_cast<const char*> (files[i].c_str()), RTLD_LAZY );
		if( !_instance ) {
			std::cerr << "open error: " << dlerror() << "\n";
			return;
		}
		dlerror();

		pfnCreatePTZController pfnCreate = (pfnCreatePTZController) dlsym( _instance, "create" );
		const char* err = dlerror();
		if( err ) {
			std::cerr << "create error: " << err << std::endl;
			return;
		}

		_controller = (pfnCreate)();
		if( _controller )
		{
			if( vendor == _controller->get_vendor_device_id()
					&& vendor_device == _controller->get_vendor_device_id()
					&& protocol == _controller->get_vendor_device_id()
					&& firmware_version == _controller->get_vendor_device_version_id() )
			{
				_is_created = true;
				_is_run = true;
				pthread_create( &_tid, NULL, (void* (*)(void*))backend_ptz_controller::run, (void*)this );
			}
			else
			{
				pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController) dlsym( _instance, "destroy" );
				const char* err = dlerror();
				if( err ) {
					std::cerr << "destroy error: " << err << std::endl;
					return;
				}
				(pfnDestroy)( &_controller );
			}
		}
		dlclose( _instance );
	}
#endif
}

#if defined(UBUNTU) || defined(ARM)
int backend_ptz_controller::getdir( std::string dir, std::vector<std::string> &files ) {
	std::string soExt = ".so";

	DIR* dp;
	struct dirent *dirp;
	if( (dp=opendir(dir.c_str())) == NULL ) {
		std::cout << "Error(" << strerror(errno) << "} opening " << dir << std::endl;
		return errno;
	}

	while(( dirp = readdir(dp)) != NULL) {
		std::string tmp(dirp->d_name);
		unsigned found = tmp.find( soExt );
		if( found != std::string::npos )
			files.push_back( tmp );
	}

	closedir(dp);
	return 0;
}
#endif

backend_ptz_controller::~backend_ptz_controller( void )
{
	if( !_block_mode )
	{
		if( _is_created )
		{
#if defined(WIN32)
			do{	Sleep(10); } while( !_is_entered );
#elif defined(UBUNTU) || defined(ARM)
			do{ sleep(10); } while( !_is_entered );
#endif
			_is_run = false;
			pthread_join( _tid, NULL );
			_is_created = false;
		}
		pthread_mutex_destroy( &_mutex );
	}
	if( _instance )
	{
#if defined(WIN32)
		pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController)::GetProcAddress( _instance, "destroy" );
		if( _controller )
		{
			(pfnDestroy)( &_controller );
			_controller = 0;
		}
		FreeLibrary( _instance );
		_instance = 0;
#elif defined(UBUNTU) || defined(ARM)
		pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController) dlsym( _instance, "destroy");
		if( _controller )
		{
			(pfnDestroy)( &_controller );
			_controller = 0;
		}
		dlclose( _instance );
		_instance = 0;
#endif

	}
}

unsigned short backend_ptz_controller::get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length )
{
	return ptz_module_mapper::instance().get_module_vendor_information( vendor_ids, vendor_names, length );
/*
#if defined(WIN32)
#elif defined(UBUNTU) || defined(ARM)
	std::map<int,std::string> vendor_ids_names;
	std::string dir = std::string("../../lib/Debug/ptz_implements/");
	std::vector<std::string> files = std::vector<std::string>();

	getdir(dir, files);

	void* instance = 0;
	base_ptz_controller* controller = 0;

	for( unsigned int i=0; i < files.size(); i++ ) {
		std::cout << files[i] << std::endl;

		instance = dlopen( const_cast<const char*>((std::string("../../lib/Debug/ptz_imiplements/")+files[i]).c_str()), RTLD_LAZY );
		if( !instance ) {
			std::cerr << "open error: " << dlerror() << "\n";
			return VMS_PTZ_FAIL;
		}
		dlerror();

		pfnCreatePTZController pfnCreate = (pfnCreatePTZController) dlsym( instance, "create" );
		const char* err = dlerror();
		if( err ) {
			std::cerr << "create error: " << err << std::endl;
			return VMS_PTZ_FAIL;
		}

		controller = (pfnCreate)();
		if( controller )
		{
			std::map<int,std::string>::iterator iter = vendor_ids_names.find( controller->get_vendor_id() );
			if( iter==vendor_ids_names.end() )
				vendor_ids_names.insert( std::make_pair(controller->get_vendor_id(), controller->get_vendor_name() ) );

			pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController) dlsym( instance, "destroy");
			const char* err = dlerror();
			if( err ) {
				std::cerr << "destroy error: " << err << std::endl;
				return VMS_PTZ_FAIL;
			}
			(pfnDestroy)( &controller );
		}
		dlclose( instance );
	}

	(*length) = vendor_ids_names.size();
	if( (*length) > 0 )
	{
		(*vendor_ids)		= static_cast<int**>( calloc(sizeof(int*), (*length)));
		(*vendor_names)		= static_cast<char**>( calloc( sizeof(char*), (*length)));
		std::map<int, std::string>::iterator iter;
		int index=0;
		for( iter=vendor_ids_names.begin(); iter!=vendor_ids_names.end(); iter++, index++)
		{
			(*vendor_ids)[index]		= static_cast<int*>( malloc(sizeof(int)));
			(*(*vendor_ids))[index]		= (*iter).first;
			(*vendor_names)[index]		= strdup( ((*iter).second).c_str());
		}
	}
	else
	{
		(*vendor_ids)		= 0;
		(*vendor_names)		= 0;
	}

	return VMS_PTZ_SUCCESS;
#endif
*/
}

unsigned short backend_ptz_controller::get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length )
{
	return ptz_module_mapper::instance().get_module_vendor_device_informations( vendor_id, vendor_device_ids, vendor_device_names, length );
/*
#if defined(WIN32)
#elif defined(UBUNTU) || defined(ARM)
	std::map<int,std::string>		vendor_device_ids_names;
	void*							instance = 0;
	base_ptz_controller				*controller;

	std::string dir = std::string("../../lib/Debug/ptz_implements/");
	std::vector<std::string> files = std::vector<std::string>();

	getdir(dir, files);

	for( unsigned int i=0; i < files.size(); i++ ) {
		std::cout << files[i] << std::endl;

		instance = dlopen( const_cast<const char*>((std::string("../../lib/Debug/ptz_implements")+files[i]).c_str()), RTLD_LAZY );
		if( !instance ) {
			std::cerr << "open error: " << dlerror() << "\n";
			return VMS_PTZ_FAIL;
		}
		dlerror();

		pfnCreatePTZController pfnCreate = (pfnCreatePTZController) dlsym( instance, "create" );
		const char* err = dlerror();
		if( err ) {
			std::cerr << "create error: " << err << std::endl;
			return VMS_PTZ_FAIL;
		}

		controller = (pfnCreate) ();
		if( controller )
		{
			if( vendor_id == controller->get_vendor_id() ) {
				std::map<int,std::string>::iterator iter = vendor_device_ids_names.find( controller->get_vendor_device_id() );
				if( iter == vendor_device_ids_names.end())
					vendor_device_ids_names.insert( std::make_pair( controller->get_vendor_device_id(), controller->get_vendor_device_name()));
			}

			pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController) dlsym( instance, "destroy" );
			const char* err = dlerror();
			if( err ) {
				std::cerr << "destroy error: " << err << std::endl;
				return VMS_PTZ_FAIL;
				(pfnDestroy)( &controller );
			}
		}
		dlclose( instance );
	}

	(*length) = vendor_device_ids_names.size();
	if( (*length) > 0 )
	{
		(*vendor_device_ids)			= static_cast<int**>( calloc(sizeof(int*), (*length)));
		(*vendor_device_names)			= static_cast<char**>( calloc(sizeof(char*), (*length)));
		std::map<int,std::string>::iterator		iter;
		int index = 0;
		for( iter=vendor_device_ids_names.begin(); iter != vendor_device_ids_names.end(); iter++, index++)
		{
			(*vendor_device_ids)[index]			= static_cast<int*>( malloc(sizeof(int)) );
			(*(*vendor_device_ids)[index])		= (*iter).first;
			(*vendor_device_names)[index]		= strdup( ((*iter).second).c_str() );
		}
	}
	else
	{
		(*vendor_device_ids)			= 0;
		(*vendor_device_names) 			= 0;
	}
#endif
*/
}

unsigned short backend_ptz_controller::get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length )
{
	return ptz_module_mapper::instance().get_module_vendor_device_protocol_informations( vendor_id, vendor_device_id, vendor_device_protocol_ids, vendor_device_protocol_names, length );
/*
#if defined(WIN32)
#elif defined(UBUNTU) || defined(ARM)
	std::map<int,std::string>			vendor_device_protocol_ids_names;
	void*								instance = 0;
	base_ptz_controller					*controller;

	std::string dir = std::string("../../lib/Debug/ptz_implements/");
	std::vector<std::string> files = std::vector<std::string>();
	getdir( dir, files );

	for( unsigned int i=0; i<files.size(); i++ ) {
		std::cout << files[i] << std::endl;

		instance = dlopen( const_cast<const char*> ((std::string("../../lib/Debug/ptz_implements/")+files[i]).c_str()), RTLD_LAZY );
		if( !instance ) {
			std::cerr << "open error: " << dlerror() << "\n";
			return VMS_PTZ_FAIL;
		}
		dlerror();

		pfnCreatePTZController pfnCreate = (pfnCreatePTZController) dlsym( instance, "create" );
		const char* err = dlerror();
		if( err ) {
			std::cerr << "create error: " << err << std::endl;
			return VMS_PTZ_FAIL;
		}

		controller = (pfnCreate)();
		if(controller)
		{
			if( vendor_id == controller->get_vendor_id()
					&& vendor_device_id == controller->get_vendor_device_id() )
			{
				std::map<int,std::string>::iterator iter = vendor_device_protocol_ids_names.find( controller->get_vendor_device_protocol_id() );
				if( iter==vendor_device_protocol_ids_names.end() )
					vendor_device_protocol_ids_names.insert( std::make_pair( controller->get_vendor_device_protocol_id(), controller->get_vendor_device_protocol_name()));
			}

			pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController) dlsym( instance, "destroy");
			const char* err = dlerror();
			if( err ) {
				std::cerr << "destroy error: " << err << std::endl;
				return VMS_PTZ_FAIL;
				(pfnDestroy)( &controller );
			}
		}
		dlclose( instance );
	}

	(*length) = vendor_device_protocol_ids_names.size();
	if( (*length) > 0 )
	{
		(*vendor_device_protocol_ids)		= static_cast<int**>( calloc(sizeof(int*), (*length)));
		(*vendor_device_protocol_names)	= static_cast<char**>( calloc(sizeof(char*), (*length)));
		std::map<int,std::string>::iterator iter;
		int index = 0;
		for( iter = vendor_device_protocol_ids_names.begin(); iter != vendor_device_protocol_ids_names.end(); iter++, index++ )
		{
			(*vendor_device_protocol_ids)[index]		= static_cast<int*>( malloc(sizeof(int)));
			(*(*vendor_device_protocol_ids)[index])		= (*iter).first;
			(*vendor_device_protocol_names)[index]		= strdup( ((*iter).second).c_str() );
		}
	}
	else
	{
		(*vendor_device_protocol_ids)		= 0;
		(*vendor_device_protocol_names)		= 0;
	}

	return VMS_PTZ_SUCCESS;
#endif
*/
}

unsigned short backend_ptz_controller::get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length )
{
	return ptz_module_mapper::instance().get_module_device_version_informations( vendor_id, vendor_device_id, vendor_device_protocol_id, vendor_device_version_ids, vendor_device_version_names, length );
/*
#if defined(WIN32)
#elif defined(UBUNTU) || defined(ARM)
	std::map<int,std::string>	vendor_device_version_ids_names;
	void*						instance = 0;
	base_ptz_controller*		controller;

	std::string dir = std::string("../../lib/Debug/ptz_implements/");
	std::vector<std::string> files = std::vector<std::string>();
	getdir( dir, files );

	for( unsigned int i=0; i < files.size(); i++ ) {
		std::cout << files[i] << std::endl;

		instance = dlopen( const_cast<const char*>((std::string("../../lib/Debug/ptz_implement/")+files[i]).c_str()), RTLD_LAZY);
		if( !instance ) {
			std::cerr << "open error: " << dlerror() << "\n";
			return VMS_PTZ_FAIL;
		}
		dlerror();

		pfnCreatePTZController pfnCreate = (pfnCreatePTZController) dlsym( instance, "create" );
		const char* err = dlerror();
		if( err ) {
			std::cerr << "create error: " << err << std::endl;
			return VMS_PTZ_FAIL;
		}

		controller = (pfnCreate)();
		if( controller )
		{
			if( vendor_id == controller->get_vendor_id()
					&& vendor_device_id == controller->get_vendor_device_id()
					&& vendor_device_protocol_id == controller->get_vendor_device_protocol_id())
			{
				std::map<int,std::string>::iterator iter = vendor_device_version_ids_names.find( controller->get_vendor_device_version_id() );
				if( iter==vendor_device_version_ids_names.end() )
					vendor_device_version_ids_names.insert( std::make_pair( controller->get_vendor_device_version_id(), controller->get_vendor_device_version_name()));
			}

			pfnDestroyPTZController pfnDestroy = (pfnDestroyPTZController) dlsym( instance, "destroy" );
			const char* err = dlerror();
			if( err ) {
				std::cerr << "destroy error: " << err << std::endl;
				return VMS_PTZ_FAIL;
				(pfnDestroy)( &controller );
			}
		}
		dlclose( instance );
	}

	return VMS_PTZ_SUCCESS;
#endif
*/
}

char* backend_ptz_controller::get_vendor_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_name();
}

char* backend_ptz_controller::get_vendor_device_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_name();
}

char* backend_ptz_controller::get_vendor_device_protocol_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_protocol_name();
}

char* backend_ptz_controller::get_vendor_device_version_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_version_name();
}

unsigned short backend_ptz_controller::get_vendor_id( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_vendor_id();
}

unsigned short backend_ptz_controller::get_vendor_device_id( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_id();
}

unsigned short backend_ptz_controller::get_vendor_device_protocol_id( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_protocol_id();
}

unsigned short backend_ptz_controller::get_vendor_device_version_id( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_version_id();
}

unsigned short backend_ptz_controller::set_host_name( char *host_name )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_host_name( host_name );
}

unsigned short backend_ptz_controller::set_port_number( unsigned short port_number )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_port_number( port_number );
}

unsigned short backend_ptz_controller::set_user_id( char *user_id )
{
	if( !_controller ) return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_user_id( user_id );
}

unsigned short backend_ptz_controller::set_user_password( char *password )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_user_password( password );
}

unsigned short backend_ptz_controller::set_angle_inverse( bool inverse )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_angle_inverse( inverse );
}

unsigned short backend_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_pan_sensitive_boundary( min, max, number_place );
}

unsigned short backend_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_tilt_sensitive_boundary( min, max, number_place );
}

unsigned short backend_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_zoom_sensitive_boundary( min, max, number_place );
}

unsigned short backend_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_speed_sensitive_boundary( min, max, number_place );
}

unsigned short backend_ptz_controller::set_profile_token( char *token )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_profile_token( token );
}

unsigned short backend_ptz_controller::is_enable_osd_menu( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_osd_menu();
}

unsigned short backend_ptz_controller::is_enable_home_position( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_home_position();
}

unsigned short backend_ptz_controller::is_enable_preset( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_preset();
}

unsigned short backend_ptz_controller::is_enable_preset_tour( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_preset_tour();
}

unsigned short backend_ptz_controller::is_enable_continuous_move( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_continuous_move();
}

unsigned short backend_ptz_controller::is_enable_relative_move( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_relative_move();
}

unsigned short backend_ptz_controller::is_enable_absolute_move( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_absolute_move();
}

unsigned short backend_ptz_controller::is_preset_name_numberic( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_name_numberic();
}

unsigned short backend_ptz_controller::is_preset_tour_name_numberic( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_tour_name_numberic();
}

unsigned short backend_ptz_controller::is_preset_name_changable( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_name_changable();
}

unsigned short backend_ptz_controller::is_preset_tour_name_changable( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_tour_name_changable();
}

unsigned short backend_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->osd_menu( osd );
}

unsigned short backend_ptz_controller::goto_home_position( float speed )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_GOTO_HOME_POSITION;
		_command_info->speed			= speed;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->goto_home_position( speed );
}

unsigned short backend_ptz_controller::set_home_position( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_home_position();
}

unsigned short backend_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_list( aliases, length );
}

unsigned short backend_ptz_controller::add_preset( char *alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset( alias );
}

unsigned short backend_ptz_controller::remove_preset( char *alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset( alias );
}

unsigned short backend_ptz_controller::goto_preset( char *alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_GOTO_PRESET;
		strcpy( _command_info->preset_token, alias );
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->goto_preset( alias );
}

unsigned short backend_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_list2( aliases, length );
}

unsigned short backend_ptz_controller::add_preset2( int &alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset2( alias );
}

unsigned short backend_ptz_controller::remove_preset2( int alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset2( alias );
}

unsigned short backend_ptz_controller::goto_preset2( int alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_GOTO_PRESET;
		_command_info->preset_token_numberic = alias;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->goto_preset2( alias );
}

unsigned short backend_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset_tour( tour_name, size );
}

unsigned short backend_ptz_controller::remove_preset_tour( char *tour_name )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset_tour( tour_name );
}

unsigned short backend_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_OPERATE_PRESET_TOUR;
		_command_info->tour_command		= cmd;
		strcpy( _command_info->preset_tour_token, tour_name );
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->operate_preset_tour( tour_name, cmd );
}

unsigned short backend_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset_tour2( tour_name, size );
}

unsigned short backend_ptz_controller::remove_preset_tour2( int tour_name )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset_tour2( tour_name );
}

unsigned short backend_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_OPERATE_PRESET_TOUR;
		_command_info->tour_command		= cmd;
		_command_info->preset_tour_token_numberic = tour_name;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->operate_preset_tour2( tour_name, cmd );
}

unsigned short backend_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_preset_tour( tour );
}

unsigned short backend_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_tour( tour );
}

unsigned short backend_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_tours( tour, size_of_tours );
}

unsigned short backend_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_CONTINUOUS_MOVE;
		_command_info->pan_sensitive	= pan_sensitive;
		_command_info->tilt_sensitive	= tilt_sensitive;
		_command_info->zoom_sensitive	= zoom_sensitive;
		_command_info->timeout			= timeout;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->continuous_move( pan_sensitive, tilt_sensitive, zoom_sensitive, timeout );
}

unsigned short backend_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_CONTINUOUS_MOVE2;
		_command_info->cmove			= move;
		_command_info->speed			= speed;
		_command_info->timeout			= timeout;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->continuous_move( move, speed, timeout );
}

unsigned short backend_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_RELATIVE_MOVE;
		_command_info->pan_sensitive	= pan_sensitive;
		_command_info->tilt_sensitive	= tilt_sensitive;
		_command_info->zoom_sensitive	= zoom_sensitive;
		_command_info->speed			= speed;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->relative_move( pan_sensitive, tilt_sensitive, zoom_sensitive, speed );
}

unsigned short backend_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info				= new PTZ_COMMAND_INFO_T();
		_command_info->command		= PTZ_COMMAND_RELATIVE_MOVE2;
		_command_info->rmove		= move;
		_command_info->sensitive	= sensitive;
		_command_info->speed		= speed;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->relative_move( move, sensitive, speed );
}

unsigned short backend_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
			return VMS_PTZ_CURRENTLY_WORKING;

		pthread_mutex_lock( &_mutex );
		_command_info					= new PTZ_COMMAND_INFO_T();
		_command_info->command			= PTZ_COMMAND_ABSOLUTE_MOVE;
		_command_info->pan_sensitive	= pan_sensitive;
		_command_info->tilt_sensitive	= tilt_sensitive;
		_command_info->zoom_sensitive	= zoom_sensitive;
		_command_info->speed			= speed;
		pthread_mutex_unlock( &_mutex );
		return VMS_PTZ_SUCCESS;
	}
	else
		return _controller->absolute_move( pan_sensitive, tilt_sensitive, zoom_sensitive, speed );
}

unsigned short	backend_ptz_controller::stop_move( void )
{
	if( !_controller )
		return VMS_PTZ_UNDEFINED_DEVICE;

	if( !_block_mode )
	{
		if( _command_info )
		{
			for( int index=0; _command_info && index<100; index++ )
				sleep_millisecond( 10 );
			return _controller->stop_move(); // stop_move is high priority action, so though other ptz command is working stop command must working above all
		}
		else
		{
			pthread_mutex_lock( &_mutex );
			_command_info					= new PTZ_COMMAND_INFO_T();
			_command_info->command			= PTZ_COMMAND_STOP_MOVE;
			pthread_mutex_unlock( &_mutex );
			return VMS_PTZ_SUCCESS;
		}
	}
	else
		return _controller->stop_move();
}

unsigned short	backend_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	if( !_controller )
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_status( pan, tilt, zoom );
}

void* backend_ptz_controller::run( void *param )
{
	backend_ptz_controller *self = static_cast<backend_ptz_controller*>( param );
	if( !self )
		return 0;
	self->process();
	return 0;
}

void backend_ptz_controller::process( void )
{
	while( _is_run )
	{
		_is_entered = true;
		if( !_command_info )
		{
			sleep_millisecond( 10 );
			continue;
		}

		pthread_mutex_lock( &_mutex );
		switch( _command_info->command )
		{
			case PTZ_COMMAND_CONTINUOUS_MOVE :
			{
				_controller->continuous_move( _command_info->pan_sensitive, _command_info->tilt_sensitive, _command_info->zoom_sensitive, _command_info->timeout );
				break;
			}
			case PTZ_COMMAND_CONTINUOUS_MOVE2 :
			{
				_controller->continuous_move( _command_info->cmove, _command_info->speed, _command_info->timeout );
				break;
			}
			case PTZ_COMMAND_RELATIVE_MOVE : 
			{
				_controller->relative_move( _command_info->pan_sensitive, _command_info->tilt_sensitive, _command_info->zoom_sensitive, _command_info->speed );
				break;
			}
			case PTZ_COMMAND_RELATIVE_MOVE2 :
			{
				_controller->relative_move( _command_info->rmove, _command_info->sensitive, _command_info->speed );
				break;
			}
			case PTZ_COMMAND_ABSOLUTE_MOVE : 
			{
				_controller->absolute_move( _command_info->pan_sensitive, _command_info->tilt_sensitive, _command_info->zoom_sensitive, _command_info->speed );
				break;
			}
			case PTZ_COMMAND_STOP_MOVE : 
			{
				_controller->stop_move();
				break;
			}
			case PTZ_COMMAND_GOTO_HOME_POSITION : 
			{
				_controller->goto_home_position( _command_info->speed );
				break;
			}
			case PTZ_COMMAND_GOTO_PRESET :
			{
				if( _controller->is_preset_name_numberic()==VMS_PTZ_TRUE )
					_controller->goto_preset2( _command_info->preset_token_numberic );
				else
					_controller->goto_preset( _command_info->preset_token );
				break;
			}
			case PTZ_COMMAND_OPERATE_PRESET_TOUR :
			{
				if( _controller->is_preset_tour_name_numberic()==VMS_PTZ_TRUE )
					_controller->operate_preset_tour2( _command_info->preset_tour_token_numberic, _command_info->tour_command );
				else
					_controller->operate_preset_tour( _command_info->preset_tour_token, _command_info->tour_command );
				break;
			}
		};
		delete _command_info;
		_command_info = 0;
		pthread_mutex_unlock( &_mutex );
	}
}
