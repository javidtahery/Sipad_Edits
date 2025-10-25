#include "SSL_Process.h"

uint8_t server_packet_retry_number = 0;
uint8_t socket_connection_attempts = 0;

const uint8_t sim_stage_timeout[] = {1,         // Disable Power
                                     1,         // Enable Power
                                     1,         // Module OFF
                                     1,         // Module ON
                                     1,         // Reset Module
                                     1,         // Test Serial
                                     1,         // Disable ECHO
                                     1,         // Enable CMEE
                                     1,         // cfg CREG
                                     1,         // cfg URC
                                     1,         // cfg SMS mode
                                     1,         // cfg SMS URC
                                     1,         // cfg RI
                                     1,         // cfg BUSY LINE
                                     1,         // DEL SMSs
                                     1,         // Save Setting
                                     1,         // Query IMEI
                                     5,         // Query CPIN
                                     1,         // Query Network
                                     1,         // Query IMSI
                                     1,         // Query CSQ
                                     1,         // Query foreground context
                                     1,         // cfg foreground context
                                     1,         // Query MUX
                                     1,         // cfg MUX
                                     1,         // Query GPRS Attachment
                                     30,        // cfg GPRS Attachment
                                     5,         // Query SSL Connection Status
                                     2,         // Query CA0 File
                                     2,         // Query CC0 File
                                     2,         // Query CK0 File
                                     2,         // Delete CA0 File
                                     2,         // Delete CC0 File
                                     2,         // Delete CK0 File
                                     2,         // Pre Upload CA0 File
                                     2,         // Pre Upload CC0 File
                                     2,         // Pre Upload CK0 File
                                     2,         // Upload CA0 File
                                     2,         // Upload CC0 File
                                     2,         // Upload CK0 File
                                     5,         // cfg APN
                                     120,       // Connect GPRS
                                     120,       // Activate FGCNT
                                     30,        // Query Local IP
                                     1,         // cfg SSL Version
                                     1,         // cfg SSL Cipher
                                     1,         // cfg SSL Sec lvl
                                     1,         // cfg SSL Ignore Time
                                     1,         // cfg SSL CA0
                                     1,         // cfg SSL CC0
                                     1,         // cfg SSL CK0
                                     95,        // Open Socket
                                     5,         // Pre Send Data
                                     35,        // Send Data
                                     35,        // Receive Server Response
                                     5,         // Read Server Response
                                     35,        // Close Socket
                                     30,        // CIPSHUT
                                     2,         // Seek OK
                                     };

const uint8_t sim_stage_max_number_of_retries[] = { 1,          // Disable Power
                                                    1,          // Enable Power
                                                    1,          // Module OFF
                                                    1,          // Module ON
                                                    3,          // Reset Module
                                                    3,          // Test Serial
                                                    3,          // Disable ECHO
                                                    3,          // Enable CMEE
                                                    3,          // cfg CREG
                                                    3,          // cfg URC
                                                    3,          // cfg SMS mode
                                                    3,          // cfg SMS URC
                                                    3,          // cfg RI
                                                    3,          // cfg BUSY LINE
                                                    3,          // DEL SMSs
                                                    3,          // Save Setting
                                                    3,          // Query IMEI
                                                    3,          // Query CPIN
                                                    20,          // Query Network
                                                    3,          // Query IMSI
                                                    3,          // Query CSQ
                                                    3,          // Query foreground context
                                                    3,          // cfg foreground context
                                                    3,          // Query MUX
                                                    3,          // cfg MUX
                                                    1,          // Query GPRS Attachment
                                                    2,          // cfg GPRS Attachment
                                                    5,          // Query SSL Connection Status
                                                    2,          // Query CA0 File
                                                    2,          // Query CC0 File
                                                    2,          // Query CK0 File
                                                    2,          // Delete CA0 File
                                                    2,          // Delete CC0 File
                                                    2,          // Delete CK0 File
                                                    2,          // Pre Upload CA0 File
                                                    2,          // Pre Upload CC0 File
                                                    2,          // Pre Upload CK0 File
                                                    2,          // Upload CA0 File
                                                    2,          // Upload CC0 File
                                                    2,          // Upload CK0 File
                                                    3,          // cfg APN
                                                    2,          // Connect GPRS
                                                    3,          // Activate FGCNT
                                                    3,          // Query Local IP
                                                    2,          // cfg SSL Version
                                                    2,          // cfg SSL Cipher
                                                    2,          // cfg SSL Sec lvl
                                                    2,          // cfg SSL Ignore Time
                                                    2,          // cfg SSL CA0
                                                    2,          // cfg SSL CC0
                                                    2,          // cfg SSL CK0
                                                    3,          // Open Socket
                                                    3,          // Pre Send Data
                                                    2,          // Send Data
                                                    3,          // Receive Server Response
                                                    3,          // Read Server Response
                                                    2,          // Close Socket
                                                    2,          // CIPSHUT
                                                    1,          // Seek OK
                                                   };

