#include "stdafx.h"
#include "dk_rtsp_receiver.h"
#include "dk_image_creator.h"

static unsigned char const singleBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

#define MAX_LENGTH 32

static void shift_bits(unsigned char* toBasePtr, unsigned toBitOffset, unsigned char const* fromBasePtr, unsigned fromBitOffset, unsigned numBits)
{
	if (numBits == 0)
		return;

	/* Note that from and to may overlap, if from>to */
	unsigned char const* fromBytePtr = fromBasePtr + fromBitOffset / 8;
	unsigned fromBitRem = fromBitOffset % 8;
	unsigned char* toBytePtr = toBasePtr + toBitOffset / 8;
	unsigned toBitRem = toBitOffset % 8;

	while (numBits-- > 0) 
	{
		unsigned char fromBitMask = singleBitMask[fromBitRem];
		unsigned char fromBit = (*fromBytePtr)&fromBitMask;
		unsigned char toBitMask = singleBitMask[toBitRem];

		if (fromBit != 0) 
		{
			*toBytePtr |= toBitMask;
		}
		else 
		{
			*toBytePtr &= ~toBitMask;
		}

		if (++fromBitRem == 8) 
		{
			++fromBytePtr;
			fromBitRem = 0;
		}
		if (++toBitRem == 8) 
		{
			++toBytePtr;
			toBitRem = 0;
		}
	}
}

static void remove_emulation_bytes(uint8_t *dst, uint8_t *src, uint32_t max_size, uint32_t num_bytes_in_nal_unit, uint32_t *copy_size)
{
	unsigned int i;
	if (num_bytes_in_nal_unit > max_size) return;
	*copy_size = 0;
	for (i = 0; i < num_bytes_in_nal_unit; i++) 
	{
		if (i + 2 < num_bytes_in_nal_unit && src[i] == 0 && src[i + 1] == 0 && src[i + 2] == 3) 
		{
			dst[(*copy_size)++] = src[i++];
			dst[(*copy_size)++] = src[i++];
		}
		else 
		{
			dst[(*copy_size)++] = src[i];
		}
	}
}

static uint32_t __inline log2bin(uint32_t value)
{
	uint32_t n = 0;
	while (value)
	{
		value >>= 1;
		n++;
	}
	return n;
}

CBitVector::CBitVector(unsigned char* baseBytePtr, unsigned baseBitOffset, unsigned totNumBits) 
{
	setup(baseBytePtr, baseBitOffset, totNumBits);
}

void CBitVector::setup(unsigned char* baseBytePtr, unsigned baseBitOffset, unsigned totNumBits) 
{
	fBaseBytePtr = baseBytePtr;
	fBaseBitOffset = baseBitOffset;
	fTotNumBits = totNumBits;
	fCurBitIndex = 0;
}

void CBitVector::putBits(unsigned from, unsigned numBits) 
{
	if (numBits == 0) 
		return;

	unsigned char tmpBuf[4];
	unsigned overflowingBits = 0;

	if (numBits > MAX_LENGTH) 
	{
		numBits = MAX_LENGTH;
	}

	if (numBits > fTotNumBits - fCurBitIndex) 
	{
		overflowingBits = numBits - (fTotNumBits - fCurBitIndex);
	}

	tmpBuf[0] = (unsigned char)(from >> 24);
	tmpBuf[1] = (unsigned char)(from >> 16);
	tmpBuf[2] = (unsigned char)(from >> 8);
	tmpBuf[3] = (unsigned char)from;

	shift_bits(fBaseBytePtr, fBaseBitOffset + fCurBitIndex, /* to */tmpBuf, MAX_LENGTH - numBits, /* from */numBits - overflowingBits /* num bits */);
	fCurBitIndex += numBits - overflowingBits;
}

void CBitVector::put1Bit(unsigned bit) 
{
	// The following is equivalent to "putBits(..., 1)", except faster:
	if (fCurBitIndex >= fTotNumBits) 
	{ /* overflow */
		return;
	}
	else 
	{
		unsigned totBitOffset = fBaseBitOffset + fCurBitIndex++;
		unsigned char mask = singleBitMask[totBitOffset % 8];
		if (bit) 
		{
			fBaseBytePtr[totBitOffset / 8] |= mask;
		}
		else 
		{
			fBaseBytePtr[totBitOffset / 8] &= ~mask;
		}
	}
}

