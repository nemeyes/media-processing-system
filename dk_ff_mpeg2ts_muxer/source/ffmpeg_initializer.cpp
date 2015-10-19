#include "ffmpeg_initializer.h"
#if defined(_WINDOWS)
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

#if defined(_WINDOWS)
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

ffmpeg_initializer::ffmpeg_initializer(void)
{
	av_register_all();
#if defined(_WINDOWS)
	av_lockmgr_register(ffmpeg_lock_manager);
#endif
}

ffmpeg_initializer::~ffmpeg_initializer(void)
{
#if defined(_WINDOWS)
	av_lockmgr_register(0);
#endif
}

static ffmpeg_initializer ffmpeg;