const uint8_t sim_stage_retries_redirect[] = {GSM_STAGE_DIS_POWER,              // Disable Power
                                              GSM_STAGE_DIS_POWER,              // Enable Power
                                              SIM_STAGE_MODULE_OFF,             // Module OFF
                                              SIM_STAGE_MODULE_OFF,             // Module ON
                                              GSM_STAGE_DIS_POWER,              // Reset Module
                                              GSM_STAGE_DIS_POWER,              // Test Serial
                                              SIM_STAGE_RESET_MODULE,           // Disable ECHO
                                              SIM_STAGE_RESET_MODULE,           // Enable CMEE
                                              SIM_STAGE_RESET_MODULE,           // cfg CREG
                                              SIM_STAGE_RESET_MODULE,           // cfg URC
                                              SIM_STAGE_RESET_MODULE,           // cfg SMS mode
                                              SIM_STAGE_RESET_MODULE,           // cfg SMS URC
                                              SIM_STAGE_RESET_MODULE,           // cfg RI
                                              SIM_STAGE_RESET_MODULE,           // cfg BUSY LINE
                                              SIM_STAGE_RESET_MODULE,           // DEL SMSs
                                              SIM_STAGE_RESET_MODULE,           // Save Setting
                                              SIM_STAGE_RESET_MODULE,           // Query IMEI
                                              SIM_STAGE_RESET_MODULE,           // Query CPIN
                                              GSM_STAGE_DIS_POWER,              // Query Network
                                              SIM_STAGE_RESET_MODULE,           // Query IMSI
                                              SIM_STAGE_RESET_MODULE,           // Query CSQ
                                              SIM_STAGE_RESET_MODULE,           // Query foreground context
                                              SIM_STAGE_RESET_MODULE,           // cfg foreground context
                                              SIM_STAGE_RESET_MODULE,           // Query MUX
                                              SIM_STAGE_RESET_MODULE,           // cfg MUX
                                              SIM_STAGE_RESET_MODULE,           // Query GPRS Attachment
                                              SIM_STAGE_RESET_MODULE,           // cfg GPRS Attachment
                                              SIM_STAGE_RESET_MODULE,           // Query SSL Connection Status
                                              SIM_STAGE_RESET_MODULE,           // Query CA0 File
                                              SIM_STAGE_RESET_MODULE,           // Query CC0 File
                                              SIM_STAGE_RESET_MODULE,           // Query CK0 File
                                              SIM_STAGE_RESET_MODULE,           // Delete CA0 File
                                              SIM_STAGE_RESET_MODULE,           // Delete CC0 File
                                              SIM_STAGE_RESET_MODULE,           // Delete CK0 File
                                              SIM_STAGE_RESET_MODULE,           // Pre Upload CA0 File
                                              SIM_STAGE_RESET_MODULE,           // Pre Upload CC0 File
                                              SIM_STAGE_RESET_MODULE,           // Pre Upload CK0 File
                                              SIM_STAGE_RESET_MODULE,           // Upload CA0 File
                                              SIM_STAGE_RESET_MODULE,           // Upload CC0 File
                                              SIM_STAGE_RESET_MODULE,           // Upload CK0 File
                                              SIM_STAGE_RESET_MODULE,           // cfg APN
                                              SIM_STAGE_MODULE_OFF,             // Connect GPRS
                                              SIM_STAGE_MODULE_OFF,             // Activate FGCNT
                                              SIM_STAGE_RESET_MODULE,           // Query Local IP
                                              SIM_STAGE_RESET_MODULE,           // cfg SSL Version
                                              SIM_STAGE_RESET_MODULE,           // cfg SSL Cipher
                                              SIM_STAGE_RESET_MODULE,           // cfg SSL Sec lvl
                                              SIM_STAGE_RESET_MODULE,           // cfg SSL Ignore Time
                                              SIM_STAGE_RESET_MODULE,           // cfg SSL CA0
                                              SIM_STAGE_RESET_MODULE,           // cfg SSL CC0
                                              SIM_STAGE_RESET_MODULE,           // cfg SSL CK0
                                              SIM_STAGE_CIPSHUT,                // Open Socket
                                              SIM_STAGE_CLOSE_SOCKET,           // Pre Send Data
                                              SIM_STAGE_CLOSE_SOCKET,           // Send Data
                                              SIM_STAGE_CLOSE_SOCKET,                // Receive Server Response
                                              SIM_STAGE_CLOSE_SOCKET,                // Read Server Response
                                              SIM_STAGE_CIPSHUT,             // Close Socket
                                              SIM_STAGE_MODULE_OFF,             // CIPSHUT
                                              SIM_STAGE_TEST_SERIAL             // Seek OK
                                             };

const uint8_t sim_stage_TO_CME_Error[] = {GSM_STAGE_DIS_POWER,                  // Disable Power
                                          GSM_STAGE_DIS_POWER,                  // Enable Power
                                          SIM_STAGE_MODULE_OFF,                 // Module OFF
                                          SIM_STAGE_MODULE_OFF,                 // Module ON
                                          SIM_STAGE_MODULE_OFF,                 // Reset Module
                                          SIM_STAGE_TEST_SERIAL,                // Test Serial
                                          SIM_STAGE_DISABLE_ECHO,               // Disable ECHO
                                          SIM_STAGE_ENABLE_CMEE,                // Enable CMEE
                                          SIM_STAGE_CFG_CREG,                   // cfg CREG
                                          SIM_STAGE_CFG_URC,                    // cfg URC
                                          SIM_STAGE_CFG_SMS_MODE,               // cfg SMS mode
                                          SIM_STAGE_CFG_SMS_URC,                // cfg SMS URC
                                          SIM_STAGE_CFG_RI,                     // cfg RI
                                          SIM_STAGE_CFG_CALL_CARRIER,           // cfg BUSY LINE
                                          SIM_STAGE_DEL_SMS,                    // DEL SMSs
                                          SIM_STAGE_SAVE_SET,                   // Save Setting
                                          SIM_STAGE_QUERY_IMEI,                 // Query IMEI
                                          SIM_STAGE_QUERY_SIMCARD,              // Query CPIN
                                          SIM_STAGE_QUERY_NETWORK,              // Query Network
                                          SIM_STAGE_QUERY_NETWORK_PROVIDER,     // Query IMSI
                                          SIM_STAGE_QUERY_SIGNAL_QUALITY,       // Query CSQ
                                          SIM_STAGE_QUERY_FOREGROUND_CONTEXT,   // Query foreground context
                                          SIM_STAGE_CFG_FOREGROUND_CONTEXT,     // cfg foreground context
                                          SIM_STAGE_QUERY_MUX,                  // Query MUX
                                          SIM_STAGE_CONFIG_MUX,                 // cfg MUX
                                          SIM_STAGE_QUERY_GPRS_ATTACHMENT,      // Query GPRS Attachment
                                          SIM_STAGE_CFG_GPRS_ATTACHMENT,        // cfg GPRS Attachment
                                          SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS,// Query SSL Connection Status
                                          SIM_STAGE_QUERY_RAM_CA0_FILE,         // Query CA0 File
                                          SIM_STAGE_QUERY_RAM_CC0_FILE,         // Query CC0 File
                                          SIM_STAGE_QUERY_RAM_CK0_FILE,         // Query CK0 File
                                          SIM_STAGE_DELETE_RAM_CA0_FILE,        // Delete CA0 File
                                          SIM_STAGE_DELETE_RAM_CC0_FILE,        // Delete CC0 File
                                          SIM_STAGE_DELETE_RAM_CK0_FILE,        // Delete CK0 File
                                          SIM_STAGE_PRE_UPLOAD_RAM_CA0_FILE,    // Pre Upload CA0 File
                                          SIM_STAGE_PRE_UPLOAD_RAM_CC0_FILE,    // Pre Upload CC0 File
                                          SIM_STAGE_PRE_UPLOAD_RAM_CK0_FILE,    // Pre Upload CK0 File
                                          SIM_STAGE_UPLOAD_RAM_CA0_FILE,        // Upload CA0 File
                                          SIM_STAGE_UPLOAD_RAM_CC0_FILE,        // Upload CC0 File
                                          SIM_STAGE_UPLOAD_RAM_CK0_FILE,        // Upload CK0 File
                                          SIM_STAGE_CONFIG_APN,                 // cfg APN
                                          SIM_STAGE_CONNECT_GPRS,               // Connect GPRS
                                          SIM_STAGE_ACTIV_FGCNT,                // Activate FGCNT
                                          SIM_STAGE_QUERY_LOCAL_IP,             // Query Local IP
                                          SIM_STAGE_CFG_SSL_VERSION,            // cfg SSL Version
                                          SIM_STAGE_CFG_SSL_CIPHER,             // cfg SSL Cipher
                                          SIM_STAGE_CFG_SSL_SEC_LVL,            // cfg SSL Sec lvl
                                          SIM_STAGE_CFG_SSL_IGNORE_TIME,        // cfg SSL Ignore Time
                                          SIM_STAGE_CFG_SSL_CA_CERT,            // cfg SSL CA0
                                          SIM_STAGE_CFG_SSL_CC_CERT,            // cfg SSL CC0
                                          SIM_STAGE_CFG_SSL_CK_CERT,            // cfg SSL CK0
                                          SIM_STAGE_OPEN_SOCKET,                // Open Socket
                                          SIM_STAGE_PRE_SEND_DATA,              // Pre Send Data
                                          SIM_STAGE_SEND_DATA,                  // Send Data
                                          SIM_STAGE_RECEIVE_SERVER_RESPONSE,    // Receive Server Response
                                          SIM_STAGE_READ_SERVER_RESPONSE,       // Read Server Response
                                          SIM_STAGE_CLOSE_SOCKET,               // Close Socket
                                          SIM_STAGE_CIPSHUT,                    // CIPSHUT
                                          SIM_STAGE_SEEK_OK                     // Seek OK
                                };


