#ifndef __SYSTEM_UTILITY_H
#define __SYSTEM_UTILITY_H

#include "main.h"

#define IWDG_ReloadCounter()            IWDG->KR = 0x0000AAAA
#define INITIAL_DEVICE_VERSION          0x02310004      // 02.31.00.04
#define APP_RELEASE_DATE                0x00250426      // 00YYMMDD

extern const char Software_version[];
extern const char Hardware_Version[];

extern __IO uint16_t cfg_timeout_counter;
extern __IO uint32_t GPS_rx_timeout;
extern uint8_t crystal_is_failed;


void System_Config(void);
void Sys_cfg_clock_HSI(void);
void Set_zero(uint8_t* data, uint16_t data_size);
uint8_t Check_bit(uint8_t data, uint8_t bit);
uint8_t Set_bit(uint8_t data, uint8_t bit);
uint8_t Reset_bit(uint8_t data, uint8_t bit);
void Tick_decrement(void);
void Delay(uint32_t time);

#endif