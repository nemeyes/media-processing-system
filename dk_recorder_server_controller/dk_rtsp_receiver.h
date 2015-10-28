#pragma once
#include <windows.h>
#include <cstdint>
#include <dk_rtsp_client.h>
#include <dk_ff_video_decoder.h>
#include <dk_ff_mpeg2ts_muxer.h>

#define DEBUG_PRINT(x) do {x = x;} while (0)

typedef enum
{
	BASELINE = 66,
	MAIN = 77,
	EXTENDED = 88,
	HIGH = 100,
	FREXT_Hi10P = 110,
	FREXT_Hi422 = 122,
	FREXT_Hi444 = 244,
	FREXT_CAVLC444 = 44,
} H264_PROFILE;

class CBitVector 
{
public:
	CBitVector(unsigned char* baseBytePtr,
		unsigned baseBitOffset,
		unsigned totNumBits);

	void setup(unsigned char* baseBytePtr,
		unsigned baseBitOffset,
		unsigned totNumBits);

	void putBits(unsigned from, unsigned numBits); // "numBits" <= 32
	void put1Bit(unsigned bit);

	unsigned getBits(unsigned numBits); // "numBits" <= 32
	unsigned get1Bit();

	void skipBits(unsigned numBits);

	unsigned curBitIndex() const { return fCurBitIndex; }
	unsigned totNumBits() const { return fTotNumBits; }
	unsigned numBitsRemaining() const { return fTotNumBits - fCurBitIndex; }

	unsigned get_expGolomb();
	// Returns the value of the next bits, assuming that they were encoded using an exponential-Golomb code of order 0

private:
	unsigned char* fBaseBytePtr;
	unsigned fBaseBitOffset;
	unsigned fTotNumBits;
	unsigned fCurBitIndex;
};


class dk_rtsp_receiver : public dk_rtsp_client
{
public:
	dk_rtsp_receiver(void);
	~dk_rtsp_receiver(void);

	void start_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HANDLE handle);
	void stop_preview(void);

	void start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option);
	void stop_recording(void);

	void on_begin_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time);
	void on_recv_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time);

private:
	// A general bit copy operation:
	void parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height);
	int parse_pps(uint8_t* data, int sizeOfSPS);
	int parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height);
	int parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	int parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	void make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig);

	/*void shift_bits(unsigned char* toBasePtr, unsigned toBitOffset, unsigned char const* fromBasePtr, unsigned fromBitOffset, unsigned numBits);
	uint32_t __inline Log2Bin(uint32_t value);
	void remove_emulation_bytes(uint8_t *dst, uint8_t *src, uint32_t max_size, uint32_t num_bytes_in_nal_unit, uint32_t *copy_size);*/

private:
	bool _is_preview_enabled;
	bool _is_recording_enabled;

	dk_ff_video_decoder * _decoder;
	dk_ff_video_decoder::configuration_t _config;	

	dk_ff_mpeg2ts_muxer * _mpeg2ts_muxer;

};

