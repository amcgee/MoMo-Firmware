//gsm_serial.c

#include "platform.h"
#include "gsm_serial.h"
#include "gsm_strings.h"
#include "mib12_api.h"
#include <string.h>

uint8 gsm_buffer[32];
uint8 buffer_len;

uint8 open_gsm_module()
{
	gsm_buffer[0] = 'A';
	gsm_buffer[1] = 'T';
	gsm_buffer[2] = '\r';

	buffer_len = 3;

	send_buffer();

	return receive_response();
}

void send_buffer()
{
	uint8 i;

	for (i=0; i<buffer_len; ++i)
	{
		while(TXIF == 0)
			;

		TXREG = gsm_buffer[i];

		asm("nop");
		asm("nop");
	}
}

uint8 receive_response()
{
	RCREG;
	RCREG;
	buffer_len = 0;

	if (OERR)
	{
		CREN = 0;
		CREN = 1;
	}

	RCREG;
	RCREG;

	while(1)
	{
		while(!RCIF)
			;

		gsm_buffer[buffer_len++] = RCREG;

		if (gsm_buffer[buffer_len-1] == '\n')
		{
			RC2 = !RC2;
			if (match_okay_response())
			{
				RC2 = !RC2;
				return 0;
			}
			else if (match_error_response())
				return 1;
		}

		if (buffer_len == 32)
			return 2;
	}
}

void copy_mib()
{
	uint8 i;

	buffer_len = mib_buffer_length();

	for (i=0; i<buffer_len; ++i)
		gsm_buffer[i] = mib_buffer[i];
}

void append_carriage()
{
	gsm_buffer[buffer_len++] = '\r';
}