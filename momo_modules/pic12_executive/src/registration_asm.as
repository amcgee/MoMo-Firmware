;registration_asm.as

#include <xc.inc>
#include "constants.h"
#include "asm_branches.inc"
#include "asm_locations.h"
#include "constants.h"

#define __DEFINES_ONLY__
#include "mib_definitions.h"

ASM_INCLUDE_GLOBALS()

global _mib_to_fsr0, _get_magic, _get_mib_block
global _copy_fsr, _bus_master_rpc_sync
global _load_module_id, _save_module_id

PSECT text_asm_register,local,class=CODE,delta=2

BEGINFUNCTION _register_module
	;check if there's a valid application module loaded
	call 	_get_magic
	skipnel kMIBMagicNumber
		goto app_register
	
	;no app module, send default data
	movlw module_info & 0xFF
	movwf FSR1L
	movlw 1 << 7 | ((module_info >> 8) & 0xFF)
	movwf FSR1H
	goto copy_and_send

	app_register:
	;movlw 0 not needed because w is 0 from xor above
	call _get_mib_block

	copy_and_send:
	call _mib_to_fsr0
	movlw kModuleDescriptorSize
	call _copy_fsr

	;load the current ID into the rest of the buffer, it might be 0
	call _load_module_id

	;mib_buffer now has the module descriptor
	;send it to controller endpoint(40, 0)
	banksel _mib_data
	movlw 42
	movwf BANKMASK(bus_feature)
	
	clrf  BANKMASK(bus_command)
	
	movlw plist_with_buffer(0, kRegistrationPayloadSize)
	movwf BANKMASK(bus_spec)

	movlw kMIBControllerAddress
	call _bus_master_rpc_sync

	;Check if we were successful (return value == 0)
	;return 0 if we failed
	skipel 0
		retlw 0

	;If the call was successful, our address is in the first byte of the 
	;mib buffer, and the id is in the rest
	call _save_module_id

	banksel _mib_data
	movf BANKMASK(mib_buffer),w
	return
ENDFUNCTION _register_module

BEGINREGION module_info
retlw kModuleHWType
retlw kMIBExecutiveOnlyType
retlw (kMIBVersion1 << 4) | 0
db	  'N','O',' ','A','P','P',' '
retlw 1
ENDREGION module_info
