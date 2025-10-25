#ifndef __GSM_UTILITY_H
#define __GSM_UTILITY_H

// ----------------- Include ----------------- //
#include "main.h"

// ----------------- Define ----------------- //
#define GSM_MSG_DEFAULT_TIMEOUT         1000
#define GSM_TIMER_DEFAULT               250 

#define Sim_USART_BUFFERSIZE            350 


// ----------------- Enum ----------------- //
enum{
  GSM_TIMEOUT                           = 0,
  GSM_OK,
  GSM_ERROR,
  GSM_HALT,
  GSM_WAITING,
  GSM_SEND_OK,
  GSM_SEND_FAIL,
  GSM_WAITINGMSG,
  GSM_CME_ERROR,
  GSM_CMS_ERROR,
  GSM_CPIN,
  GSM_CGREG,
  GSM_CREG,
  GSM_COPS,
  GSM_CSQ,
  GSM_IMEI,
  GSM_CMT,
  GSM_CMTI,
  GSM_CMGS,
  GSM_CMGR,
  GSM_CPMS,
  GSM_CALL,
  GSM_HANGUP,
  GSM_CLCC,
  GSM_DATA_MODE,
  GSM_SIM_PIN,
  GSM_SIM_PUK,
  GSM_SIM_PIN2,
  GSM_SIM_PUK2,
  GSM_QCFG,
  GSM_QGBAND,
  GSM_QLTS,
  GSM_LF,       // LineFeed
  SMS_RECEIVED,
  // TCP
  GSM_CONNECT_OK,
  GSM_CONNECT_FAIL,
  GSM_QINDI,
  GSM_GET_TCP_RESP,
  GSM_READ_TCP_RESP,
  GSM_QIMUX,
  // HTTP
  GSM_HTTP_DL,
  // SSL
  GSM_CONNECTION_STATUS,
  GSM_QIFGCNT,
  GSM_QSECREAD,
  GSM_CONNECT,
  GSM_QSECWRITE,
  GSM_SSLOPEN,
  GSM_RECV_SERVER_RESP,
  GSM_READ_SERVER_RESP,
  GSM_CLOSE_OK,
  GSM_ERR_CLOSED,
  GSM_IPSHUT,
  GSM_DEACT_PDP,
  // FTP
  GSM_FTP_STAT,
  GSM_FTP_OPEN,
  GSM_FTP_PATH,
  GSM_FTP_NLST,
  GSM_FTP_SIZE,
  GSM_FTP_GET,
  GSM_FTP_CLOSE,
  // File System
  GSM_FS_OPEN,
  //
  GSM_SAPBR,
  GSM_CGATT,
  // GSM not related
  GSM_UNSPECIFIED,
  GSM_SEND_NEXT_PCKT,
  GSM_RESEND_PCKT,
  GSM_FOTA_SAVED_FAILED,
  GSM_FOTA_SAVED,
  GSM_BT_PAIR_REQ,
  GSM_HTTP_ERROR,
  GSM_HTTP_CHECK_NEXT_REQUEST,
  GSM_HTTP_DONE_PROCESS,
};

