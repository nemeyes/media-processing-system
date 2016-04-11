#include "platform.h"
#include <dk_auto_lock.h>
#include "mmwave_input_source.h"
#include "file_input_source.h"
#include "audio_backchannel_device_info.h"
#include "backend_audio_backchannel_controller.h"

#define DATACOUNT	21			//21
#define DATALENGTH	40			//40

#include "audio_backchannel_module_mapper.h"

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
static int g726_pack( unsigned char *buf, unsigned char *cw, unsigned char num_cw, int bps )
{
	int i, bits = 0, x = 0;
	for(i=0; i<num_cw; i++)
	{
		buf[x] |= cw[i] << bits;
		bits += bps;
		// assert((bits != 8) || (i==num_cw-1));
		if(bits > 8)
		{
			bits &= 0x07;
			x++;
			buf[x] |= cw[i] >> (bps - bits);
		}
	}
	return (num_cw * bps / 8);
}

static int g726_unpack( unsigned char *cw, unsigned char *buf, unsigned char num_cw, int bps )
{
	int i = 0, bits = 0, x = 0;
	unsigned char mask = 0;
	while(i < bps)
	{
		mask |= 1 << i;
		i++;
	}

	for(i = 0; i < num_cw; i++)
	{
		cw[i] = (buf[x] >> bits) & mask;
		bits += bps;
		// assert((bits != 8) || (i == num_cw - 1));
		if(bits > 8)
		{
			bits &= 0x07;
			x++;
			cw[i] |= buf[x] << (bps - bits);
			cw[i] &= mask;
		}
	}
	return (num_cw * bps / 8);
}
#endif

void create_audio_backchannel_render_buffer( LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T audiobc_buffer, int size, char *source )
{
	audiobc_buffer->buffer_size = size;
	audiobc_buffer->buffer = static_cast<char*>( malloc(audiobc_buffer->buffer_size) );
	memcpy( audiobc_buffer->buffer, source, audiobc_buffer->buffer_size );
}

void destroy_audio_backchannel_render_buffer( LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T audiobc_buffer )
{
	if( audiobc_buffer->buffer )
	{
		free( audiobc_buffer->buffer );
		audiobc_buffer->buffer = 0;
	}
	audiobc_buffer->buffer_size = 0;
}

typedef base_audio_backchannel_controller* (*pfnCreateAudioBackChannelController)();
typedef void (*pfnDestroyAudioBackChannelController)( base_audio_backchannel_controller **conroller );

backend_audio_backchannel_controller::backend_audio_backchannel_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version )
	: _controller(0)
	, _instance(0)
	, _encoded_temp_buffer(0)
	, _microphone(true)
	, _input_source(0)
	, _enable(false)
	, _av_codec_context(0)
	, _tid(0)
{
	InitializeCriticalSection( &_mutex );

	audio_backchannel_module_mapper::instance().load();
	HINSTANCE self;
#if defined(_DEBUG)
	self = ::GetModuleHandle( _T("audio_backchannel_controllerd.dll") );
#else
	self = ::GetModuleHandle( _T("audio_backchannel_controller.dll") );
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
	snprintf( szModulePath, sizeof(szModulePath), "%s%s", szModulePath, "audio_backchannel_implements" );
	audio_backchannel_module_mapper::instance().get_module_name( vendor, vendor_device, protocol, firmware_version, szModuleName );
	if( strlen(szModulePath)>0 && strlen(szModuleName)>0 )
	{
		SetDllDirectoryA( szModulePath );
		_instance = LoadLibraryA( szModuleName );
		if( _instance )
		{
			pfnCreateAudioBackChannelController pfnCreate = (pfnCreateAudioBackChannelController)::GetProcAddress( _instance, "create" );
			if( pfnCreate )
			{
				_controller = (pfnCreate)();
				if( !_controller )
					FreeLibrary( _instance );
			}
			else
				FreeLibrary( _instance );
		}
	}
}

backend_audio_backchannel_controller::~backend_audio_backchannel_controller( void )
{
	if( _instance )
	{
		pfnDestroyAudioBackChannelController pfnDestroy = (pfnDestroyAudioBackChannelController)::GetProcAddress( _instance, "destroy" );
		if( _controller )
		{
			(pfnDestroy)( &_controller );
			_controller = 0;
		}
		FreeLibrary( _instance );
		_instance = 0;
	}
	DeleteCriticalSection( &_mutex );
}

