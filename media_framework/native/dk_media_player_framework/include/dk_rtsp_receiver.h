#ifndef _DK_RTSP_RECEIVER_H_
#define _DK_RTSP_RECEIVER_H_

#include <dk_base_receiver.h>
#include <dk_live_rtsp_client.h>
#include <dk_ff_video_decoder.h>
#include <dk_aac_decoder.h>
#include <dk_ff_mp3_decoder.h>
#include <dk_directdraw_renderer.h>
#include <dk_mmwave_renderer.h>

class dk_rtsp_receiver : public dk_base_receiver, public dk_live_rtsp_client
{
public:
	dk_rtsp_receiver(void);
	virtual ~dk_rtsp_receiver(void);

	void play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, bool repeat, HWND hwnd);
	void stop(void);

	void on_begin_video(dk_live_rtsp_client::vsubmedia_type smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
	void on_recv_video(dk_live_rtsp_client::vsubmedia_type smt, const uint8_t * data, size_t data_size, long long timestamp);
	void on_begin_audio(dk_live_rtsp_client::asubmedia_type smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
	void on_recv_audio(dk_live_rtsp_client::asubmedia_type smt, const uint8_t * data, size_t data_size, long long timestamp);

private:
	int64_t _frame_count;
};
















#endif
