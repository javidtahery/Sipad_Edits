#ifndef __DIGITAL_IO_H
#define __DIGITAL_IO_H

/* --------------------- Include --------------------- */
#include "main.h"


/* --------------------- Define --------------------- */
#define IO_SYSTEM_LED_GPIO                      GPIOD
#define IO_SYSTEM_LED_PIN                       GPIO_BSRR_BS0

#define IO_GSM_LED_GPIO                         GPIOA
#define IO_GSM_LED_PIN                          GPIO_BSRR_BS15

#define IO_GPS_LED_GPIO                         GPIOD
#define IO_GPS_LED_PIN                          GPIO_BSRR_BS1

#define IO_DIGITAL_OUT_GPIO                     GPIOA
#define IO_DIGITAL_OUT_PIN                      GPIO_BSRR_BS15

#define IO_DIGITAL_IN_1_GPIO                    GPIOA           // Change if digital in 1 available
#define IO_DIGITAL_IN_1_PIN                     GPIO_IDR_ID11

#define IO_DIGITAL_IN_2_GPIO                    GPIOA           // Change if digital in 2 available
#define IO_DIGITAL_IN_2_PIN                     GPIO_IDR_ID7

#define IO_ACC_GPIO                             GPIOA
#define IO_ACC_PIN                              GPIO_IDR_ID11

#define IO_VCC_DIGITAL_GPIO                     GPIOA
#define IO_VCC_DIGITAL_PIN                      GPIO_IDR_ID7

#define SYS_LED_OFF                             IO_SYSTEM_LED_GPIO->BRR = IO_SYSTEM_LED_PIN
#define SYS_LED_ON                              IO_SYSTEM_LED_GPIO->BSRR = IO_SYSTEM_LED_PIN
#define GPS_LED_OFF                             IO_GPS_LED_GPIO->BRR = IO_GPS_LED_PIN
#define GPS_LED_ON                              IO_GPS_LED_GPIO->BSRR = IO_GPS_LED_PIN
#define GSM_LED_OFF                             IO_GSM_LED_GPIO->BRR = IO_GSM_LED_PIN
#define GSM_LED_ON                              IO_GSM_LED_GPIO->BSRR = IO_GSM_LED_PIN
#define DIGITAL_OUT_OFF                         IO_DIGITAL_OUT_GPIO->BRR = IO_DIGITAL_OUT_PIN
#define DIGITAL_OUT_ON                          IO_DIGITAL_OUT_GPIO->BSRR = IO_DIGITAL_OUT_PIN

#define ON                                      SET
#define OFF                                     RESET

#define SYSTICK_CHECK_IO_INTERVAL               300
#define NUMBER_OF_DIGITAL_INPUTS                2
#define MAXIMUM_NUM_IO_SAMPLING                 4

#define DIGITAL_IO_MAIN_PWR                     0
#define DIGITAL_IO_ACC                          1
#define DIGITAL_IO_TAMPER                       2
#define DIGITAL_IO_D_IN_1                       3
#define DIGITAL_IO_D_IN_2                       4
#define DIGITAL_IO_D_OUT_1                      5

/* --------------------- Structure --------------------- */
typedef struct{
  uint8_t       ignition;
  uint8_t       vcc_digital;
  uint8_t       digital_out;
}io_typedef;

typedef struct{
  uint8_t       high_state_count;
  uint8_t       low_state_count;
}io_states_typedef;
/* --------------------- Enum --------------------- */


/* --------------------- Extern --------------------- */
extern io_typedef       IO_Digital;

extern __IO uint16_t systick_check_IO_counter;
extern uint8_t digital_in_1_changed;
extern uint8_t digital_in_2_changed;
extern uint8_t ignition_changed;
extern uint8_t vcc_digital_changed;

/* --------------------- Prototype --------------------- */
void IO_cfg_Digital_IOs(void);
void IO_ON_OFF_Relay(uint8_t state);
void IO_Check_IOs(void);
void IO_Ignition_Changed(void);
void IO_vcc_digital_Changed(void);
void IO_digital_in_1_Changed(void);
void IO_digital_in_2_Changed(void);


#endif  /* __DIGITAL_IO_H */