#ifndef _DK_VIDEO_DECODER_BASE_H_
#define _DK_VIDEO_DECODER_BASE_H_

class dk_video_decoder_base
{
public:
	dk_video_decoder_base(void);
	~dk_video_decoder_base(void);

	virtual int initialize(void) = 0;
	virtual int decode(uint8_t * bitstream, size_t nb) = 0;
	virtual int get_frame(uint8_t * decoded, size_t & ndecoded) = 0;
	virtual int release(void) = 0;



};











#endif