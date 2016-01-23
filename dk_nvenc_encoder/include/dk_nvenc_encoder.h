#ifndef _DK_NVENC_ENCODER_H_
#define _DK_NVENC_ENCODER_H_

#include <dk_video_base.h>

#include <vector>
class nvenc_encoder;
class EXP_CLASS dk_nvenc_encoder : public dk_video_encoder
{
public:

	enum _COLOR_PRIMARIES
	{
		COLOR_PRIMARIES_BT709 = 1,
		COLOR_PRIMARIES_UNSPECIFIED,
		COLOR_PRIMARIES_BT470M = 4,
		COLOR_PRIMARIES_BT470BG,
		COLOR_PRIMARIES_SMPTE170M,
		COLOR_PRIMARIES_SMPTE240M,
		COLOR_PRIMARIES_FILM,
		COLOR_PRIMARIES_BT2020
	} COLOR_PRIMARIES;

	enum _COLOR_TRANSFER
	{
		COLOR_TRANSFER_BT709 = 1,
		COLOR_TRANSFER_UNSPECIFIED,
		COLOR_TRANSFER_BT470M = 4,
		COLOR_TRANSFER_BT470BG,
		COLOR_TRANSFER_SMPTE170M,
		COLOR_TRANSFER_SMPTE240M,
		COLOR_TRANSFER_LINEAR,
		COLOR_TRANSFER_Log100,
		COLOR_TRANSFER_Log316,
		COLOR_TRANSFER_IEC6196624,
		COLOR_TRANSFER_BT1361,
		COLOR_TRANSFER_IEC6196621,
		COLOR_TRANSFER_BT202010,
		COLOR_TRANSFER_BT202012
	} COLOR_TRANSFER;

	enum _COLOR_MATRIX
	{
		COLOR_MATRIX_GBR = 0,
		COLOR_MATRIX_BT709,
		COLOR_MATRIX_UNSPECIFIED,
		COLOR_MATRIX_BT470M = 4,
		COLOR_MATRIX_BT470BG,
		COLOR_MATRIX_SMPTE170M,
		COLOR_MATRIX_SMPTE240M,
		COLOR_MATRIX_YCGCO,
		COLOR_MATRIX_BT2020NCL,
		COLOR_MATRIX_BT2020CL
	} COLOR_MATRIX;

	/*typedef enum _PIC_TYPE
	{
		PIC_TYPE_P = 0x0,     //< Forward predicted
		PIC_TYPE_B = 0x01,    //< Bi-directionally predicted picture
		PIC_TYPE_I = 0x02,    //< Intra predicted picture
		PIC_TYPE_IDR = 0x03,    //< IDR picture
		PIC_TYPE_BI = 0x04,    //< Bi-directionally predicted with only Intra MBs
		PIC_TYPE_SKIPPED = 0x05,    //< Picture is skipped
		PIC_TYPE_INTRA_REFRESH = 0x06,    //< First picture in intra refresh cycle
		PIC_TYPE_UNKNOWN = 0xFF     //< Picture type unknown 
	} PIC_TYPE;*/

	typedef enum _CODEC_PROFILE_TYPE
	{
		CODEC_PROFILE_TYPE_AUTOSELECT,
		CODEC_PROFILE_TYPE_BASELINE,
		CODEC_PROFILE_TYPE_MAIN,
		CODEC_PROFILE_TYPE_HIGH
	} CODEC_PROFILE_TYPE;

	typedef enum _RC_MODE
	{
		RC_MODE_CONSTQP = 0x0,       /**< Constant QP mode */
		RC_MODE_VBR = 0x1,       /**< Variable bitrate mode */
		RC_MODE_CBR = 0x2,       /**< Constant bitrate mode */
		RC_MODE_VBR_MINQP = 0x4,       /**< Variable bitrate mode with MinQP */
		RC_MODE_2_PASS_QUALITY = 0x8,       /**< Multi pass encoding optimized for image quality and works only with low latency mode */
		RC_MODE_2_PASS_FRAMESIZE_CAP = 0x10,      /**< Multi pass encoding optimized for maintaining frame size and works only with low latency mode */
		RC_MODE_2_PASS_VBR = 0x20       /**< Multi pass VBR */
	} RC_MODE;

	typedef enum _PRESET_TYPE
	{
		PRESET_TYPE_DEFAULT,
		PRESET_TYPE_HP,
		PRESET_TYPE_HQ,
		PRESET_TYPE_BD,
		PRESET_TYPE_LOW_LATENCY_DEFAULT,
		PRESET_TYPE_LOW_LATENCY_HQ,
		PRESET_TYPE_LOW_LATENCY_HP,
		PRESET_TYPE_LOSSLESS_DEFAULT,
		PRESET_TYPE_LOSSLESS_HP
	} PRESET_TYPE;

