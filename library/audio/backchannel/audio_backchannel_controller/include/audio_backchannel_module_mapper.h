#ifndef _AUDIO_BACKCHANNEL_MODULE_LOADER_H_
#define _AUDIO_BACKCHANNEL_MODULE_LOADER_H_

typedef struct _firmware_version
{
	std::string name;
	std::string module;
	_firmware_version( void ) {}
	_firmware_version( const _firmware_version & clone )
	{
		name = clone.name;
		module = clone.module;
	}
	_firmware_version & operator=( const _firmware_version & clone )
	{
		name = clone.name;
		module = clone.module;
		return (*this);
	}
} firmware_version;
typedef std::map<unsigned int,firmware_version> firmware_versions;

typedef struct _protocol
{
	std::string			name;
	firmware_versions	firmware_version_list;
	_protocol( void ) {}
	_protocol( const _protocol & clone )
	{
		name = clone.name;
		firmware_version_list = clone.firmware_version_list;
	}
	_protocol& operator=( const _protocol & clone )
	{
		name = clone.name;
		firmware_version_list = clone.firmware_version_list;
		return (*this);
	}
} protocol;
typedef std::map<unsigned int,protocol> protocols;

typedef struct _device
{
	std::string			name;
	protocols			protocol_list;
	_device( void ) {}
	_device( const _device & clone )
	{
		name = clone.name;
		protocol_list = clone.protocol_list;
	}
	_device & operator=( const _device & clone )
	{
		name = clone.name;
		protocol_list = clone.protocol_list;
		return (*this);
	}
} device;
typedef std::map<unsigned int,device> devices;

typedef struct _vendor
{
	std::string			name;
	devices				device_list;
	_vendor( void ) {}
	_vendor( const _vendor & clone )
	{
		name = clone.name;
		device_list = clone.device_list;
	}
	_vendor & operator=( const _vendor & clone )
	{
		name = clone.name;
		device_list = clone.device_list;
		return (*this);
	}
} vendor;
typedef std::map<unsigned int,vendor> vendors;


class audio_backchannel_module_mapper
{
public:
	static audio_backchannel_module_mapper & instance( void );

	unsigned short	get_module_vendor_information( int ***vendor_ids, char ***vendor_names, int *length );
	unsigned short	get_module_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length );
	unsigned short	get_module_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length );
	unsigned short	get_module_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length );
	void			get_module_name( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version, char *module_name );
	void			load( void );
private:
	audio_backchannel_module_mapper( void );
	~audio_backchannel_module_mapper( void );

	vendors		_audio_backchannel_vendors;
	bool		_is_mapped;

	CRITICAL_SECTION _mutex;
};

#endif