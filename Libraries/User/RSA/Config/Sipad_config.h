#ifndef __SIPAD_Config_H
#define __SIPAD_Config_H

#include "main.h"

#include "stm32f4xx.h"

#define SETTING_METADATA                   236710
#define FIRMWARE_PACK_VERSION    (uint32_t)10000000
//#define FIRMWARE_VERSION         (uint32_t)0x02310004
#define SIPAD_DEVICE_VER                   0x0004
//#define SEP2SIPID                          0x31 
#define SIP2XXPID                          0x32 

/*use to locate fw in header update pack
  FW_OFFSET = N * 28
  Sep2sip N=0
  SIP2XX  N=1
  .... 
*/
#define FW_OFFSET 28  

//#define USE_SPIF_DBG
#define USE_SP_DBG
#define DEBUG
#define DEBUG_FOTA
//#define USE_ECU_DBG
//#define USE_ADXL_DBG
#define USE_GNSS_DBG
//#define USE_GSM_DBG
#define USE_EVENT_DBG

#define USE_BTDUMY_MAC

#define USE_GPIO_Other
#define USE_FMS
#define USE_IDWDG    			        //-> use for independent watch dog in project
//#define USE_ADXL
#define USE_RTC
#define USE_GPIO_Other		                // -> Use for ADC-GPIO-OBD-IWG & Other Pripheral
#define USE_SepahtanTicks                       //-> use for similar systick generated between Routine & update Variable Reg in project
//#define USE_ST7567
//#define USE_GLCD
//#define USE_BUZZER
//#define USE_GLCD_BLK_PWR_SAVE

// Event Production Redundancy
#define UNALLOWED_SPEED_REDUNDANCY              60
#define NOT_FIXED_REDUNDANCY                    60
#define NO_GSM_SIGNAL_REDUNDANCY                60
#define LOW_BATT_REDUNDANCY                     15

#define GPS_SHORT_CIR_THR                       1000
#define GPS_OPEN_CIR_THR                        20

//not used
#define BATT_Critical_VALUE (uint8_t)           37  //3.7v

#define NTPSERVER      "ntp.day.ir"
#define CACERT_STR     "UFS:CA.pem"
#define CLIENTCERT_STR "UFS:CC.pem"
#define CLIENTKEY_STR  "UFS:CK.pem"


//#define USE_USART_DEBUG	  //-> use for debuging in can monitor packet is recived

//////////////////////////////////////////////////////////////////////////////////

#define USE_GSM_FORWARD_DBG
//#define USE_SP_DBG
//#define USE_GSM_DBG
#define USE_SPIF_DBG
//#define USE_PP_DBG

// Police Protocol debuge print
#ifdef USE_SP_DBG
#define sp_msg(...)              printf(__VA_ARGS__)
#else
#define sp_msg(...)              /*__ASM("nop");*/
#endif

// GSM debug print
#ifdef USE_GSM_DBG
#define gsm_msg(...)              printf(__VA_ARGS__)
#else
#define gsm_msg(...)              /*__ASM("nop");*/
#endif

// GNSS debug print
#ifdef USE_GNSS_DBG
#define gnss_msg(...)              printf(__VA_ARGS__)
#else
#define gnss_msg(...)              /*__ASM("nop");*/
#endif

// SPIF debug print
#ifdef USE_SPIF_DBG
#define spif_msg(...)              printf(__VA_ARGS__)
#else
#define spif_msg(...)              /*__ASM("nop");*/
#endif

// ADXL debug print
#ifdef USE_adxl_DBG
#define adxl_msg(...)              printf(__VA_ARGS__)
#else
#define adxl_msg(...)              /*__ASM("nop");*/
#endif

// CAN debug print
#ifdef USE_CAN_DBG
#define can_msg(...)              printf(__VA_ARGS__)
#else
#define can_msg(...)              /*__ASM("nop");*/
#endif

// io debug print
#ifdef USE_IO_DBG
#define io_msg(...)              printf(__VA_ARGS__)
#else
#define io_msg(...)              /*__ASM("nop");*/
#endif


#ifdef DEBUG_FOTA
#define Trace_printf(...)      printf(__VA_ARGS__)
#else
#define Trace_printf(...)      __ASM("nop");
#endif

#endif

