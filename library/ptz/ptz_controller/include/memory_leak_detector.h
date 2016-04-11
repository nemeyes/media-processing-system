#ifndef _MEMORY_LEAK_DETECTOR_H_
#define _MEMORY_LEAK_DETECTOR_H_
#if defined(WIN32)
#if defined(_DEBUG)
#include <crtdbg.h> 
static _CRT_REPORT_HOOK prev_hook; 
static bool				swallow_report; 
class memory_leak_detector
{ 
public: 
        memory_leak_detector(); 
        virtual ~memory_leak_detector(); 
}; 

int reporting_hook( int report_type, char *user_message, int *ret_val ); 
#endif
#endif
#endif