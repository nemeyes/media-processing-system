#if defined(WIN32)
#include <Windows.h>
#elif defined(UBUNTU)
#elif defined(ARM)
#endif

#include "platform.h"
#include <ptz_device_info.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include "hanil_ptz_controller.h"
#include "http_client.h"

hanil_ptz_controller::hanil_ptz_controller( void )
{

}

hanil_ptz_controller::~hanil_ptz_controller( void )
{

}

char* hanil_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][VENDOR];
}

char* hanil_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][DEVICE];
}

char* hanil_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][PROTOCOL];
}

char* hanil_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][VERSION];
}

unsigned short hanil_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][VENDOR];
}

unsigned short hanil_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][DEVICE];
}

unsigned short hanil_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][PROTOCOL];
}

unsigned short hanil_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS160_PS100_PELCOD_V_1_00Q][VERSION];
}

unsigned short hanil_ptz_controller::set_host_name( char *host_name )
{
	if( host_name && (strlen(host_name)>0) ) 
	{
		strcpy( _hostname, host_name );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hanil_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hanil_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	hanil_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hanil_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hanil_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hanil_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::goto_home_position(float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::set_home_position(void)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	std::map<int,int> preset_map;
	std::map<int,int>::iterator it;
	
	for(int i=1;i<=20;i++)
	{   
		preset_map[i] = i;
	}

	(*length) = preset_map.size();
	if( (*length)>0 )
	{
		(*aliases) = static_cast<int*>( malloc(sizeof(int**)*(*length)) );
		int index = 0;
		for( it=preset_map.begin(); it!=preset_map.end(); it++, index++ )
		{
			(*aliases)[index] = it->second;
		}
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short	hanil_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::add_preset2( int &alias )
{
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;
	if(VMS_PTZ_FAIL == send_data("ADD_PRESET", alias)) return VMS_PTZ_FAIL;
	else return VMS_PTZ_SUCCESS;
}

unsigned short	hanil_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::remove_preset2( int alias )
{
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	if(VMS_PTZ_FAIL == send_data("REMOVE_PRESET", alias)) return VMS_PTZ_FAIL;
	else return VMS_PTZ_SUCCESS;
}

unsigned short	hanil_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::goto_preset2( int alias )
{
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	if(VMS_PTZ_FAIL == send_data("GOTO_PRESET", alias)) return VMS_PTZ_FAIL;
	else return VMS_PTZ_SUCCESS;
}

unsigned short	hanil_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	unsigned short value = VMS_PTZ_SUCCESS;

	if(pan_sensitive==0 && tilt_sensitive==0&&zoom_sensitive==0) //Á¤Áö
	{
		value = stop_move();
	}
	else if(pan_sensitive!=0 && tilt_sensitive==0)//ÁÂ¿ì
	{
		if(pan_sensitive<0) //left
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFT, pan_sensitive, 0);
		}
		else //right
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHT, pan_sensitive, 0);
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//»óÇÏ
	{
		if(tilt_sensitive>0)
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_UP, tilt_sensitive, 0);
		}
		else
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_DOWN, tilt_sensitive, 0);
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//ÁÂ»ó
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTUP, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//¿ìÇÏ
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTDOWN, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//ÁÂÇÏ
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTDOWN, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//¿ì»ó
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTUP, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(zoom_sensitive>0)//ÁÜÀÎ
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_ZOOMIN, zoom_sensitive, 0);
	}
	else if(zoom_sensitive<0)//ÁÜ¾Æ¿ô
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_ZO0MOUT, zoom_sensitive, 0);
	}
	
	return value;
}

float hanil_ptz_controller::get_max_abs_value( float a, float b )
{
	if(abs(a)>abs(b)) return abs(a);
	else return abs(b);
}

unsigned short hanil_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	float real_spd = get_continuous_sensitive_value(speed);
	int spd = (int)real_spd;

	switch( UINT8(move) )
	{
		case PTZ_CONTINUOUS_MOVE_UP :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_UP", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFT :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_LEFT", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHT :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_RIGHT", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_DOWN :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_DOWN", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTUP :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_LEFTUP", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTUP :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_RIGHTUP", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTDOWN :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_LEFTDOWN", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTDOWN :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_RIGHTDOWN", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZOOMIN :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_ZOOMIN", spd);
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZO0MOUT :
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_ZO0MOUT", spd);
			break;
		}
	}

	if(value == VMS_PTZ_UNSUPPORTED_COMMAND) return value;
	
	return value;
}