unsigned short backend_audio_backchannel_controller::get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length )
{
	return audio_backchannel_module_mapper::instance().get_module_vendor_information( vendor_ids, vendor_names, length );
}

unsigned short backend_audio_backchannel_controller::get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length )
{
	return audio_backchannel_module_mapper::instance().get_module_vendor_device_informations( vendor_id, vendor_device_ids, vendor_device_names, length );
}

unsigned short backend_audio_backchannel_controller::get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length )
{
	return audio_backchannel_module_mapper::instance().get_module_vendor_device_protocol_informations( vendor_id, vendor_device_id, vendor_device_protocol_ids, vendor_device_protocol_names, length );
}

unsigned short backend_audio_backchannel_controller::get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length )
{
	return audio_backchannel_module_mapper::instance().get_module_device_version_informations( vendor_id, vendor_device_id, vendor_device_protocol_id, vendor_device_version_ids, vendor_device_version_names, length );
}

char* backend_audio_backchannel_controller::get_vendor_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_name();
}

char* backend_audio_backchannel_controller::get_vendor_device_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_name();
}

char* backend_audio_backchannel_controller::get_vendor_device_protocol_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_protocol_name();
}

char* backend_audio_backchannel_controller::get_vendor_device_version_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_version_name();
}

unsigned short backend_audio_backchannel_controller::get_vendor_id( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_id();
}

unsigned short backend_audio_backchannel_controller::get_vendor_device_id( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_id();
}

unsigned short backend_audio_backchannel_controller::get_vendor_device_protocol_id( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_protocol_id();
}

unsigned short backend_audio_backchannel_controller::get_vendor_device_version_id( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_version_id();
}

unsigned short	backend_audio_backchannel_controller::set_host_name( char *host_name )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_host_name( host_name );
}

unsigned short	backend_audio_backchannel_controller::set_port_number( unsigned short port_number )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_port_number( port_number );
}

unsigned short	backend_audio_backchannel_controller::set_user_id( char *user_id )
{
	if( !_controller ) return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_user_id( user_id );
}

unsigned short	backend_audio_backchannel_controller::set_user_password( char *password )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_user_password( password );
}

unsigned short backend_audio_backchannel_controller::connect( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->connect();
}

unsigned short backend_audio_backchannel_controller::disconnect( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->disconnect();
}

unsigned short backend_audio_backchannel_controller::get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_codec_type( codec_type, bit_depth, sample_rate );
}

unsigned short backend_audio_backchannel_controller::get_duration( float &duration )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_duration( duration );
}

