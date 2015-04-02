#include "gsm_tx.h"
#include "gsm.h"
#include "gsm_serial.h"
#include "gsmstream.h"
#include "global_state.h"
#include "mib12_api.h"
#include "sms.h"
#include "gprs.h"
#include "http.h"
#include "simcard.h"

//Defined in buffers.as
extern char comm_destination[65]; // TODO: Factor better

static void transmit_callback()
{
	bus_master_begin_rpc();
	mib_packet.param_spec = plist_no_buffer(2);
	mib_packet.feature = 60;
	mib_packet.command = 0xF0;

	((uint16*)mib_buffer)[0] = state.tx_state == kTxReady ? 0 : 1;
	((uint16*)mib_buffer)[1] = state.tx_error;

	bus_master_send_rpc(8);
}

static bool tx_prepare()
{
	if ( state.tx_type == kTxSMS )
 	{
 		uint8 len = 0;
 		while ( comm_destination[len] != '\0' )
 			++len;

 		if ( !sms_prepare( comm_destination, len ) )
 		{
 			return false;
 		}
 	}
 	else
 	{
 		if ( !gprs_register()
 			|| !gprs_connect()
 			|| !http_init()
 			|| !http_write_prepare( plist_get_int16(0) ) )
 		{
 			return false;
 		}
 	}
 	return true;
}
static bool tx_connect()
{
	if ( !gsm_on() )
		return false;

	if ( gsm_register() )
	{
		gsm_remember_band();
		if ( !tx_prepare() )
		{
			state.tx_error = kTxErrorPrepare;
		}
		else
		{
			return true;
		}
	}
	else
	{
		state.tx_error = kTxErrorNetwork;
		gsm_forget_band(); // registration failed: forget the band, try again.
	}

	gsm_off();
	return false;
}

static bool tx_confirm()
{
	if ( state.tx_type == kTxSMS )
	{
		gsm_expect( "+CMGS:" );
		gsm_expect2( "ERROR" );

		if ( gsm_await( 10 ) == 1 )
		{
			return true;
		}
		else
		{
			state.tx_error = kTxErrorSend;
			return false;
		}
	}
	else
	{
		if ( http_await_response() ) {
			if ( http_status() != 200 )
			{
				state.tx_error = kTxErrorHTTPNot200;
				return false;
			}
			return true;
		}
		else
		{
			state.tx_error = kTxErrorSend;
			return false;
		}
	}
}

// static void capture_error(void)
// {
// 	uint8 len = gsm_read( mib_buffer, kBusMaxMessageSize );
// 	if ( len == 0 )
// 		return;

// 	bus_master_begin_rpc();
// 	bus_master_prepare_rpc( 42, 0x20, plist_with_buffer( 0, len ) );
// 	bus_master_send_rpc( 8 );
// }

// extern char* uint_buf;

bool gsm_tx_start( TransmissionType type )
{
	if ( state.tx_state != kTxIdle
	  || !simdet_detect() )
		return false;

	state.tx_type = type;
 	state.tx_state = kTxConnecting;
 	state.tx_error = kTxErrorUnknown; // Fail unless we explicitly succeed
 	return true;
}

// Process state machine transition.  Run in the main task loop.
void gsm_tx_iterate()
{
	uint8 counter;
	if ( state.tx_state == kTxIdle )
		return;

	switch ( state.tx_state )
		{
		case kTxConnecting:
			counter = 4;
			while ( !tx_connect() && --counter > 0 )
				continue;

			if ( counter == 0 )
				state.tx_state = kTxIdle;
			else
				state.tx_state = kTxReady;
			
			transmit_callback();
			break;
		case kTxStreaming:
			//pass
			break;
		case kTxSending:
			if ( tx_send() )
			{
				counter = TX_CONFIRMATION_TIMEOUT;
				while ( !tx_confirm() && --counter > 0 )
					continue;

				if ( counter > 0 )
					state.tx_error = kTxErrorNone; // The only explicit success condition
			}

			state.tx_state = kTxIdle;
			transmit_callback();
			
			//TODO: Log the error?
			
			gsm_off();
			break;
		};
}

void gsm_tx_run() // Mainline loop
{
	while ( state.tx_state != kTxIdle )
	{
		gsm_tx_iterate();
	}
	gsm_off(); // make sure everything is cleaned up
}

static bool tx_send()
{
	if ( state.tx_type == kTxSMS )
	{
		if ( sms_send() )
			gsm_off(); // This is actually a success condition, since the sms could send before we try to confirm
		return true;
	}
	else
	{
		return http_post(comm_destination);
	}
}
bool gsm_tx_finish()
{
	if ( state.tx_state != kTxStreaming )
		return false;
	state.tx_state = kTxSending;
	return true;
}

void gsm_tx_abandon()
{
	state.tx_state = kTxIdle;
}