#include "onvif_client.h"
#include "gsoap/ptzH.h"

onvif_client::onvif_client( void )
{

}

onvif_client::~onvif_client( void )
{

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