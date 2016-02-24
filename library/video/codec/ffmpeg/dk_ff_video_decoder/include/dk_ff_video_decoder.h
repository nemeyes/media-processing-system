#ifndef _DK_FF_VIDEO_DECODER_H_
#define _DK_FF_VIDEO_DECODER_H_

#include <dk_video_base.h>

class ffmpeg_decoder;
class EXP_CLASS dk_ff_video_decoder : public dk_video_decoder
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

	dk_ff_video_decoder(void);
	virtual ~dk_ff_video_decoder(void);

	ERR_CODE initialize_decoder(void * config);
	ERR_CODE release_decoder(void);
	ERR_CODE decode(dk_ff_video_decoder::dk_video_entity_t * encoded, dk_ff_video_decoder::dk_video_entity_t * decoded);
	ERR_CODE decode(dk_ff_video_decoder::dk_video_entity_t * encoded);
	ERR_CODE get_queued_data(dk_ff_video_decoder::dk_video_entity_t * decoded);

private:
	ffmpeg_decoder * _core;
};

#endif