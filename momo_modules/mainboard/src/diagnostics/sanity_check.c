#include "sanity_check.h"
#include "scheduler.h"
#include "module_manager.h"
#include "system_log.h"
#include "log_definitions.h"
#include "rtcc.h"

ScheduledTask sanity_check_task;
extern rtcc_timestamp rpc_start_time;

void sanity_check_schedule()
{
	scheduler_schedule_task( sanity_check_run, kEveryHour, 0, &sanity_check_task, NULL );
}

void sanity_check_run( void* arg )
{
	uint8 count = module_count();
	TimeIntervalDirection dir;
	rtcc_timestamp now = rtcc_get_timestamp();
	if ( count != SANITY_CHECK_MODULE_COUNT 
		|| rtcc_timestamp_difference( rpc_start_time, now, &dir ) > 60 * 30 )
	{
		LOG_CRITICAL(kSanityCheckResetNotice);
		LOG_INT(SANITY_CHECK_MODULE_COUNT);
		LOG_INT(count);
		LOG_FLUSH();
		asm volatile("reset");
	}
}