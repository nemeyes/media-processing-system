#ifndef _M3U8_FILE_H_
#define _M3U8_FILE_H_

#include "dk_mpeg2ts_segmenter.h"

class m3u8_file
{
public:
	m3u8_file(void);
	~m3u8_file(void);

	dk_mpeg2ts_segmenter::ERR_CODE initialize(wchar_t * ofile_name, wchar_t * opath, int32_t duration, int32_t sequence);
	dk_mpeg2ts_segmenter::ERR_CODE release(void);

	dk_mpeg2ts_segmenter::ERR_CODE insert_media(int32_t duration);
	//dk_mpeg2ts_segmenter::ERR_CODE insert_play_list()


private:
	dk_mpeg2ts_segmenter::ERR_CODE insert(char * resource, int32_t duration, char * description);
	bool puts(HANDLE file, char * b);

	

private:
	HANDLE _m3u8_file;
	char _ofile_name[500];
	char _opath[500];
	int32_t _duration;
	int32_t _sequence;
	int32_t _counter;


};









#endif