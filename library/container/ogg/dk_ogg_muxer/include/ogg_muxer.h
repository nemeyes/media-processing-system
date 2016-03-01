#ifndef _OGG_MUXER_H_
#define _OGG_MUXER_H_

#include <cstdint>
#include <ogg/ogg.h>
#include "dk_ogg_muxer.h"


class ogg_muxer
{
public:
	typedef struct _opus_header_t
	{
		int32_t			version;
		int32_t			channels; /* Number of channels: 1..255 */
		int32_t			preskip;
		ogg_uint32_t	input_sample_rate;
		int32_t			gain; /* in dB S7.8 should be zero whenever possible */
		int32_t			channel_mapping;
		/* The rest is only used if channel_mapping != 0 */
		int32_t			nb_streams;
		int32_t			nb_coupled;
		uint8_t			stream_map[255];
	} opus_header_t;

	typedef struct _opus_packet_t 
	{
		unsigned char *data;
		int maxlen;
		int pos;
	} opus_packet_t;

	typedef struct _opus_ro_packet_t
	{
		const unsigned char *data;
		int maxlen;
		int pos;
	} opus_ro_packet_t;

	ogg_muxer(void);
	~ogg_muxer(void);


	dk_ogg_muxer::ERR_CODE initialize(dk_ogg_muxer::configuration_t * config);
	dk_ogg_muxer::ERR_CODE multiplex(void);
	dk_ogg_muxer::ERR_CODE release(void);

private:
	int32_t opus_header_parse(const uint8_t * packet, int32_t length, opus_header_t * h);
	int32_t opus_header_to_packet(const opus_header_t * h, uint8_t * packet, int32_t length);

	static int32_t write_uint32(ogg_muxer::opus_packet_t * p, ogg_uint32_t val);
	static int32_t write_uint16(ogg_muxer::opus_packet_t * p, ogg_uint16_t val);
	static int32_t write_chars(ogg_muxer::opus_packet_t * p, const unsigned char * str, int nb_chars);
	static int32_t read_uint32(ogg_muxer::opus_ro_packet_t * p, ogg_uint32_t * val);
	static int32_t read_uint16(ogg_muxer::opus_ro_packet_t * p, ogg_uint16_t * val);
	static int32_t read_chars(ogg_muxer::opus_ro_packet_t * p, unsigned char * str, int nb_chars);


private:
	dk_ogg_muxer::configuration_t * _config;
};












#endif