#ifndef __SIM800C_SMS_H
#define __SIM800C_SMS_H

/* ---------------- Include ---------------- */
#include "main.h"

/* ---------------- #define ---------------- */
#define SMS_DEFAULT_PASSWORD                            123456
#define MAX_SEND_SMS_SIZE                               300
#define SMS_INFO_TYPE                                   7000
#define ADMIN_NUMBER_MAX_LENGTH                         30
#define SMS_CONTEN_BUFFER_SIZE                          128

/* ---------------- Enum ---------------- */
enum{
  SMS_SETTING                           = 0,
  SMS_INFO,
  SMS_COMMAND,
};

enum{
  SMS_SETTING_CODE_SW_HW_VERSION                         = 100,
  SMS_SETTING_CODE_SERVER,                              // 103
  SMS_SETTING_CODE_APN,                                 // 104
  SMS_SETTING_CODE_REBOOT,                              // 105
  SMS_SETTING_CODE_SAMPLE_RATE_TIME,                    // 106
  SMS_SETTING_CODE_SAMPLE_RATE_METER,                   // 107
  SMS_SETTING_CODE_SAMPLE_RATE_HBT,                     // 108
  SMS_SETTING_CODE_ANGLE_THRESHOLD,                     // 109
  SMS_SETTING_CODE_ERASE_RECORDS,                       // 110
  SMS_SETTING_CODE_FACTORY_DEFAULT,                     // 111
  SMS_SETTING_CODE_HTTP_SERVER,                         // 113
  SMS_SETTING_CODE_DISABLE_HTTP,                        // 114
  SMS_SETTING_CODE_ERRORS,                              // 115
  SMS_SETTING_CODE_DEBUG_MODE,                          // 
};

enum{
  SMS_IO_CODE_IGNITION                                  = 30,
  SMS_IO_CODE_MOVEMENT,
  SMS_IO_CODE_DATA_MODE,
  SMS_IO_CODE_GSM_SIGNAL_LEVEL,
  SMS_IO_CODE_GNSS_STATUS,
  SMS_IO_CODE_DIGITAL_IN_1,
  SMS_IO_CODE_DIGITAL_OUT_1,
  SMS_IO_CODE_PDOP,
  SMS_IO_CODE_HDOP,
  SMS_IO_CODE_EXT_VOLTAGE,
  SMS_IO_CODE_GSM_OPERATOR,
  SMS_IO_CODE_TRIP_ODOMETER,
  SMS_IO_CODE_TOTAL_ODOMETER,
  SMS_IO_CODE_ANALOG_IN_1,
};

enum{
  SMS_EVENT_PRIORITY_CODE_IGNITION                      = 60,
  SMS_EVENT_PRIORITY_CODE_OVERSPEEDING,
  SMS_EVENT_PRIORITY_CODE_IDLING,
  SMS_EVENT_PRIORITY_CODE_UNPLUG,
  SMS_EVENT_PRIORITY_CODE_CRASH,
};

enum{
  SMS_COMMAND_CODE_RESTART_DEVICE                       = 90,
  SMS_COMMAND_CODE_ERASE_EEPROM,
  SMS_COMMAND_CODE_CURRENT_LOCATION,
};

enum{
  SMS_READ_MESSAGE                       = 0,
  SMS_VALIDATE_MESSAGE,                 // 1
  SMS_SET_FORMAT,                       // 2
  SMS_PRE_SEND,                         // 3
  SMS_SEND_CONTENT,                     // 4
  SMS_DELETE_MESSAGE,                   // 5
  SMS_CHECK_UNREAD_MSG,                 // 6
  SMS_DEL_ALL_MSG,                      // 7
  SMS_STAGE_SEEK_OK,                    // 8
};

/* ---------------- Structure ---------------- */


/* ---------------- Extern ---------------- */
extern int16_t GSM_HALT_SMS_CALL_Timer;
extern uint8_t SMS_phone_number[30];
extern uint8_t SMS_phone_num_length;
extern uint8_t SMS_content[128];
extern uint8_t SMS_content_length;
extern uint8_t SMS_send_buffer[MAX_SEND_SMS_SIZE];
extern uint16_t sms_send_length;
extern const uint8_t sim_SMS_stage_timeout_timer[];
extern uint8_t SMS_has_unicode_letters;
extern uint8_t SMS_code_input_format;
extern uint8_t current_sms_index;
extern uint8_t last_sms_index;
extern uint8_t system_has_short_message;

extern uint8_t SMS_stage_action;
extern uint8_t SMS_stage;
extern uint8_t SMS_stage_number_of_retries;


/* ---------------- Prototype ---------------- */
void RI_EXTI_Callback(void);
void SMS_extract_phone_number(uint8_t* phone_num, uint8_t* num_length);
uint16_t SMS_extract_SMS_content(uint8_t* msg, uint8_t* msg_length);
void SMS_parse_short_message(uint8_t* msg, uint8_t msg_length, uint16_t* setting_id);
uint8_t SMS_parse_CMD(uint16_t id, uint8_t* data, uint8_t data_start_index);
uint16_t GSM_send_SMS_routine_pro(uint8_t timeout, uint8_t function_index);


#endif  /* __SIM800C_SMS_H */