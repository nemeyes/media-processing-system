#ifndef _DK_RECORDER_MODULE_H_
#define _DK_RECORDER_MODULE_H_


#include <dk_basic_type.h>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_RECORDER_MODULE_LIB)
#  define EXP_RECORDER_MODULE_DLL __declspec(dllexport)
# else
#  define EXP_RECORDER_MODULE_DLL __declspec(dllimport)
# endif
#else
# define EXP_RECORDER_MODULE_DLL
#endif

namespace debuggerking
{
	class module_core;
	class EXP_RECORDER_MODULE_DLL recorder_module : public foundation
	{
	public:
		typedef struct _nalu_type_t
		{
			static const uint8_t none = uint8_max;
			static const uint8_t sps = 0;
			static const uint8_t pps = 1;
			static const uint8_t idr = 2;
			static const uint8_t vcl = 3;
			static const uint8_t sei = 3;
		} nalu_type_t;

		recorder_module(long long chunk_size_bytes);
		virtual ~recorder_module(void);

		bool seek(const char * single_recorder_file_path, long long timestamp, bool read=true); //reading and writing
		void write(uint8_t * data, size_t data_size, long long timestamp);
		void read(uint8_t * data, size_t & data_size, long long & interval, long long & timestamp);

		static void get_years(const char * content_path, const char * uuid, int years[], int capacity, int & size);
		static void get_months(const char * content_path, const char * uuid, int year, int months[], int capacity, int & size);
		static void get_days(const char * content_path, const char * uuid, int year, int month, int days[], int capacity, int & size);
		static void get_hours(const char * content_path, const char * uuid, int year, int month, int day, int hours[], int capacity, int & size);
		static void get_minutes(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size);
		static void get_seconds(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size);

		const uint8_t * get_vps(size_t & vps_size) const;
		const uint8_t * get_sps(size_t & sps_size) const;
		const uint8_t * get_pps(size_t & pps_size) const;

		static void set_file_position(HANDLE file, uint32_t offset, uint32_t flag);
		static long long get_file_size(HANDLE file);
		static long long get_elapsed_msec_from_epoch(void);
		static void get_time_from_elapsed_msec_from_epoch(long long elapsed_time, char * time_string, int32_t time_string_size);
		static void get_time_from_elapsed_msec_from_epoch(long long elapsed_time, int32_t & year, int32_t & month, int32_t & day, int32_t & hour, int32_t & minute, int32_t & second);


		static bool is_vps(int32_t smt, uint8_t nal_unit_type);
		static bool is_sps(int32_t smt, uint8_t nal_unit_type);
		static bool is_pps(int32_t smt, uint8_t nal_unit_type);
		static bool is_idr(int32_t smt, uint8_t nal_unit_type);
		static bool is_vcl(int32_t smt, uint8_t nal_unit_type);
	private:
		module_core * _core;
	};
};





#endif