#ifndef _SOAP_FAULT_H_
#define _SOAP_FAULT_H_

struct soap;
#if defined(WIN32)
class __declspec(dllexport) soap_api_fault
#else
class soap_api_fault
#endif
{
public:
	explicit soap_api_fault( void );
	~soap_api_fault( void );
	void set_code( char *val );
	void set_parent_subcode( char *val );
	void set_subcode( char *val );
	void set_reason( char *val );

	const char* get_code( void ) const;
	const char* get_parent_subcode( void )  const;
	const char* get_subcode( void )  const;
	const char* get_reason( void )  const;

private:
	char *_code;
	char *_parent_subcode;
	char *_subcode;
	char *_reason;
};

#endif