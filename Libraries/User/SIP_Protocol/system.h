#ifndef __SYSTEM_H
#define __SYSTEM_H

/* ---------------- Include ---------------- */
#include "main.h"


/* ---------------- Defines ---------------- */
#define ACCEL_EXECUTE_RECOVER_COUNTER_THRESH            60
#define SYSTEM_LOW_BATTERY_STATE                        3700    // mV
#define SYSTEM_BATTERY_CRITICAL_STATE                   3400    // mV

/* ................ Enums ................*/
typedef enum{
  SYSTEM_INITIAL        = 0,
  SYSTEM_RUNNING,       // 1
  SYSTEM_SLEEP,         // 2
  SYSTEM_ON_BATTERY,    // 3
}system_states;


/* ---------------- Structure ---------------- */
typedef struct{
  uint8_t accel_error;
  uint8_t gps_error;
  uint8_t IMEI_error;
  uint8_t certificate_error;
  uint8_t no_certificate;
  uint8_t server_error;
  uint8_t socket_error;
  uint8_t SPIF_error;
  uint8_t setting_error;
  uint8_t internet_error;
  uint8_t internet_error_count;
  uint8_t socket_error_count;
  uint8_t server_error_count;
}sys_err_typedef;


/* ---------------- Extern ---------------- */
extern sys_err_typedef system_error;

extern uint8_t system_has_error;
extern uint8_t recover_Accel_error;
extern uint16_t recover_accel_error_counter;
extern uint8_t system_pause_change_state_counter;
extern system_states system_current_state;


/* ---------------- Prototype ---------------- */
void System_check_errors(void);
void System_check_state(void);


#endif  /* __SYSTEM_H */