unsigned CBitVector::getBits(unsigned numBits) 
{
	if (numBits == 0) 
		return 0;

	unsigned char tmpBuf[4];
	unsigned overflowingBits = 0;

	if (numBits > MAX_LENGTH) 
	{
		numBits = MAX_LENGTH;
	}

	if (numBits > fTotNumBits - fCurBitIndex) 
	{
		overflowingBits = numBits - (fTotNumBits - fCurBitIndex);
	}

	shift_bits(tmpBuf, 0, /* to */fBaseBytePtr, fBaseBitOffset + fCurBitIndex, /* from */numBits - overflowingBits /* num bits */);
	fCurBitIndex += numBits - overflowingBits;

	unsigned result = (tmpBuf[0] << 24) | (tmpBuf[1] << 16) | (tmpBuf[2] << 8) | tmpBuf[3];
	result >>= (MAX_LENGTH - numBits); // move into low-order part of word
	result &= (0xFFFFFFFF << overflowingBits); // so any overflow bits are 0
	return result;
}

unsigned CBitVector::get1Bit() 
{
	// The following is equivalent to "getBits(1)", except faster:
	if (fCurBitIndex >= fTotNumBits) 
	{ /* overflow */
		return 0;
	}
	else 
	{
		unsigned totBitOffset = fBaseBitOffset + fCurBitIndex++;
		unsigned char curFromByte = fBaseBytePtr[totBitOffset / 8];
		unsigned result = (curFromByte >> (7 - (totBitOffset % 8))) & 0x01;
		return result;
	}
}

void CBitVector::skipBits(unsigned numBits) 
{
	if (numBits > fTotNumBits - fCurBitIndex) 
	{ /* overflow */
		fCurBitIndex = fTotNumBits;
	}
	else 
	{
		fCurBitIndex += numBits;
	}
}

unsigned CBitVector::get_expGolomb() 
{
	unsigned numLeadingZeroBits = 0;
	unsigned codeStart = 1;

	while (get1Bit() == 0 && fCurBitIndex < fTotNumBits)
	{
		++numLeadingZeroBits;
		codeStart *= 2;
	}

	return codeStart - 1 + getBits(numLeadingZeroBits);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

dk_rtsp_receiver::dk_rtsp_receiver(void)
	: _is_preview_enabled(false)
	, _is_recording_enabled(false)
	, _frame_count(0)
{
	_buffer = static_cast<uint8_t*>(malloc(1920 * 1080 * 4));
}

dk_rtsp_receiver::~dk_rtsp_receiver(void)
{
	free(_buffer);
}

void dk_rtsp_receiver::start_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HWND handle)
{
	_decoder = new dk_ff_video_decoder();
	_renderer = new dk_ddraw_video_renderer();
	_is_preview_enabled = true;
	_normal_hwnd = handle;
	dk_rtsp_client::play(url, username, password, transport_option, recv_option, 1, true);
}

void dk_rtsp_receiver::stop_preview(void)
{
	dk_rtsp_client::stop();
	if (_decoder)
	{
		_decoder->release_decoder();
		delete _decoder;
		_decoder = nullptr;
	}

	_is_preview_enabled = false;
}

void dk_rtsp_receiver::start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option)
{
	_mpeg2ts_saver = new dk_mpeg2ts_saver();
	_is_recording_enabled = true;
	dk_rtsp_client::play(url, username, password, transport_option, recv_option, 1, true);
}

void dk_rtsp_receiver::stop_recording(void)
{
	dk_rtsp_client::stop();
	_is_recording_enabled = false;
	if (_mpeg2ts_saver)
	{
		delete _mpeg2ts_saver;
		_mpeg2ts_saver = nullptr;
	}
}