void SSL_Disable_GSM_Power(void)
{
  GSM_DIS_POWER;
  
  // Change LED Pattern
  if(system_current_state == SYSTEM_RUNNING)
    LED_change_mode(GSM_LED, LED_OFF);
  
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.stage = GSM_STAGE_EN_POWER;
  GSM_Parameters.number_of_retries_command = 0;
  GSM_HALT_Timer = 4;
}

void SSL_Disable_GSM_Power_resp(uint16_t* resp_msg)
{
  asm("nop");
}

void SSL_Enable_GSM_Power(void)
{
  GSM_EN_POWER;
  
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.stage = SIM_STAGE_MODULE_ON;
  GSM_Parameters.number_of_retries_command = 0;
  GSM_HALT_Timer = 5;
}

void SSL_Enable_GSM_Power_resp(uint16_t* resp_msg)
{
  asm("nop");
}

void SSL_Module_OFF(void)
{
  turn_off_gsm = SET;
  server_socket = RESET;
  GSM_HALT_Timer = 10;
  
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.stage = SIM_STAGE_MODULE_ON;
  GSM_Parameters.number_of_retries_command = 0;
}

void SSL_Module_OFF_resp(uint16_t* resp_msg)
{
  asm("nop");
}

void SSL_Module_ON(void)
{
  GSM_clear_RX_buffer();

  GSM_Parameters.nw_provider = 0;
  GSM_Parameters.nw_status = 0;
  GSM_Parameters.signal_quality = 0;
  GSM_Parameters.simcard_available = 0;
  GSM_Parameters.internet_connection = RESET;

  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.stage = SIM_STAGE_TEST_SERIAL;
  GSM_Parameters.number_of_retries_command = 0;
  GSM_HALT_Timer = 5;

  turn_on_gsm = SET;

  socket_connection_attempts = 0;
  sending_data_is_in_progress = RESET;
}

void SSL_Module_ON_resp(uint16_t* resp_msg)
{
  asm("nop");
}

void SSL_reset_module(void)
{
  GSM_Send_Usart("AT+CFUN=1,1\r", strlen("AT+CFUN=1,1\r"));

  GSM_clear_RX_buffer();

  GSM_Parameters.nw_provider = 0;
  GSM_Parameters.nw_status = 0;
  GSM_Parameters.signal_quality = 0;
  GSM_Parameters.simcard_available = 0;
  GSM_Parameters.internet_connection = RESET;

  server_socket = RESET;
  sending_data_is_in_progress = RESET;

  // Change LED Pattern
  if(system_current_state == SYSTEM_RUNNING)
    LED_change_mode(GSM_LED, LED_ON_BLINK_500MS);
  GSM_HALT_Timer = 6;
}

void SSL_reset_module_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_TEST_SERIAL;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_HALT_Timer = 5;
  }
}

void SSL_test_serial_port(void)
{
  uint8_t msg[1] = {0x1B};
  GSM_Send_Usart(msg, 1);
  Delay(200);
  GSM_Send_Usart("AT\r", strlen("AT\r"));
}

void SSL_test_serial_port_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
    GSM_Parameters.prev_stage = 0;

    if(GSM_module_is_configured == RESET)
      GSM_Parameters.stage = SIM_STAGE_DISABLE_ECHO;
    else if(GSM_Parameters.simcard_available == 0)
      GSM_Parameters.stage = SIM_STAGE_QUERY_SIMCARD;
    else
    {
      GSM_Parameters.stage = GSM_Parameters.next_stage;     // When the buffer has error and we want to check the serial port.
      GSM_Parameters.next_stage = 0;
    }
  }
}

void SSL_disable_echo(void)
{
  GSM_Send_Usart("ATE0\r", strlen("ATE0\r"));
}

void SSL_disable_echo_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_ENABLE_CMEE;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_enable_CME_ERROR(void)
{
  GSM_Send_Usart("AT+CMEE=1\r", strlen("AT+CMEE=1\r"));
}

void SSL_enable_CME_ERROR_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_CFG_CREG;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_CREG(void)
{
  GSM_Send_Usart("AT+CREG=0\r", strlen("AT+CREG=0\r"));
}

void SSL_cfg_CREG_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_CFG_URC;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_URC(void)
{
  GSM_Send_Usart("AT+QIURC=0\r", strlen("AT+QIURC=0\r"));
}

void SSL_cfg_URC_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_CFG_SMS_MODE;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_SMS_mode(void)
{
  GSM_Send_Usart("AT+CMGF=1\r", strlen("AT+CMGF=1\r"));
}

void SSL_cfg_SMS_mode_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_CFG_SMS_URC;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_SMS_URC(void)
{
  GSM_Send_Usart("AT+CNMI=2,1,0,0,0\r", strlen("AT+CNMI=2,1,0,0,0\r"));
}

void SSL_cfg_SMS_URC_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_CFG_RI;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_RI(void)
{
  GSM_Send_Usart("AT+QINDRI=0\r", strlen("AT+QINDRI=0\r"));
}

void SSL_cfg_RI_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_CFG_CALL_CARRIER;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_Call_Carrier(void)
{
  GSM_Send_Usart("AT+CLIP=1\r", strlen("AT+CLIP=1\r"));
}

void SSL_cfg_Call_Carrier_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_DEL_SMS;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
    GSM_HALT_Timer = 2;
  }
}


void SSL_cfg_SMS_memory(void)
{
  GSM_Send_Usart("AT+CPMS=\"ME\",\"ME\",\"ME\"\r", strlen("AT+CPMS=\"ME\",\"ME\",\"ME\"\r"));
}


void SSL_cfg_SMS_memory_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CPMS)
  {
    uint64_t tmp_data = 0;
    
    GSM_get_number_from_line_buffer(',', &tmp_data);
    
    if(tmp_data > 0)
    {
      current_sms_index = 1;
      system_has_short_message = SET;
    }

    GSM_Parameters.stage = SIM_STAGE_SAVE_SET;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void SSL_save_setting(void)
{
  GSM_Send_Usart("AT&W\r", strlen("AT&W\r"));
}

void SSL_save_setting_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage = SIM_STAGE_QUERY_IMEI;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
    GSM_module_is_configured = SET;
  }
}

void SSL_query_IMEI(void)
{
  GSM_Send_Usart("AT+GSN\r", strlen("AT+GSN\r"));
}

void SSL_query_IMEI_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_IMEI)
  {
    // Parse GSN response
    GSM_parse_IMEI();
  }
}