float hanil_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	sensitive = abs(sensitive);
	query_limits();

	if( _speed_min==_min_cspeed && _speed_max==_max_cspeed )
		return sensitive;
	else if( sensitive<_speed_min ) 
		return _min_cspeed;
	else if( sensitive>_speed_max ) 
		return _max_cspeed;
	else
	{
		float real_sensitive = float(sensitive*(_max_cspeed-_min_cspeed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

unsigned short	hanil_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	return value;
}

unsigned short	hanil_ptz_controller::stop_move( void )
{
	if(VMS_PTZ_FAIL == send_data("STOP")) return VMS_PTZ_FAIL;
	else return VMS_PTZ_SUCCESS;
}

unsigned short	hanil_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::query_limits( void )
{
	_min_pan = 0.0f;
	_max_pan = 0.0f;
	_min_tilt = 0.0f;
	_max_tilt = 0.0f;
	_min_zoom = 0.0f;;
	_max_zoom = 0.0f;
	_min_speed = 0.0f;
	_max_speed = 0.0f;
	_min_cspeed = 0.0f;
	_max_cspeed = 53.0f;//63.0f;
	return VMS_PTZ_SUCCESS;
}

unsigned short	hanil_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

// Grouping
unsigned short	hanil_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short  hanil_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hanil_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

float hanil_ptz_controller::get_rpan_sensitive_value( float sensitive )
{	
	query_limits();

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return sensitive;

	if( abs(sensitive)>abs(_pan_max-_pan_min) ) 
	{
		if( sensitive<0 )
			return _min_pan;
		else
			return _max_pan;
	}
	else
	{
		//float real_sensitive = float(sensitive*(_max_pan-_min_pan))/float(_pan_max-_pan_min);	
		float real_sensitive = ((sensitive-_pan_min)*(_max_pan-_min_pan)/(_pan_max-_pan_min)+_min_pan);
		if(real_sensitive>0) real_sensitive = real_sensitive+(_max_pan+_min_pan)/2;
		else real_sensitive = abs(real_sensitive);
		return real_sensitive;
	}				
}

float hanil_ptz_controller::get_rtilt_sensitive_value( float sensitive )
{
	query_limits();

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return sensitive;
	
    if( abs(sensitive)>abs(_tilt_max-_tilt_min) ) 
	{
		if( sensitive<0 )
			return _min_tilt;
		else
			return _max_tilt;
	}
	else
	{
		float real_sensitive = (sensitive*(_max_tilt-_min_tilt))/(_tilt_max-_tilt_min);
		return real_sensitive;
	}
}

float hanil_ptz_controller::get_rzoom_sensitive_value( float sensitive )
{
	query_limits();

	if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
		return sensitive;
	
	if( abs(sensitive)>abs(_zoom_max-_zoom_min) ) 
	{
		if( sensitive<0 )
			return _min_zoom;
		else
			return _max_zoom;
	}
	else
	{
		float real_sensitive = ((sensitive)*(_max_zoom-_min_zoom))/(_zoom_max-_zoom_min)/2;
		return real_sensitive;
	}
}

float hanil_ptz_controller::get_apan_sensitive_value( float sensitive )
{
	query_limits();	

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return sensitive;

	if( sensitive<_pan_min ) 
		return float(_min_pan);
	else if( sensitive>_pan_max ) 
		return float(_max_pan);
	else
	{
		float real_sensitive = (sensitive-_pan_min)*(_max_pan-_min_pan)/(_pan_max-_pan_min)+_min_pan;
		return real_sensitive;
	}
}

float hanil_ptz_controller::get_atilt_sensitive_value( float sensitive )
{
	query_limits();

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return sensitive;

	if( sensitive<_tilt_min ) 
		return float(_min_tilt);
	else if( sensitive>_tilt_max ) 
		return float(_max_tilt);
	else
	{
		float real_sensitive = (sensitive-_tilt_min)*(_max_tilt-_min_tilt)/(_tilt_max-_tilt_min)+_min_tilt;
		return real_sensitive;
	}
}

float hanil_ptz_controller::get_azoom_sensitive_value( float sensitive )
{
	query_limits();

	if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
		return sensitive;

	if( sensitive<_zoom_min ) 
		return _min_zoom;
	else if( sensitive>_zoom_max ) 
		return _max_zoom;
	else
	{
		float real_sensitive = (sensitive-_zoom_min)*(_max_zoom-_min_zoom)/(_zoom_max-_zoom_min)+_min_zoom;
		return real_sensitive;
	}
}

float hanil_ptz_controller::get_speed_sensitive_value( float sensitive )
{
	query_limits();

	if( _speed_min==_min_speed && _speed_max==_max_speed )
		return sensitive;

	if( abs(sensitive)>abs(_speed_max-_speed_min) ) 
	{
		if( sensitive<0 )
			return _min_speed;
		else
			return _max_speed;
	}
	else
	{
		float real_sensitive = (sensitive-_speed_min)*(_max_speed-_min_speed)/(_speed_max-_speed_min)+_min_speed;
		if(real_sensitive>0) real_sensitive = real_sensitive+(_max_pan+_min_pan)/2;
		else real_sensitive = abs(real_sensitive);
		return real_sensitive;


	}
}

float hanil_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
{
	query_limits();	

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return real_sensitive;

	if( real_sensitive<_min_pan ) 
		return _pan_min;
	else if( real_sensitive>_max_pan ) 
		return _pan_max;
	else
	{
		float sensitive = ((real_sensitive-_min_pan)*(_pan_max-_pan_min))/(_max_pan-_min_pan)+_pan_min;
		if( sensitive>_pan_max )
			sensitive = _pan_max;
		if( sensitive<_pan_min )
			sensitive = _pan_min;
		return sensitive;
	}
}

float hanil_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return real_sensitive;

	if( real_sensitive<_min_tilt ) 
		return _tilt_min;
	else if( real_sensitive>_max_tilt ) 
		return _tilt_max;
	else
	{
		float sensitive = ((real_sensitive-_min_tilt)*(_tilt_max-_tilt_min))/(_max_tilt-_min_tilt)+_tilt_min;
		if( sensitive>_tilt_max )
			sensitive = _tilt_max;
		if( sensitive<_tilt_min )
			sensitive = _tilt_min;
		return sensitive;
	}
}

float hanil_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
		return real_sensitive;

	if( real_sensitive<_min_zoom ) 
		return _zoom_min;
	else if( real_sensitive>_max_zoom ) 
		return _zoom_max;
	else
	{
		float sensitive = ((real_sensitive-_min_zoom)*(_zoom_max-_zoom_min))/(_max_zoom-_min_zoom)+_zoom_min;
		if( sensitive>_zoom_max )
			sensitive = _zoom_max;
		if( sensitive<_zoom_min )
			sensitive = _zoom_min;
		return sensitive;
	}
}

