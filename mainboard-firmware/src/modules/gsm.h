#ifndef __gsm_h__
#define __gsm_h__

#include "common.h"

typedef enum {
  gsm_status_off   = 0,
  gsm_status_on    = 1,
  gsm_status_ready = 2
} GSMStatus;

void gsm_init();

void gsm_receive_char( char c );

bool gsm_send_at_cmd( const char* cmd );
bool gsm_send_sms( const char* destination, const char* message );

void gsm_on();
void gsm_off();

GSMStatus gsm_status();
bool gsm_check_SIM();

void gsm_configure_serial();
void gsm_disable_serial();

#endif //__gsm_h__
