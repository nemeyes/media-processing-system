#ifndef _DK_FF_VIDEO_DECODER_H_
#define _DK_FF_VIDEO_DECODER_H_

#include <dk_video_base.h>

class ffmpeg_decoder;
class EXP_CLASS dk_ff_video_decoder : public dk_video_decoder
{
public:
	typedef struct EXP_CLASS _configuration_t : public dk_video_decoder::configuration_t
	{
		uint8_t * extradata;
		size_t extradata_size;
		_configuration_t(void)
			: extradata(nullptr)
			, extradata_size(0)
		{}

		_configuration_t(const _configuration_t & clone)
		{
			extradata = clone.extradata;
			extradata_size = clone.extradata_size;
		}

		_configuration_t & operator=(const _configuration_t & clone)
		{
			extradata = clone.extradata;
			extradata_size = clone.extradata_size;
			return (*this);
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
	} configuration_t;

	dk_ff_video_decoder(void);
	virtual ~dk_ff_video_decoder(void);

	dk_ff_video_decoder::err_code initialize_decoder(void * config);
	dk_ff_video_decoder::err_code release_decoder(void);
	dk_ff_video_decoder::err_code decode(dk_ff_video_decoder::dk_video_entity_t * encoded, dk_ff_video_decoder::dk_video_entity_t * decoded);
	dk_ff_video_decoder::err_code decode(dk_ff_video_decoder::dk_video_entity_t * encoded);
	dk_ff_video_decoder::err_code get_queued_data(dk_ff_video_decoder::dk_video_entity_t * decoded);

private:
	ffmpeg_decoder * _core;
};

#endif