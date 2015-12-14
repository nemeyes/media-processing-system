#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3d11.h>
#include <amf/components/VideoEncoderVCE.h>
#include "AMFPlatform.h"
#include "PlatformWindows.h"
#include "Thread.h"

#include "dk_vce_encoder.h"

#define MAX_ENCODE_QUEUE 32
class vce_encoder;
class vce_polling_thread : public AMFThread
{
public:
	vce_polling_thread(vce_encoder * core);
	~vce_polling_thread(void);
	virtual void Run(void);
protected:
	vce_encoder * _core;
};

class vce_encoder
{
	friend class vce_polling_thread;
public:
	template<class T>
	class am_enc_queue
	{
		T ** buffer;
		unsigned int size;
		unsigned int pending_count;
		unsigned int available_idx;
		unsigned int pending_idx;
	public:
		am_enc_queue(void)
			: buffer(NULL)
			, size(0)
			, pending_count(0)
			, available_idx(0)
			, pending_idx(0)
		{
		}

		~am_enc_queue(void)
		{
			delete[] buffer;
		}

		bool initialize(T * items, unsigned int size)
		{
			this->size = size;
			pending_count = 0;
			available_idx = 0;
			pending_idx = 0;
			buffer = new T *[size];
			for (unsigned int i = 0; i < size; i++)
			{
				buffer[i] = &items[i];
			}
			return true;
		}

		T * get_available(void)
		{
			if (pending_count == size)
				return NULL;
			T * item = buffer[available_idx];
			available_idx = (available_idx + 1) % size;
			pending_count += 1;
			return item;
		}

		T * get_pending(void)
		{
			if (pending_count == 0)
				return NULL;

			T * item = buffer[pending_idx];
			pending_idx = (pending_idx + 1) % size;
			pending_count -= 1;
			return item;
		}
	};

	typedef struct _am_enc_buffer_t
	{
		unsigned int bitstream_buffer_size;
		unsigned char * bitstream_buffer;
	} am_enc_buffer_t;

	vce_encoder(dk_vce_encoder * front);
	~vce_encoder(void);

	dk_vce_encoder::ERR_CODE initialize(dk_vce_encoder::configuration_t * config);
	dk_vce_encoder::ERR_CODE release(void);

	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream, dk_vce_encoder::dk_video_entity_t * bitstream);
	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream);
	dk_vce_encoder::ERR_CODE get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream);

	//dk_vce_encoder::ERR_CODE encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, dk_vce_encoder::PIC_TYPE & pic_type, bool flush = false);

private:
	void fill_surface(amf::AMFContext * context, amf::AMFSurface * surface, amf_int32 i);

	dk_vce_encoder::ERR_CODE allocate_io_buffers(void);
	dk_vce_encoder::ERR_CODE release_io_buffers(void);
private:
	dk_vce_encoder::configuration_t * _config;
	dk_vce_encoder * _front;

	//amf::AMF_MEMORY_TYPE _mem_type;
    amf::AMFContextPtr _context;
    amf::AMFComponentPtr _encoder;
    amf::AMFSurfacePtr _surface;
	amf_int32 _submitted;
	amf::AMF_SURFACE_FORMAT _cs;
	int _bitstream_buffer_size;

	vce_polling_thread * _polling_thread;
	//??
	amf_int32 _rect_size;
	amf_int32 _x_pos;
	amf_int32 _y_pos;
	amf_int32 _frame_count;


	vce_encoder::am_enc_buffer_t _enc_buffer[MAX_ENCODE_QUEUE];
	int _enc_buffer_count;
	vce_encoder::am_enc_queue<vce_encoder::am_enc_buffer_t> _enc_buffer_queue;
};
