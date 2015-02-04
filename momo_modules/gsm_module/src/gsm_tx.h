#ifndef __gsm_tx_h__
#define __gsm_tx_h__

#include "global_state.h"

// 8 minutes in 10 second intervals
#define TX_CONFIRMATION_TIMEOUT 8*6

bool gsm_tx_start( TransmissionType type );
void gsm_tx_run();
bool gsm_tx_finish();

void gsm_tx_abandon();

#endif