/*void dk_rtsp_receiver::on_begin_media_h264(uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, uint8_t * idr, size_t idrsize)
{
	if (_is_preview_enabled)
	{
		do
		{
			if (parse_sps((BYTE*)(sps), spssize, &_decoder_config.iwidth, &_decoder_config.iheight, &_decoder_config.sarw, &_decoder_config.sarh) > 0)
			{
				_decoder_config.owidth = _decoder_config.iwidth;
				_decoder_config.oheight = _decoder_config.iheight;
				_decoder_config.ismt = dk_ff_video_decoder::SUBMEDIA_TYPE_H264;
				_decoder_config.osmt = dk_ff_video_decoder::SUBMEDIA_TYPE_RGB32;

				dk_ff_video_decoder::ERR_CODE decode_err = _decoder->initialize_decoder(&_decoder_config);
				if (decode_err == dk_ff_video_decoder::ERR_CODE_SUCCESS)
				{
					dk_video_entity_t encoded = { 0, };
					dk_video_entity_t decoded = { _buffer, 0, 1920 * 1080 * 4 };
					encoded.data = (uint8_t*)idr;
					encoded.data_size = idrsize;
					decode_err = _decoder->decode(&encoded, &decoded);
					if ((decode_err == dk_ff_video_decoder::ERR_CODE_SUCCESS) && (decoded.data_size > 0))
					{
						dk_render_entity_t render = { 0, };
						render.data = decoded.data;
						render.data_size = decoded.data_size;
						_renderer->render(&render);
					}
				}

			}
		} while (0);

		_renderer_config.normal_hwnd = _normal_hwnd;
		_renderer_config.width = _decoder_config.iwidth;
		_renderer_config.height = _decoder_config.iheight;
		_renderer->initialize_renderer(&_renderer_config);
	}
	else if (_is_recording_enabled)
	{
		//dk_ff_mpeg2ts_muxer::configuration_t config;
		//config.extra_data_size = data_size;
		//memcpy(config.extra_data, data, data_size);
		//config.width = 1280;
		//config.height = 720;
		//config.fps = 30;
		//config.stream_index = 0;
		//config.bitrate = 4000000;
		//_mpeg2ts_muxer->initialize(config);
	}
	//TRACE(_T("on_begin_media : received video data size is %d\n"), data_size);
}*/

void dk_rtsp_receiver::on_begin_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, 
									  uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, 
									  struct timeval presentation_time)
{
	if (_is_preview_enabled)
	{
		do
		{
			if (parse_sps((BYTE*)(sps), spssize, &_decoder_config.iwidth, &_decoder_config.iheight, &_decoder_config.sarw, &_decoder_config.sarh) > 0)
			{
				_decoder_config.owidth = _decoder_config.iwidth;
				_decoder_config.oheight = _decoder_config.iheight;
				_decoder_config.ismt = dk_ff_video_decoder::SUBMEDIA_TYPE_H264;
				_decoder_config.osmt = dk_ff_video_decoder::SUBMEDIA_TYPE_RGB32;

				_renderer_config.normal_hwnd = _normal_hwnd;
				_renderer_config.width = _decoder_config.owidth;
				_renderer_config.height = _decoder_config.oheight;

				dk_ff_video_decoder::ERR_CODE decode_err = _decoder->initialize_decoder(&_decoder_config);
				dk_ddraw_video_renderer::ERR_CODE render_err = _renderer->initialize_renderer(&_renderer_config);

				if (decode_err == dk_ff_video_decoder::ERR_CODE_SUCCESS)
				{
					dk_video_entity_t encoded = { 0, };
					dk_video_entity_t decoded = { _buffer, 0, 1920 * 1080 * 4 };
					encoded.data = (uint8_t*)data;
					encoded.data_size = data_size;
					decode_err = _decoder->decode(&encoded, &decoded);
					if ((decode_err == dk_ff_video_decoder::ERR_CODE_SUCCESS) && (decoded.data_size > 0))
					{
						if (render_err == dk_ddraw_video_renderer::ERR_CODE_SUCCESS)
						{
							dk_render_entity_t render = { 0, };
							render.data = decoded.data;
							render.data_size = decoded.data_size;
							_renderer->render(&render);
						}
					}
				}
			}
		} while (0);
	}
	if (_is_recording_enabled)
	{
		dk_ff_mpeg2ts_muxer::configuration_t config;
		config.vconfig.extradata_size = data_size;
		memcpy(config.vconfig.extradata, data, data_size);
		config.vconfig.width = 1280;
		config.vconfig.height = 720;
		config.vconfig.fps = 30;
		config.vconfig.stream_index = 0;
		config.vconfig.bitrate = 4000000;
		_mpeg2ts_saver->initialize(&config);
	}
	//TRACE(_T("on_begin_media : received video data size is %d\n"), data_size);
}

