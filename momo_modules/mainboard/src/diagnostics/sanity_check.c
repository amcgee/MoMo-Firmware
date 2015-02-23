#include "sanity_check.h"
#include "scheduler.h"
#include "module_manager.h"
#include "system_log.h"
#include "log_definitions.h"

ScheduledTask sanity_check_task;

void sanity_check_schedule()
{
	scheduler_schedule_task( sanity_check_run, kEveryHour, 0, &sanity_check_task, NULL );
}

void sanity_check_run( void* arg )
{
	uint8 count = module_count();
	if ( count != SANITY_CHECK_MODULE_COUNT )
	{
		LOG_CRITICAL(kSanityCheckResetNotice);
		LOG_INT(SANITY_CHECK_MODULE_COUNT);
		LOG_INT(count);
		LOG_FLUSH();
		asm volatile("reset");
	}
}