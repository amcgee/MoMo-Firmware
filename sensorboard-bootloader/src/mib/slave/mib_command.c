//mib_command.c 

#include "mib_command.h"
#include "bus_slave.h"
#include <string.h>

//Include command map
#include "commands.h"

extern volatile unsigned char 	mib_buffer[kBusMaxMessageSize];

int find_handler(unsigned char feature, unsigned char cmd)
{
	unsigned int i, num_cmds;
	int found_feat = -1;

	for (i=0; i<kNumFeatures; ++i)
	{
		if (features[i] == feature)
		{
			found_feat = i;
			break;
		}
	}

	if (found_feat == -1)
		return -1;

	num_cmds = commands[found_feat+1] - commands[found_feat];

	if (cmd >= num_cmds)
		return -1;

	return commands[found_feat] + cmd;
}

mib_callback get_handler(int index)
{
	if (index < 0)
		return NULL;

	return handlers[index];
}

volatile MIBParamList *	build_params(int handler_index)
{
	unsigned char spec 			= param_specs[handler_index];
	unsigned char num_params 	= extract_param_count(spec);
	volatile MIBParamList *list = bus_allocate_param_list(num_params);
	unsigned int i;
	
	for (i=0; i<num_params; ++i)
	{
		unsigned char type = extract_param_type(spec, i);

		if (type == kMIBInt16Type)
			list->params[i] = (MIBParameterHeader*)bus_allocate_int_param();
		else if(type == kMIBBufferType && i == (num_params-1))
		{
			//If we have 1 buffer as the last parameter, allocate all the remaining space for it from the mib_buffer
			list->params[i] = (MIBParameterHeader*)bus_allocate_buffer_param(0);
		}
		else
			return NULL; //We cannot allocate buffers that are not the last parameter since we don't know how big they should be
	}

	return list;
}