void dk_rtsp_receiver::on_recv_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, struct timeval presentation_time)
{
	if (mt == dk_rtsp_client::MEDIA_TYPE_VIDEO)
	{
		if (_is_preview_enabled)
		{
			dk_video_entity_t encoded = { 0, };
			dk_video_entity_t decoded = { _buffer, 0, 1920 * 1080 * 4 };
			encoded.data = (uint8_t*)data;
			encoded.data_size = data_size;
			dk_ff_video_decoder::ERR_CODE decode_err = _decoder->decode(&encoded, &decoded);
			if ((decode_err == dk_ff_video_decoder::ERR_CODE_SUCCESS) && (decoded.data_size > 0))
			{
				/*
				if (_frame_count>1000 && _frame_count<1100)
				{
					dk_image_creator bmp_creator(_decoder_config.owidth, _decoder_config.oheight);
					memcpy(bmp_creator.pixel_buffer, _buffer, _decoder_config.owidth*_decoder_config.oheight * 4);
					wchar_t filename[100] = { 0, };
					_snwprintf_s(filename, sizeof(filename), L"%d.bmp", _frame_count);
					bmp_creator.save(filename);
				}
				_frame_count++;
				*/

				dk_render_entity_t render = { 0, };
				render.data = decoded.data;
				render.data_size = decoded.data_size;
				_renderer->render(&render);
			}
		}
		else if (_is_recording_enabled)
		{
			if ((data[3] & 0x1F)==0x05)
				_mpeg2ts_saver->put_video_stream((unsigned char*)data, data_size, 0, true);
			else
				_mpeg2ts_saver->put_video_stream((unsigned char*)data, data_size, 0, false);
		}
		//TRACE(_T("on_recv_media : received video data size is %d\n"), data_size);
	}
	else if (mt == dk_rtsp_client::MEDIA_TYPE_AUDIO)
	{
		//TRACE(_T("on_recv_media : received audio data size is %d\n"), data_size);
	}
}

// Local Functions
void dk_rtsp_receiver::parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height)
{
	unsigned aspect_ratio_info_present_flag = bv.get1Bit();
	DEBUG_PRINT(aspect_ratio_info_present_flag);
	if (aspect_ratio_info_present_flag) {
		unsigned aspect_ratio_idc = bv.getBits(8);
		DEBUG_PRINT(aspect_ratio_idc);
		switch (aspect_ratio_idc)
		{
		case 0: case 1: { *sar_width = 1; *sar_height = 1; break; }
		case 2:         { *sar_width = 12; *sar_height = 11; break; }
		case 3:         { *sar_width = 10; *sar_height = 11; break; }
		case 4:         { *sar_width = 16; *sar_height = 11; break; }
		case 5:         { *sar_width = 40; *sar_height = 33; break; }
		case 6:         { *sar_width = 24; *sar_height = 11; break; }
		case 7:         { *sar_width = 20; *sar_height = 11; break; }
		case 8:         { *sar_width = 32; *sar_height = 11; break; }
		case 9:         { *sar_width = 80; *sar_height = 33; break; }
		case 10:        { *sar_width = 18; *sar_height = 11; break; }
		case 11:        { *sar_width = 15; *sar_height = 11; break; }
		case 12:        { *sar_width = 64; *sar_height = 33; break; }
		case 13:        { *sar_width = 160; *sar_height = 99; break; }
		case 255:       { *sar_width = bv.getBits(16); *sar_height = bv.getBits(16); break; }
		}
	}
	unsigned overscan_info_present_flag = bv.get1Bit();
	DEBUG_PRINT(overscan_info_present_flag);
	if (overscan_info_present_flag) {
		bv.skipBits(1); // overscan_appropriate_flag
	}
	unsigned video_signal_type_present_flag = bv.get1Bit();
	DEBUG_PRINT(video_signal_type_present_flag);
	if (video_signal_type_present_flag) {
		bv.skipBits(4); // video_format; video_full_range_flag
		unsigned colour_description_present_flag = bv.get1Bit();
		DEBUG_PRINT(colour_description_present_flag);
		if (colour_description_present_flag) {
			bv.skipBits(24); // colour_primaries; transfer_characteristics; matrix_coefficients
		}
	}
	unsigned chroma_loc_info_present_flag = bv.get1Bit();
	DEBUG_PRINT(chroma_loc_info_present_flag);
	if (chroma_loc_info_present_flag) {
		(void)bv.get_expGolomb(); // chroma_sample_loc_type_top_field
		(void)bv.get_expGolomb(); // chroma_sample_loc_type_bottom_field
	}
	unsigned timing_info_present_flag = bv.get1Bit();
	DEBUG_PRINT(timing_info_present_flag);
	if (timing_info_present_flag) {
		num_units_in_tick = bv.getBits(32);
		DEBUG_PRINT(num_units_in_tick);
		time_scale = bv.getBits(32);
		DEBUG_PRINT(time_scale);
		fixed_frame_rate_flag = bv.get1Bit();
		DEBUG_PRINT(fixed_frame_rate_flag);
	}
}

