#ifndef _SOAP_CLIENT_H_
#define _SOAP_CLIENT_H_

#include "soap_api_fault.h"

struct soap;
#if defined(WIN32)
class __declspec(dllexport) soap_api_client
#else
class soap_api_client
#endif
{
public:
	explicit soap_api_client( void );
	virtual ~soap_api_client( void );

	void					set_xaddress( char *xaddr );
	void					set_wsse_token( char *username, char *password );
	void					full_fault_code( struct soap *soap );
	const soap_api_fault*	get_fault_code( void ) const;

	virtual struct soap*	get_soap( void ) const;

protected:
	char			*_xaddr;
	char			*_username;
	char			*_password;
	soap_api_fault _fault;

};
#endif