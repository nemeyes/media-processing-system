#ifndef _ONVIF_CLIENT_H_
#define _ONVIF_CLIENT_H_

#include "onvif_fault.h"

struct soap;
#if defined(WIN32)
class __declspec(dllexport) onvif_client
#else
class onvif_client
#endif
{
public:
	explicit onvif_client( void );
	virtual ~onvif_client( void );

	void	fill_fault_code( struct soap *soap );
	const	onvif_fault* get_fault_code( void ) const;
	virtual struct soap *get_soap( void ) const;

protected:
	onvif_fault _fault;

};
#endif