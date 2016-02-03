#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <cstdint>
#include <iocp_io_context.h>

#define COMMAND_SIZE sizeof(int)
#define CMD_ASSOCIATION_REQUEST			10
#define CMD_ASSOCIATION_RESPONSE		11
#define CMD_LEAVE_REQUEST				12
#define CMD_LEAVE_RESPONSE				13
#define CMD_HEARTBEAT_REQUEST			14
#define CMD_HEARTBEAT_RESPONSE			15

namespace ic
{
	typedef struct _header_t
	{
		int8_t	destination[64];
		int8_t	source[64];
		int32_t sequence;
		int32_t length;
	} header_t;

	typedef struct _cmd_base_payload_t
	{
		int32_t	code;
	} cmd_base_payload_t;

	typedef struct _cmd_assoc_payload_t : public _cmd_base_payload_t
	{
	} cmd_assoc_payload_t;

	typedef struct _cmd_leave_payload_t : public _cmd_base_payload_t
	{
	} cmd_leave_payload_t;

	typedef struct _cmd_heartbeat_payload_t : public _cmd_base_payload_t
	{
		int32_t index;
	} cmd_heartbeat_payload_t;
};

class BusinessProcessor;
class abstract_command
{
public:
	abstract_command(int32_t command_id)
		: _command_id(command_id)
	{
	}

	VOID	SetProcessor(BusinessProcessor	*processor)
	{
		_processor = processor;
	}

	UINT	GetCommandID(VOID)
	{
		return _command_id;
	}

	virtual VOID Execute(UINT dst, UINT src, USHORT seq, BOOL direct, USHORT length,
		UINT command_id, CHAR *payload, LPPER_SOCKET_CONTEXT_T socket_context = NULL) = 0;

protected:
	BusinessProcessor	*_processor;
	int32_t	_command_id;
};
