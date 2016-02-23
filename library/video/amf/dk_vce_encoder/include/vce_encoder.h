#ifndef _VCE_ENCODER_H_
#define _VCE_ENCODER_H_

#include <stdio.h>
#include <tchar.h>
#include <amf/components/VideoEncoderVCE.h>
#include "AMFPlatform.h"
#include "PlatformWindows.h"
#include "Thread.h"

#include "dk_vce_encoder.h"
#include "vce_surface_observer.h"

#define MAX_ENCODE_QUEUE 32

#if defined(WITH_ENCODING_THREAD)
#define WITH_AMF_CALLBACK_THREAD
#endif

class vce_encoder
{
public:
	vce_encoder(dk_vce_encoder * front);
	~vce_encoder(void);

	dk_vce_encoder::ENCODER_STATE state(void);

	dk_vce_encoder::ERR_CODE initialize_encoder(dk_vce_encoder::configuration_t * config);
	dk_vce_encoder::ERR_CODE release_encoder(void);

	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * input, dk_vce_encoder::dk_video_entity_t * bitstream);
	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * input);
	dk_vce_encoder::ERR_CODE get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream);

	dk_vce_encoder::ERR_CODE encode_async(dk_video_encoder::dk_video_entity_t * input);
	dk_vce_encoder::ERR_CODE check_encoding_finish(void);

private:
#if defined(WITH_AMF_CALLBACK_THREAD)
	static unsigned __stdcall query_output_callback(void * param);
	void query_output();
#endif

#if defined(WITH_ENCODING_THREAD)
	static unsigned __stdcall process_encoding_callback(void * param);
	void process_encoding(void);
#endif

	//static const int next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);

private:
#if defined(WITH_AMF_CALLBACK_THREAD)
	bool _cb_run;
	HANDLE _cb_thread;
	HANDLE _cb_event;
#endif

#if defined(WITH_ENCODING_THREAD)
	dk_vce_encoder::dk_video_entity_t _encoding_param;
	bool _encoding_run;
	bool _encoding;
	HANDLE _encoding_thread;
	HANDLE _encoding_finish_event;
#endif

	dk_vce_encoder::ENCODER_STATE _state;
	dk_vce_encoder::configuration_t * _config;
	dk_vce_encoder * _front;
	vce_surface_observer * _surface_observer;

	amf::AMFContextPtr _context;
	amf::AMFComponentPtr _encoder;
	amf::AMFSurfacePtr _surface;
	amf::AMF_SURFACE_FORMAT _cs;

	int32_t _submited;

#if defined(WITH_DEBUG_ES)
	HANDLE _file;
#endif
};


#endif