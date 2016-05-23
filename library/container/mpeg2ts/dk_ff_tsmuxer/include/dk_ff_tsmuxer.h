#ifndef _DK_FF_TSMUXER_H_
#define _DK_FF_TSMUXER_H_

#include <dk_basic_type.h>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_DLL __declspec(dllexport)
# else
#  define EXP_DLL __declspec(dllimport)
# endif
#else
# define EXP_DLL
#endif

namespace debuggerking
{
	class ffmpeg_tsmuxer;
	class EXP_DLL ff_tsmuxer : public foundation
	{
		friend class ffmpeg_tsmuxer;
	public:
		typedef enum _tsmuxer_state
		{
			state_none,
			state_initialized,
		} tsmuxer_state;

		typedef struct EXP_DLL _configuration_t
		{
			typedef struct EXP_DLL _video_configuration_t
			{
				int32_t stream_index;
				int32_t width;
				int32_t height;
				int32_t bitrate;
				int32_t fps;
				int32_t extradata_size;
				uint8_t extradata[MAX_PATH];
				char	file_path[MAX_PATH];
				_video_configuration_t(void);
				_video_configuration_t(const _video_configuration_t & clone);
				_video_configuration_t operator=(const _video_configuration_t & clone);
			} video_configuration_t;

			video_configuration_t vconfig;

			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t operator=(const _configuration_t & clone);
		} configuration_t;

		ff_tsmuxer(void);
		virtual ~ff_tsmuxer(void);

		int32_t initialize(configuration_t * config);
		int32_t release(void);
		ff_tsmuxer::tsmuxer_state state(void);

		int32_t put_video_stream(const uint8_t * buffer, size_t nb, int64_t timestamp, bool keyframe);

		virtual int32_t after_demuxing_callback(uint8_t * ts, size_t stream_size);

	private:
		ffmpeg_tsmuxer * _core;
		HANDLE _ts_file;
	};
};







#endif