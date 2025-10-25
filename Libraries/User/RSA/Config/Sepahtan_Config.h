
#ifndef __Sepahtan_Config_H
#define __Sepahtan_Config_H

#include "stm32f4xx.h"

#define FIRMWARE_VERSION    (uint32_t) 980710

//#define USE_FMS		 			  //-> use for can fms is used in program
//#define USE_CAN_Send_Test                      //-> use for debuging in can Send packet Out if defined pa.0 is input key to switch send pocket with value or zero sending

#define USE_IDWDG    			        //-> use for independent watch dog in project

#define USE_ADXL

//#define USE_USBMSC

#define USE_RTC

#define USE_USART_Police	  //-> use for debuging in can monitor packet is recived

//////////////////////////////////////////////////////////////////////////////////
#define USE_GPIO_Other		  // -> Use for ADC-GPIO-OBD-IWG & Other Pripheral
#define USE_SepahtanTicks     //-> use for similar systick generated between Routine & update Variable Reg in project

#define USE_ST7567
#define USE_GLCD
#define USE_BUZZER
//#define USE_GLCD_BLK_PWR_SAVE     

//#define USE_PP_DBG
#define USE_GSM_DBG
#define USE_SPIF_DBG

// Police Protocol debuge print
#ifdef USE_PP_DBG
#define pp_msg(...)              printf(__VA_ARGS__)
#else
#define pp_msg(...)              /*__ASM("nop");*/
#endif

// GSM debuge print
#ifdef USE_GSM_DBG
#define gsm_msg(...)              printf(__VA_ARGS__)
#else
#define gsm_msg(...)              /*__ASM("nop");*/
#endif

// SPIF debuge print
#ifdef USE_SPIF_DBG
#define spif_msg(...)              printf(__VA_ARGS__)
#else
#define spif_msg(...)              /*__ASM("nop");*/
#endif

#endif

/*
revision status & Comment ATT : after 95-11-21
95-11-21 -> reduce sample sound & test play sound with 8kb/s


*/