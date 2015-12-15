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
class vce_encoder;
class vce_polling_thread : public AMFThread
{
public:
	vce_polling_thread(vce_encoder * core);
	~vce_polling_thread(void);
	virtual void Run(void);
protected:
	vce_encoder * _core;
};

class vce_encoder
{
	friend class vce_polling_thread;
public:
	vce_encoder(dk_vce_encoder * front);
	~vce_encoder(void);

	dk_vce_encoder::ERR_CODE initialize(dk_vce_encoder::configuration_t * config);
	dk_vce_encoder::ERR_CODE release(void);

	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream, dk_vce_encoder::dk_video_entity_t * bitstream);
	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream);
	dk_vce_encoder::ERR_CODE get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream);

private:
	dk_vce_encoder::configuration_t * _config;
	dk_vce_encoder * _front;
	vce_surface_observer * _surface_observer;


    amf::AMFContextPtr _context;
    amf::AMFComponentPtr _encoder;
    amf::AMFSurfacePtr _surface;
#if defined(_DEBUG)
	amf_int32 _submitted;
#endif
	amf::AMF_SURFACE_FORMAT _cs;
	
	vce_polling_thread * _polling_thread;
};


#endif