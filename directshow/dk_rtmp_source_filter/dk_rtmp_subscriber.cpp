#include "dk_rtmp_subscriber.h"
#include <dk_string_helper.h>
#include <dk_media_buffering.h>
#include <dk_fileio.h>

dk_rtmp_subscriber::dk_rtmp_subscriber(void)
	: dk_rtmp_client()
	, _recv_option(dk_rtmp_subscriber::RECV_AUDIO_VIDEO)
	, _recv_timeout(0)
	, _conn_timeout(0)
	, _repeat(true)
	, _width(-1)
	, _height(-1)
	, _sar_width(-1)
	, _sar_height(-1)
{
#if defined(WITH_DEBUG_VIDEO_ES)
	_video_file = ::open_file_write("rtmp_video.h264");
#endif
}

dk_rtmp_subscriber::~dk_rtmp_subscriber(void)
{
#if defined(WITH_DEBUG_VIDEO_ES)
	::close_file(_video_file);
#endif
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::play(void)
{
	dk_rtmp_subscriber::ERR_CODE code = dk_rtmp_subscriber::ERR_CODE_FAIL;
	if (wcslen(_url) < 0)
		return dk_rtmp_subscriber::ERR_CODE_FAIL;

	char * url = 0;
	char * username = 0;
	char * password = 0;
	dk_string_helper::convert_wide2multibyte(_url, &url);
	if (wcslen(_username)>0 && wcslen(_password)>0)
	{
		dk_string_helper::convert_wide2multibyte(_username, &username);
		dk_string_helper::convert_wide2multibyte(_password, &password);
	}
	code = subscribe_begin(url, username, password, int32_t(_recv_option), _repeat);
	if (url)
		free(url);
	if (username)
		free(username);
	if (password)
		free(password);

	return code;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::stop(void)
{
	return subscribe_end();
}

int32_t dk_rtmp_subscriber::get_width(void)
{
	return _width;
}

int32_t dk_rtmp_subscriber::get_height(void)
{
	return _height;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::set_url(wchar_t * url)
{
	wcscpy_s(_url, url);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::set_username(wchar_t * username)
{
	wcscpy_s(_username, username);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::set_password(wchar_t * password)
{
	wcscpy_s(_password, password);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::set_recv_option(uint16_t option)
{
	_recv_option = dk_rtmp_subscriber::RECV_OPTION_T(option);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::set_recv_timeout(uint64_t timeout)
{
	_recv_timeout = timeout;
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::set_connection_timeout(uint64_t timeout)
{
	_conn_timeout = timeout;
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::set_repeat(bool repeat)
{
	_repeat = repeat;
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::get_url(wchar_t ** url)
{
	size_t length = wcsnlen_s(_url, sizeof(_url)/sizeof(wchar_t));
	*url = (wchar_t*)malloc((length + 1)*sizeof(wchar_t));
	wcscpy_s(*url, length, _url);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::get_username(wchar_t ** username)
{
	size_t length = wcsnlen_s(_username, sizeof(_username) / sizeof(wchar_t));
	*username = (wchar_t*)malloc((length + 1)*sizeof(wchar_t));
	wcscpy_s(*username, length, _username);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::get_password(wchar_t ** password)
{
	size_t length = wcsnlen_s(_password, sizeof(_password) / sizeof(wchar_t));
	*password = (wchar_t*)malloc((length + 1)*sizeof(wchar_t));
	wcscpy_s(*password, length, _password);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::get_recv_option(uint16_t & option)
{
	option = uint16_t(_recv_option);
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::get_recv_timeout(uint64_t & timeout)
{
	timeout = _recv_timeout;
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::get_connection_timeout(uint64_t & timeout)
{
	timeout = _conn_timeout;
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}

dk_rtmp_subscriber::ERR_CODE dk_rtmp_subscriber::get_repeat(bool & repeat)
{
	repeat = _repeat;
	return dk_rtmp_subscriber::ERR_CODE_SUCCESS;
}


void dk_rtmp_subscriber::on_begin_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, struct timeval presentation_time)
{
	if (parse_sps((BYTE*)(sps), spssize, &_width, &_height, &_sar_width, &_sar_height) < 1)
	{
		_width = -1;
		_height = -1;
	}
	else
	{
#if defined(WITH_DEBUG_VIDEO_ES)
		DWORD nbytes = 0;
		if (_video_file != INVALID_HANDLE_VALUE)
		{
			uint32_t bytes2write = data_size;
			uint32_t bytes_written = 0;
			do
			{
				uint32_t nb_write = 0;
				write_file(_video_file, (uint8_t*)data, bytes2write, &nb_write, NULL);
				bytes_written += nb_write;
				if (bytes2write == bytes_written)
					break;
			} while (1);
		}
#endif
		dk_media_buffering::instance().push_video((uint8_t*)data, data_size);
	}
}

void dk_rtmp_subscriber::on_recv_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, struct timeval presentation_time)
{
#if defined(WITH_DEBUG_VIDEO_ES)
	DWORD nbytes = 0;
	if (_video_file != INVALID_HANDLE_VALUE)
	{
		uint32_t bytes2write = data_size;
		uint32_t bytes_written = 0;
		do
		{
			uint32_t nb_write = 0;
			write_file(_video_file, (uint8_t*)data, bytes2write, &nb_write, NULL);
			bytes_written += nb_write;
			if (bytes2write == bytes_written)
				break;
		} while (1);
	}
#endif
	dk_media_buffering::instance().push_video((uint8_t*)data, data_size);
}

void dk_rtmp_subscriber::on_begin_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, struct timeval presentation_time)
{

}

void dk_rtmp_subscriber::on_recv_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, struct timeval presentation_time)
{

}

void dk_rtmp_subscriber::parse_vui(dk_bit_vector & bv, unsigned & num_units_in_tick, unsigned & time_scale, unsigned & fixed_frame_rate_flag, int * sar_width, int * sar_height)
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

int dk_rtmp_subscriber::parse_pps(uint8_t * pps, int pps_size)
{
	if (pps_size <= 0 || pps == NULL)
		return 0;

	dk_bit_vector bv(pps, 0, 8 * pps_size);

	bv.skipBits(8); // nal_unit_type

	unsigned pps_id = bv.get_expGolomb();
	if (pps_id > 255) goto PARSE_ERROR; // Invalid pps_id

	unsigned sps_id = bv.get_expGolomb();
	if (sps_id > 31) goto PARSE_ERROR; // Invalid sps_id

	return 1;
PARSE_ERROR:
	return 0;
}

int dk_rtmp_subscriber::parse_sps(uint8_t * data, int sizeOfSPS, int * width, int * height, int * sar_width, int * sar_height)
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

	dk_bit_vector bv(sps, 0, 8 * sps_size);

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

int dk_rtmp_subscriber::parse_mpeg(uint8_t * data, int size, int * width, int * height, int * sar_width, int * sar_height)
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
	dk_bit_vector bv(&data[i + 8], 0, 8 * (((size - i - 8)>128) ? 128 : (size - i - 8)));

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

int dk_rtmp_subscriber::parse_jpeg(uint8_t * data, int size, int * width, int * height, int * sar_width, int * sar_height)
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