int dk_rtsp_receiver::parse_pps(uint8_t * pps, int pps_size)
{
	if (pps_size <= 0 || pps == NULL)
		return 0;

	CBitVector bv(pps, 0, 8 * pps_size);

	bv.skipBits(8); // nal_unit_type

	unsigned pps_id = bv.get_expGolomb();
	if (pps_id > 255) goto PARSE_ERROR; // Invalid pps_id

	unsigned sps_id = bv.get_expGolomb();
	if (sps_id > 31) goto PARSE_ERROR; // Invalid sps_id

	return 1;
PARSE_ERROR:
	return 0;
}

int dk_rtsp_receiver::parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height)
{
	uint8_t* sps;
	uint32_t sps_size;
	int separate_colour_plane_flag;
	unsigned log2_max_frame_num;
	int frame_mbs_only_flag;

	if (sizeOfSPS <= 0 || data == NULL)
		return 0;

	sps = (uint8_t*)malloc(sizeOfSPS);

	remove_emulation_bytes(sps, data + 4, sizeOfSPS, sizeOfSPS - 4, &sps_size);
	//remove_emulation_bytes(sps, data, sizeOfSPS, sizeOfSPS, &sps_size);

	CBitVector bv(sps, 0, 8 * sps_size);

	bv.skipBits(8); // forbidden_zero_bit; nal_ref_idc; nal_unit_type

	unsigned profile_idc = bv.getBits(8);
	DEBUG_PRINT(profile_idc);

	if ((profile_idc != BASELINE) &&
		(profile_idc != MAIN) &&
		(profile_idc != EXTENDED) &&
		(profile_idc != HIGH) &&
		(profile_idc != FREXT_Hi10P) &&
		(profile_idc != FREXT_Hi422) &&
		(profile_idc != FREXT_Hi444) &&
		(profile_idc != FREXT_CAVLC444))
		goto PARSE_ERROR; // Invalid profile_idc
	unsigned constraint_setN_flag = bv.getBits(8); // also "reserved_zero_2bits" at end
	DEBUG_PRINT(constraint_setN_flag);
	unsigned level_idc = bv.getBits(8);
	DEBUG_PRINT(level_idc);
	unsigned seq_parameter_set_id = bv.get_expGolomb();
	DEBUG_PRINT(seq_parameter_set_id);
	if (seq_parameter_set_id >= 32) goto PARSE_ERROR; // Invalid seq_parameter_set_id
	unsigned chroma_format_idc = 1;

	if (profile_idc >= 100) {
		chroma_format_idc = bv.get_expGolomb();
		DEBUG_PRINT(chroma_format_idc);
		if (chroma_format_idc>3) goto PARSE_ERROR; // Invalid chroma_format_idc
		if (chroma_format_idc == 3) {
			separate_colour_plane_flag = bv.get1Bit();
			DEBUG_PRINT(separate_colour_plane_flag);
		}
		unsigned bit_depth_luma_minus8 = bv.get_expGolomb(); // bit_depth_luma_minus8
		if (bit_depth_luma_minus8>4) goto PARSE_ERROR; //invalid bit_depth_luma_minus8
		unsigned bit_depth_chroma_minus8 = bv.get_expGolomb(); // bit_depth_chroma_minus8
		if (bit_depth_chroma_minus8>4) goto PARSE_ERROR; // invalid bit_depth_chroma_minus8
		bv.skipBits(1); // qpprime_y_zero_transform_bypass_flag
		unsigned seq_scaling_matrix_present_flag = bv.get1Bit();
		DEBUG_PRINT(seq_scaling_matrix_present_flag);
		if (seq_scaling_matrix_present_flag) {
			for (int i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); ++i) {
				DEBUG_PRINT(i);
				unsigned seq_scaling_list_present_flag = bv.get1Bit();
				DEBUG_PRINT(seq_scaling_list_present_flag);
				if (seq_scaling_list_present_flag) {
					unsigned sizeOfScalingList = i < 6 ? 16 : 64;
					unsigned lastScale = 8;
					unsigned nextScale = 8;
					for (unsigned j = 0; j < sizeOfScalingList; ++j) {

						DEBUG_PRINT(j);
						DEBUG_PRINT(nextScale);
						if (nextScale != 0) {
							unsigned delta_scale = bv.get_expGolomb();
							DEBUG_PRINT(delta_scale);
							nextScale = (lastScale + delta_scale + 256) % 256;
						}
						lastScale = (nextScale == 0) ? lastScale : nextScale;
						DEBUG_PRINT(lastScale);
					}
				}
			}
		}
	}

	unsigned log2_max_frame_num_minus4 = bv.get_expGolomb();
	DEBUG_PRINT(log2_max_frame_num_minus4);
	if (log2_max_frame_num_minus4>12) goto PARSE_ERROR; // Invalid log2_max_frame_num_minus4
	log2_max_frame_num = log2_max_frame_num_minus4 + 4;
	unsigned pic_order_cnt_type = bv.get_expGolomb();
	DEBUG_PRINT(pic_order_cnt_type);
	if (pic_order_cnt_type>2) goto PARSE_ERROR; // Invalid pic_order_cnt_type
	if (pic_order_cnt_type == 0) {
		unsigned log2_max_pic_order_cnt_lsb_minus4 = bv.get_expGolomb();
		DEBUG_PRINT(log2_max_pic_order_cnt_lsb_minus4);
	}
	else if (pic_order_cnt_type == 1) {
		bv.skipBits(1); // delta_pic_order_always_zero_flag
		(void)bv.get_expGolomb(); // offset_for_non_ref_pic
		(void)bv.get_expGolomb(); // offset_for_top_to_bottom_field
		unsigned num_ref_frames_in_pic_order_cnt_cycle = bv.get_expGolomb();
		DEBUG_PRINT(num_ref_frames_in_pic_order_cnt_cycle);
		if (num_ref_frames_in_pic_order_cnt_cycle>255) goto PARSE_ERROR; // invalid num_ref_frames_in_pic_order_cnt_cycle
		for (unsigned i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; ++i) {
			(void)bv.get_expGolomb(); // offset_for_ref_frame[i]
		}
	}
	unsigned max_num_ref_frames = bv.get_expGolomb();
	DEBUG_PRINT(max_num_ref_frames);
	if (max_num_ref_frames > 16) goto PARSE_ERROR; // invalid max_num_ref_frames. Too many reference frame
	unsigned gaps_in_frame_num_value_allowed_flag = bv.get1Bit();
	DEBUG_PRINT(gaps_in_frame_num_value_allowed_flag);
	unsigned pic_width_in_mbs_minus1 = bv.get_expGolomb();
	DEBUG_PRINT(pic_width_in_mbs_minus1);
	unsigned pic_height_in_map_units_minus1 = bv.get_expGolomb();
	DEBUG_PRINT(pic_height_in_map_units_minus1);

	frame_mbs_only_flag = bv.get1Bit();
	DEBUG_PRINT(frame_mbs_only_flag);
	if (!frame_mbs_only_flag) {
		bv.skipBits(1); // mb_adaptive_frame_field_flag
	}
	unsigned direct_8x8_inference_flag = bv.get1Bit(); // direct_8x8_inference_flag
	// This stream was generated by a broken encoder, invalid 8x8 inference
	if (!frame_mbs_only_flag && !direct_8x8_inference_flag)
		goto PARSE_ERROR;

	unsigned frame_cropping_flag = bv.get1Bit();
	DEBUG_PRINT(frame_cropping_flag);
	int left, right, top, bottom;
	int CropUnitX = 0, CropUnitY = 0;
	if (frame_cropping_flag) {
		left = bv.get_expGolomb(); // frame_crop_left_offset
		right = bv.get_expGolomb(); // frame_crop_right_offset
		top = bv.get_expGolomb(); // frame_crop_top_offset
		bottom = bv.get_expGolomb(); // frame_crop_bottom_offset
		switch (chroma_format_idc)
		{
		case 0:
			CropUnitX = 1;
			CropUnitY = 2 - frame_mbs_only_flag;
			break;
		case 1:
			CropUnitX = 2;
			CropUnitY = 2 * (2 - frame_mbs_only_flag);
			break;
		case 2:
			CropUnitX = 2;
			CropUnitY = 1 * (2 - frame_mbs_only_flag);
			break;
		case 3:
			CropUnitX = 1;
			CropUnitY = 1 * (2 - frame_mbs_only_flag);
			break;
		}
	}


	// Ignore top offset/left offset
	if (frame_cropping_flag)
	{
		*width = (pic_width_in_mbs_minus1 + 1) * 16 - (CropUnitX*right);
		*height = (pic_height_in_map_units_minus1 + 1) * 16 - (CropUnitY*bottom);
	}
	else
	{
		*width = (pic_width_in_mbs_minus1 + 1) * 16;
		*height = (pic_height_in_map_units_minus1 + 1) * 16;
	}


	if (sar_width != NULL && sar_height != NULL)
	{
		unsigned vui_parameters_present_flag = bv.get1Bit();
		DEBUG_PRINT(vui_parameters_present_flag);
		*sar_width = 1;
		*sar_height = 1;
		if (vui_parameters_present_flag) {
			unsigned num_units_in_tick, time_scale, fixed_frame_rate_flag;
			parse_vui(bv, num_units_in_tick, time_scale, fixed_frame_rate_flag, sar_width, sar_height);
		}
	}

	free(sps);
	return 1;
