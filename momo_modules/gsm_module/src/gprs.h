#ifndef __gprs_h__
#define __gprs_h__

#include "platform.h"

#define GPRS_REGISTRATION_TIMEOUT_S 120
#define GPRS_CONNECT_TIMEOUT_S	30		//It can take a long time for the gprs connection to activate

void gprs_init_buffers();
void gprs_set_apn();

bool gprs_register();
bool gprs_registered();

bool gprs_connect();
bool gprs_connected();
void gprs_disconnect();

#endif