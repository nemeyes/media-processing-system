
#include "dk_com_initiator.h"
#include <atlbase.h>
#include <atlconv.h>

dk_com_initiator::dk_com_initiator(void)
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
}

dk_com_initiator::~dk_com_initiator(void)
{
	::CoUninitialize();
}

static dk_com_initiator com_initiator;