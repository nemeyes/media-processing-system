#ifndef _LIVE_MEDIA_SOURCE_ENTITY_H_
#define _LIVE_MEDIA_SOURCE_ENTITY_H_

#include <string>
#include <map>

class live_media_source_entity
{
public:
	typedef struct _media_source_t
	{
		char uuid[64];
		char url[260];
		char username[260];
		char password[260];
		_media_source_t(void);
		_media_source_t(const _media_source_t & clone);
		~_media_source_t(void);
		_media_source_t operator=(const _media_source_t & clone);
	} media_source_t;
	live_media_source_entity(void);
	virtual ~live_media_source_entity(void);

	static live_media_source_entity & instance(void);

	bool add_live_media_source(const char * uuid, const char * url, const char * username, const char * password);
	bool remove_live_media_source(const char * uuid);
	live_media_source_entity::media_source_t * get_live_media_source(const char * uuid);

private:
	std::map<std::string, live_media_source_entity::media_source_t*> _media_sources;
};


#endif