void SSL_query_CPIN(void)
{
  GSM_Send_Usart("AT+CPIN?\r", strlen("AT+CPIN?\r"));
}

void SSL_query_CPIN_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CPIN)
  {
    // Parse CPIN response
    GSM_parse_CPIN();

    // Change LED Pattern
    if(system_current_state == SYSTEM_RUNNING)
      LED_change_mode(GSM_LED, LED_ON_BLINK_500MS);
  }
}

void SSL_query_network(void)
{
  GSM_Send_Usart("AT+CREG?\r", strlen("AT+CREG?\r"));
}

void SSL_query_network_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CREG)
  {
    // Parse CREG response
    GSM_parse_CREG();
  }
}

void SSL_query_IMSI(void)
{
  GSM_Send_Usart("AT+CIMI\r", strlen("AT+CIMI\r"));
}

void SSL_query_IMSI_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_COPS)
  {
    // Parse COPS response
    GSM_parse_IMSI();
  }
}

void SSL_query_CSQ(void)
{
  GSM_Send_Usart("AT+CSQ\r", strlen("AT+CSQ\r"));
}

void SSL_query_CSQ_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CSQ)
  {
    // Parse CSQ response
    GSM_parse_CSQ();
  }
}

void SSL_query_foreground_context(void)
{
  GSM_Send_Usart("AT+QIFGCNT?\r", strlen("AT+QIFGCNT?\r"));
}

void SSL_query_foreground_context_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QIFGCNT)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer(',', &recv_num) == GSM_OK)
    {
      if(recv_num == 0)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CA0_FILE;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_CFG_FOREGROUND_CONTEXT;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_QUERY_FOREGROUND_CONTEXT;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_cfg_foreground_context(void)
{
  GSM_Send_Usart("AT+QIFGCNT=0\r", strlen("AT+QIFGCNT=0\r"));
}

void SSL_cfg_foreground_context_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_RAM_CA0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_FOREGROUND_CONTEXT;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_query_MUX(void)
{
  GSM_Send_Usart("AT+QIMUX?\r", strlen("AT+QIMUX?\r"));
}

void SSL_query_MUX_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QIFGCNT)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer('\r', &recv_num) == GSM_OK)
    {
      if(recv_num == 0)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_GPRS_ATTACHMENT;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_CONFIG_MUX;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_QUERY_MUX;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_cfg_MUX(void)
{
  GSM_Send_Usart("AT+CIPMUX=0\r", strlen("AT+CIPMUX=0\r"));
}

void SSL_cfg_MUX_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_GPRS_ATTACHMENT;
    GSM_Parameters.prev_stage = SIM_STAGE_CONFIG_MUX;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_Query_GPRS_attachment(void)
{
  GSM_Send_Usart("AT+CGATT?\r", strlen("AT+CGATT?\r"));
}

void SSL_Query_GPRS_attachment_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CGATT)
  {
    uint64_t recv_num = 0;

    if(GSM_get_number_from_line_buffer('\r', &recv_num) == GSM_OK)
    {
      if(recv_num == 1)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_CFG_GPRS_ATTACHMENT;
      }

      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_QUERY_GPRS_ATTACHMENT;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_cfg_GPRS_attachment(void)
{
  GSM_Send_Usart("AT+CGATT=1\r", strlen("AT+CGATT=1\r"));
}

void SSL_cfg_GPRS_attachment_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_GPRS_ATTACHMENT;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_query_current_connection_Status(void)
{
  GSM_Send_Usart("AT+QSSLSTATE\r", strlen("AT+QSSLSTATE\r"));
}

void SSL_query_current_connection_Status_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CONNECTION_STATUS)
  {
    // Parse STATUS response
    GSM_parse_Connection_STATUS();
  }
}

void SSL_query_RAM_CA0_file(void)
{
  GSM_Send_Usart("AT+QSECREAD=\"RAM:CA0.pem\"\r", strlen("AT+QSECREAD=\"RAM:CA0.pem\"\r"));
}

void SSL_query_RAM_CA0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QSECREAD)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer(',', &recv_num) == GSM_OK)
    {
      if(recv_num == 1)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CC0_FILE;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_DELETE_RAM_CA0_FILE;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_QUERY_RAM_CA0_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_query_RAM_CC0_file(void)
{
  GSM_Send_Usart("AT+QSECREAD=\"RAM:CC0.pem\"\r", strlen("AT+QSECREAD=\"RAM:CC0.pem\"\r"));
}

void SSL_query_RAM_CC0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QSECREAD)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer(',', &recv_num) == GSM_OK)
    {
      if(recv_num == 1)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CK0_FILE;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_DELETE_RAM_CC0_FILE;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_QUERY_RAM_CC0_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_query_RAM_CK0_file(void)
{
  GSM_Send_Usart("AT+QSECREAD=\"RAM:CK0.pem\"\r", strlen("AT+QSECREAD=\"RAM:CK0.pem\"\r"));
}

void SSL_query_RAM_CK0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QSECREAD)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer(',', &recv_num) == GSM_OK)
    {
      if(recv_num == 1)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_CONFIG_APN;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_DELETE_RAM_CK0_FILE;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_QUERY_RAM_CK0_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_delete_RAM_CA0_file(void)
{
  GSM_Send_Usart("AT+QSECDEL=\"RAM:CK0.pem\"\r", strlen("AT+QSECDEL=\"RAM:CK0.pem\"\r"));
}

void SSL_delete_RAM_CA0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_PRE_UPLOAD_RAM_CA0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_DELETE_RAM_CA0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_delete_RAM_CC0_file(void)
{
  GSM_Send_Usart("AT+QSECDEL=\"RAM:CC0.pem\"\r", strlen("AT+QSECDEL=\"RAM:CC0.pem\"\r"));
}

void SSL_delete_RAM_CC0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_PRE_UPLOAD_RAM_CC0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_DELETE_RAM_CC0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_delete_RAM_CK0_file(void)
{
  GSM_Send_Usart("AT+QSECDEL=\"RAM:CK0.pem\"\r", strlen("AT+QSECDEL=\"RAM:CK0.pem\"\r"));
}

void SSL_delete_RAM_CK0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_PRE_UPLOAD_RAM_CK0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_DELETE_RAM_CK0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_pre_upload_RAM_CA0_file(void)
{
  uint16_t cert_data_size = SPIF_read_certificate_file_size(SERVER_CERT_FILE);
  
  if(cert_data_size != 0 && cert_data_size != 0xFFFF)
  {
    uint8_t msg[50] = "AT+QSECWRITE=\"RAM:CA0.pem\",";
    uint16_t buff_tmp_index = strlen("AT+QSECWRITE=\"RAM:CA0.pem\",");
    GSM_add_char_number_to_buffer(msg, &buff_tmp_index, cert_data_size );
    msg[buff_tmp_index++] = ',';
    msg[buff_tmp_index++] = '1';
    msg[buff_tmp_index++] = '0';
    msg[buff_tmp_index++] = '0';
    msg[buff_tmp_index++] = '\r';
    
    GSM_Send_Usart(msg, buff_tmp_index);
  }
  else
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_RAM_CA0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_PRE_UPLOAD_RAM_CA0_FILE;
    GSM_Parameters.number_of_retries_command = 0;

    sending_data_is_in_progress = RESET;
    GSM_status = GSM_IDLE;
    
    system_error.certificate_error = SET;
    reset_server_unreachable_counter = 3600;
    server_is_unreachable = SET;
  }
}

