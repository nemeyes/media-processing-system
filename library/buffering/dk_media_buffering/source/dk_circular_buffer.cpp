#include <assert.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "dk_circular_buffer.h"

//std::string dk_circular_buffer_gets(dk_circular_buffer_t * buffer, int amount);

#define circular_buffer_available_data(B) (((B)->end + 1) % (B)->length - (B)->start - 1)
#define circular_buffer_available_space(B) ((B)->length - (B)->end - 1)
#define circular_buffer_full(B) (circular_buffer_available_data((B)) - (B)->length == 0)
#define circular_buffer_empty(B) (circular_buffer_available_data((B)) == 0)
//#define dk_circular_buffer_puts(B, D) dk_circular_buffer_write((B), bdata((D)), blength((D)))
//#define dk_circular_buffer_get_all(B) dk_circular_buffer_gets((B), RingBuffer_available_data((B)))
#define circular_buffer_starts_at(B) ((B)->buffer + (B)->start)
#define circular_buffer_ends_at(B) ((B)->buffer + (B)->end)
#define circular_buffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)
#define circular_buffer_commit_write(B, A) ((B)->end = ((B)->end + (A)) % (B)->length)

debuggerking::circular_buffer_t * debuggerking::circular_buffer_t::create(int32_t length)
{
	debuggerking::circular_buffer_t * buffer = static_cast<debuggerking::circular_buffer_t*>(calloc(1, sizeof(debuggerking::circular_buffer_t)));
	buffer->length = length + 1;
	buffer->start = 0;
	buffer->end = 0;
	buffer->buffer = static_cast<uint8_t*>(calloc(buffer->length, 1));
	memset(buffer->buffer, 0x00, buffer->length);
	return buffer;
}

int32_t debuggerking::circular_buffer_t::write(circular_buffer_t * buffer, const uint8_t * data, int32_t length)
{
	if (circular_buffer_available_data(buffer) == 0)
	{
		buffer->start = buffer->end = 0;
	}

	//check(length <= dk_circular_buffer_available_space(buffer), "Not enough space: %d request, %d available", dk_circular_buffer_available_data(buffer), length);
	if (length > circular_buffer_available_space(buffer))
		return -1;

	void * result = memcpy(circular_buffer_ends_at(buffer), data, length);
	if (!result) //check(result != NULL, "Failed to write data into buffer.");
		return -1;

	circular_buffer_commit_write(buffer, length);

	return length;
}

int32_t debuggerking::circular_buffer_t::read(circular_buffer_t * buffer, uint8_t * target, int32_t amount)
{
	//check_debug(amount <= dk_circular_buffer_available_data(buffer), "Not enough in the buffer: has %d, needs %d", dk_circular_buffer_available_data(buffer), amount);
	if (amount > available_data(buffer))
		return -1;

	void *result = memcpy(target, circular_buffer_starts_at(buffer), amount);
	if (!result) //check(result != NULL, "Failed to write buffer into data.");
		return -1;

	circular_buffer_commit_read(buffer, amount);

	if (buffer->end == buffer->start)
	{
		buffer->start = buffer->end = 0;
	}

	return amount;
}

int32_t	debuggerking::circular_buffer_t::empty(circular_buffer_t * buffer)
{
	return circular_buffer_empty(buffer);
}

int32_t	debuggerking::circular_buffer_t::full(circular_buffer_t * buffer)
{
	return circular_buffer_full(buffer);
}

int32_t	debuggerking::circular_buffer_t::available_data(circular_buffer_t * buffer)
{
	return circular_buffer_available_data(buffer);
}

int32_t	debuggerking::circular_buffer_t::available_space(circular_buffer_t * buffer)
{
	return circular_buffer_available_space(buffer);
}

void debuggerking::circular_buffer_t::destroy(circular_buffer_t * buffer)
{
	if (buffer) 
	{
		free(buffer->buffer);
		free(buffer);
	}
}


/*std::string dk_circular_buffer_gets(dk_circular_buffer_t * buffer, int amount) 
{
//	check(amount > 0, "Need more than 0 for gets, you gave: %d ", amount);
//	check_debug(amount <= RingBuffer_available_data(buffer), "Not enough data in the buffer.");
//
//	bstring result = blk2bstr(RingBuffer_starts_at(buffer), amount);
//
//	check(result != NULL, "Failed to create gets results.");
//
//	check(blength(result) == amount, "Wrong result length.");
//
//	RingBuffer_commit_read(buffer, amount);
//	assert(RingBuffer_available_data(buffer) >= 0 && "Error in read commit");
//	return result;
//error:
	return "";
}*/