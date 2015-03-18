//app_main.c

#include "platform.h"
#include "protocol.h"
#include "gsm_defines.h"
#include "watchdog.h"
#include "gsm_serial.h"
#include "gsm.h"
#include "gsm_tx.h"

extern uint16 http_response_status;
void task(void)
{
	wdt_disable();
	gsm_tx_run();
}

void interrupt_handler(void)
{

}

void initialize(void)
{	
	debug_val = 0;
	http_response_status = 0;
	gsm_init();
}

/*
 * Do not use, required to be defined by startup code that xc8 apparently cannot *NOT* link in even though it is optimized away later.
 */	
void main()
{
	
}