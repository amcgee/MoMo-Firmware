//app_main.c

#include "platform.h"
#include "protocol.h"
#include "gsm_defines.h"
#include "watchdog.h"
#include "gsm_serial.h"
#include "gsm.h"
#include "gsm_tx.h"

#include <string.h>

void task(void)
{
	uint8 counter;
	wdt_disable();
	
	//Don't sleep while the module's on so that we don't miss a
	//serial message
	while(state.module_on)
	{
		gsm_tx_iterate();
		gsm_rx();
	}
	gsm_tx_abandon(); // make sure everything is cleaned up
}

void interrupt_handler(void)
{

}

void initialize(void)
{	
	debug_val = 0;
	gsm_init();
}

/*
 * Do not use, required to be defined by startup code that xc8 apparently cannot *NOT* link in even though it is optimized away later.
 */	
void main()
{
	
}