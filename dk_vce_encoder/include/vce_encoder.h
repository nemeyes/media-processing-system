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

#if defined(WITH_AMF_THREAD)
class vce_encoder;
class vce_polling_thread : public AMFThread
{
	friend class vce_encoder;
public:
	vce_polling_thread(vce_encoder * core);
	~vce_polling_thread(void);
	virtual void Run(void);
private:
	vce_encoder * _core;
};
#endif

class vce_encoder
{
#if defined(WITH_AMF_THREAD)
	friend class vce_polling_thread;
#endif
public:
	vce_encoder(dk_vce_encoder * front);
	~vce_encoder(void);

	dk_vce_encoder::ERR_CODE initialize_encoder(dk_vce_encoder::configuration_t * config);
	dk_vce_encoder::ERR_CODE release_encoder(void);

	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream, dk_vce_encoder::dk_video_entity_t * bitstream);
	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream);
	dk_vce_encoder::ERR_CODE get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream);

private:
	static const int next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);

private:
	dk_vce_encoder::configuration_t * _config;
	dk_vce_encoder * _front;
	vce_surface_observer * _surface_observer;

	void * _prev_surface;
	amf::AMFContextPtr _context;
	amf::AMFComponentPtr _encoder;
	amf::AMFSurfacePtr _surface;
	amf::AMF_SURFACE_FORMAT _cs;

#if defined(WITH_DEBUG_ES)
	HANDLE _file;
#endif

#if defined(WITH_AMF_THREAD)
	vce_polling_thread * _polling_thread;
#endif
};


#endif