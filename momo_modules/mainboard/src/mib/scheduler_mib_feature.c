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
} scheduled_callback;

static scheduled_callback callbacks[16];
static uint16 callback_map;

static void callback( void* arg )
{
	scheduled_callback *cb = (scheduled_callback*) arg;

	MIBUnified cmd;
	cmd.address = cb->address;
  cmd.bus_command.feature = cb->feature;
  cmd.bus_command.command = cb->command;
  cmd.bus_command.param_spec = plist_empty();
	bus_master_rpc_async( NULL, &cmd );
}

void schedule_callback()
{
	uint8 index = 0;
	for ( index = 0; index < 16; ++index )
	{
		if ( !( callback_map | 0x1<<index ) )
		{
			callbacks[index].address = plist_get_int16(0);
			callbacks[index].feature = plist_get_int16(1);
			callbacks[index].command = plist_get_int16(2);

			scheduler_schedule_task( callback, callbacks[index].frequency, kScheduleForever, &callbacks[index].task, (void*) &callbacks[index] );
		}
	}
}

void stop_scheduled_callback()
{
	uint8 index;
	for ( index = 0; index < 16; ++index )
	{
		if ( callback_map | 0x1<<index &&
			   callbacks[index].address == plist_get_int16(0) &&
			   callbacks[index].feature == plist_get_int16(1) &&
			   callbacks[index].command == plist_get_int16(2) )
		{
			scheduler_remove_task( &callbacks[index].task );
			callback_map &= ~( callback_map | 0x1<<index );
		}
	}
}

DEFINE_MIB_FEATURE_COMMANDS(scheduler) {
	{0x00, schedule_callback, plist_spec(3,false) },
	{0x01, stop_scheduled_callback, plist_spec(3,false) }
};
DEFINE_MIB_FEATURE(scheduler);