
#include "platform.h"

typedef enum {
	kStreamIdle = 0,
	kStreamConnecting = 1,
	kStreamReady = 2,
	kStreamTransmitting = 3
} StreamState;

typedef enum {
	kStreamErrorNone       = 0,
	kStreamErrorNetwork    = 1,
	kStreamErrorPrepare    = 2,
	kStreamErrorSend       = 3,
	kStreamErrorHTTPNot200 = 4,
	kStreamErrorUnknown    = 5
} StreamErrorCode;

//Global State Types
typedef union 
{
	struct
	{
		volatile uint8 module_on:1;
		volatile uint8 streaming:1;
		volatile uint8 stream_type:1;
		volatile uint8 stream_state:2;
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

prefix uint8 stream_id;

//GSM Module Status
prefix ModuleState state;
