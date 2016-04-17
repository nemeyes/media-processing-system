#ifndef _COMMANDS_PAYLOAD_H_
#define _COMMANDS_PAYLOAD_H_

#include <cstdint>

#define CMD_GET_MEDIASOURCES_REQUEST	100
#define CMD_GET_MEDIASOURCES_RESPONSE	101
#define CMD_GET_YEARS_REQUEST			102
#define CMD_GET_YEARS_RESPONSE			103
#define CMD_GET_MONTHS_REQUEST			104
#define CMD_GET_MONTHS_RESPONSE			105
#define CMD_GET_DAYS_REQUEST			106
#define CMD_GET_DAYS_RESPONSE			107
#define CMD_GET_HOURS_REQUEST			108
#define CMD_GET_HOURS_RESPONSE			109
#define CMD_GET_MINUTES_REQUEST			110
#define CMD_GET_MINUTES_RESPONSE		111
#define CMD_GET_SECONDS_REQUEST			112
#define CMD_GET_SECONDS_RESPONSE		113

namespace ic
{
	typedef struct _CMD_GET_YEARS_REQ_T
	{
		char uuid[64];
	} CMD_GET_YEARS_REQ_T;

	typedef struct _CMD_GET_YEARS_RES_T
	{
		int32_t code;
		int32_t count;
		int32_t years[10];
	} CMD_GET_YEARS_RES_T;

	typedef struct _CMD_GET_MONTHS_REQ_T
	{
		char uuid[64];
		int32_t year;
	} CMD_GET_MONTHS_REQ_T;

	typedef struct _CMD_GET_MONTHS_RES_T
	{
		int32_t code;
		int32_t year;
		int32_t count;
		int32_t months[12];
	} CMD_GET_MONTHS_RES_T;

	typedef struct _CMD_GET_DAYS_REQ_T
	{
		char uuid[64];
		int32_t year;
		int32_t month;
	} CMD_GET_DAYS_REQ_T;

	typedef struct _CMD_GET_DAYS_RES_T
	{
		int32_t code;
		int32_t year;
		int32_t month;
		int32_t count;
		int32_t days[31];
	} CMD_GET_DAYS_RES_T;

	typedef struct _CMD_GET_HOURS_REQ_T
	{
		char uuid[64];
		int32_t year;
		int32_t month;
		int32_t day;
	} CMD_GET_HOURS_REQ_T;

	typedef struct _CMD_GET_HOURS_RES_T
	{
		int32_t code;
		int32_t year;
		int32_t month;
		int32_t day;
		int32_t count;
		int32_t hours[24];
	} CMD_GET_HOURS_RES_T;

	typedef struct _CMD_GET_MINUTES_REQ_T
	{
		char uuid[64];
		int32_t year;
		int32_t month;
		int32_t day;
		int32_t hour;
	} CMD_GET_MINUTES_REQ_T;

	typedef struct _CMD_GET_MINUTES_RES_T
	{
		int32_t code;
		int32_t year;
		int32_t month;
		int32_t day;
		int32_t hour;
		int32_t count;
		int32_t minutes[60];
	} CMD_GET_MINUTES_RES_T;

	typedef struct _CMD_GET_SECONDS_REQ_T
	{
		char uuid[64];
		int32_t year;
		int32_t month;
		int32_t day;
		int32_t hour;
		int32_t minute;
	} CMD_GET_SECONDS_REQ_T;

	typedef struct _CMD_GET_SECONDS_RES_T
	{
		int32_t code;
		int32_t year;
		int32_t month;
		int32_t day;
		int32_t hour;
		int32_t minute;
		int32_t count;
		int32_t seconds[60];
	} CMD_GET_SECONDS_RES_T;
};
















#endif