void SSL_pre_upload_RAM_CA0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CONNECT)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_UPLOAD_RAM_CA0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_PRE_UPLOAD_RAM_CA0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_pre_upload_RAM_CC0_file(void)
{
  uint16_t cert_data_size = SPIF_read_certificate_file_size(CLIENT_CERT_FILE);
  
  if(cert_data_size != 0 && cert_data_size != 0xFFFF)
  {
    uint8_t msg[50] = "AT+QSECWRITE=\"RAM:CC0.pem\",";
    uint16_t buff_tmp_index = strlen("AT+QSECWRITE=\"RAM:CC0.pem\",");
    GSM_add_char_number_to_buffer(msg, &buff_tmp_index, cert_data_size );
    msg[buff_tmp_index++] = ',';
    msg[buff_tmp_index++] = '1';
    msg[buff_tmp_index++] = '0';
    msg[buff_tmp_index++] = '0';
    msg[buff_tmp_index++] = '\r';
    
    GSM_Send_Usart(msg, buff_tmp_index);
  }
  else
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_RAM_CC0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_PRE_UPLOAD_RAM_CC0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
    
    sending_data_is_in_progress = RESET;
    GSM_status = GSM_IDLE;
    
    system_error.certificate_error = SET;
    reset_server_unreachable_counter = 3600;
    server_is_unreachable = SET;
  }
}

void SSL_pre_upload_RAM_CC0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CONNECT)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_UPLOAD_RAM_CC0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_PRE_UPLOAD_RAM_CC0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_pre_upload_RAM_CK0_file(void)
{
  uint16_t cert_data_size = SPIF_read_certificate_file_size(CLIENT_KEY_FILE);
  
  if(cert_data_size != 0 && cert_data_size != 0xFFFF)
  {
    uint8_t msg[50] = "AT+QSECWRITE=\"RAM:CK0.pem\",";
    uint16_t buff_tmp_index = strlen("AT+QSECWRITE=\"RAM:CK0.pem\",");
    GSM_add_char_number_to_buffer(msg, &buff_tmp_index, cert_data_size );
    msg[buff_tmp_index++] = ',';
    msg[buff_tmp_index++] = '1';
    msg[buff_tmp_index++] = '0';
    msg[buff_tmp_index++] = '0';
    msg[buff_tmp_index++] = '\r';
    
    GSM_Send_Usart(msg, buff_tmp_index);
  }
  else
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_RAM_CK0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_PRE_UPLOAD_RAM_CK0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
    
    sending_data_is_in_progress = RESET;
    GSM_status = GSM_IDLE;
    
    system_error.certificate_error = SET;
    reset_server_unreachable_counter = 3600;
    server_is_unreachable = SET;
  }
}

void SSL_pre_upload_RAM_CK0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CONNECT)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_UPLOAD_RAM_CK0_FILE;
    GSM_Parameters.prev_stage = SIM_STAGE_PRE_UPLOAD_RAM_CK0_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_upload_RAM_CA0_file(void)
{
  uint8_t cert_data[1300] = {0};
  uint16_t cert_data_size = SPIF_read_certificate_file_size(SERVER_CERT_FILE);
  SPIF_read_certificate_file(SERVER_CERT_FILE, cert_data, cert_data_size);
  
  GSM_Send_Usart(cert_data, cert_data_size);
}

void SSL_upload_RAM_CA0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QSECWRITE)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer(',', &recv_num) == GSM_OK)
    {
      uint16_t cert_data_size = SPIF_read_certificate_file_size(SERVER_CERT_FILE);
      if( recv_num == cert_data_size)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CC0_FILE;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CA0_FILE;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_UPLOAD_RAM_CA0_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_upload_RAM_CC0_file(void)
{
  uint8_t cert_data[1300] = {0};
  uint16_t cert_data_size = SPIF_read_certificate_file_size(CLIENT_CERT_FILE);
  SPIF_read_certificate_file(CLIENT_CERT_FILE, cert_data, cert_data_size);
  
  GSM_Send_Usart(cert_data, cert_data_size);
}

void SSL_upload_RAM_CC0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QSECWRITE)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer(',', &recv_num) == GSM_OK)
    {
      uint16_t cert_data_size = SPIF_read_certificate_file_size(CLIENT_CERT_FILE);
      if( recv_num == cert_data_size)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CK0_FILE;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CC0_FILE;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_UPLOAD_RAM_CC0_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_upload_RAM_CK0_file(void)
{
  uint8_t cert_data[1300] = {0};
  uint16_t cert_data_size = SPIF_read_certificate_file_size(CLIENT_KEY_FILE);
  SPIF_read_certificate_file(CLIENT_KEY_FILE, cert_data, cert_data_size);
  
  GSM_Send_Usart(cert_data, cert_data_size);
}

void SSL_upload_RAM_CK0_file_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_QSECWRITE)
  {
    uint64_t recv_num = 0;
    
    if(GSM_get_number_from_line_buffer(',', &recv_num) == GSM_OK)
    {
      uint16_t cert_data_size = SPIF_read_certificate_file_size(CLIENT_KEY_FILE);
      if( recv_num == cert_data_size)
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CA0_FILE;
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
        GSM_Parameters.next_stage = SIM_STAGE_QUERY_RAM_CK0_FILE;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_UPLOAD_RAM_CK0_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}

void SSL_cfg_APN(void)
{
  GSM_Send_Set_APN_cmd();
}

void SSL_cfg_APN_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CONNECT_GPRS;
    GSM_Parameters.prev_stage = SIM_STAGE_CONFIG_APN;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_Connect_GPRS(void)
{
  GSM_Send_Usart("AT+QIREGAPP\r", strlen("AT+QIREGAPP\r"));
}

void SSL_Connect_GPRS_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_ACTIV_FGCNT;
    GSM_Parameters.prev_stage = SIM_STAGE_CONNECT_GPRS;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_Activate_FGCNT(void)
{
  GSM_Send_Usart("AT+QIACT\r", strlen("AT+QIACT\r"));
}

void SSL_Activate_FGCNT_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_LOCAL_IP;
    GSM_Parameters.prev_stage = SIM_STAGE_ACTIV_FGCNT;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_query_Local_IP(void)
{
  GSM_Send_Usart("AT+QILOCIP\r", strlen("AT+QILOCIP\r"));
}

void SSL_query_Local_IP_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_UNSPECIFIED)
  {
    uint8_t tmp_var = 0;
    for(uint8_t i = 0; i <= (line_buffer_counter - 2); i++)
    {
      if(Rx_line_buffer[i] == '.')
        tmp_var++;
    }
    if(tmp_var == 3)
    {
      uint64_t recv_num = 0;
      line_buffer_rd_index = 0;
      
      GSM_get_number_from_line_buffer('.', &recv_num);
      GSM_local_IP += (recv_num << 24);
      GSM_get_number_from_line_buffer('.', &recv_num);
      GSM_local_IP += (recv_num << 16);
      GSM_get_number_from_line_buffer('.', &recv_num);
      GSM_local_IP += (recv_num << 8);
      GSM_get_number_from_line_buffer('\r', &recv_num);
      GSM_local_IP += recv_num;
    }
    
    GSM_Parameters.internet_connection = SET;
    
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CFG_SSL_VERSION;
    GSM_Parameters.prev_stage = SIM_STAGE_QUERY_LOCAL_IP;
    GSM_HALT_Timer = 2;
  }
}

