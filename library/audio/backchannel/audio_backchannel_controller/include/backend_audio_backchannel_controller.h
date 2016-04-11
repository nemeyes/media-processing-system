#include "audio_backchannel_controller.h"
#include <vector>
extern "C" 
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
}

#define _USE_THIRDPARTY_G726_LIBRARY

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
# include "g726/g72x.h"
#endif

typedef struct _AUDIO_BACKCHANNEL_RENDER_BUFFER_T
{
	int		buffer_size;
	char	*buffer;
} AUDIO_BACKCHANNEL_RENDER_BUFFER_T, *LPAUDIO_BACKCHANNEL_RENDER_BUFFER_T;

class base_input_source;
class backend_audio_backchannel_controller
{
public:
	backend_audio_backchannel_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version );
	virtual ~backend_audio_backchannel_controller( void );

	static unsigned short	get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length );
	static unsigned short	get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length );
	static unsigned short	get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length );
	static unsigned short	get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length );

	char*			get_vendor_name( void );
	char*			get_vendor_device_name( void );
	char*			get_vendor_device_protocol_name( void );
	char*			get_vendor_device_version_name( void );

	unsigned short	get_vendor_id( void );
	unsigned short	get_vendor_device_id( void );
	unsigned short	get_vendor_device_protocol_id( void );
	unsigned short	get_vendor_device_version_id( void );

	unsigned short	set_host_name( char *host_name );
	unsigned short	set_port_number( unsigned short port_number );
	unsigned short	set_user_id( char *user_id );
	unsigned short	set_user_password( char *password );

	unsigned short	connect( void );
	unsigned short	disconnect( void );
	unsigned short	get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate );
	unsigned short	get_duration( float &duration );
	unsigned short	process( void *input, int input_size, void *output, int &output_size );

	unsigned short	start( bool microphone, char *path, audio_backchannel_progress progress );
	unsigned short	stop( void );

	unsigned short	initialize_encoder( AUDIO_BACKCHANNEL_CODEC_TYPE_T codec_type, unsigned short & channels, unsigned long & sample_per_second, unsigned short & bit_per_sample, int & sample_size );
	unsigned short	release_encoder( void );

private:
	static unsigned __stdcall process_render( void *param );
#if defined(_USE_THIRDPARTY_G726_LIBRARY)
	int				encode_g726_16( unsigned char *src, unsigned char *dst, int src_size );
	int				encode_g726_24( unsigned char *src, unsigned char *dst, int src_size );
	int				encode_g726_32( unsigned char *src, unsigned char *dst, int src_size );
	int				encode_g726_40( unsigned char *src, unsigned char *dst, int src_size );
#endif

private:
	AUDIO_BACKCHANNEL_CODEC_TYPE_T		_codec_type;
	//g.711 and aac
	uint8_t								*_encoded_temp_buffer;
	AVCodec								*_av_codec;
	AVCodecContext						*_av_codec_context;

#if defined(_USE_THIRDPARTY_G726_LIBRARY)
	//g.726
	struct g726_state_s					_g726;
	short								_ou_enc_unpacked[480+1];
#endif

	base_audio_backchannel_controller	*_controller;
	HINSTANCE							_instance;

	bool								_microphone;
	char								_wav_file_path[MAX_PATH];


	base_input_source					*_input_source;

	bool								_enable;
	CRITICAL_SECTION					_mutex;
	HANDLE								_tid;
	std::vector<AUDIO_BACKCHANNEL_RENDER_BUFFER_T*>	_render_buffers;


};