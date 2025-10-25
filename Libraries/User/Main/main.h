#ifndef __MAIN_H
#define __MAIN_H

#include "stm32g0xx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "System_Utility.h"

/* -------------- Digital IO -------------- */
#include "Digital_IO\Digital_IO.h"

/* -------------- SPIF -------------- */
#include "SPIF\SPIF_spi.h"
#include "SPIF\SPIF_Driver.h"
#include "SPIF\SPIF.h"

/* -------------- GSM -------------- */
#include "GSM\Module_USART.h"
#include "GSM\GSM_Utility.h"
#include "GSM\SSL_Process.h"
#include "GSM\GSM_BT.h"
#include "GSM\SMS_Process.h"
#include "GSM\HTTP_Process.h"

/* -------------- GPS -------------- */
#include "GPS\GPS_USART.h"
#include "GPS\GPS.h"

/* -------------- Timers -------------- */
#include "Timers\System_Timers.h"
#include "Timers\LEDs.h"

/* ---------------- Sippad Protocol ---------------- */
#include "SIP_Protocol\sip_protocol.h"
#include "SIP_Protocol\circular_buffer.h"
#include "SIP_Protocol\system.h"

/* ---------------- Analog ---------------- */
#include "ADC\ADC_Config.h"

/* ---------------- Accelerometer ---------------- */
#include "Accelerometer\accel_i2c.h"
#include "Accelerometer\KX023.h"
#include "Accelerometer\dev_attachment.h"

/* ---------------- Crypto ---------------- */   
#include "Crypto/PKC/rsa.h"
#include "Crypto/certificate/pem_import.h"
#include "RSA_Cryptographic.h"

/* ---------------- Flash ---------------- */
#include "Flash/Flash.h"


#define APPLICATION_ADDRESS                     (uint32_t)0x08004000
//#define ENABLE_ODOMETER                         1

extern uint32_t Bootloader_Password;
extern uint32_t Bootloader_release_date;
extern uint32_t system_Unixtime;
extern uint8_t system_is_initiating;
extern uint32_t                reset_count;;

extern uint8_t timers_does_not_work;

#endif