#ifndef _DK_CIRCULAR_BUFFER_H_
#define _DK_CIRCULAR_BUFFER_H_

#include <cstdint>
#include <cstring>

namespace debuggerking
{
	typedef struct _circular_buffer_t circular_buffer_t;
	typedef struct _circular_buffer_t
	{
	public:
		static circular_buffer_t * create(int32_t capacity);
		static void		destroy(circular_buffer_t * buffer);
		static int32_t	read(circular_buffer_t * buffer, uint8_t * target, int32_t amount);
		static int32_t	write(circular_buffer_t * buffer, const uint8_t * data, int32_t length);
		static int32_t	empty(circular_buffer_t * buffer);
		static int32_t	full(circular_buffer_t * buffer);
		static int32_t	available_data(circular_buffer_t * buffer);
		static int32_t	available_space(circular_buffer_t * buffer);

	public:
		uint8_t * buffer;
		int32_t length;
		int32_t start;
		int32_t end;

	} circular_buffer_t;
}

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif