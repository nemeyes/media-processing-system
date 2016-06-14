#ifndef _DK_RTSP_RECEIVER_H_
#define _DK_RTSP_RECEIVER_H_

#include <dk_base_receiver.h>
#include <dk_rtsp_client.h>
#include <dk_ff_video_decoder.h>
#include <dk_aac_decoder.h>
#include <dk_ff_mp3_decoder.h>
#include <dk_directdraw_renderer.h>
#include <dk_bitmap_renderer.h>
#include <dk_mmwave_renderer.h>

namespace debuggerking
{
	class rtsp_async_callback
	{
	public:
		virtual void invoke(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second) = 0;
	};

	class rtsp_receiver : public base_receiver, public rtsp_client
	{
	public:
		rtsp_receiver(rtsp_async_callback * cb = nullptr);
		virtual ~rtsp_receiver(void);

		int32_t enable_osd(bool enable);
		int32_t set_osd_position(int32_t x, int32_t y);
		int32_t get_last_time(int32_t & year, int32_t & month, int32_t & day, int32_t & hour, int32_t & minute, int32_t & second);

		int32_t play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, float scale, bool repeat, HWND hwnd);
		int32_t stop(void);

		void on_begin_video(int32_t smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);
		void on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);

	private:
		rtsp_async_callback * _cb;
		int64_t _frame_count;

		bool _osd_enable;
		int32_t _osd_x;
		int32_t _osd_y;

		int32_t _last_year;
		int32_t _last_month;
		int32_t _last_day;
		int32_t _last_hour;
		int32_t _last_minute;
		int32_t _last_second;
	};
};

#endif
