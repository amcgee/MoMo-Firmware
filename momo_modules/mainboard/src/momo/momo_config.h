#ifndef __momo_config_h__
#define __momo_config_h__

#include "rtcc.h"
#include "flash_queue.h"

#define CONTROLLER_UUID_SIZE 4

typedef struct {
  bool registered;
  uint8 controller_uuid[ CONTROLLER_UUID_SIZE ];
  uint16 allocated_module_id_count;
} MoMoState;

#ifndef MOMO_STATE_CONTROLLER
extern MoMoState current_momo_state;
#endif

void init_momo_config( unsigned int subsection_index );
void reset_momo_state();
void save_momo_state();
void load_momo_state();
void flush_config_to_memory( void* );

#endif