// Orginally: 3 => TLS1.2
// For future support we use general configuration
void SSL_cfg_ssl_version(void)
{
  GSM_Send_Usart("AT+QSSLCFG=\"sslversion\",0,4\r", strlen("AT+QSSLCFG=\"sslversion\",0,4\r"));
}

void SSL_cfg_ssl_version_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CFG_SSL_CIPHER;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_SSL_VERSION;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

// Orginally: 0x003D => TLS_RSA_WITH_AES_256_CBC_SHA256
// For future support we use general configuration
void SSL_cfg_ssl_cipher(void)
{
  GSM_Send_Usart("AT+QSSLCFG=\"ciphersuite\",0,\"0xFFFF\"\r", strlen("AT+QSSLCFG=\"ciphersuite\",0,\"0xFFFF\"\r"));
}

void SSL_cfg_ssl_cipher_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CFG_SSL_SEC_LVL;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_SSL_CIPHER;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_ssl_sec_lvl(void)
{
  GSM_Send_Usart("AT+QSSLCFG=\"seclevel\",0,2\r", strlen("AT+QSSLCFG=\"seclevel\",0,2\r"));
}

void SSL_cfg_ssl_sec_lvl_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CFG_SSL_IGNORE_TIME;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_SSL_SEC_LVL;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_ssl_ignore_time(void)
{
  GSM_Send_Usart("AT+QSSLCFG=\"ignorertctime\",1\r", strlen("AT+QSSLCFG=\"ignorertctime\",1\r"));
}

void SSL_cfg_ssl_ignore_time_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CFG_SSL_CA_CERT;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_SSL_SEC_LVL;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void SSL_cfg_ssl_CA_cer(void)
{
  GSM_Send_Usart("AT+QSSLCFG=\"cacert\",0,\"RAM:CA0.pem\"\r", strlen("AT+QSSLCFG=\"cacert\",0,\"RAM:CA0.pem\"\r"));
}

void SSL_cfg_ssl_CA_cer_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CFG_SSL_CC_CERT;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_SSL_CA_CERT;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_ssl_CC_cer(void)
{
  GSM_Send_Usart("AT+QSSLCFG=\"clientcert\",0,\"RAM:CC0.pem\"\r", strlen("AT+QSSLCFG=\"clientcert\",0,\"RAM:CC0.pem\"\r"));
}

void SSL_cfg_ssl_CC_cer_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_CFG_SSL_CK_CERT;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_SSL_CC_CERT;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_cfg_ssl_CK_cer(void)
{
  GSM_Send_Usart("AT+QSSLCFG=\"clientkey\",0,\"RAM:CK0.pem\"\r", strlen("AT+QSSLCFG=\"clientkey\",0,\"RAM:CK0.pem\"\r"));
}

void SSL_cfg_ssl_CK_cer_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_OPEN_SOCKET;
    GSM_Parameters.prev_stage = SIM_STAGE_CFG_SSL_CK_CERT;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_Open_Socket(void)
{
  uint8_t msg[70] = "AT+QSSLOPEN=0,0,\"";
  uint16_t buff_tmp_index = 17;

  memcpy(&msg[buff_tmp_index], &setting.server_url[1], setting.server_url_length);
  buff_tmp_index += setting.server_url_length;

  msg[buff_tmp_index++] = '\"';
  msg[buff_tmp_index++] = ',';
  GSM_add_char_number_to_buffer(msg, &buff_tmp_index, setting.server_port);
  msg[buff_tmp_index++] = ',';
  msg[buff_tmp_index++] = '0';          // Connect Mode
  msg[buff_tmp_index++] = ',';
  msg[buff_tmp_index++] = '9';          // Timeout
  msg[buff_tmp_index++] = '0';          // Timeout
  msg[buff_tmp_index++] = '\r';

  GSM_Send_Usart(msg, buff_tmp_index);
  
  socket_connection_attempts++;
}

void SSL_Open_Socket_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_SSLOPEN)
  {
    uint64_t recv_num = 0;
    
    GSM_read_until_from_line_buffer(',');
    if(GSM_get_number_from_line_buffer('\r', &recv_num) == GSM_OK)
    {
      if(recv_num == 0)
      {
        GSM_Parameters.stage = SIM_STAGE_PRE_SEND_DATA;
        GSM_Parameters.next_stage = 0;
        
        socket_connection_attempts = 0;
        server_socket = SET;
	
        // Clear Errors
        system_error.socket_error = RESET;
        system_error.certificate_error = RESET;
        system_error.no_certificate = RESET;
        
        // Change LED Pattern
        if(system_current_state == SYSTEM_RUNNING)
          LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_1SEC);
        
        if(logined_to_the_server == SET)
        {
          // Change LED Pattern
          if(system_current_state == SYSTEM_RUNNING)
            LED_change_mode(GSM_LED, LED_ON_STEADY);
        }
      }
      else
      {
        GSM_Parameters.stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
        GSM_Parameters.next_stage = 0;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = SIM_STAGE_OPEN_SOCKET;
      GSM_Parameters.number_of_retries_command = 0;
      
      if(socket_connection_attempts >= 3)
      {
        GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
        
        *resp_msg = GSM_RESEND_PCKT;
        reset_server_unreachable_counter = 3600;
        server_is_unreachable = SET;
        
        system_error.socket_error = SET;
        system_error.socket_error_count++;
      }
    }
  }
}

void SSL_Pre_send_Data(void)
{
  uint8_t msg[40] = {0};
  uint16_t buff_tmp_index = 0;
  
  memcpy(&msg[buff_tmp_index], "AT+QSSLSEND=0,", 14);
  buff_tmp_index += 14;
  
  GSM_add_char_number_to_buffer(msg, &buff_tmp_index, data_packet_length);
  msg[buff_tmp_index++] = '\r';
  
  GSM_Send_Usart(msg, buff_tmp_index);
}

void SSL_Pre_send_Data_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_DATA_MODE)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_SEND_DATA;
    GSM_Parameters.prev_stage = SIM_STAGE_PRE_SEND_DATA;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void SSL_Send_Data(void)
{
  GSM_Send_Usart(data_packet, data_packet_length);
}

void SSL_Send_Data_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_SEND_FAIL)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_PRE_SEND_DATA;
  }
  if(*resp_msg == GSM_SEND_OK)
  {
    GSM_Parameters.stage_action = SIM_RCV_RESP;
    GSM_Parameters.stage = SIM_STAGE_RECEIVE_SERVER_RESPONSE;
  }

  GSM_Parameters.prev_stage = SIM_STAGE_SEND_DATA;
  GSM_Parameters.number_of_retries_command = 0;
}

