#include "module_identification.h"
#include "mib12_api.h"

eeprom uint8 controller_uuid[4] = {0x0, 0x0, 0x0, 0x0};
eeprom uint8 module_id[2] = {0x0, 0x0};

void load_module_id()
{
    // TODO: This should probably be in assembly?
    *(mib_buffer+kModuleDescriptorSize+0) = controller_uuid[0];
    *(mib_buffer+kModuleDescriptorSize+1) = controller_uuid[1];
    *(mib_buffer+kModuleDescriptorSize+2) = controller_uuid[2];
    *(mib_buffer+kModuleDescriptorSize+3) = controller_uuid[3];
    *(mib_buffer+kModuleDescriptorSize+4) = module_id[0];
    *(mib_buffer+kModuleDescriptorSize+5) = module_id[1];
}
void save_module_id()
{
	if ( mib_buffer_length() < 7 )
		return;

	//The first byte is the volatile module address, the rest is the (possibly new) durable ID
	controller_uuid[0] = *(mib_buffer+1);
	controller_uuid[1] = *(mib_buffer+2);
	controller_uuid[2] = *(mib_buffer+3);
	controller_uuid[3] = *(mib_buffer+4);
	module_id[0] = *(mib_buffer+5);  
	module_id[1] = *(mib_buffer+6);
}