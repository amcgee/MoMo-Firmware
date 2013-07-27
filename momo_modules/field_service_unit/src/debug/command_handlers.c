#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "serial_commands.h"
#include "rtcc.h"
#include "utilities.h"
#include "oscillator.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>
#include "scheduler.h"
#include "reset_manager.h"
#include "pic24asm.h"
#include "debug_utilities.h"
#include "bus_master.h"

extern volatile unsigned int adc_buffer[kADCBufferSize];
ScheduledTask test_task;

CommandStatus handle_echo_params(command_params *params)
{
  unsigned int i;

  if (params->num_params == 0) {
    print( "No parameters were passed.\r\n");
    return kFailure;
  }
  else
  {
    for (i=0; i<params->num_params; ++i)
    {
      print( get_param_string(params, i));
      print( "\r\n");
    }
  }
  return kSuccess;
}

CommandStatus handle_adc(command_params *params)
{
    char *cmd;

    if (params->num_params < 1)
    {
        print( "You must pass a subcommand to the adc command.\r\n");
        return kFailure;
    }

    cmd = get_param_string(params, 0);

    if (strcmp(cmd, "acquire") == 0)
    {
        int num_samples = 1;
        if (params->num_params == 2)
        {
            atoi_small( get_param_string(params, 1), &num_samples );
        }
        ADCConfig config;
        unsigned int channels = 0;

        ADD_CHANNEL(channels, 1);
        ADD_CHANNEL(channels, 5);
        ADD_CHANNEL(channels, kBandgapReference);
        ADD_CHANNEL(channels, kHalfBandgapReference);

        config.output_format = kUIntegerFormat;
        config.trigger = kInternalCounter;
        config.reference = kVDDVSS;
        config.enable_in_idle = 0;
        config.sample_autostart = 1;
        config.scan_input = 0;
        config.alternate_muxes = 0;
        config.autosample_wait = 0b11111;

        config.oneshot = 1;
        config.num_samples = num_samples;

        adc_configure(&config);
        adc_setup_scan(channels);
        _PCFG1 = 0;
        _TRISA1 = 1;
        adc_enable();
        sendf(DEBUG_UART, "ADC Enabled, acquiring %d samples\r\n", num_samples);
    }
    else if (strcmp(cmd, "get") == 0)
    {
        ADCConfig config;
        unsigned int val;

        config.output_format = kUIntegerFormat;
        config.trigger = kInternalCounter;
        config.reference = kVDDVSS;
        config.enable_in_idle = 0;
        config.sample_autostart = 1;
        config.scan_input = 0;
        config.alternate_muxes = 0;
        config.autosample_wait = 0b11111;

        config.oneshot = 1;
        config.num_samples = 1;

        adc_configure(&config);
        adc_set_channel(1);
        _PCFG1 = 0;
        _TRISA1 = 1;
        val = adc_convert_one();

        sendf(DEBUG_UART, "Read: %d\r\n", val);
    }
    else if (strcmp(cmd, "cal") == 0)
    {
        ADCConfig config;
        unsigned int val;

        config.output_format = kUIntegerFormat;
        config.trigger = kInternalCounter;
        config.reference = kVDDVSS;
        config.enable_in_idle = 0;
        config.sample_autostart = 1;
        config.scan_input = 0;
        config.alternate_muxes = 0;
        config.autosample_wait = 0b11111;

        config.oneshot = 1;
        config.num_samples = 1;

        adc_configure(&config);
        adc_set_channel(1);
        _PCFG1 = 0;
        _TRISA1 = 1;
        _OFFCAL = 1;
        val = adc_convert_one();
        _OFFCAL = 0;

        sendf(DEBUG_UART, "Calibration: %d\r\n", val);
    }
    else if (strcmp(cmd, "read") == 0)
    {
        unsigned int i=0;
        sends(DEBUG_UART, "Dumping ADC buffer\r\n");
        for (i=0; i<kADCBufferSize; ++i)
        {
            sendf(DEBUG_UART, "Reading %d: %d\r\n", i, adc_buffer[i]);
        }
    }
    else if (strcmp(cmd, "disable") == 0)
    {
        adc_disable();
        sends(DEBUG_UART, "ADC Disabled\r\n");
    }
    else
    {
        sendf(DEBUG_UART, "Unknown adc command: %s", cmd);
        return kFailure;
    }
    return kSuccess;
}

CommandStatus handle_device(command_params *params)
{
    char *cmd;

    if (params->num_params < 1)
    {
        print( "You must pass a subcommand to the device command.\r\n");
        return kFailure;
    }

    cmd = get_param_string(params, 0);

    if (strcmp(cmd, "reset") == 0)
    {
        print( "Resetting the device...\r\n");
        set_command_result( kSuccess );
        asm_reset();
        return kNone;
    }
    else if (strcmp(cmd, "rtype") == 0)
    {
        sendf(DEBUG_UART, "Last reset type: %d\r\n", last_reset_type());
    }
    else if (strcmp(cmd, "sleep") == 0)
    {
        print("Sleeping\r\n");
        asm_sleep();
    }
    return kSuccess;
}