void SSL_Receive_Server_response(void)
{
  asm("nop");
}

void SSL_Receive_Server_response_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_ERR_CLOSED)
  {
    // bug in packet
    GSM_Parameters.stage = SIM_STAGE_OPEN_SOCKET;

    GSM_status = GSM_IDLE;
    sending_data_is_in_progress = RESET;
  }
  else if(*resp_msg == GSM_RECV_SERVER_RESP)
  {
    GSM_Parameters.stage = SIM_STAGE_READ_SERVER_RESPONSE;
  }

  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.prev_stage = SIM_STAGE_RECEIVE_SERVER_RESPONSE;
  GSM_Parameters.number_of_retries_command = 0;
}

void SSL_Read_server_response(void)
{
  GSM_clear_RX_buffer();
  
  GSM_Send_Usart("AT+QSSLRECV=0,0,1500\r", strlen("AT+QSSLRECV=0,0,1500\r"));
}

void SSL_Read_server_response_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_READ_SERVER_RESP)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_SIGNAL_QUALITY;//SIM_STAGE_SEEK_OK;
    //GSM_Parameters.next_stage = SIM_STAGE_QUERY_SIGNAL_QUALITY;
    GSM_Parameters.prev_stage = SIM_STAGE_READ_SERVER_RESPONSE;
    GSM_Parameters.number_of_retries_command = 0;
    
    uint64_t server_response_length = 0;
    GSM_read_until_from_line_buffer(',');
    GSM_read_until_from_line_buffer(',');
    if(GSM_get_number_from_line_buffer('\r', &server_response_length) == GSM_OK)
    {
      uint8_t server_response = 0;
      server_response = GSM_handle_server_response((uint16_t)server_response_length);
      if(server_response == SIP_PROTOCOL_OK)
      {
        *resp_msg = GSM_SEND_NEXT_PCKT;
        
        server_packet_retry_number = 0;
      }
      else if(server_response == SIP_DATA_ERROR)
      {
        *resp_msg = GSM_RESEND_PCKT;
        
        server_packet_retry_number++;
        if(server_packet_retry_number > 10)
        {
          GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
          server_packet_retry_number = 0;
        }
      }
    }
    else
    {
      *resp_msg = GSM_RESEND_PCKT;
    }
  }
}

void SSL_Close_Socket(void)
{
  GSM_Send_Usart("AT+QSSLCLOSE=0\r", strlen("AT+QSSLCLOSE=0\r"));
}

void SSL_Close_Socket_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_CLOSE_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_OPEN_SOCKET;
    GSM_Parameters.prev_stage = SIM_STAGE_CLOSE_SOCKET;
    GSM_Parameters.number_of_retries_command = 0;

    server_socket = RESET;
    sending_data_is_in_progress = RESET;
    GSM_status = GSM_IDLE;
    // Change LED Pattern
    if(system_current_state == SYSTEM_RUNNING)
      LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
  }
}

void SSL_CIPSHUT(void)
{
  GSM_Send_Usart("AT+QIDEACT\r", strlen("AT+QIDEACT\r"));
}

void SSL_CIPSHUT_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_DEACT_PDP)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
    GSM_Parameters.prev_stage = SIM_STAGE_CIPSHUT;
    GSM_Parameters.number_of_retries_command = 0;

    server_socket = RESET;
    GSM_Parameters.internet_connection = RESET;
    sending_data_is_in_progress = RESET;
    GSM_status = GSM_IDLE;
    
    if(socket_connection_attempts >= 3)
    {
      socket_connection_attempts = 0;
      GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
      
      reset_server_unreachable_counter = 3600;
      server_is_unreachable = SET;
      
      system_error.socket_error = SET;
      system_error.socket_error_count++;
    }
  }
}

void SSL_seek_OK(void)
{
  asm("nop");
}

void SSL_seek_OK_resp(uint16_t* resp_msg)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = GSM_Parameters.next_stage;
    GSM_Parameters.number_of_retries_command = 0;
    GSM_Parameters.next_stage = 0;
    GSM_Parameters.prev_stage = 0;
  }
}


void SSL_check_cer_file_stages(void)
{
  if(GSM_Parameters.stage == SIM_STAGE_QUERY_RAM_CA0_FILE || GSM_Parameters.stage == SIM_STAGE_DELETE_RAM_CA0_FILE)
    GSM_Parameters.stage = SIM_STAGE_PRE_UPLOAD_RAM_CA0_FILE;
  else if(GSM_Parameters.stage == SIM_STAGE_QUERY_RAM_CC0_FILE || GSM_Parameters.stage == SIM_STAGE_DELETE_RAM_CC0_FILE)
    GSM_Parameters.stage = SIM_STAGE_PRE_UPLOAD_RAM_CC0_FILE;
  else if(GSM_Parameters.stage == SIM_STAGE_QUERY_RAM_CK0_FILE || GSM_Parameters.stage == SIM_STAGE_DELETE_RAM_CK0_FILE)
    GSM_Parameters.stage = SIM_STAGE_PRE_UPLOAD_RAM_CK0_FILE;
}

void (*SSL_Request_ptr[])(void) = {&SSL_Disable_GSM_Power,
                               &SSL_Enable_GSM_Power,
                               &SSL_Module_OFF,
                               &SSL_Module_ON,
                               &SSL_reset_module,
                               &SSL_test_serial_port,
                               &SSL_disable_echo,
                               &SSL_enable_CME_ERROR,
                               &SSL_cfg_CREG,
                               &SSL_cfg_URC,
                               &SSL_cfg_SMS_mode,
                               &SSL_cfg_SMS_URC,
                               &SSL_cfg_RI,
                               &SSL_cfg_Call_Carrier,
                               &SSL_cfg_SMS_memory,
                               &SSL_save_setting,
                               &SSL_query_IMEI,
                               &SSL_query_CPIN,
                               &SSL_query_network,
                               &SSL_query_IMSI,
                               &SSL_query_CSQ,
                               &SSL_query_foreground_context,
                               &SSL_cfg_foreground_context,
                               &SSL_query_MUX,
                               &SSL_cfg_MUX,
                               &SSL_Query_GPRS_attachment,
                               &SSL_cfg_GPRS_attachment,
                               &SSL_query_current_connection_Status,
                               &SSL_query_RAM_CA0_file,
                               &SSL_query_RAM_CC0_file,
                               &SSL_query_RAM_CK0_file,
                               &SSL_delete_RAM_CA0_file,
                               &SSL_delete_RAM_CC0_file,
                               &SSL_delete_RAM_CK0_file,
                               &SSL_pre_upload_RAM_CA0_file,
                               &SSL_pre_upload_RAM_CC0_file,
                               &SSL_pre_upload_RAM_CK0_file,
                               &SSL_upload_RAM_CA0_file,
                               &SSL_upload_RAM_CC0_file,
                               &SSL_upload_RAM_CK0_file,
                               &SSL_cfg_APN,
                               &SSL_Connect_GPRS,
                               &SSL_Activate_FGCNT,
                               &SSL_query_Local_IP,
                               &SSL_cfg_ssl_version,
                               &SSL_cfg_ssl_cipher,
                               &SSL_cfg_ssl_sec_lvl,
                               &SSL_cfg_ssl_ignore_time,
                               &SSL_cfg_ssl_CA_cer,
                               &SSL_cfg_ssl_CC_cer,
                               &SSL_cfg_ssl_CK_cer,
                               &SSL_Open_Socket,
                               &SSL_Pre_send_Data,
                               &SSL_Send_Data,
                               &SSL_Receive_Server_response,
                               &SSL_Read_server_response,
                               &SSL_Close_Socket,
                               &SSL_CIPSHUT,
                               &SSL_seek_OK
                              };

