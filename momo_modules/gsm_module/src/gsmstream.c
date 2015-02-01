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
#include "gsm.h"
#include "global_state.h"
#include "simcard.h"
#include "sms.h"
#include "gprs.h"
#include "http.h"
#include <string.h>

char comm_destination[65] = { '\0' };

void gsm_setupstream()
{

}

void gsm_rpc_setcommdestination()
{
	if ( plist_get_int8(0) + mib_buffer_length() > 64 )
		bus_slave_setreturn(pack_return_status(7,0));

	memcpy( comm_destination + plist_get_int8(0), mib_buffer+2, mib_buffer_length() );
	comm_destination[plist_get_int8(0) + mib_buffer_length()] = '\0';
	bus_slave_setreturn(pack_return_status(0,0));
}

bool gsm_stream_prepare() // Run in mainline code
{
	if ( comm_destination[0] == '+' )
 	{
 		state.stream_type = kStreamSMS;
 		if ( !sms_prepare( comm_destination, strlen(comm_destination) ) )
 		{
 			return false;
 		}
 	}
 	else
 	{
 		state.stream_type = kStreamGPRS;
 		if ( !gprs_connect()
 			|| !http_init()
 			|| !http_write_prepare( plist_get_int16(0) ) )
 		{
 			return false;
 		}
 	}
 	return true;
}

void gsm_openstream()
{
 	if ( state.stream_state != kStreamIdle )
 	{
 		bus_slave_setreturn(pack_return_status(7,0)); //TODO: Busy MIB status code
 		return;
 	}

 	if (simdet_detect() == 0)
 	{
 		bus_slave_setreturn(pack_return_status(6,0));
 		return;
 	}

 	state.streaming = 1;
 	state.stream_state = kStreamConnecting;
 	bus_slave_setreturn(pack_return_status(0,0));
}

void gsm_putstream()
{
 	if ( state.stream_state != kStreamReady || !state.streaming )
 	{
 		bus_slave_setreturn(pack_return_status(7,0));
 		return;
 	}

 	gsm_write( mib_buffer, mib_buffer_length() );

 	bus_slave_setreturn(pack_return_status(0,0));
}

void gsm_closestream()
{
	if ( state.stream_state != kStreamReady || !state.streaming )
 	{
 		bus_slave_setreturn(pack_return_status(7,0));
 		return;
 	}

	if ( state.stream_type == kStreamSMS )
	{
		if ( sms_send() )
	 		gsm_off();
	 	else
	 		state.stream_state = kStreamTransmitting;  // Shutdown the module after we've sent the message (or timed out)
	 	bus_slave_setreturn(pack_return_status(0,0));
	}
	else
	{
		if ( !http_post(comm_destination) )
		{
			state.streaming = 0;
			state.stream_state = kStreamIdle;
			gsm_off();
			bus_slave_setreturn(pack_return_status(6,0));
		}
		else
		{
			state.stream_state = kStreamTransmitting;
			bus_slave_setreturn(pack_return_status(0,0));
		}
	}
}

bool gsm_stream_confirm( uint8 timeout )
{
	if ( state.stream_type == kStreamSMS )
	{
		gsm_expect( "+CMGS:" );
		gsm_expect2( "ERROR" );

		if ( gsm_await( timeout ) == 1 ) {
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if ( http_await_response( timeout ) ) {
			return true;
		}
		else
		{
			return false;
		}
	}
}

 void gsm_abandonstream()
 {
 	gsm_off();
 	bus_slave_setreturn(pack_return_status(0,0));
 }