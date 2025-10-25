#ifndef __SSL_PROCESS_H
#define __SSL_PROCESS_H

// ----------------- Include ----------------- //
#include "main.h"

// ----------------- Define ----------------- //

// ----------------- Enum ----------------- //

// ----------------- Structure ----------------- //

// ----------------- Extern ----------------- //
extern const uint8_t sim_stage_timeout[];
extern uint8_t socket_connection_attempts;

// ----------------- Prototype ----------------- //
uint16_t GSM_SSL_routine_pro(uint8_t function_index);


#endif  /*__SSL_PROCESS_H */