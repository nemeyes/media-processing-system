#ifndef _DK_VIDEO_BASE_H_
#define _DK_VIDEO_BASE_H_

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <memory>

#if defined(WIN32)
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#define EXP_CLASS
#endif

typedef struct _dk_video_entity_t
{
	uint8_t *	data;
	size_t		data_size;
	size_t		data_capacity;
} dk_video_entity_t;

class EXP_CLASS dk_video_base
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED,
		ERR_CODE_NOT_IMPLEMENTED,
	} ERR_CODE;

	typedef enum _SUBMEDIA_TYPE
	{
		SUBMEDIA_TYPE_UNKNOWN = -1,
		SUBMEDIA_TYPE_H264,
		SUBMEDIA_TYPE_H264_BP,
		SUBMEDIA_TYPE_H264_HP,
		SUBMEDIA_TYPE_H264_MP,
		SUBMEDIA_TYPE_H264_EP,
		SUBMEDIA_TYPE_MP4V,
		SUBMEDIA_TYPE_MP4V_SP,
		SUBMEDIA_TYPE_MP4V_ASP,
		SUBMEDIA_TYPE_MJPEG,
		SUBMEDIA_TYPE_RGB32,
		SUBMEDIA_TYPE_RGB24,
		SUBMEDIA_TYPE_YUY2,
		SUBMEDIA_TYPE_I420,
		SUBMEDIA_TYPE_YV12,
		SUBMEDIA_TYPE_NV12,
	} SUBMEDIA_TYPE;

	dk_video_base(void);
	virtual ~dk_video_base(void);

	ERR_CODE put_video(dk_video_entity_t * bitstream);
	ERR_CODE get_video(dk_video_entity_t * bitstream);
};

class EXP_CLASS dk_video_decoder : public dk_video_base
{
public:
	typedef struct EXP_CLASS _configuration_t
	{
		int32_t iwidth;
		int32_t iheight;
		int32_t sarw;
		int32_t sarh;
		int32_t owidth;
		int32_t oheight;
		int32_t ostride;
		SUBMEDIA_TYPE ismt;
		SUBMEDIA_TYPE osmt;
		uint8_t * extradata;
		size_t extradata_size;
		_configuration_t(void)
			: iwidth(0)
			, iheight(0)
			, owidth(0)
			, oheight(0)
			, ostride(0)
			, ismt(SUBMEDIA_TYPE_UNKNOWN)
			, osmt(SUBMEDIA_TYPE_UNKNOWN)
			, extradata(nullptr)
			, extradata_size(0)
		{}

		_configuration_t(const _configuration_t & clone)
		{
			iwidth = clone.iwidth;
			iheight = clone.iheight;
			owidth = clone.owidth;
			oheight = clone.oheight;
			ostride = clone.ostride;
			ismt = clone.ismt;
			osmt = clone.osmt;
			extradata = clone.extradata;
			extradata_size = clone.extradata_size;
		}

		~_configuration_t(void)
		{
			release_extradata();
		}

		void allocate_extradata(const uint8_t * data, size_t size)
		{
			extradata = static_cast<uint8_t*>(malloc(size));
			extradata_size = size;
			memcpy(extradata, data, extradata_size);
		}

		void release_extradata(void)
		{
			if (extradata && (extradata_size > 0))
			{
				free(extradata);
				extradata = nullptr;
			}
			extradata_size = 0;
		}

		_configuration_t & operator=(const _configuration_t & clone)
		{
			iwidth = clone.iwidth;
			iheight = clone.iheight;
			owidth = clone.owidth;
			oheight = clone.oheight;
			ostride = clone.ostride;
			ismt = clone.ismt;
			osmt = clone.osmt;
			extradata = clone.extradata;
			extradata_size = clone.extradata_size;
			return (*this);
		}
	} configuration_t;

	dk_video_decoder(void);
	virtual ~dk_video_decoder(void);

	virtual ERR_CODE initialize_decoder(_configuration_t * config);
	virtual ERR_CODE release_decoder(void);

	virtual ERR_CODE decode(dk_video_entity_t * bitstream, dk_video_entity_t * decoded);
	virtual ERR_CODE decode(dk_video_entity_t * bitstream);

protected:
	_configuration_t * _config;
};

class EXP_CLASS dk_video_encoder : public dk_video_base
{
public:
	dk_video_encoder(void);
	virtual ~dk_video_encoder(void);

	virtual ERR_CODE initialize_encoder(void);
	virtual ERR_CODE release_encoder(void);

	virtual ERR_CODE encode(dk_video_entity_t * bitstream) = 0;
};








#endif