	typedef enum _PIC_STRUCT
	{
		PIC_STRUCT_FRAME = 0x01,                 /**< Progressive frame */
		PIC_STRUCT_FIELD_TOP_BOTTOM = 0x02,                 /**< Field encoding top field first */
		PIC_STRUCT_FIELD_BOTTOM_TOP = 0x03                  /**< Field encoding bottom field first */
	} PIC_STRUCT;

	typedef struct EXP_CLASS _configuration_t
	{
		int cs;
		int width;
		int height;
		int max_width;
		int max_height;
		int bitrate;
		int rc_mode;
		int keyframe_interval;
		int profile;
		int fps;
		//int qp;
		int codec;
		int preset;
		int vbv_max_bitrate;
		int vbv_size;
		int invalidate_ref_frames_enable_flag;
		int intra_refresh_enable_flag;
		int intra_refresh_period;
		int intra_refresh_duration;
		int numb;
		int bitstream_buffer_size;
		_configuration_t(void)
			: cs(dk_nvenc_encoder::SUBMEDIA_TYPE_YV12)
			, width(1280)
			, height(1024)
			, max_width(1920)
			, max_height(1080)
			, bitrate(4000000)
			, fps(60)
			, vbv_max_bitrate(4000000)
			, vbv_size(4000000)
			, rc_mode(RC_MODE_CBR)
			, keyframe_interval(2)
			, profile(dk_nvenc_encoder::CODEC_PROFILE_TYPE_HIGH)
			//, qp(28)
			, codec(dk_nvenc_encoder::SUBMEDIA_TYPE_H264)
			, preset(dk_nvenc_encoder::PRESET_TYPE_HQ)
			, invalidate_ref_frames_enable_flag(0)
			, intra_refresh_enable_flag(0)
			, intra_refresh_period(0)
			, intra_refresh_duration(0)
			, numb(0)
			, bitstream_buffer_size(0)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
			cs = clone.cs;
			width = clone.width;
			height = clone.height;
			max_width = clone.max_width;
			max_height = clone.max_height;
			bitrate = clone.bitrate;
			rc_mode = clone.rc_mode;
			keyframe_interval = clone.keyframe_interval;
			profile = clone.profile;
			fps = clone.fps;
			//qp = clone.qp;
			preset = clone.preset;
			vbv_max_bitrate = clone.vbv_max_bitrate;
			vbv_size = clone.vbv_size;
			invalidate_ref_frames_enable_flag = clone.invalidate_ref_frames_enable_flag;
			intra_refresh_enable_flag = clone.intra_refresh_enable_flag;
			intra_refresh_period = clone.intra_refresh_period;
			intra_refresh_duration = clone.intra_refresh_duration;
			numb = clone.numb;
			bitstream_buffer_size = clone.bitstream_buffer_size;
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			cs = clone.cs;
			width = clone.width;
			height = clone.height;
			max_width = clone.max_width;
			max_height = clone.max_height;
			bitrate = clone.bitrate;
			rc_mode = clone.rc_mode;
			keyframe_interval = clone.keyframe_interval;
			profile = clone.profile;
			fps = clone.fps;
			//qp = clone.qp;
			preset = clone.preset;
			vbv_max_bitrate = clone.vbv_max_bitrate;
			vbv_size = clone.vbv_size;
			invalidate_ref_frames_enable_flag = clone.invalidate_ref_frames_enable_flag;
			intra_refresh_enable_flag = clone.intra_refresh_enable_flag;
			intra_refresh_period = clone.intra_refresh_period;
			intra_refresh_duration = clone.intra_refresh_duration;
			numb = clone.numb;
			bitstream_buffer_size = clone.bitstream_buffer_size;
			return (*this);
		}
	} configuration_t;

	dk_nvenc_encoder(void);
	~dk_nvenc_encoder(void);

	dk_nvenc_encoder::ERR_CODE initialize_encoder(void * config);
	dk_nvenc_encoder::ERR_CODE release_encoder(void);

	dk_nvenc_encoder::ERR_CODE encode(dk_nvenc_encoder::dk_video_entity_t * rawstream, dk_nvenc_encoder::dk_video_entity_t * bitstream);
	dk_nvenc_encoder::ERR_CODE encode(dk_nvenc_encoder::dk_video_entity_t * rawstream);
	dk_nvenc_encoder::ERR_CODE get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream);

public:
	//cuda check
	dk_nvenc_encoder::ERR_CODE initialize(dk_nvenc_encoder::configuration_t config, unsigned int * pitch);
	dk_nvenc_encoder::ERR_CODE release(void);
	dk_nvenc_encoder::ERR_CODE encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, PIC_TYPE & pic_type, bool flush = false);

private:
	nvenc_encoder * _core;
};













#endif 