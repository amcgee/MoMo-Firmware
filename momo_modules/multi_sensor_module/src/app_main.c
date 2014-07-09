//app_main.c

#include "platform.h"
#include "watchdog.h"
#include "sensor_defines.h"
#include "port.h"
#include "sample.h"
#include "mib12_api.h"
#include "log.h"
#include "digital_amp.h"
#include "watchdog.h"
#include "state.h"
#include "pulse.h" 
#include "alarm_repeat_times.h"

MultiSensorState state;
extern unsigned int adc_result;

#define VENTURI_SAMPLE_COUNT 50

void task(void)
{
	if ( state.push_pending )
	{
		bus_master_begin_rpc();
		mib_buffer[0] = mib_address;
		mib_buffer[1] = 0;

		mib_buffer[2] = 0;
		mib_buffer[3] = 0; //metadata

		uint16 value = read_venturi();

		mib_buffer[4] = value & 0xFF;
		mib_buffer[5] = (value >> 8) & 0xFF;
		mib_buffer[6] = 0;
		mib_buffer[7] = 0;

		bus_master_prepare_rpc(70, 0, plist_with_buffer(2, 4));
		bus_master_send_rpc(8);

		aggregate_counter = 0;
	}
}

void interrupt_handler(void)
{

}

void initialize(void)
{
	PIN_DIR(VOLT1, INPUT);
	PIN_TYPE(VOLT1, ANALOG);

	PIN_DIR(VOLT2, INPUT);
	PIN_TYPE(VOLT2, ANALOG);
	
	PIN_DIR(VOLT3, INPUT);
	PIN_TYPE(VOLT3, ANALOG);
	
	LATCH(AN_POWER) = 0;
	//PIN_TYPE(AN_POWER, DIGITAL); AN_POWER is digital only (A6)
	PIN_DIR(AN_POWER, OUTPUT);

	LATCH(AN_SELECT) = 0;
	PIN_DIR(AN_SELECT, OUTPUT);
	PIN_TYPE(AN_SELECT, DIGITAL);

	LATCH(AN_INVERT) = 0;
	PIN_DIR(AN_INVERT, OUTPUT);
	PIN_TYPE(AN_INVERT, DIGITAL);

	LATCH(AN_PROG) = 0;
	PIN_DIR(AN_PROG, OUTPUT);
	PIN_TYPE(AN_PROG, DIGITAL);

	PIN_TYPE(AN_VOLTAGE, ANALOG);
	PIN_DIR(AN_VOLTAGE, INPUT);

	//PIN_TYPE(PULSE_IN, DIGITAL); A7 is digital only
	PIN_DIR(PULSE_IN, INPUT);

	damp_init();
	state.combined_state = 0;

	aggregate_counter = 0;

	bus_master_begin_rpc();

	mib_buffer[0] = mib_address;
	mib_buffer[1] = 0;

	mib_buffer[2] = 8;
	mib_buffer[3] = 20;

	mib_buffer[4] = kEvery10Seconds;
	mib_buffer[5] = 0;
	bus_master_prepare_rpc(43, 0, plist_ints(3));

	bus_master_send_rpc(8);
}

void main()
{

}

void check_v1()
{
	sample_v1();

	mib_buffer[0] = (adc_result & 0xFF);
	mib_buffer[1] = (adc_result >> 8) & 0xFF;

	bus_slave_setreturn(pack_return_status(0, 2));
}

void check_v2()
{
	sample_v2();

	mib_buffer[0] = (adc_result & 0xFF);
	mib_buffer[1] = (adc_result >> 8) & 0xFF;

	bus_slave_setreturn(pack_return_status(0, 2));
}

uint16 read_venturi(void)
{
	uint8 i = VENTURI_SAMPLE_COUNT;
	uint16 avg = 0;
	uint16 offset = 0;

	damp_set_parameter( kInvertInputParamter, false );
	damp_enable();
	sample_analog(); // +signal + offset
	damp_disable();
	offset = adc_result;
	
	damp_set_parameter( kInvertInputParamter, true );
	damp_enable();
	sample_analog(); // -signal + offset
	damp_disable();
	offset += adc_result; // = 2 * offset
	offset /= 2;

	damp_set_parameter( kInvertInputParamter, false );
	damp_enable();
	i = VENTURI_SAMPLE_COUNT;
	while ( i > 0 )
	{
		sample_analog();
		avg += adc_result;
	}
	damp_disable();
	return ( avg / VENTURI_SAMPLE_COUNT ) - offset;
}

void check_v3()
{
	sample_v3();

	mib_buffer[0] = (adc_result & 0xFF);
	mib_buffer[1] = (adc_result >> 8) & 0xFF;

	bus_slave_setreturn(pack_return_status(0, 2));
}

void acquire_voltage()
{
	damp_enable();
	sample_analog();
	damp_disable();

	mib_buffer[0] = (adc_result & 0xFF);
	mib_buffer[1] = (adc_result >> 8) & 0xFF;

	bus_slave_setreturn(pack_return_status(0, 2));
}

void set_damp_parameter()
{
	if (mib_buffer[0] >= kNumParameters)
	{
		bus_slave_setreturn(pack_return_status(6, 0));
		return;
	}

	damp_set_parameter((AmplifierSetting)mib_buffer[0], mib_buffer[2]);
	bus_slave_setreturn(pack_return_status(0, 0));
}

void power_analog()
{
	set_analog_power(mib_buffer[0]);
	bus_slave_setreturn(pack_return_status(0, 0));
}

void acquire_pulse()
{
	state.acquire_pulse = 1;
	bus_slave_setreturn(pack_return_status(0, 0));
}

void read_pulses()
{
	mib_buffer[0] = pulse_count() & 0xFF;
	mib_buffer[1] = pulse_count() >> 8;

	bus_slave_setreturn(pack_return_status(0, 2));
}

void scheduled_callback()
{
	if ( state.acquire_pulse == 0 )
	{
		state.push_pending = 1;
	}
}