enum{
  GSM_STAGE_DIS_POWER                            = 0,
  GSM_STAGE_EN_POWER,//-------------------------// 1
  SIM_STAGE_MODULE_OFF,//-----------------------// 2
  SIM_STAGE_MODULE_ON,//------------------------// 3
  SIM_STAGE_RESET_MODULE,//---------------------// 4
  SIM_STAGE_TEST_SERIAL,//----------------------// 5
  SIM_STAGE_DISABLE_ECHO,//---------------------// 6
  SIM_STAGE_ENABLE_CMEE,//----------------------// 7
  SIM_STAGE_CFG_CREG,//-------------------------// 8
  SIM_STAGE_CFG_URC,//--------------------------// 9
  SIM_STAGE_CFG_SMS_MODE,//---------------------// 10
  SIM_STAGE_CFG_SMS_URC,//----------------------// 11
  SIM_STAGE_CFG_RI,//---------------------------// 12
  SIM_STAGE_CFG_CALL_CARRIER,//-----------------// 13
  SIM_STAGE_DEL_SMS,//--------------------------// 14
  SIM_STAGE_SAVE_SET,//-------------------------// 15
  SIM_STAGE_QUERY_IMEI,//-----------------------// 16
  SIM_STAGE_QUERY_SIMCARD,//--------------------// 17
  SIM_STAGE_QUERY_NETWORK,//--------------------// 18
  SIM_STAGE_QUERY_NETWORK_PROVIDER,//-----------// 19
  SIM_STAGE_QUERY_SIGNAL_QUALITY,//-------------// 20
  SIM_STAGE_QUERY_FOREGROUND_CONTEXT,//---------// 21
  SIM_STAGE_CFG_FOREGROUND_CONTEXT,//-----------// 22
  SIM_STAGE_QUERY_MUX,//------------------------// 23
  SIM_STAGE_CONFIG_MUX,//-----------------------// 24
  SIM_STAGE_QUERY_GPRS_ATTACHMENT,//------------// 25
  SIM_STAGE_CFG_GPRS_ATTACHMENT,//--------------// 26
  SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS,//--// 27
  SIM_STAGE_QUERY_RAM_CA0_FILE,//---------------// 28
  SIM_STAGE_QUERY_RAM_CC0_FILE,//---------------// 29
  SIM_STAGE_QUERY_RAM_CK0_FILE,//---------------// 30
  SIM_STAGE_DELETE_RAM_CA0_FILE,//--------------// 31
  SIM_STAGE_DELETE_RAM_CC0_FILE,//--------------// 32
  SIM_STAGE_DELETE_RAM_CK0_FILE,//--------------// 33
  SIM_STAGE_PRE_UPLOAD_RAM_CA0_FILE,//----------// 34
  SIM_STAGE_PRE_UPLOAD_RAM_CC0_FILE,//----------// 35
  SIM_STAGE_PRE_UPLOAD_RAM_CK0_FILE,//----------// 36
  SIM_STAGE_UPLOAD_RAM_CA0_FILE,//--------------// 37
  SIM_STAGE_UPLOAD_RAM_CC0_FILE,//--------------// 38
  SIM_STAGE_UPLOAD_RAM_CK0_FILE,//--------------// 39
  SIM_STAGE_CONFIG_APN,//-----------------------// 40
  SIM_STAGE_CONNECT_GPRS,//---------------------// 41
  SIM_STAGE_ACTIV_FGCNT,//----------------------// 42
  SIM_STAGE_QUERY_LOCAL_IP,//-------------------// 43
  SIM_STAGE_CFG_SSL_VERSION,//------------------// 44
  SIM_STAGE_CFG_SSL_CIPHER,//-------------------// 45
  SIM_STAGE_CFG_SSL_SEC_LVL,//------------------// 46
  SIM_STAGE_CFG_SSL_IGNORE_TIME,//--------------// 47
  SIM_STAGE_CFG_SSL_CA_CERT,//------------------// 48
  SIM_STAGE_CFG_SSL_CC_CERT,//------------------// 49
  SIM_STAGE_CFG_SSL_CK_CERT,//------------------// 50
  SIM_STAGE_OPEN_SOCKET,//----------------------// 51
  SIM_STAGE_PRE_SEND_DATA,//--------------------// 52
  SIM_STAGE_SEND_DATA,//------------------------// 53
  SIM_STAGE_RECEIVE_SERVER_RESPONSE,//----------// 54
  SIM_STAGE_READ_SERVER_RESPONSE,//-------------// 55
  SIM_STAGE_CLOSE_SOCKET,//---------------------// 56
  SIM_STAGE_CIPSHUT,//--------------------------// 57
  SIM_STAGE_SEEK_OK,//--------------------------// 58
};

