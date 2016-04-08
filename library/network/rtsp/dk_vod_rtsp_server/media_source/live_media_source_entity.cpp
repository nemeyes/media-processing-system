#include "live_media_source_entity.h"
#include <windows.h>

live_media_source_entity::_media_source_t::_media_source_t(void)
{
	memset(uuid, 0x00, sizeof(uuid));
	memset(url, 0x00, sizeof(url));
	memset(username, 0x00, sizeof(username));
	memset(password, 0x00, sizeof(password));
}

live_media_source_entity::_media_source_t::_media_source_t(const live_media_source_entity::_media_source_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
}

live_media_source_entity::_media_source_t::~_media_source_t(void)
{

}

live_media_source_entity::_media_source_t live_media_source_entity::_media_source_t::operator=(const live_media_source_entity::_media_source_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
	return (*this);
}

live_media_source_entity::live_media_source_entity(void)
{

}

live_media_source_entity::~live_media_source_entity(void)
{
	std::map<std::string, live_media_source_entity::media_source_t*>::iterator iter;
	for (iter = _media_sources.begin(); iter != _media_sources.end(); iter++)
	{
		live_media_source_entity::media_source_t * media_source = media_source = iter->second;
		if (media_source)
		{
			delete media_source;
			media_source = nullptr;
		}
	}
	_media_sources.clear();
}

live_media_source_entity & live_media_source_entity::instance(void)
{
	static live_media_source_entity _instance;
	return _instance;
}

bool live_media_source_entity::add_live_media_source(const char * uuid, const char * url, const char * username, const char * password)
{
	live_media_source_entity::media_source_t * media_source = new live_media_source_entity::media_source_t();
	strncpy_s(media_source->uuid, uuid, sizeof(media_source->uuid));
	strncpy_s(media_source->url, url, sizeof(media_source->url));
	if (username && strlen(username)>0)
		strncpy_s(media_source->username, username, sizeof(media_source->username));
	if (password && strlen(password)>0)
		strncpy_s(media_source->password, password, sizeof(media_source->password));
	_media_sources.insert(std::make_pair(media_source->uuid, media_source));

	return true;
}

bool live_media_source_entity::remove_live_media_source(const char * uuid)
{
	bool status = false;
	std::map<std::string, live_media_source_entity::media_source_t*>::iterator iter;
	iter = _media_sources.find(uuid);
	if (iter != _media_sources.end())
	{
		live_media_source_entity::media_source_t * media_source = iter->second;
		if (media_source)
		{
			delete media_source;
			media_source = nullptr;
		}
		_media_sources.erase(uuid);
		status = true;
	}
	else
	{
		status = false;
	}
	return status;
}

live_media_source_entity::media_source_t * live_media_source_entity::get_live_media_source(const char * uuid)
{
	std::map<std::string, live_media_source_entity::media_source_t*>::iterator iter;
	iter = _media_sources.find(uuid);
	if (iter != _media_sources.end())
	{
		live_media_source_entity::media_source_t * media_source = nullptr;
		media_source = iter->second;
		return media_source;
	}
	else
	{
		return nullptr;
	}
}