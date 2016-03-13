#include "define.h"
#include "shared_memory_queue_manager.h"

void shared_memory_queue_manager::init(SHARED_VIDEO_QUEUE_T *q)
{
	q->front = -1;
	q->rear = -1;
	for (int i = 0; i<QSIZE; i++)
	{
		memset(q->queue[i].frame, 0x00, FRAMESIZE);
		q->queue[i].size = 0;
	}
}

int shared_memory_queue_manager::full(SHARED_VIDEO_QUEUE_T *q)
{
	if ((q->front == q->rear + 1) || (q->front == 0 && q->rear == QSIZE - 1))
		return 1;
	return 0;
}

int shared_memory_queue_manager::empty(SHARED_VIDEO_QUEUE_T *q)
{
	if (q->front == -1)
		return 1;
	return 0;
}

int shared_memory_queue_manager::push(SHARED_VIDEO_QUEUE_T *q, unsigned char* frame, int size)
{
	if (!full(q))
	{
		if (q->front == -1)
			q->front = 0;
		q->rear = (q->rear + 1) % QSIZE;
		q->queue[q->rear].size = size;
		memset(q->queue[q->rear].frame, 0x00, FRAMESIZE);
		memcpy(q->queue[q->rear].frame, frame, size);
		return 1;
	}
	else
		return 0;
}

int shared_memory_queue_manager::pop(SHARED_VIDEO_QUEUE_T *q, unsigned char** frame, int* size)
{
	if (empty(q))
	{
		(*frame) = 0;
		(*size) = 0;
		return 0;
	}
	else
	{
		(*frame) = q->queue[q->front].frame;
		(*size) = q->queue[q->front].size;
		if (q->front == q->rear)
		{
			q->front = -1;
			q->rear = -1;
		}
		else
		{
			q->front = (q->front + 1) % QSIZE;
		}
		return 1;
	}
}