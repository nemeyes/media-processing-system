#ifndef _ONVIF_FAULT_H_
#define _ONVIF_FAULT_H_

#if defined(WIN32)
class __declspec(dllexport) onvif_fault
#else
class onvif_fault
#endif
{
public:
	explicit onvif_fault( void );
	~onvif_fault( void );
	void		set_code( char* val );
	void		set_parent_subcode( char* val );
	void		set_subcode( char* val );
	void		set_reason( char* val );

	const char* get_code( void ) const;
	const char* get_parent_subcode( void )  const;
	const char* get_subcode( void )  const;
	const char* get_reason( void )  const;

private:
	char	*_code;
	char	*_parent_subcode;
	char	*_subCode;
	char	*_reason;
};
#endif