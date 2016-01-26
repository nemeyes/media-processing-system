#pragma once

#if defined(USE_HIGH_BITRATE)
#define FRAMESIZE 2000000
#else
#define FRAMESIZE 1000000
#endif

typedef struct _VIDEO_ENTITY_T
{
	int				seq;
	int				size;
	unsigned char	frame[FRAMESIZE];
} VIDEO_ENTITY_T;

#define QSIZE 120            /* Size of Circular Queue */
typedef struct _SHARED_VIDEO_QUEUE_T
{
	int				fps;
	int				bitrate;
	int				front;
	int				rear; /* Global declarations */
	int				sps_size;
	int				pps_size;
	char			sps[100];
	char			pps[100];
	VIDEO_ENTITY_T	queue[QSIZE];
} SHARED_VIDEO_QUEUE_T;

class shared_memory_queue_manager
{
public:
	static void init(SHARED_VIDEO_QUEUE_T *q);
	static int	full(SHARED_VIDEO_QUEUE_T *q);
	static int	empty(SHARED_VIDEO_QUEUE_T *q);
	static int	push(SHARED_VIDEO_QUEUE_T *q, unsigned char *frame, int size);
	static int	pop(SHARED_VIDEO_QUEUE_T *q, unsigned char **frame, int *size);
};