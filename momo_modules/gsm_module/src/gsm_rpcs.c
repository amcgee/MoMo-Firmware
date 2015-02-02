#include "global_state.h"
#include "gsm_defines.h"
#include "gsm.h"
#include "gsm_serial.h"
#include "mib12_api.h"
#include "protocol.h"
#include "watchdog.h"
#include "gprs.h"
#include "http.h"
#include "simcard.h"
#include "gsmstream.h"
#include "port.h"

//MIB Endpoints
void gsm_rpc_on()
{
	wdt_disable();
	
	mib_buffer[0] = gsm_on();
	mib_buffer[1] = 0;

	bus_slave_setreturn(pack_return_status(0,2));
}
void gsm_rpc_off()
{
	gsm_off();
	bus_slave_setreturn(pack_return_status(0,0));
}

void gsm_rpc_power_on()
{
	ENSURE_DIGITAL(MODULEPOWERPIN);
	DRIVE_HI(MODULEPOWERPIN);
	bus_slave_setreturn(pack_return_status(0,0));
}

void gsm_rpc_power_off()
{
	ENSURE_DIGITAL(MODULEPOWERPIN);
	DRIVE_LOW(MODULEPOWERPIN);
	bus_slave_setreturn(pack_return_status(0,0));	
}

void gsm_rpc_testsim()
{
	mib_buffer[0] = simdet_detect();
	mib_buffer[1] = 0;

	bus_slave_setreturn(pack_return_status(0,2));	
}

void gsm_rpc_dumpbuffer()
{
	bus_slave_setreturn(pack_return_status(0, gsm_readback( mib_buffer, kBusMaxMessageSize ) ));
}
void gsm_rpc_debug()
{
	mib_buffer[0] = state.module_on;
	mib_buffer[1] = state.tx_state;
	mib_buffer[2] = state.tx_error;
	mib_buffer[3] = rx_buffer_start;
	mib_buffer[4] = rx_buffer_end;
	mib_buffer[5] = debug_val;

	bus_slave_setreturn(pack_return_status(0, 6));
}

void gsm_rpc_download()
{
	
}

void gsm_rpc_sendcommand()
{
	// When we're in the "ready" state we don't want to accidentally transmit this command.
	if (state.module_on && state.tx_state != kTxReady )
	{
		gsm_write( mib_buffer, mib_buffer_length() );
		uint8 result = gsm_cmd( "" );
		if ( result == GSM_SERIAL_TIMEDOUT || result == GSM_SERIAL_NODATA )
		{
			bus_slave_setreturn( pack_return_status(6,0) );
			return;
		}

		gsm_rpc_dumpbuffer();
		return;
	}

	bus_slave_setreturn(pack_return_status(7,0));
}