PARSE_ERROR:
	free(sps);
	return 0;
}

int dk_rtsp_receiver::parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height)
{
	// First, Find VOL Header
	int i;
	for (i = 0; i<size - 8; i++)
	{
		// Find Startcode
		if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x01)
		{
			if (data[i + 3] >= 0x00 && data[i + 3] <= 0x1F) // video_object_start_code
			{
				if (data[i + 4] == 0x00 && data[i + 5] == 0x00 && data[i + 6] == 0x01 && (data[i + 7] >= 0x20 && data[i + 7] <= 0x2F)) // video_object_layer_start_code
				{
					break;
				}
				else
					return 0;
			}
			switch (data[i + 3])
			{
			case 0xB1: // visual_object_sequence_end_code. end here
			case 0xB3: // group_of_vop_start_code. end here
			case 0xB6: // vop_start_code. end here
				return 0;
			default:
				break;
			}
		}
	}
	// FOL HEADER FOUND
	uint32_t vol_ver_id = 1;
	unsigned video_object_layer_verid;
	CBitVector bv(&data[i + 8], 0, 8 * (((size - i - 8)>128) ? 128 : (size - i - 8)));

	unsigned random_accessible_vol = bv.getBits(1);       // random accessible vol
	DEBUG_PRINT(random_accessible_vol);

	unsigned video_object_type_indication = bv.getBits(8); // video object type indication
	DEBUG_PRINT(video_object_type_indication);

	unsigned is_objec_layer_identifier = bv.getBits(1);
	DEBUG_PRINT(is_objec_layer_identifier);
	if (is_objec_layer_identifier)      // is_object_layer_identifier
	{
		video_object_layer_verid = bv.getBits(4);       // video_object_layer_verid
		DEBUG_PRINT(video_object_layer_verid);
		unsigned video_object_layer_priority = bv.getBits(3);     // video_object_layer_priority
		DEBUG_PRINT(video_object_layer_priority);
	}

	unsigned aspect_ratio_info = bv.getBits(4);
	DEBUG_PRINT(aspect_ratio_info);
	switch (aspect_ratio_info)
	{
	case 0x0: case 0x1: { *sar_width = 1;  *sar_height = 1;  break; }
	case 0x2:           { *sar_width = 12; *sar_height = 11; break; }
	case 0x3:           { *sar_width = 10; *sar_height = 11; break; }
	case 0x4:           { *sar_width = 16; *sar_height = 11; break; }
	case 0x5:           { *sar_width = 40; *sar_height = 33; break; }
	case 0xF:           { *sar_width = bv.getBits(8); *sar_height = bv.getBits(8); break; }
	}

	unsigned vol_control_parameters = bv.getBits(1);
	DEBUG_PRINT(vol_control_parameters);
	if (vol_control_parameters)      // vol control parameters
	{
		unsigned chroma_format = bv.getBits(2);     // chroma_format
		DEBUG_PRINT(chroma_format);
		unsigned low_delay = bv.getBits(1);
		DEBUG_PRINT(low_delay);
		unsigned vbv_parameters = bv.getBits(1);
		DEBUG_PRINT(vbv_parameters);
		if (vbv_parameters)
		{
			unsigned first_half_bit_rate = bv.getBits(15);
			DEBUG_PRINT(first_half_bit_rate);
			bv.skipBits(1);
			unsigned latter_half_bit_rate = bv.getBits(15);
			DEBUG_PRINT(latter_half_bit_rate);
			bv.skipBits(1);
			unsigned first_half_vbv_buffer_size = bv.getBits(15);
			DEBUG_PRINT(first_half_vbv_buffer_size);
			bv.skipBits(1);
			unsigned latter_half_vbv_buffer_size = bv.getBits(3);
			DEBUG_PRINT(latter_half_vbv_buffer_size);
			unsigned first_half_vbv_occupancy = bv.getBits(11);
			DEBUG_PRINT(first_half_vbv_occupancy);
			bv.skipBits(1);
			unsigned latter_half_vbv_occupancy = bv.getBits(15);
			DEBUG_PRINT(latter_half_vbv_occupancy);
			bv.skipBits(1);
		}
	}

	unsigned video_object_layer_shape = bv.getBits(2);
	DEBUG_PRINT(video_object_layer_shape);
	if (video_object_layer_shape == 3 && video_object_layer_verid != 1)
	{
		unsigned video_object_layer_shape_extension = bv.getBits(4);
		DEBUG_PRINT(video_object_layer_shape_extension);
	}
	bv.skipBits(1);

	unsigned vop_time_increment_resolution = bv.getBits(16);
	DEBUG_PRINT(vop_time_increment_resolution);
	bv.skipBits(1);