void (*SSL_Response_ptr[])(uint16_t*) = {&SSL_Disable_GSM_Power_resp,
                                    &SSL_Enable_GSM_Power_resp,
                                    &SSL_Module_OFF_resp,
                                    &SSL_Module_ON_resp,
                                    &SSL_reset_module_resp,
                                    &SSL_test_serial_port_resp,
                                    &SSL_disable_echo_resp,
                                    &SSL_enable_CME_ERROR_resp,
                                    &SSL_cfg_CREG_resp,
                                    &SSL_cfg_URC_resp,
                                    &SSL_cfg_SMS_mode_resp,
                                    &SSL_cfg_SMS_URC_resp,
                                    &SSL_cfg_RI_resp,
                                    &SSL_cfg_Call_Carrier_resp,
                                    &SSL_cfg_SMS_memory_resp,
                                    &SSL_save_setting_resp,
                                    &SSL_query_IMEI_resp,
                                    &SSL_query_CPIN_resp,
                                    &SSL_query_network_resp,
                                    &SSL_query_IMSI_resp,
                                    &SSL_query_CSQ_resp,
                                    &SSL_query_foreground_context_resp,
                                    &SSL_cfg_foreground_context_resp,
                                    &SSL_query_MUX_resp,
                                    &SSL_cfg_MUX_resp,
                                    &SSL_Query_GPRS_attachment_resp,
                                    &SSL_cfg_GPRS_attachment_resp,
                                    &SSL_query_current_connection_Status_resp,
                                    &SSL_query_RAM_CA0_file_resp,
                                    &SSL_query_RAM_CC0_file_resp,
                                    &SSL_query_RAM_CK0_file_resp,
                                    &SSL_delete_RAM_CA0_file_resp,
                                    &SSL_delete_RAM_CC0_file_resp,
                                    &SSL_delete_RAM_CK0_file_resp,
                                    &SSL_pre_upload_RAM_CA0_file_resp,
                                    &SSL_pre_upload_RAM_CC0_file_resp,
                                    &SSL_pre_upload_RAM_CK0_file_resp,
                                    &SSL_upload_RAM_CA0_file_resp,
                                    &SSL_upload_RAM_CC0_file_resp,
                                    &SSL_upload_RAM_CK0_file_resp,
                                    &SSL_cfg_APN_resp,
                                    &SSL_Connect_GPRS_resp,
                                    &SSL_Activate_FGCNT_resp,
                                    &SSL_query_Local_IP_resp,
                                    &SSL_cfg_ssl_version_resp,
                                    &SSL_cfg_ssl_cipher_resp,
                                    &SSL_cfg_ssl_sec_lvl_resp,
                                    &SSL_cfg_ssl_ignore_time_resp,
                                    &SSL_cfg_ssl_CA_cer_resp,
                                    &SSL_cfg_ssl_CC_cer_resp,
                                    &SSL_cfg_ssl_CK_cer_resp,
                                    &SSL_Open_Socket_resp,
                                    &SSL_Pre_send_Data_resp,
                                    &SSL_Send_Data_resp,
                                    &SSL_Receive_Server_response_resp,
                                    &SSL_Read_server_response_resp,
                                    &SSL_Close_Socket_resp,
                                    &SSL_CIPSHUT_resp,
                                    &SSL_seek_OK_resp
                                   };


uint16_t GSM_SSL_routine_pro(uint8_t function_index)
{
  uint16_t check_msg_result = GSM_WAITING;

  if(GSM_HALT_Timer == GSM_TIMER_DEFAULT)
  {
    switch(GSM_Parameters.stage_action)
    {
    case SIM_SEND_REQ:
      {
        GSM_Parameters.number_of_retries_command++;
        
        if(GSM_Parameters.number_of_retries_command > sim_stage_max_number_of_retries[function_index])
        {
          if(function_index == SIM_STAGE_QUERY_NETWORK)
          {
            if(++GSM_network_search_counter > 5)
            {
              GSM_Creg_two_hour_counter = 1;
              GSM_network_search_counter = 0;
              disable_any_gsm_activity = SET;
            }
          }
          else if(function_index == SIM_STAGE_ACTIV_FGCNT)
          {
            server_is_unreachable = SET;
            reset_server_unreachable_counter = 1800;   // 0.5 Hours
            
            system_error.internet_error = SET;
            system_error.internet_error_count++;
          }
          
          GSM_Parameters.stage = sim_stage_retries_redirect[function_index];
          GSM_Parameters.number_of_retries_command = 0;
          
          break;
        }

        GSM_Parameters.stage_action = SIM_RCV_RESP;
        gsm_pt_timeout = sim_stage_timeout[GSM_Parameters.stage]*GSM_MSG_DEFAULT_TIMEOUT;
        (*SSL_Request_ptr[function_index])();
        break;
      }
    case SIM_RCV_RESP:
      {
        int16_t error_code = 0;
        check_msg_result = GSM_check_msg(&error_code);
        if(gsm_pt_timeout == 0 && check_msg_result == GSM_WAITING)
        {
          GSM_Parameters.stage_action = SIM_SEND_REQ;
          GSM_Parameters.stage = sim_stage_TO_CME_Error[function_index];
          return GSM_TIMEOUT;
        }
        else
        {
          if(check_msg_result == GSM_LF || check_msg_result == GSM_WAITING)
            break;
          if(check_msg_result == GSM_CME_ERROR)
          {
            GSM_Parameters.stage_action = SIM_SEND_REQ;

            if(error_code == CMEE_NO_SIMCARD)
            {
              GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
              GSM_Parameters.simcard_available = RESET;
              GSM_Parameters.nw_provider = 0;
              GSM_status = GSM_INITIATING;
            }
            else if(error_code == CMEE_FILE_NOT_FOUND)
            {
              SSL_check_cer_file_stages();
            }
            else
              GSM_Parameters.stage = sim_stage_TO_CME_Error[function_index];
          }
          else if(check_msg_result == GSM_CMS_ERROR)
          {
            GSM_Parameters.stage_action = SIM_SEND_REQ;
            
            if(error_code == CMEE_FILE_NOT_FOUND)
            {
              SSL_check_cer_file_stages();
            }
            else
              GSM_Parameters.stage = sim_stage_TO_CME_Error[function_index];
          }
          else
            (*SSL_Response_ptr[function_index])(&check_msg_result);
        }
        break;
      }
    } /* end of switch(GSM_Parameters.stage_action) */
  }
  else
    check_msg_result = GSM_HALT;

  return check_msg_result;
}