#ifndef __global_state_h__
#define __global_state_h__

#include "platform.h"

typedef enum {
	kTxIdle = 0,
	kTxConnecting = 1,
	kTxReady = 2,
	kTxSending = 3
} TransmissionState;

typedef enum {
	kTxErrorNone       = 0,
	kTxErrorNetwork    = 1,
	kTxErrorPrepare    = 2,
	kTxErrorSend       = 3,
	kTxErrorHTTPNot200 = 4,
	kTxErrorUnknown    = 5
} TransmissionError;

typedef enum {
	kTxSMS = 0,
	kTxHTTP = 1
} TransmissionType;

//Global State Types
typedef union 
{
	struct
	{
		volatile uint8 module_on:1;
		volatile uint8 tx_type:1;
		volatile uint8 tx_state:2;
		volatile uint8 tx_error:3;
	};

	volatile uint8 gsm_state;
} ModuleState;

#ifndef DEFINE_STATE
#define prefix extern
#else
#define prefix
#endif

//GSM Serial Communication Receive Buffer
prefix uint8 debug_val;

//GSM Module Status
prefix ModuleState state;

#endif