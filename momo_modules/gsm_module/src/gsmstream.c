/* gsmstream.c
 *
 * Endpoints for opening and dumping data to a text message stream.
 *
 *
 *
 */

#include "mib12_api.h"
#include "gsm_defines.h"

 bit stream_open;
 uint8 bytes_remaining;

 void gsm_openstream()
 {
 	if (GSMSTATUSPIN == 0)
 	{
 		bus_slave_setreturn(pack_return_status(6,0));
 		return;
 	}

 	


 }

 void gsm_putstream()
 {

 }

 void gsm_closestream()
 {
 	
 }