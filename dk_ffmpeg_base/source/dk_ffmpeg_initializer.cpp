#include "dk_ffmpeg_initializer.h"
#if defined(_WIN32)
#include <windows.h>
#endif

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/opt.h>
	#include <libavutil/mathematics.h>
}

#if defined(_WIN32)
int ffmpeg_lock_manager(void **mutex, enum AVLockOp op)
{
	CRITICAL_SECTION ** lock = (CRITICAL_SECTION**)mutex;
	switch (op)
	{
	case AV_LOCK_CREATE:
		(*lock) = new CRITICAL_SECTION();
		InitializeCriticalSection((*lock));
		break;
	case AV_LOCK_OBTAIN:
		EnterCriticalSection((*lock));
		break;
	case AV_LOCK_RELEASE:
		LeaveCriticalSection((*lock));
		break;
	case AV_LOCK_DESTROY:
		DeleteCriticalSection((*lock));
		delete (*lock);
		break;
	}
	return 0;
}
#endif

dk_ffmpeg_initializer::dk_ffmpeg_initializer(void)
{
	av_register_all();
#if defined(_WIN32)
	av_lockmgr_register(ffmpeg_lock_manager);
#endif
}

dk_ffmpeg_initializer::~dk_ffmpeg_initializer(void)
{
#if defined(_WIN32)
	av_lockmgr_register(0);
#endif
}

static dk_ffmpeg_initializer ffmpeg;