unsigned short hanil_ptz_controller::send_data(std::string cmd, int data)
{
	std::string send_str_data = "";
	char pelcod_data[7] = {0,};
	pelcod_data[0] = 0xff;//Sync Byte 
	pelcod_data[1] = 0x01;//Address

	if(cmd=="PTZ_CONTINUOUS_MOVE_UP")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x08; //Command 2
	    pelcod_data[4] = 0x00; //pan speed
	    pelcod_data[5] = data; //tilt speed
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFT")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x04; //Command 2
	    pelcod_data[4] = data; //pan speed
	    pelcod_data[5] = 0x00; //tilt speed
		
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHT")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x02; //Command 2
	    pelcod_data[4] = data; //pan speed
	    pelcod_data[5] = 0x00; //tilt speed
		
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_DOWN")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x10; //Command 2
	    pelcod_data[4] = 0x00; //pan speed
	    pelcod_data[5] = data; //tilt speed
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFTUP")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x0c; //Command 2
	    pelcod_data[4] = data; //pan speed
	    pelcod_data[5] = data; //tilt speed
		
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHTUP")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x0a; //Command 2
	    pelcod_data[4] = data; //pan speed
	    pelcod_data[5] = data; //tilt speed
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFTDOWN")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x14; //Command 2
	    pelcod_data[4] = data; //pan speed
	    pelcod_data[5] = data; //tilt speed
		
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHTDOWN")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x12; //Command 2
	    pelcod_data[4] = data; //pan speed
	    pelcod_data[5] = data; //tilt speed
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_ZOOMIN")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x20; //Command 2
	    pelcod_data[4] = data; //pan speed
	    pelcod_data[5] = 0x00; //tilt speed
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_ZO0MOUT")
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x40; //Command 2
	    pelcod_data[4] = 0x00; //pan speed
	    pelcod_data[5] = 0x00; //tilt speed
	}
	else if(cmd=="STOP") 
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x00; //Command 2
	    pelcod_data[4] = 0x00; //pan speed
	    pelcod_data[5] = 0x00; //tilt speed
	}
	else if(cmd=="GOTO_PRESET") 
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x07; //Command 2
	    pelcod_data[4] = 0x00; //pan speed
	    pelcod_data[5] = data; //tilt speed
	}
	else if(cmd=="ADD_PRESET") 
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x03; //Command 2
	    pelcod_data[4] = 0x00; //pan speed
	    pelcod_data[5] = data; //tilt speed
	}
	else if(cmd=="REMOVE_PRESET") 
	{
		pelcod_data[2] = 0x00; //Command 1
		pelcod_data[3] = 0x05; //Command 2
	    pelcod_data[4] = 0x00; //pan speed
	    pelcod_data[5] = data; //tilt speed
	}
	
	pelcod_data[6]= (pelcod_data[1]+pelcod_data[2]+pelcod_data[3]+pelcod_data[4]+pelcod_data[5]); //checksum

	//send_str_data
	for(int i=0;i<7;i++)
	{
		char tmp_data[100] = {0,};
		snprintf(tmp_data, sizeof(tmp_data), "%.2x", pelcod_data[i]);
		std::string tmp_str = tmp_data;
		send_str_data.append(tmp_str.substr(0,2));
	}

	http_client client( _hostname, _port_number, "/cgi-bin/common/serial.cgi" );
	client.put_variable( "web_or_app", "app" );
	client.put_variable( "action", "send" );
	client.put_variable( "Serial.Data", send_str_data.c_str() );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;

}

base_ptz_controller* create( void )
{
	return new hanil_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	hanil_ptz_controller *ini_controller = dynamic_cast<hanil_ptz_controller*>( (*ptz_controller) );
	delete ini_controller;
	(*ptz_controller) = 0;
}
