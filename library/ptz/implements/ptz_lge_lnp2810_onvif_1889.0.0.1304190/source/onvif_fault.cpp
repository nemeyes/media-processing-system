#include "onvif_fault.h"
#include <cstdlib>
#include <string>

onvif_fault::onvif_fault( void )
{
	_code = 0;
	_parent_subcode = 0;
	_subCode = 0;
	_reason = 0;
}

onvif_fault::~onvif_fault( void )
{
	if( _code ) free( _code );
	if( _parent_subcode ) free( _parent_subcode );
	if( _subCode ) free( _subCode );
	if( _reason ) free( _reason );
}

void onvif_fault::set_code( char* val )
{
	if( val==0 )
	{
		if( _code ) free( _code );
	}
	else 
		_code = _strdup( val );
}

void onvif_fault::set_parent_subcode( char* val )
{
	if( val==0 )
	{
		if( _parent_subcode ) free( _parent_subcode );
	}
	else 
		_parent_subcode = _strdup( val );
}


void onvif_fault::set_subcode( char* val )
{
	if( val==0 )
	{
		if( _subCode ) free( _subCode );
	}
	else 
		_subCode = _strdup( val );
}

void onvif_fault::set_reason( char* val )
{
	if( val==0 )
	{
		if( _reason ) free( _reason );
	}
	else 
		_reason = _strdup( val );
}


const char* onvif_fault::get_code( void ) const
{
	return _code;
}


const char* onvif_fault::get_parent_subcode( void )  const
{
	return _parent_subcode;
}

const char* onvif_fault::get_subcode( void )  const
{
	return _subCode;
}

const char* onvif_fault::get_reason( void )  const
{
	return _reason;
}
