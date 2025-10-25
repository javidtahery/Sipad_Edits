#ifndef __SYSTEM_TIMERS_H
#define __SYSTEM_TIMERS_H

#include "main.h"

/* ................ Defines ................*/

#define OneSec_Timer                            TIM6
#define OneSec_Timer_IRQ                        TIM6_IRQn
#define OneSec_Timer_IRQHandler                 TIM6_IRQHandler

#define Fifty_Hz_Timer                          TIM7
#define Fifty_Hz_Timer_IRQ                      TIM7_IRQn
#define Fifty_Hz_Timer_IRQHandler               TIM7_IRQHandler

#define Twentyfive_Hz_Timer                     TIM3
#define Twentyfive_Hz_Timer_IRQ                 TIM3_IRQn
#define Twentyfive_Hz_Timer_IRQHandler          TIM3_IRQHandler

#define ACCEL_CORRECTION_WINDOW                 20


/* ................ Enums ................*/

/* ................ Structures ................*/

/* ................ Externs ................*/
extern uint32_t system_uptime;


/* ................ Prototypes ................*/
void Odo_Timer1_Init(void);
void OneSec_Timer_Init(void);
void OneSec_handler(void);
void Fifty_Hz_Timer_Init(void);
void Fifty_Hz_handler(void);
void Twentyfive_Hz_Timer_Init(void);
void Twentyfive_Hz_handler(void);


#endif