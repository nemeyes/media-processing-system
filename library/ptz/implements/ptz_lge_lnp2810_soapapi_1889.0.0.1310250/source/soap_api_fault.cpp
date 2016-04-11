#include "platform.h"
#include "soap_api_fault.h"
#include <cstdlib>
#include <string>

soap_api_fault::soap_api_fault( void )
{
	_code = 0;
	_parent_subcode = 0;
	_subcode = 0;
	_reason = 0;
}

soap_api_fault::~soap_api_fault( void )
{
	if( _code ) 
		free( _code );
	if( _parent_subcode ) 
		free( _parent_subcode );
	if( _subcode ) 
		free( _subcode );
	if( _reason ) 
		free( _reason );
}

void soap_api_fault::set_code( char* val )
{
	if( val==0 )
	{
		if( _code ) 
			free( _code );
	}
	else 
		_code = strdup( val );
}

void soap_api_fault::set_parent_subcode( char* val )
{
	if( val==0 )
	{
		if( _parent_subcode ) 
			free( _parent_subcode );
	}
	else 
		_parent_subcode = strdup( val );
}


void soap_api_fault::set_subcode( char* val )
{
	if( val==0 )
	{
		if( _subcode ) 
			free( _subcode );
	}
	else 
		_subcode = strdup( val );
}

void soap_api_fault::set_reason( char* val )
{
	if( val==0 )
	{
		if( _reason ) 
			free( _reason );
	}
	else 
		_reason = strdup( val );
}


const char* soap_api_fault::get_code( void ) const
{
	return _code;
}


const char* soap_api_fault::get_parent_subcode( void )  const
{
	return _parent_subcode;
}

const char* soap_api_fault::get_subcode( void )  const
{
	return _subcode;
}

const char* soap_api_fault::get_reason( void )  const
{
	return _reason;
}
