#include <assert.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "eap_circular_buffer.h"

	
eap_circular_buffer_t * eap_circular_buffer_create(int32_t length)
{
	eap_circular_buffer_t * buffer = static_cast<eap_circular_buffer_t*>(calloc(1, sizeof(eap_circular_buffer_t)));
	buffer->length = length + 1;
	buffer->start = 0;
	buffer->end = 0;
	buffer->buffer = static_cast<uint8_t*>(calloc(buffer->length, 1));
	memset(buffer->buffer, 0x00, buffer->length);
	return buffer;
}

void eap_circular_buffer_destroy(eap_circular_buffer_t * buffer)
{
	if (buffer) 
	{
		free(buffer->buffer);
		free(buffer);
	}
}

int32_t eap_circular_buffer_write(eap_circular_buffer_t * buffer, uint8_t * data, int32_t length)
{
	if (eap_circular_buffer_available_data(buffer) == 0)
	{
		buffer->start = buffer->end = 0;
	}

	//check(length <= eap_circular_buffer_available_space(buffer), "Not enough space: %d request, %d available", eap_circular_buffer_available_data(buffer), length);
	if (length > eap_circular_buffer_available_space(buffer))
		return -1;

	void * result = memcpy(eap_circular_buffer_ends_at(buffer), data, length);
	if (!result) //check(result != NULL, "Failed to write data into buffer.");
		return -1;
	
	eap_circular_buffer_commit_write(buffer, length);

	return length;
//error:
//	return -1;
}

int32_t eap_circular_buffer_read(eap_circular_buffer_t * buffer, uint8_t * target, int32_t amount)
{
	//check_debug(amount <= eap_circular_buffer_available_data(buffer), "Not enough in the buffer: has %d, needs %d", eap_circular_buffer_available_data(buffer), amount);
	if (amount > eap_circular_buffer_available_data(buffer))
		return -1;

	void *result = memcpy(target, eap_circular_buffer_starts_at(buffer), amount);
	if (!result) //check(result != NULL, "Failed to write buffer into data.");
		return -1;

	eap_circular_buffer_commit_read(buffer, amount);

	if (buffer->end == buffer->start) 
	{
		buffer->start = buffer->end = 0;
	}

	return amount;
//error:
//	return -1;
}