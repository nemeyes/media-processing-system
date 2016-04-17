#ifndef _DK_PARALLEL_RECORDER_CONTROLLER_H_
#define _DK_PARALLEL_RECORDER_CONTROLLER_H_

#include <dk_ipc_client.h>

typedef struct _parallel_recorder_t parallel_recorder_t;
class dk_parallel_recorder_controller : public ic::dk_ipc_client
{
public:
	dk_parallel_recorder_controller(parallel_recorder_t * parallel_recorder);
	virtual ~dk_parallel_recorder_controller(void);

private:
	void assoc_completion_callback(void);
	void leave_completion_callback(void);

private:
	parallel_recorder_t * _parallel_recorder;
};













#endif