enum{
  SIM_SEND_REQ                          = 0,
  SIM_RCV_RESP,
};

enum{
  CMEE_NO_SIMCARD               = 10,
  CMEE_SM_NOT_READY             = 3517,
  CMEE_INVALID_INPUT_VAL        = 3765,
  CMEE_HTTP_GET_NO_REQ          = 3804,
  CMEE_HTTP_DNS_ERROR           = 3813,
  CMEE_HTTP_READ_TIMEOUT        = 3821,
  CMEE_HTTP_RESP_FAILED         = 3822,
  CMEE_WAIT_HTTP_RESP_TO        = 3827,
  CMEE_HTTP_NEED_RELOCATION     = 3829,
  CMEE_FILE_NOT_FOUND           = 4010,
  CMEE_INVALID_PARAM            = 4013,
  CMEE_ACCESS_DENIED            = 4016,
  CMEE_FILE_TOO_LARGE           = 4017,
};

enum{
  CMSE_INVALID_PARAM            = 321,
};
// ----------------- Structure ----------------- //
typedef struct{
  uint8_t stage_action;
  uint8_t stage;
  uint8_t prev_stage;
  uint8_t next_stage;
  uint8_t simcard_available;
  uint8_t nw_status;
  uint8_t signal_quality;
  uint8_t internet_connection;
  uint8_t number_of_retries_command;
  uint16_t nw_provider;
}gsm_param_typeDef;

// ----------------- Extern ----------------- //
extern gsm_param_typeDef GSM_Parameters;

extern uint8_t Rx_line_buffer[Sim_USART_BUFFERSIZE];
extern uint16_t line_buffer_rd_index;
extern uint16_t line_buffer_counter;

extern int16_t GSM_HALT_Timer;
extern __IO uint32_t gsm_pt_timeout;
extern __IO uint32_t gsm_sr_timeout;
extern uint8_t GSM_QoS_Timer;
extern uint8_t GSM_module_is_configured;

extern int16_t GSM_Creg_two_hour_counter;
extern uint8_t disable_any_gsm_activity;
extern uint8_t GSM_network_search_counter;

// ----------------- Prototype ----------------- //
uint8_t convert_char_to_int(char data);

void GSM_power_on(void);
void GSM_power_off(void);
void GSM_clear_RX_buffer(void);
uint8_t array_length(uint8_t* array);
uint16_t GSM_get_line(char str[], uint16_t limit, uint8_t* error);
uint64_t str_to_llint(char* str, uint8_t number_char);
uint16_t GSM_get_number_from_line_buffer(char terminator, uint64_t *num);
uint8_t GSM_get_string_from_buffer(uint8_t* buffer, uint8_t* string, uint8_t start_index, char terminator, uint8_t length_limit);
uint8_t GSM_read_until_from_buffer(uint8_t* data, uint8_t start_index, char terminator, uint8_t max_read_size);
uint8_t GSM_get_number_from_buffer(uint8_t* data, uint8_t start_index, char terminator, uint8_t max_read_size, uint64_t *num);
uint16_t GSM_read_until_from_line_buffer(char terminator);
uint8_t GSM_add_char_number_to_buffer(uint8_t* data, uint16_t* start_index, uint64_t number);
uint8_t GSM_get_upper_case_char(uint8_t character);
uint16_t GSM_check_msg(int16_t* error);
void GSM_parse_IMEI(void);
void GSM_parse_CPIN(void);
void GSM_parse_CREG(void);
void GSM_parse_IMSI(void);
void GSM_parse_CSQ(void);
void GSM_parse_Connection_STATUS(void);
void GSM_Send_Set_APN_cmd(void);
uint8_t GSM_handle_server_response(uint16_t response_length);
void GSM_send_cmd_to_GSM(void);
void Increment_GSM_rx_rd_index(uint16_t inc_mount);


#endif  /*__GSM_UTILITY_H */