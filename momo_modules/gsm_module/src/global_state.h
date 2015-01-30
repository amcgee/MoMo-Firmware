
#include "platform.h"

typedef enum {
	kStreamReady = 0,
	kStreamSuccess = 1,
	kStreamError = 2,
	
	kStreamSearchingForNetwork = 3,
	kStreamConnecting = 4,
	kStreamTransmitting = 5
} StreamState;

typedef enum {
	kStreamErrorNetwork = 0,
	
	kStreamErrorSMSPrepare = 1,
	kStreamErrorGPRSPrepare = 2,
	
	kStreamErrorSMSSend = 3,
	kStreamErrorGPRSSend = 4,
	
	kStreamErrorHTTPNot200 = 5
} StreamErrorCode;

//Global State Types
typedef union 
{
	struct
	{
		volatile uint8 module_on:1;
		volatile uint8 stream_type:1;
		volatile uint8 stream_state:3;
		volatile uint8 stream_error:3;
	};

	volatile uint8 gsm_state;
} ModuleState;

#define kStreamSMS 0
#define kStreamGPRS 1

#ifndef DEFINE_STATE
#define prefix extern
#else
#define prefix
#endif

//GSM Serial Communication Receive Buffer
#define RX_BUFFER_LENGTH 64
prefix uint8 gsm_rx_buffer[RX_BUFFER_LENGTH];
prefix uint8 rx_buffer_start;
prefix uint8 rx_buffer_end;
prefix uint8 rx_buffer_len;
prefix uint8 debug_val;

//GSM Module Status
prefix ModuleState state;