#define MAX(a,b)  ( (a) > (b) ? (a) : (b) )

	unsigned time_increment_bits;
	if (vop_time_increment_resolution>0)
		time_increment_bits = MAX(log2bin(vop_time_increment_resolution - 1), 1);
	else
		time_increment_bits = 1;

	unsigned fixed_vop_rate = bv.getBits(1);
	DEBUG_PRINT(fixed_vop_rate);
	if (fixed_vop_rate)
	{
		unsigned fixed_vop_time_increment = bv.getBits(time_increment_bits);
		DEBUG_PRINT(fixed_vop_time_increment);
	}

	if (video_object_layer_shape != 2)
	{
		if (video_object_layer_shape == 0)
		{
			bv.skipBits(1);
			unsigned video_object_layer_width = bv.getBits(13);
			*width = video_object_layer_width;
			DEBUG_PRINT(video_object_layer_width);
			bv.skipBits(1);
			unsigned video_object_layer_height = bv.getBits(13);
			*height = video_object_layer_height;
			DEBUG_PRINT(video_object_layer_height);
			bv.skipBits(1);
		}
	}

	return 1;
}

int dk_rtsp_receiver::parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height)
{
	int i, found = 0;

	*sar_width = 1;
	*sar_height = 1;
	for (i = 0; i<size - 9; i++)
	{
		if (data[i] == 0xFF && data[i + 1] == 0xC0)
		{
			*height = ((data[i + 5] << 5) | (data[i + 6] >> 3)) * 8;
			*width = ((data[i + 7] << 5) | (data[i + 8] >> 3)) * 8;
			found = 1;
			break;
		}
	}
	return found;
}

void dk_rtsp_receiver::make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig)
{
	CBitVector bv(data, 0, 72);

	bv.putBits(0xFFF, 12);
	bv.putBits(0, 1);
	bv.putBits(0, 2);
	bv.putBits(1, 1);
	bv.putBits(audioObjectType, 2);
	bv.putBits(samplingFreqIndex, 4);
	bv.putBits(0, 1);
	bv.putBits(channelConfig, 3);
	bv.putBits(0, 1);
	bv.putBits(0, 1);
	bv.putBits(0, 1);
	bv.putBits(0, 1);
	bv.putBits(7 + size, 13);
	bv.putBits(0x7FF, 11);
	bv.putBits(0, 2);
}