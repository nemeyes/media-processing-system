#include <assert.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "dk_circular_buffer.h"

dk_circular_buffer_t * dk_circular_buffer_create(int32_t length)
{
	dk_circular_buffer_t * buffer = static_cast<dk_circular_buffer_t*>(calloc(1, sizeof(dk_circular_buffer_t)));
	buffer->length = length + 1;
	buffer->start = 0;
	buffer->end = 0;
	buffer->buffer = static_cast<uint8_t*>(calloc(buffer->length, 1));
	memset(buffer->buffer, 0x00, buffer->length);
	return buffer;
}

void dk_circular_buffer_destroy(dk_circular_buffer_t * buffer)
{
	if (buffer) 
	{
		free(buffer->buffer);
		free(buffer);
	}
}

int32_t dk_circular_buffer_write(dk_circular_buffer_t * buffer, uint8_t * data, int32_t length)
{
	if (dk_circular_buffer_available_data(buffer) == 0)
	{
		buffer->start = buffer->end = 0;
	}

	//check(length <= dk_circular_buffer_available_space(buffer), "Not enough space: %d request, %d available", dk_circular_buffer_available_data(buffer), length);
	if (length > dk_circular_buffer_available_space(buffer))
		return -1;

	void * result = memcpy(dk_circular_buffer_ends_at(buffer), data, length);
	if (!result) //check(result != NULL, "Failed to write data into buffer.");
		return -1;
	
	dk_circular_buffer_commit_write(buffer, length);

	return length;
//error:
//	return -1;
}

int32_t dk_circular_buffer_read(dk_circular_buffer_t * buffer, uint8_t * target, int32_t amount)
{
	//check_debug(amount <= dk_circular_buffer_available_data(buffer), "Not enough in the buffer: has %d, needs %d", dk_circular_buffer_available_data(buffer), amount);
	if (amount > dk_circular_buffer_available_data(buffer))
		return -1;

	void *result = memcpy(target, dk_circular_buffer_starts_at(buffer), amount);
	if (!result) //check(result != NULL, "Failed to write buffer into data.");
		return -1;

	dk_circular_buffer_commit_read(buffer, amount);

	if (buffer->end == buffer->start) 
	{
		buffer->start = buffer->end = 0;
	}

	return amount;
//error:
//	return -1;
}