CommandStatus handle_rtcc(command_params *params)
{
    char *cmd;

    if (params->num_params < 1)
    {
        print( "You must pass a subcommand to the rtcc command.\r\n");
        return kFailure;
    }

    cmd = get_param_string(params, 0);

    //Farm out the subcommand to our subhandlers

    if (strcmp(cmd, "status") == 0)
    {
        unsigned int enabled = rtcc_enabled();


        sendf(DEBUG_UART, "Realtime Clock Status: %s\r\n", enabled? "Enabled" : "Disabled");
        rtcc_datetime time;

        rtcc_get_time(&time);

        sendf(DEBUG_UART, "Current Time: %d/%d/%d %d:%d:%d\r\n", time.month, time.day, time.year, time.hours, time.minutes, time.seconds);
    }
    else if (strcmp(cmd, "set") == 0)
    {
        rtcc_datetime time_spec;
        char *date, *time;
        char temp[3];
        int m;

        temp[2] = '\0';

        if (params->num_params < 3)
        {
            print( "usage: rtcc set mm/dd/yy hh:mm:ss\r\n");
            return kFailure;
        }

        date = get_param_string(params, 1);
        time = get_param_string(params, 2);

        sendf(DEBUG_UART, "Date string: %s\r\nTime string: %s\r\n", date, time);

        time_spec.month = get_2byte_number(date);
        time_spec.day = get_2byte_number(date+3);
        time_spec.year = get_2byte_number(date+6);
        time_spec.hours = get_2byte_number(time);
        time_spec.minutes = get_2byte_number(time+3);
        time_spec.seconds = get_2byte_number(time+6);

        m = time_spec.month;

        sendf(DEBUG_UART, "Input month was: %d\r\n", m);

        rtcc_set_time(&time_spec);
    }
    else if (strcmp(cmd, "enable") == 0)
    {
        enable_rtcc();
        print("RTCC Enabled\r\n");
    }
    else
    {
        sendf(DEBUG_UART, "Unknown rtcc command: %s\r\n", cmd);
        return kFailure;
    }
    return kSuccess;
}

static void rpc_callback(unsigned char status) 
{
    if ( status != kNoMIBError )
    {
        sendf( DEBUG_UART, "An error occurred, code %d.\n", status );
        set_command_result( false );
        return;
    }
    if ( bus_get_returnvalue_length() > 0 )
    {
        if ( bus_get_returnvalue_length() == kIntSize ) { //TODO: MACRO
            print_byte( plist_get_int16(0) ); // TODO: Typed return value
        }
        else
        {
            plist_get_buffer(0)[bus_get_returnvalue_length()] = '\0';
            print(plist_get_buffer(0));
            print("\n");
        }
    } else {
        print( "(success, nothing returned)\n" );
    }
    set_command_result( true );
}

CommandStatus handle_rpc(command_params *params)
{
    int  address, feature, command;

    if (params->num_params < 3) {
        print( "You must pass a device address, feature and a command to execute a RPC.\r\n");
        return kFailure;
    }

    if (!atoi_small( get_param_string(params, 0), &address)) {
        print("Invalid MIB Address");
        return kFailure;
    }

    if ( !atoi_small( get_param_string( params, 1 ), &feature )
      || !atoi_small( get_param_string( params, 2 ), &command ) ) {
        print( "Bad feature or command argument." );
        return kFailure;
    }

    unsigned int argc = params->num_params - 3;
    if ( argc > 3 ) {
        print( "A maximum of 3 params is allowed\n" );
        return kFailure;
    }

    unsigned char param_spec = plist_spec_empty();
    if ( argc > 0 )
    {
        int i;
        int intParams[3];
        char* bufferParam = NULL;
        uint8 intCount = argc;

        for ( i=3; i<params->num_params; ++i) {
            char* str = get_param_string( params, i );
            if ( !atoi_small( str, &intParams[i-2] ) ) {
                if ( i == params->num_params-1 ) {
                    bufferParam = str;
                    intCount--;
                } else {
                    print( "Only one param can be a buffer, and it must be the last param.\n" );
                    return kFailure;
                }
            }
        }

        if ( bufferParam ) {
            param_spec = plist_spec( intCount, strlen(bufferParam) );
            memcpy( plist_get_buffer(intCount), bufferParam, strlen(bufferParam) );
        } else {
            param_spec = plist_ints( intCount );
        }

        for ( i=0; i<intCount; ++i )
            plist_set_int16(i, intParams[i]);
    }
    
    bus_master_rpc_async(rpc_callback, address, feature&0xFF, command&0xFF, param_spec );
    print( "Sending RPC...\n" );
    return kPending;
}
