#pragma once

#include <cstdint>

typedef struct _eap_circular_buffer_t
{
	uint8_t * buffer;
	int32_t length;
	int32_t start;
	int32_t end;
} eap_circular_buffer_t;

eap_circular_buffer_t * eap_circular_buffer_create(int32_t capacity);
void	eap_circular_buffer_destroy(eap_circular_buffer_t * buffer);
int32_t eap_circular_buffer_read(eap_circular_buffer_t * buffer, uint8_t * target, int32_t amount);
int32_t eap_circular_buffer_write(eap_circular_buffer_t * buffer, uint8_t * data, int32_t length);
int32_t eap_circular_buffer_empty(eap_circular_buffer_t * buffer);
int32_t eap_circular_buffer_full(eap_circular_buffer_t * buffer);
int32_t eap_circular_buffer_available_data(eap_circular_buffer_t * buffer);
int32_t eap_circular_buffer_available_space(eap_circular_buffer_t * buffer);


#define eap_circular_buffer_available_data(B) (((B)->end + 1) % (B)->length - (B)->start - 1)
#define eap_circular_buffer_available_space(B) ((B)->length - (B)->end - 1)
#define eap_circular_buffer_full(B) (RingBuffer_available_data((B)) - (B)->length == 0)
#define eap_circular_buffer_empty(B) (RingBuffer_available_data((B)) == 0)
#define eap_circular_buffer_puts(B, D) RingBuffer_write((B), bdata((D)), blength((D)))
#define eap_circular_buffer_get_all(B) RingBuffer_gets((B), RingBuffer_available_data((B)))
#define eap_circular_buffer_starts_at(B) ((B)->buffer + (B)->start)
#define eap_circular_buffer_ends_at(B) ((B)->buffer + (B)->end)
#define eap_circular_buffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)
#define eap_circular_buffer_commit_write(B, A) ((B)->end = ((B)->end + (A)) % (B)->length)