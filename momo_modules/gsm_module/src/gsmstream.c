/* gsmstream.c
 *
 * Endpoints for opening and dumping data to a text message stream.
 *
 *
 *
 */

#include "gsmstream.h"

#include "mib12_api.h"
#include "gsm_serial.h"
#include "global_state.h"
#include "gsm_tx.h"
#include <string.h>

#define _XTAL_FREQ			4000000

//Defined in buffers.as
extern char comm_destination[65];
void gsm_rpc_setcommdestination()
{
	if (set_comm_destination() == 1)
		bus_slave_setreturn(pack_return_status(7,0));
	else
		bus_slave_setreturn(pack_return_status(0,0));
}

void gsm_openstream()
{
 	if ( state.tx_state != kTxIdle
 		|| comm_destination[0] == '\0' )
 	{
 		bus_slave_setreturn(pack_return_status(7,0)); //TODO: Busy MIB status code
 		return;
 	}

 	//TODO: Prioritize later streams by cancelling the current one?
 	
 	if ( !gsm_tx_start( comm_destination[0] == '+' ? kTxSMS : kTxHTTP ) )
 		bus_slave_setreturn(pack_return_status(6,0));
 	else
 		bus_slave_setreturn(pack_return_status(0,0));
}

void gsm_putstream()
{
 	if ( state.tx_state != kTxReady )
 	{
 		bus_slave_setreturn(pack_return_status(7,0));
 		return;
 	}

 	gsm_write( mib_buffer, mib_buffer_length() );
 	__delay_ms(1); //TODO: Needed?

 	bus_slave_setreturn(pack_return_status(0,0));
}

void gsm_closestream()
{
	if ( state.tx_state != kTxReady )
 	{
 		bus_slave_setreturn(pack_return_status(7,0));
 		return;
 	}

	if ( gsm_tx_finish() )
		bus_slave_setreturn(pack_return_status(0,0));
	else
		bus_slave_setreturn(pack_return_status(6,0));
}

 void gsm_abandonstream()
 {
 	gsm_tx_abandon();
 	bus_slave_setreturn(pack_return_status(0,0));
 }