unsigned short backend_audio_backchannel_controller::initialize_encoder( AUDIO_BACKCHANNEL_CODEC_TYPE_T codec_type, unsigned short & channels, unsigned long & sample_per_second, unsigned short & bit_per_sample, int & sample_size )
{
	_codec_type = codec_type;
	if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_AAC )
	{
		_av_codec = avcodec_find_encoder((AVCodecID)AV_CODEC_ID_AAC);
		_av_codec_context = avcodec_alloc_context3(_av_codec);
	}
	else if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G711A )
	{
		channels			= 1;	//channels mono or stereo
		//sample_per_second	= 8000;	//sample rate
		//bit_per_sample	= 16;	//bit depth

		_av_codec = avcodec_find_encoder((AVCodecID)AV_CODEC_ID_PCM_ALAW);
		_av_codec_context = avcodec_alloc_context3(_av_codec);
		_av_codec_context->channels = channels;
		_av_codec_context->sample_rate = sample_per_second;	//8khz
		_av_codec_context->bit_rate = sample_per_second*bit_per_sample*channels;//
		_av_codec_context->sample_fmt = *(_av_codec->sample_fmts);
		avcodec_open2(_av_codec_context, _av_codec, NULL);
		_av_codec_context->frame_size = DATALENGTH*DATACOUNT*4;					//價Ы偎熱	
		sample_size = _av_codec_context->frame_size*2;
		_encoded_temp_buffer = (uint8_t*)malloc(sample_size);
	}
	else if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G711U )
	{
		channels			= 1;	//channels mono or stereo
		//sample_per_second	= 8000;	//sample rate
		//bit_per_sample	= 16;	//bit depth

		_av_codec = avcodec_find_encoder((AVCodecID)AV_CODEC_ID_PCM_MULAW);
		_av_codec_context = avcodec_alloc_context3(_av_codec);
		_av_codec_context->channels = channels;
		_av_codec_context->sample_rate = sample_per_second;	//8khz
		_av_codec_context->bit_rate = sample_per_second*bit_per_sample*channels;//
		_av_codec_context->sample_fmt = *(_av_codec->sample_fmts);
		avcodec_open2(_av_codec_context, _av_codec, NULL);
		_av_codec_context->frame_size = DATALENGTH*DATACOUNT*4;					//價Ы偎熱	
		sample_size = _av_codec_context->frame_size*2;
		_encoded_temp_buffer = (uint8_t*)malloc(sample_size);
	}
	else if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_16 )
	{
		channels = 1;	//channels mono or stereo
		//sample_per_second	= 8000;	//sample rate
		//bit_per_sample = 16;	//bit depth

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
# define AUDIO_REC_RAW_SIZE (160 * 2 * 12)
		g726_init_state(&_g726);
		sample_size = AUDIO_REC_RAW_SIZE;
		//_encoded_temp_buffer = (uint8_t*)malloc( DATALENGTH*DATACOUNT*4*2 );
#else
		_av_codec = avcodec_find_encoder((AVCodecID)AV_CODEC_ID_ADPCM_G726);
		_av_codec_context = avcodec_alloc_context3(_av_codec);
		_av_codec_context->channels = channels;
		_av_codec_context->sample_rate = sample_per_second;	//8khz
		_av_codec_context->bit_rate = 16000;//sample_per_second*bit_per_sample*channels;//
		//_av_codec_context->bits_per_coded_sample =	2;//bit_per_sample;
		_av_codec_context->sample_fmt = *(_av_codec->sample_fmts);
		avcodec_open( _av_codec_context, _av_codec );
		_av_codec_context->frame_size = DATALENGTH*DATACOUNT*4;					//價Ы偎熱	
		sample_size = _av_codec_context->frame_size*2;
		_encoded_temp_buffer = (uint8_t*)malloc( sample_size );
#endif
	}
	else if(_codec_type == AUDIO_BACKCHANNEL_CODEC_TYPE_G726_24)
	{
		channels = 1;	//channels mono or stereo
		//sample_per_second	= 8000;	//sample rate
		//bit_per_sample = 16;	//bit depth

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
		g726_init_state( &_g726 );
		_encoded_temp_buffer = (uint8_t*)malloc(DATALENGTH*DATACOUNT*4*2);
#else


#endif
	}
	else if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_32 )
	{
		channels = 1;	//channels mono or stereo
		//sample_per_second	= 8000;	//sample rate
		//bit_per_sample = 16;	//bit depth

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
		g726_init_state( &_g726 );
		_encoded_temp_buffer = (uint8_t*)malloc(DATALENGTH*DATACOUNT*4*2);
#else


#endif
	}
	else if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_40 )
	{
		channels = 1;	//channels mono or stereo
		//sample_per_second	= 8000;	//sample rate
		//bit_per_sample = 16;	//bit depth

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
		g726_init_state( &_g726 );
		sample_size = DATALENGTH*DATACOUNT*4*2;
		//_encoded_temp_buffer = (uint8_t*)malloc( DATALENGTH*DATACOUNT*4*2 );
#else


#endif
	}
	else if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_MP4A_LATM )
	{
		channels			= 1;	//channels mono or stereo
		//sample_per_second	= 8000;	//sample rate
		//bit_per_sample	= 16;	//bit depth

		_av_codec = avcodec_find_encoder((AVCodecID)AV_CODEC_ID_AAC_LATM);
		_av_codec_context = avcodec_alloc_context3(_av_codec);

		avcodec_open2(_av_codec_context, _av_codec, NULL);
		_av_codec_context->frame_size = DATALENGTH*DATACOUNT*4;					//價Ы偎熱	
		sample_size = _av_codec_context->frame_size*2;
		_encoded_temp_buffer = (uint8_t*)malloc(sample_size);
	}
	else if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_PCM )
	{
		channels			= 1;	//channels mono
		sample_size			= DATALENGTH*DATACOUNT*4*2;
	}
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short backend_audio_backchannel_controller::release_encoder( void )
{
	
#if defined(_USE_THIRDPARTY_G726_LIBRARY)
	if( (_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_16) || (_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_24) ||
		(_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_32) || (_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_40) )
	{

	}
	else
#endif
	{
		if( _av_codec_context )
		{
			avcodec_close( _av_codec_context ); 
			av_free( _av_codec_context );
			_av_codec_context = 0;
		}
	}
	if( _encoded_temp_buffer )
	{
		free( _encoded_temp_buffer );
		_encoded_temp_buffer = 0;
	}

	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short backend_audio_backchannel_controller::start( bool microphone, char *path, audio_backchannel_progress progress )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;

	if( connect()!=VMS_AUDIO_BACKCHANNEL_SUCCESS )
	{
		disconnect();
		return VMS_AUDIO_BACKCHANNEL_FAIL;
	}


	if( _input_source )
	{
		_input_source->release();
		delete _input_source;
		_input_source = 0;
	}
	_microphone = microphone;

	if( !_microphone && strlen(path)>0 ) 
		strncpy( _wav_file_path, path, MAX_PATH );

	_codec_type						= AUDIO_BACKCHANNEL_CODEC_TYPE_G711U;
	unsigned short		channels	= 0;
	unsigned long		sample_rate = 0;
	unsigned short		bit_depth	= 0;
	int					sample_size = 0;
	_controller->get_codec_type( _codec_type, bit_depth, sample_rate );
	initialize_encoder( _codec_type, channels, sample_rate, bit_depth, sample_size );

	if( _microphone )
	{
		_input_source = new mmwave_input_source();
	}
	else
	{
		if( strlen(_wav_file_path)>0 )
		{
			if( progress )
				_input_source = new file_input_source( _wav_file_path, progress );
			else
				_input_source = new file_input_source( _wav_file_path, 0 );
		}
	}

	unsigned thrdid = 0;
	_tid = reinterpret_cast<HANDLE>( _beginthreadex(NULL, 0, backend_audio_backchannel_controller::process_render, this, 0, &thrdid) );
	if( _input_source )	
	{
		_input_source->initialize( this, channels, sample_rate, bit_depth, sample_size );
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	return VMS_AUDIO_BACKCHANNEL_FAIL;
}

unsigned short backend_audio_backchannel_controller::stop( void )
{
	_enable = false;
	if(_tid != 0) 
	{
		::WaitForSingleObject( _tid, INFINITE );
		CloseHandle( _tid );
	}
	if( _input_source )
	{
		_input_source->release();
		delete _input_source;
		_input_source = 0;
	}

	release_encoder();

	{
		dk_auto_lock lock( &_mutex );
		std::vector<AUDIO_BACKCHANNEL_RENDER_BUFFER_T*>::iterator iter;
		for( iter=_render_buffers.begin(); iter!=_render_buffers.end(); iter++ )
		{
			destroy_audio_backchannel_render_buffer( (*iter) );
			free( (*iter) );
			(*iter) = 0;
		}
		_render_buffers.clear();
	}

	if( disconnect()!=VMS_AUDIO_BACKCHANNEL_SUCCESS )
		return VMS_AUDIO_BACKCHANNEL_FAIL;
	else
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short backend_audio_backchannel_controller::process( void *input, int input_size, void *output, int &output_size )
{
	unsigned short value = VMS_AUDIO_BACKCHANNEL_FAIL;
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
	if( (_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_16) || (_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_24) ||
		(_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_32) || (_codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_G726_40) )
	{
		short frame_size = 160;
		unsigned char *pIn = (unsigned char*)input;
		unsigned char *pOut = (unsigned char*)_ou_enc_unpacked;
		unsigned char cw[8] = {0};
		int i = 0;
		int length = 0;
		switch( _codec_type )
		{
		case AUDIO_BACKCHANNEL_CODEC_TYPE_G726_16:
			for( int cnt=0; cnt<(input_size/(frame_size*2)); cnt++ )
			{
				length = encode_g726_16( &pIn[cnt*(frame_size*2)], pOut, frame_size*2 );
				if( length>0 )
				{
					if( _render_buffers.size()<30 )
					{
						LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T buffer_render = static_cast<LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T>( malloc(sizeof(AUDIO_BACKCHANNEL_RENDER_BUFFER_T)) );
						create_audio_backchannel_render_buffer( buffer_render, length, reinterpret_cast<char*>(pOut) );
						::EnterCriticalSection( &_mutex );
						_render_buffers.push_back( buffer_render );
						::LeaveCriticalSection( &_mutex );
					}
					value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
				}
			}
			break;
		case AUDIO_BACKCHANNEL_CODEC_TYPE_G726_24:
			break;
		case AUDIO_BACKCHANNEL_CODEC_TYPE_G726_32:
			break;
		case AUDIO_BACKCHANNEL_CODEC_TYPE_G726_40:
			break;
		}
	}
	else
#endif
	{
		if( _codec_type==AUDIO_BACKCHANNEL_CODEC_TYPE_PCM )
		{
			value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
			if( input_size>0 )
			{
				if( _render_buffers.size()<30 )
				{
					LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T buffer_render = static_cast<LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T>( malloc(sizeof(AUDIO_BACKCHANNEL_RENDER_BUFFER_T)) );
					create_audio_backchannel_render_buffer( buffer_render, input_size, reinterpret_cast<char*>(input) );
					::EnterCriticalSection( &_mutex );
					_render_buffers.push_back( buffer_render );
					::LeaveCriticalSection( &_mutex );
				}
				value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
			}
		}
		else
		{
			int encoded_size	 = 0;
			__try
			{
				encoded_size = avcodec_encode_audio2( _av_codec_context, _encoded_temp_buffer, input_size/2, (const short*)input );
				value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
				if( encoded_size>0 )
				{
					if( _render_buffers.size()<30 )
					{
						LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T buffer_render = static_cast<LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T>( malloc(sizeof(AUDIO_BACKCHANNEL_RENDER_BUFFER_T)) );
						create_audio_backchannel_render_buffer( buffer_render, encoded_size, reinterpret_cast<char*>(_encoded_temp_buffer) );
						::EnterCriticalSection( &_mutex );
						_render_buffers.push_back( buffer_render );
						::LeaveCriticalSection( &_mutex );
					}
					value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
				}
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
				value = VMS_AUDIO_BACKCHANNEL_FAIL;
			}
		}
	}
	return value;
}

unsigned __stdcall backend_audio_backchannel_controller::process_render( void *param )
{
	backend_audio_backchannel_controller *self	= static_cast<backend_audio_backchannel_controller*>( param );
	unsigned short value			= VMS_AUDIO_BACKCHANNEL_FAIL;
	int output_size					= 0;
	self->_enable					= true;
	while( self->_enable )
	{
		if( self->_render_buffers.size()<1 )
		{
			::Sleep( 10 );//for context switching
			continue;
		}

		{
			dk_auto_lock lock( &self->_mutex );
			AUDIO_BACKCHANNEL_RENDER_BUFFER_T *buffer_render = self->_render_buffers.front();
			if( buffer_render )
			{
				value = self->_controller->process( buffer_render->buffer, buffer_render->buffer_size, 0, output_size );
				destroy_audio_backchannel_render_buffer( buffer_render );
				free( buffer_render );
				buffer_render = 0;
			}
			self->_render_buffers.erase( self->_render_buffers.begin() );
		}
	}
	return 0;
}

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
int	backend_audio_backchannel_controller::encode_g726_16( unsigned char *src, unsigned char *dst, int src_size )
{
	int				cnt;
	int				dst_size = 0;
	short			*in_buf = NULL;
	unsigned char	*out_buf = NULL;
	unsigned char	cw[8];


	if (!src || !dst)	return -1;


	in_buf = (short *)src;
	out_buf = dst;

	for (cnt = 0; cnt < (src_size / 2); cnt += 4)
	{
		int r = 0;

		cw[0] = g726_16_encoder( in_buf[cnt],     AUDIO_ENCODING_LINEAR, &_g726 );
		cw[1] = g726_16_encoder( in_buf[cnt + 1], AUDIO_ENCODING_LINEAR, &_g726 );
		cw[2] = g726_16_encoder( in_buf[cnt + 2], AUDIO_ENCODING_LINEAR, &_g726 );
		cw[3] = g726_16_encoder( in_buf[cnt + 3], AUDIO_ENCODING_LINEAR, &_g726 );

		*out_buf = 0;
		r += g726_pack( out_buf, cw, 4, 2 );
		out_buf += r;
		dst_size += r;
	}
	return dst_size;
}

int	backend_audio_backchannel_controller::encode_g726_24( unsigned char *src, unsigned char *dst, int src_size )
{
	int				cnt;
	int				dst_size = 0;
	short			*in_buf = NULL;
	unsigned char	*out_buf = NULL;
	unsigned char	cw[8];


	if (!src || !dst)	return -1;


	in_buf = (short *)src;
	out_buf = dst;

	for (cnt = 0; cnt < (src_size / 2); cnt += 4)
	{
		int r = 0;
		cw[0] = g726_24_encoder( in_buf[cnt],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[1] = g726_24_encoder( in_buf[cnt+1],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[2] = g726_24_encoder( in_buf[cnt+2],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[3] = g726_24_encoder( in_buf[cnt+3],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[4] = g726_24_encoder( in_buf[cnt+4],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[5] = g726_24_encoder( in_buf[cnt+5],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[6] = g726_24_encoder( in_buf[cnt+6],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[7] = g726_24_encoder( in_buf[cnt+7],	AUDIO_ENCODING_LINEAR, &_g726 );
		*out_buf = 0;
		r += g726_pack( out_buf, cw, 8, 3 );
		out_buf += r;
		dst_size += r;
	}
	return dst_size;
}

int	backend_audio_backchannel_controller::encode_g726_32( unsigned char *src, unsigned char *dst, int src_size )
{
	int				cnt;
	int				dst_size = 0;
	short			*in_buf = NULL;
	unsigned char	*out_buf = NULL;
	unsigned char	cw[8];


	if (!src || !dst)	return -1;


	in_buf = (short *)src;
	out_buf = dst;

	for (cnt = 0; cnt < (src_size / 2); cnt += 4)
	{
		int r = 0;

		cw[0] = g726_32_encoder( in_buf[cnt],     AUDIO_ENCODING_LINEAR, &_g726 );
		cw[1] = g726_32_encoder( in_buf[cnt + 1], AUDIO_ENCODING_LINEAR, &_g726 );
		*out_buf = 0;
		r += g726_pack( out_buf, cw, 2, 4 );
		out_buf += r;
		dst_size += r;
	}
	return dst_size;
}

int	backend_audio_backchannel_controller::encode_g726_40( unsigned char *src, unsigned char *dst, int src_size )
{
	int				cnt;
	int				dst_size = 0;
	short			*in_buf = NULL;
	unsigned char	*out_buf = NULL;
	unsigned char	cw[8];


	if (!src || !dst)	return -1;


	in_buf = (short *)src;
	out_buf = dst;

	for (cnt = 0; cnt < (src_size / 2); cnt += 4)
	{
		int r = 0;

		cw[0] = g726_40_encoder( in_buf[cnt],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[1] = g726_40_encoder( in_buf[cnt+1],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[2] = g726_40_encoder( in_buf[cnt+2],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[3] = g726_40_encoder( in_buf[cnt+3],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[4] = g726_40_encoder( in_buf[cnt+4],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[5] = g726_40_encoder( in_buf[cnt+5],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[6] = g726_40_encoder( in_buf[cnt+6],	AUDIO_ENCODING_LINEAR, &_g726 );
		cw[7] = g726_40_encoder( in_buf[cnt+7],	AUDIO_ENCODING_LINEAR, &_g726 );

		*out_buf = 0;
		r += g726_pack( out_buf, cw, 8, 5 );
		out_buf += r;
		dst_size += r;
	}
	return dst_size;
}
#endif