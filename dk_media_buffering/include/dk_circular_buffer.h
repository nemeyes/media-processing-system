#ifndef _DK_CIRCULAR_BUFFER_H_
#define _DK_CIRCULAR_BUFFER_H_

#include <cstdint>
#include <cstring>

typedef struct _dk_circular_buffer_t
{
	uint8_t * buffer;
	int32_t length;
	int32_t start;
	int32_t end;
} dk_circular_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

dk_circular_buffer_t * dk_circular_buffer_create(int32_t capacity);
void	dk_circular_buffer_destroy(dk_circular_buffer_t * buffer);
int32_t dk_circular_buffer_read(dk_circular_buffer_t * buffer, uint8_t * target, int32_t amount);
int32_t dk_circular_buffer_write(dk_circular_buffer_t * buffer, uint8_t * data, int32_t length);
int32_t dk_circular_buffer_empty(dk_circular_buffer_t * buffer);
int32_t dk_circular_buffer_full(dk_circular_buffer_t * buffer);
int32_t dk_circular_buffer_available_data(dk_circular_buffer_t * buffer);
int32_t dk_circular_buffer_available_space(dk_circular_buffer_t * buffer);
//std::string dk_circular_buffer_gets(dk_circular_buffer_t * buffer, int amount);

#define dk_circular_buffer_available_data(B) (((B)->end + 1) % (B)->length - (B)->start - 1)
#define dk_circular_buffer_available_space(B) ((B)->length - (B)->end - 1)
#define dk_circular_buffer_full(B) (dk_circular_buffer_available_data((B)) - (B)->length == 0)
#define dk_circular_buffer_empty(B) (dk_circular_buffer_available_data((B)) == 0)
//#define dk_circular_buffer_puts(B, D) dk_circular_buffer_write((B), bdata((D)), blength((D)))
//#define dk_circular_buffer_get_all(B) dk_circular_buffer_gets((B), RingBuffer_available_data((B)))
#define dk_circular_buffer_starts_at(B) ((B)->buffer + (B)->start)
#define dk_circular_buffer_ends_at(B) ((B)->buffer + (B)->end)
#define dk_circular_buffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)
#define dk_circular_buffer_commit_write(B, A) ((B)->end = ((B)->end + (A)) % (B)->length)

#ifdef __cplusplus
}
#endif

#endif