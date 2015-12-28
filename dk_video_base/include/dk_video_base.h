#ifndef _DK_VIDEO_BASE_H_
#define _DK_VIDEO_BASE_H_

#if defined(WIN32)
#include <windows.h>
#include <d3d9.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d11.h>
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#include <pthreads.h>
#define EXP_CLASS
#endif

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <memory>

typedef struct _dk_circular_buffer_t dk_circular_buffer_t;
class EXP_CLASS dk_video_base
{
public:
	typedef struct _vbuffer_t
	{
		size_t amount;
		_vbuffer_t * prev;
		_vbuffer_t * next;
	} vbuffer_t;

	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAIL,
		ERR_CODE_NOT_IMPLEMENTED,
	} ERR_CODE;

	typedef enum _SUBMEDIA_TYPE
	{
		SUBMEDIA_TYPE_UNKNOWN = -1,
		SUBMEDIA_TYPE_AVC, //H264 bitstream without start code(avcc)
		SUBMEDIA_TYPE_H264,//H264 bitstream with start code(annex B)
		SUBMEDIA_TYPE_H264_BP,
		SUBMEDIA_TYPE_H264_HP,
		SUBMEDIA_TYPE_H264_MP,
		SUBMEDIA_TYPE_H264_EP,
		SUBMEDIA_TYPE_MP4V,
		SUBMEDIA_TYPE_MP4V_SP,
		SUBMEDIA_TYPE_MP4V_ASP,
		SUBMEDIA_TYPE_MJPEG,
		SUBMEDIA_TYPE_HEVC,
		SUBMEDIA_TYPE_RGB32,
		SUBMEDIA_TYPE_RGB24,
		SUBMEDIA_TYPE_YUY2,
		SUBMEDIA_TYPE_I420,
		SUBMEDIA_TYPE_YV12,
		SUBMEDIA_TYPE_NV12,
	} SUBMEDIA_TYPE;

	typedef enum _MEMORY_TYPE
	{
		MEMORY_TYPE_HOST = 0,
		MEMORY_TYPE_DX9,
		MEMORY_TYPE_DX9EX,
		MEMORY_TYPE_DX10,
		MEMORY_TYPE_DX10_1,
		MEMORY_TYPE_DX11_1,
		MEMORY_TYPE_DX11_2,
		MEMORY_TYPE_DX11_3,
		MEMORY_TYPE_DX12,
		MEMORY_TYPE_OPENGL,
		MEMORY_TYPE_OPENCL,
		MEMORY_TYPE_CUDA,
	} MEMORY_TYPE;

	typedef enum _PIC_TYPE
	{
		PICTURE_TYPE_NONE = 0,
		PICTURE_TYPE_IDR,
		PICTURE_TYPE_I,
		PICTURE_TYPE_P,
		PICTURE_TYPE_B
	} PIC_TYPE;

	typedef struct _dk_video_entity_t
	{
		MEMORY_TYPE			mem_type;
		void *				surface;
		//IDirect3DSurface9 * d3d9_surface;
		//ID3D10Texture2D *	d3d10_surface;
		//ID3D11Texture2D *	d3d11_surface;
		uint8_t *			data;
		size_t				data_size;
		size_t				data_capacity;
		PIC_TYPE			pic_type;
	} dk_video_entity_t;

	dk_video_base(void);
	virtual ~dk_video_base(void);

	ERR_CODE push(uint8_t * bs, size_t size);
	ERR_CODE pop(uint8_t * bs, size_t & size);
	ERR_CODE init(vbuffer_t * buffer);

private:
	vbuffer_t * _root;
	dk_circular_buffer_t * _vqueue;

#if defined(WIN32)
	CRITICAL_SECTION _mutex;
#else
	pthread_mutex _mutex;
#endif
};

class EXP_CLASS dk_video_decoder : public dk_video_base
{
public:
	dk_video_decoder(void);
	virtual ~dk_video_decoder(void);

	virtual ERR_CODE initialize_decoder(void * config);
	virtual ERR_CODE release_decoder(void);

	virtual ERR_CODE decode(dk_video_entity_t * bitstream, dk_video_entity_t * decoded);
	virtual ERR_CODE decode(dk_video_entity_t * bitstream);
	virtual ERR_CODE get_queued_data(dk_video_entity_t * decoded);
};

class EXP_CLASS dk_video_encoder : public dk_video_base
{
public:
	dk_video_encoder(void);
	virtual ~dk_video_encoder(void);

	virtual ERR_CODE initialize_encoder(void * config);
	virtual ERR_CODE release_encoder(void);

	virtual ERR_CODE encode(dk_video_entity_t * rawstream, dk_video_entity_t * bitstream);
	virtual ERR_CODE encode(dk_video_entity_t * rawstream);
	virtual ERR_CODE get_queued_data(dk_video_entity_t * bitstream);

};








#endif