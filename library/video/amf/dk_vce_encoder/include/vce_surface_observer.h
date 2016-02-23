#ifndef _VCE_SURFACE_OBSERVER_H_
#define _VCE_SURFACE_OBSERVER_H_

#include <amf/components/VideoEncoderVCE.h>


class vce_surface_observer : public amf::AMFSurfaceObserver
{
public:
	vce_surface_observer(void);
	~vce_surface_observer(void);

public:
	void AMF_STD_CALL OnSurfaceDataRelease(amf::AMFSurface * surface);
};










#endif