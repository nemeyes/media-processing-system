#ifndef _DK_FF_VIDEO_DECODER_H_
#define _DK_FF_VIDEO_DECODER_H_

#include <dk_video_base.h>

namespace debuggerking
{
	class ffmpeg_core;
	class EXP_CLASS ff_video_decoder : public video_decoder
	{
	public:
		typedef struct EXP_CLASS _configuration_t : public video_decoder::configuration_t
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

		ff_video_decoder(void);
		virtual ~ff_video_decoder(void);

		int32_t initialize_decoder(void * config);
		int32_t release_decoder(void);
		int32_t decode(video_decoder::entity_t * encoded, video_decoder::entity_t * decoded);
		int32_t decode(video_decoder::entity_t * encoded);
		int32_t get_queued_data(video_decoder::entity_t * decoded);
		virtual void after_decoding_callback(uint8_t * decoded, size_t size);

	private:
		ffmpeg_core * _core;
	};
};


#endif