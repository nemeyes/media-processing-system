#ifndef _DK_RTMP_RECEIVER_H_
#define _DK_RTMP_RECEIVER_H_

#include <dk_base_receiver.h>
#include <dk_rtmp_client.h>
#include <dk_ff_video_decoder.h>
#include <dk_aac_decoder.h>
#include <dk_ff_mp3_decoder.h>
#include <dk_directdraw_renderer.h>
#include <dk_mmwave_renderer.h>


namespace debuggerking
{
	class rtmp_receiver : public base_receiver, public rtmp_client
	{
	public:
		rtmp_receiver(void);
		virtual ~rtmp_receiver(void);

		int32_t play(const char * url, const char * username, const char * password, int32_t recv_option, HWND hwnd);
		int32_t stop(void);

		void on_begin_video(int32_t smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);
		void on_begin_audio(int32_t smt, uint8_t * configstr, size_t configstr_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);

	private:
		int64_t _frame_count;
	};
};
















#endif
