#include "bus_slave.h"
#include "mib_definitions.h"
#include "mib_feature_definition.h"
#include "scheduler_mib_feature.h"
#include "scheduler.h"
#include "common_types.h"
#include "bus_master.h"

typedef struct {
	uint8 address;
	uint8 feature;
	uint8 command;

	AlarmRepeatTime frequency;

	ScheduledTask task;
	uint8 rpc_id;
} RPCCallback;

static uint8 current_rpc_id = 1;


#define MAX_SCHEDULED_CALLBACKS 16
static RPCCallback callbacks[MAX_SCHEDULED_CALLBACKS];

static void rpc_done( uint8 status )
{
	uint8 index;
	for ( index = 0; index < MAX_SCHEDULED_CALLBACKS; ++index )
	{
		if ( callbacks[index].rpc_id == current_rpc_id )
		{
			callbacks[index].rpc_id = 0;
			break;
		}
	}
}
static void callback( void* arg )
{
	RPCCallback *cb = (RPCCallback*) arg;

	if ( cb->rpc_id > current_rpc_id )
		return;

	cb->rpc_id = (current_rpc_id++);
	if ( current_rpc_id == 0 )
		current_rpc_id = 1;

	MIBUnified cmd;
	cmd.address = cb->address;
  cmd.bus_command.feature = cb->feature;
  cmd.bus_command.command = cb->command;
  cmd.bus_command.param_spec = plist_empty();
	bus_master_rpc_async( rpc_done, &cmd );
}

void schedule_callback()
{
	uint8 address = plist_get_int16(0) & 0xFF;
	uint8 feature = plist_get_int16(1) >> 8;
	uint8 command = plist_get_int16(1) & 0xFF;
	uint8 frequency = plist_get_int16(2) & 0xFF;

	uint8 index = 0;
	for ( index = 0; index < MAX_SCHEDULED_CALLBACKS; ++index )
	{
		if ( callbacks[index].address == 0 )
		{
			RPCCallback* cb = &(callbacks[index]);
			cb->address = address;
			cb->feature = feature;
			cb->command = command;
			cb->frequency = frequency;
			cb->rpc_id = 0;

			scheduler_schedule_task( callback, cb->frequency, kScheduleForever, &(cb->task), (void*) cb );
			break;
		}
	}
	if ( index == 16 )
		bus_slave_seterror(kCallbackError);
}

void stop_scheduled_callback()
{
	uint8 address = plist_get_int16(0) & 0xFF;
	uint8 feature = plist_get_int16(1) >> 8;
	uint8 command = plist_get_int16(1) & 0xFF;
	uint8 frequency = plist_get_int16(2) & 0xFF;

	uint8 index;
	for ( index = 0; index < MAX_SCHEDULED_CALLBACKS; ++index )
	{
		if ( callbacks[index].address == address &&
			   callbacks[index].feature == feature &&
			   callbacks[index].command == command &&
			   callbacks[index].frequency == frequency )
		{
			scheduler_remove_task( &callbacks[index].task );
			callbacks[index].address = 0;
		}
	}
}

void get_callback_map()
{
	uint32 callback_map = 0;
	uint8 index;
	for ( index = 0; index < MAX_SCHEDULED_CALLBACKS; ++index )
	{
		if ( callbacks[index].address != 0 )
			callback_map |= 0x1 << index;
	}
	bus_slave_return_int16( callback_map );
}

void describe_callback()
{
	uint8 index = plist_get_int8(0);
	if ( index >= MAX_SCHEDULED_CALLBACKS || callbacks[index].address == 0 )
	{
		bus_slave_seterror( kCallbackError );
		return;
	}
	bus_slave_return_buffer( &(callbacks[index]), sizeof(RPCCallback) );
}

DEFINE_MIB_FEATURE_COMMANDS(scheduler) {
	{0x00, schedule_callback, plist_spec(3,false) },
	{0x01, stop_scheduled_callback, plist_spec(3,false) },
	{0x02, get_callback_map, plist_spec(0,false) },
	{0x03, describe_callback, plist_spec(1,false) }
};
DEFINE_MIB_FEATURE(scheduler);