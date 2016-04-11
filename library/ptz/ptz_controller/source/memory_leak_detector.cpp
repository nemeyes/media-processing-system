#include "memory_leak_detector.h"
#if defined(WIN32)
#if defined(_DEBUG)
#include <string.h> 
memory_leak_detector::memory_leak_detector( void ) 
{ 
    //don't swallow assert and trace reports 
    swallow_report = false; 
    //change the report function 
    prev_hook = _CrtSetReportHook(reporting_hook); 
} 

//this destructor is called after mfc has died 
memory_leak_detector::~memory_leak_detector() 
{ 
	//make sure that there is memory leak detection at the end of the program 
	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF ); 
	//reset the report function to the old one 
	_CrtSetReportHook(prev_hook); 
} 

static memory_leak_detector MLD;  //this lives as long as this file 

int reporting_hook( int report_type, char *user_message, int *ret_val ) 
{ 
//_CrtDumpMemoryLeaks() outputs "Detected memory leaks!\n" and calls 
//_CrtDumpAllObjectsSince(NULL) which outputs all leaked objects, 
//ending this (possibly long) list with "Object dump complete.\n" 
//In between those two headings I want to swallow the report. 
    if( (strcmp(user_message,"Detected memory leaks!\n")==0) || swallow_report ) 
	{ 
        if( strcmp(user_message,"Object dump complete.\n")==0 ) swallow_report = false; 
        else swallow_report = true; 
        return true;  //swallow it 
    } 
    else return false; //give it back to _CrtDbgReport() 
};
#endif
#endif