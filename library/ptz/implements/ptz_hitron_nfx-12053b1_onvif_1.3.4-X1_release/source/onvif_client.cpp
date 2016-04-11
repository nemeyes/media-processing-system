#include "onvif_client.h"
#include "gsoap/ptzH.h"

onvif_client::onvif_client( void )
{
	_xaddr = 0;
	_username = 0;
	_password = 0;
}

onvif_client::~onvif_client( void )
{
	if( _xaddr!=0 )
	{
		free( _xaddr );
		_xaddr = 0;
	}
	if( _username!=0 )
	{
		free( _username );
		_username = 0;
	}
	if( _password!=0 )
	{
		free( _password );
		_password = 0;
	}
}

void onvif_client::set_xaddress( char *xaddr )
{
	if( _xaddr!=0 )
	{
		free( _xaddr );
		_xaddr = 0;
	}
	_xaddr = _strdup( xaddr );
}

void onvif_client::set_wsse_token( char *username, char *password )
{
	if( _username!=0 ) 
	{
		free( _username );
		_username = 0;
	}
	if( _password!=0 )
	{
		free( _password );
		_password = 0;
	}
	_username = _strdup( username );
	_password = _strdup( password );
}


void onvif_client::fill_fault_code( struct soap *soap )
{
	if( soap==0 ) 
		return;
	if( soap->fault==0 ) 
		return;

	if( soap->fault->SOAP_ENV__Code!=0 && soap->fault->SOAP_ENV__Code->SOAP_ENV__Value!=0) 
		_fault.set_code( soap->fault->SOAP_ENV__Code->SOAP_ENV__Value );
	else
		_fault.set_code( 0 );

	if( soap->fault->SOAP_ENV__Code!=0 && 
		soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode!=0 && 
		soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value!=0 )
		_fault.set_parent_subcode( soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value );
	else
		_fault.set_parent_subcode( 0 );

	if( soap->fault->SOAP_ENV__Reason!=0 && soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text!=0 )
		_fault.set_subcode( soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text );
	else
		_fault.set_subcode( 0 );

	if( soap->fault->SOAP_ENV__Detail!=0 && soap->fault->SOAP_ENV__Detail->__any!=0 )
		_fault.set_reason( soap->fault->SOAP_ENV__Detail->__any );
	else
		_fault.set_reason( 0 );
}

const onvif_fault* onvif_client::get_fault_code( void ) const
{
	return &_fault;
}

struct soap* onvif_client::get_soap( void ) const
{
	return 0;
}