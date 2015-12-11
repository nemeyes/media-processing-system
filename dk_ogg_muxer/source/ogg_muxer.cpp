#include "ogg_muxer.h"
#include <memory>

const int wav_permute_matrix[8][8] =
{
	{ 0 },              /* 1.0 mono   */
	{ 0, 1 },            /* 2.0 stereo */
	{ 0, 2, 1 },          /* 3.0 channel ('wide') stereo */
	{ 0, 1, 2, 3 },        /* 4.0 discrete quadraphonic */
	{ 0, 2, 1, 3, 4 },      /* 5.0 surround */
	{ 0, 2, 1, 4, 5, 3 },    /* 5.1 surround */
	{ 0, 2, 1, 5, 6, 4, 3 },  /* 6.1 surround */
	{ 0, 2, 1, 6, 7, 4, 5, 3 } /* 7.1 surround (classic theater 8-track) */
};

dk_ogg_muxer::ERR_CODE ogg_muxer::initialize(dk_ogg_muxer::configuration_t * config)
{
	_config = config;



}

dk_ogg_muxer::ERR_CODE ogg_muxer::multiplex(void)
{

}

dk_ogg_muxer::ERR_CODE ogg_muxer::release(void)
{

}

int32_t ogg_muxer::opus_header_parse(const uint8_t * packet, int32_t length, opus_header_t * h)
{
	int i;
	char str[9];
	ogg_muxer::opus_ro_packet_t p;
	unsigned char ch;
	ogg_uint16_t shortval;

	p.data = packet;
	p.maxlen = length;
	p.pos = 0;
	str[8] = 0;
	if (length<19)
		return 0;
	read_chars(&p, (unsigned char*)str, 8);
	if (memcmp(str, "OpusHead", 8) != 0)
		return 0;

	if (!read_chars(&p, &ch, 1))
		return 0;
	h->version = ch;
	if ((h->version & 240) != 0) /* Only major version 0 supported. */
		return 0;

	if (!read_chars(&p, &ch, 1))
		return 0;
	h->channels = ch;
	if (h->channels == 0)
		return 0;

	if (!read_uint16(&p, &shortval))
		return 0;
	h->preskip = shortval;

	if (!read_uint32(&p, &h->input_sample_rate))
		return 0;

	if (!read_uint16(&p, &shortval))
		return 0;
	h->gain = (short)shortval;

	if (!read_chars(&p, &ch, 1))
		return 0;
	h->channel_mapping = ch;

	if (h->channel_mapping != 0)
	{
		if (!read_chars(&p, &ch, 1))
			return 0;

		if (ch<1)
			return 0;
		h->nb_streams = ch;

		if (!read_chars(&p, &ch, 1))
			return 0;

		if (ch>h->nb_streams || (ch + h->nb_streams)>255)
			return 0;
		h->nb_coupled = ch;

		/* Multi-stream support */
		for (i = 0; i<h->channels; i++)
		{
			if (!read_chars(&p, &h->stream_map[i], 1))
				return 0;
			if (h->stream_map[i]>(h->nb_streams + h->nb_coupled) && h->stream_map[i] != 255)
				return 0;
		}
	}
	else 
	{
		if (h->channels>2)
			return 0;
		h->nb_streams = 1;
		h->nb_coupled = h->channels>1;
		h->stream_map[0] = 0;
		h->stream_map[1] = 1;
	}
	/*For version 0/1 we know there won't be any more data
	so reject any that have data past the end.*/
	if ((h->version == 0 || h->version == 1) && p.pos != length)
		return 0;
	return 1;
}

int32_t ogg_muxer::opus_header_to_packet(const ogg_muxer::opus_header_t * h, uint8_t * packet, int32_t length)
{
	int i;
	ogg_muxer::opus_packet_t p;
	unsigned char ch;

	p.data = packet;
	p.maxlen = length;
	p.pos = 0;
	if (length<19)
		return 0;
	if (!write_chars(&p, (const unsigned char*)"OpusHead", 8))
		return 0;
	/* Version is 1 */
	ch = 1;
	if (!write_chars(&p, &ch, 1))
		return 0;

	ch = h->channels;
	if (!write_chars(&p, &ch, 1))
		return 0;

	if (!write_uint16(&p, h->preskip))
		return 0;

	if (!write_uint32(&p, h->input_sample_rate))
		return 0;

	if (!write_uint16(&p, h->gain))
		return 0;

	ch = h->channel_mapping;
	if (!write_chars(&p, &ch, 1))
		return 0;

	if (h->channel_mapping != 0)
	{
		ch = h->nb_streams;
		if (!write_chars(&p, &ch, 1))
			return 0;

		ch = h->nb_coupled;
		if (!write_chars(&p, &ch, 1))
			return 0;

		/* Multi-stream support */
		for (i = 0; i<h->channels; i++)
		{
			if (!write_chars(&p, &h->stream_map[i], 1))
				return 0;
		}
	}
	return p.pos;
}

int32_t ogg_muxer::write_uint32(ogg_muxer::opus_packet_t * p, ogg_uint32_t val)
{
	if (p->pos>p->maxlen - 4)
		return 0;
	p->data[p->pos] = (val)& 0xFF;
	p->data[p->pos + 1] = (val >> 8) & 0xFF;
	p->data[p->pos + 2] = (val >> 16) & 0xFF;
	p->data[p->pos + 3] = (val >> 24) & 0xFF;
	p->pos += 4;
	return 1;
}

int32_t ogg_muxer::write_uint16(ogg_muxer::opus_packet_t * p, ogg_uint16_t val)
{
	if (p->pos>p->maxlen - 2)
		return 0;
	p->data[p->pos] = (val)& 0xFF;
	p->data[p->pos + 1] = (val >> 8) & 0xFF;
	p->pos += 2;
	return 1;
}

int32_t ogg_muxer::write_chars(ogg_muxer::opus_packet_t * p, const unsigned char * str, int nb_chars)
{
	int i;
	if (p->pos>p->maxlen - nb_chars)
		return 0;
	for (i = 0; i<nb_chars; i++)
		p->data[p->pos++] = str[i];
	return 1;
}

int32_t ogg_muxer::read_uint32(ogg_muxer::opus_ro_packet_t * p, ogg_uint32_t * val)
{
	if (p->pos>p->maxlen - 4)
		return 0;
	*val = (ogg_uint32_t)p->data[p->pos];
	*val |= (ogg_uint32_t)p->data[p->pos + 1] << 8;
	*val |= (ogg_uint32_t)p->data[p->pos + 2] << 16;
	*val |= (ogg_uint32_t)p->data[p->pos + 3] << 24;
	p->pos += 4;
	return 1;
}

int32_t ogg_muxer::read_uint16(ogg_muxer::opus_ro_packet_t * p, ogg_uint16_t * val)
{
	if (p->pos>p->maxlen - 2)
		return 0;
	*val = (ogg_uint16_t)p->data[p->pos];
	*val |= (ogg_uint16_t)p->data[p->pos + 1] << 8;
	p->pos += 2;
	return 1;
}

int32_t ogg_muxer::read_chars(ogg_muxer::opus_ro_packet_t * p, unsigned char * str, int nb_chars)
{
	int i;
	if (p->pos>p->maxlen - nb_chars)
		return 0;
	for (i = 0; i<nb_chars; i++)
		str[i] = p->data[p->pos++];
	return 1;
}