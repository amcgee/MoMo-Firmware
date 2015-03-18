// #include "comm_stream.h"
// #include "scheduler.h"
// #include "momo_config.h"
// #include "protocol.h"
// #include "module_manager.h"
// #include <stdarg.h>
// #include <string.h>

// typedef enum {
// 	kStreamIdle             = 0,
// 	kStreamWarmingUp        = 1,
// 	kStreamConfiguringAPN   = 2, // TODO: generalize configuration
// 	kStreamConfiguringRoute = 3,
// 	kStreamOpening          = 4,
// 	kStreamOpenPending      = 5,
// 	kStreamPushing          = 6,
// 	kStreamClosing          = 7,
// 	kStreamClosePending     = 8
// } StreamState;

// typedef struct {
// 	StreamState state;
// 	uint8 state_counter;
// 	uint8 retry_count;
// 	uint8 max_retries;
// 	uint8 module_address;

// 	const char* data;
// 	uint8 data_len;
// 	const char* route;
// 	uint8 route_len;

// 	ScheduledTask retry_task;
// } StreamData;

// #define CONFIG current_momo_state.report_config
// #define MAX_ACTIVE_STREAMS 8
// static StreamData active_streams[MAX_ACTIVE_STREAMS];

// void init_comm_stream()
// {
// 	uint8 i;
// 	for ( i = 0; i < MAX_ACTIVE_STREAMS; ++i )
// 		active_streams[i].state = kStreamIdle;
// }

// void stream_rpc( uint8 module, uint8 feature, uint8 command, uint8 ints, uint8 buffer_length, va_list argp )
// {
	
// }

// void do_stream( void* arg )
// {
// 	StreamData* stream = &active_streams[(uint8) arg];
// 	const char* apn;
// 	uint8 length;
// 	switch ( stream->state )
// 	{
// 	case kStreamIdle:
// 		break;
// 	case kStreamWarmingUp:
// 		// pass
// 		stream->state_counter = 0;
// 		stream->state = kStreamConfiguringAPN;
// 		taskloop_add( do_stream, arg );
// 		break;
// 	case kStreamConfiguringAPN:
// 		apn = CONFIG.gprs_apn;
// 		length = strlen(apn);
// 		if ( length > 0 && stream->state_counter < length )
// 		{
// 			//stream_rpc( stream, 10, 9, 0, length, apn );
// 			stream->state_counter += length;
// 		}
// 		else
// 		{
// 			stream->state_counter = 0;
// 			stream->state = kStreamConfiguringRoute;
// 		}
// 		break;
// 	case kStreamConfiguringRoute:
// 		if ( stream->route_len > 0 && stream->state_counter < stream->route_len )
// 		{
// 			length = kBusMaxMessageSize - 2;
// 			if ( stream->route_len - stream->state_counter < length )
// 				length = stream->route_len - stream->state_counter;
// 			//stream_rpc( stream, 10, 9, 1, length, stream->route + stream->state_counter)
// 			stream->state_counter += length;
// 		}
// 		else
// 		{
// 			// stream_rpc( stream, 11, 4, 1, 0, stream->data_len );
// 			stream->state_counter = 0;
// 			stream->state = kStreamOpening;
// 		}
// 		break;
// 	case kStreamOpening:
// 		stream->state = kStreamOpenPending; // Wait for the "ready" callback
// 		break;
// 	case kStreamOpenPending:
// 		stream->state = kStreamPushing; // Received the "ready" callback
// 		taskloop_add( do_stream, arg );
// 		break;
// 	case kStreamPushing:
// 		if ( stream->data_len > 0 && stream->state_counter < stream->data_len )
// 		{
// 			length = kBusMaxMessageSize - 2;
// 			if ( stream->data_len - stream->state_counter < length )
// 				length = stream->data_len - stream->state_counter;
// 			//stream_rpc( stream, 10, 9, 1, length, stream->data + stream->state_counter)
// 			stream->state_counter += length;
// 		}
// 		else
// 		{
// 			// stream_rpc( stream, 11, 4, 0, 0 );
// 			stream->state_counter = 0;
// 			stream->state = kStreamClosing;
// 		}
// 		break;
// 	case kStreamClosing:
// 		stream->state = kStreamClosePending;
// 		break;
// 	case kStreamClosePending:
// 		break;
// 	}
// }

// void stream_callback( uint8 id, uint8 status, uint8 error )
// {
// 	StreamData* stream = &active_streams[id];
// 	if ( status == 0 && stream->state == kStreamOpenPending )
// 	{
// 		taskloop_add( do_stream, (void*) id );
// 	}
// 	else if ( status == 1 )
// 	{
// 		// We've succeeded or failed, do something
// 	}
// }

// #define TOO_MANY_STREAMS 0xFF
// StreamData* comm_stream_create()
// {
// 	uint8 i;
// 	for ( i = 0; i < MAX_ACTIVE_STREAMS; ++i )
// 	{
// 		if ( active_streams[i].state == kStreamIdle )
// 			break;
// 	}
// 	if ( i == MAX_ACTIVE_STREAMS )
// 		return TOO_MANY_STREAMS;

// 	active_streams[i].state = kStreamWarmingUp;
	
// 	return &active_streams[i];
// }
// void comm_stream( const char* data, const char* route, AlarmRepeatTime retry_interval, uint8 max_retries, void* callback )
// {
// 	ModuleIterator iterator = create_module_iterator( kMIBCommunicationType );
// 	while ( module_iter_next( &iterator ) ) {
// 		StreamData* stream = comm_stream_create();

// 		if ( stream == TOO_MANY_STREAMS )
// 		{
// 			//TODO: log
// 			return;
// 		}

// 		stream->state_counter = 0;
// 		stream->module_address = module_iter_address( &iterator );
// 		stream->data = data;
// 		stream->route = route;
// 		stream->max_retries = data;
// 		stream->retry_count = 0;

// 		taskloop_add( do_stream, stream - active_streams );
// 	}
// }