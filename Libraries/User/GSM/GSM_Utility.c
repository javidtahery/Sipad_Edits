#include "GSM_Utility.h"
#include "general_functions.h"

gsm_param_typeDef       GSM_Parameters;

__IO uint32_t gsm_pt_timeout;        // GSM Protocol Timeout
__IO uint32_t gsm_sr_timeout;        // GSM Serial Timeout

uint8_t Rx_line_buffer[Sim_USART_BUFFERSIZE];
uint16_t line_buffer_rd_index = 0;
uint16_t line_buffer_counter = 0;

int16_t GSM_HALT_Timer = GSM_TIMER_DEFAULT;
uint8_t GSM_QoS_Timer = GSM_TIMER_DEFAULT;
uint8_t GSM_module_is_configured = RESET;

int16_t GSM_Creg_two_hour_counter = 0;
uint8_t disable_any_gsm_activity = RESET;
uint8_t GSM_network_search_counter = 0;


extern void SSL_Close_Socket(void);
extern void SSL_CIPSHUT(void);


void GSM_power_on(void)
{
  if( (GSM_STATUS_GPIO->IDR & GSM_STATUS_PIN) != GSM_STATUS_PIN)
  {
    GSM_PWR_KEY_GPIO->BSRR = GSM_PWR_KEY_PIN;
    Delay(1200);                  // Atleast enable for 1 sec
    GSM_PWR_KEY_GPIO->BRR = GSM_PWR_KEY_PIN;
  }
  gsm_power_status = SET;
}

void GSM_power_off(void)
{
  if( (GSM_STATUS_GPIO->IDR & GSM_STATUS_PIN) == GSM_STATUS_PIN)
  {
    GSM_PWR_KEY_GPIO->BSRR = GSM_PWR_KEY_PIN;
    Delay(800);                  // Atleast enable for 1.5 sec
    GSM_PWR_KEY_GPIO->BRR = GSM_PWR_KEY_PIN;
  }
  gsm_power_status = RESET;
}


void Increment_GSM_rx_rd_index(uint16_t inc_mount)
{
  uint16_t while_iter = inc_mount;
  while(while_iter)
  {
    if(++GSM_rx_rd_index >= GSM_USART_RX_BUFFER_SIZE)
      GSM_rx_rd_index = 0;
    
    while_iter--;
  }
}

void GSM_clear_RX_buffer(void)
{
  Set_zero(GSM_rx_buffer, GSM_USART_RX_BUFFER_SIZE);
  //memset(GSM_rx_buffer, 0, GSM_USART_RX_BUFFER_SIZE);
  GSM_rx_wr_index = 0;
  GSM_rx_rd_index = 0;
  GSM_rx_counter = 0;
}

// Main function to get a character from sim_rx_buffer.
uint8_t GSM_get_char(void)
{
  uint8_t data;
  gsm_sr_timeout = 100;
  while(GSM_rx_counter == 0)
  {
    if(gsm_sr_timeout == 0)
      return EOF;
  }

  data = GSM_rx_buffer[GSM_rx_rd_index];
  Increment_GSM_rx_rd_index(1);

  --GSM_rx_counter;

  return data;
}

uint8_t GSM_get_char_from_line_buffer(void)
{
  uint8_t data;
  if(line_buffer_counter == 0)
    return 0;
  
  data = Rx_line_buffer[line_buffer_rd_index++];
  --line_buffer_counter;
  
  return data;
}

uint16_t GSM_get_line(char str[], uint16_t limit, uint8_t* error)
{
  uint8_t tmp_char = 0;
  uint8_t prev_char = 0;
  uint16_t i = 0;
  
  gsm_sr_timeout = 200;
  while(1)
  {
    if(GSM_rx_counter > 0)
    {
      if(i < limit - 1)
      {
        tmp_char = GSM_get_char();
        str[i++] = tmp_char;
        
        if(tmp_char == '>')
        {
          str[i] = '\0';
          
          *error = GSM_OK;
          return i;
        }
        else if (prev_char == '\r' && tmp_char == '\n')
        {
          str[i] = '\0';
          
          *error = GSM_OK;
          return i;
        }
        
        prev_char = tmp_char;
      }
    }
    
    if(gsm_sr_timeout == 0)
    {
      *error = GSM_TIMEOUT;
      return 0;
    }
  }
}

uint8_t convert_char_to_int(char data)
{
  return (data - 0x30);
}

char convert_int_to_char(uint8_t data)
{
  return (data + 0x30);
}

uint8_t array_length(uint8_t* array)
{
  uint8_t length = 0;
  while(*(array+length) != '\0' && *(array+length) != 0xFF)
  {
    length++;
  }
  
  return length;
}

uint64_t str_to_llint(char* str, uint8_t number_char)
{
  uint64_t tmp_data = 0;
  char char_num = 0;
  
  for(uint8_t iterator = 0; iterator < number_char; iterator++)
  {
    char_num = str[iterator];
    tmp_data = tmp_data*10 + convert_char_to_int(char_num);
  }
  
  return tmp_data;
}


uint16_t GSM_get_number_from_line_buffer(char terminator, uint64_t *num)
{
  gsm_sr_timeout = 1000;
  
  char number[10] ={0};
  uint8_t i = 0;
  
  while(1)
  {
    if (gsm_sr_timeout == 0)
      return GSM_TIMEOUT;
    
    if(line_buffer_counter > 0 && i < 10)
    {
      char c = GSM_get_char_from_line_buffer();
      if (c == terminator)
        break;
      else
      {
        if((c >= '0' && c <= '9') || c=='-')
          number[i++] = c;
        else
          return GSM_ERROR;
      }
    }
  }
  *num = str_to_llint(number, i);
  
  return GSM_OK;
}

uint16_t GSM_get_n_number_from_line_buffer(uint8_t n, uint64_t *num)
{
  gsm_sr_timeout = 1000;
  
  char number[20] ={0};
  uint8_t i = 0;
  
  while(1)
  {
    if (gsm_sr_timeout == 0)
      return GSM_TIMEOUT;
    
    if(i == n)
      break;
    if(line_buffer_counter > 0 && i<20)
    {
      char c = GSM_get_char_from_line_buffer();
      if(c >= '0' && c <= '9')
        number[i++] = c;
      else
        return GSM_ERROR;
    }
  }
  *num = str_to_llint(number, i);
  return GSM_OK;
}

uint8_t GSM_get_string_from_buffer(uint8_t* buffer, uint8_t* string, uint8_t start_index, char terminator, uint8_t length_limit)
{
  uint8_t iterator = 0;
  
  for(iterator = 0; iterator < length_limit && buffer[start_index+iterator] != terminator; iterator++)
    *(string+iterator) = buffer[start_index+iterator];
  
  return iterator;
}

uint16_t GSM_read_until_from_line_buffer(char terminator)
{
  gsm_sr_timeout = 1000;
  
  while(1)
  {
    if(gsm_sr_timeout == 0)
      return GSM_TIMEOUT;
    
    if(line_buffer_counter > 0)
    {
      char c = GSM_get_char_from_line_buffer();
      if (c == terminator)
        return GSM_OK;
    }
  }
}

uint8_t GSM_read_until_from_buffer(uint8_t* data, uint8_t start_index, char terminator, uint8_t max_read_size)
{
  uint8_t iterator = 0;
  
  for(; iterator < max_read_size; iterator++)
  {
    if(*(data+start_index+iterator) == terminator)
      break;
  }
  
  if(iterator == max_read_size && (*(data+start_index+iterator) != terminator))
    return 0;
  
  return (iterator+1);
}

uint8_t GSM_get_number_from_buffer(uint8_t* data, uint8_t start_index, char terminator, uint8_t max_read_size, uint64_t *num)
{
  char number[10] ={0};
  uint8_t iterator = 0;
  char character = 0;
  
  for(; iterator < max_read_size; iterator++)
  {
    character = *(data + start_index + iterator);
    if(character == terminator)
      break;
    if(character >= '0' && character <= '9')
      number[iterator] = character;
    else
    {
      *num = 0;
      return 0;
    }
  }
  if(iterator == max_read_size && character != terminator)
  {
    *num = 0;
    return 0;
  }
  
  *num = str_to_llint(number, iterator);
  
  return (iterator+1);
}

uint8_t GSM_add_char_number_to_buffer(uint8_t* data, uint16_t* start_index, uint64_t number)
{
  uint8_t digit_count = 0;
  uint64_t tmp_number = number;
  uint8_t remain = 0;
  uint16_t tmp_index = *start_index;
  
  if(number > 0)
  {
    while(tmp_number > 0)
    {
      digit_count++;
      tmp_number /= 10;
    }
    tmp_number = number;
    for(uint8_t i = digit_count; i > 0; i--)
    {
      remain = tmp_number%10;
      *(data+tmp_index+i-1) = convert_int_to_char(remain);
      tmp_number /= 10;
    }
  }
  else
  {
    *(data+tmp_index) = '0';
    digit_count = 1;
  }
  
  tmp_index += digit_count ;
  *start_index = tmp_index;
  
  return digit_count;
}

uint8_t GSM_get_upper_case_char(uint8_t character)
{
  uint8_t tmp = character;
  
   if(tmp >= 0x61 && tmp < 0x7B)
      tmp -= 0x20;
   
   return tmp;
}

uint16_t GSM_check_msg(int16_t* error)
{
  *error = 0;
  Set_zero(Rx_line_buffer, Sim_USART_BUFFERSIZE);
  line_buffer_rd_index = 0;
  
  uint8_t buffer_err = 0; 
  line_buffer_counter = GSM_get_line((char *)Rx_line_buffer, sizeof(Rx_line_buffer), &buffer_err);
  if(line_buffer_counter == 0)
    return GSM_WAITING;
  
  if(Rx_line_buffer[0] == '>')
    return GSM_DATA_MODE;
  if(Rx_line_buffer[0] == '8')
    return GSM_IMEI;
  if(Rx_line_buffer[0] == '4')
  {
    if(Rx_line_buffer[1] == '3')
    {
      if(Rx_line_buffer[2] == '2')
        return GSM_COPS;
    }
  }
  
  char *res;
  /* ............ TCP .............. */
  res = strstr((char *)&Rx_line_buffer[0], "STATE: ");
  if(res)
  {
    line_buffer_rd_index = strlen("STATE: ");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_CONNECTION_STATUS;
  }
  res = strstr((char *)&Rx_line_buffer[0],"CLOSE OK");
  if(res)
  {
    line_buffer_rd_index = strlen("CLOSE OK");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_CLOSE_OK;
  }
  res = strstr((char *)&Rx_line_buffer[0],"CLOSED");
  if(res)
  {
    line_buffer_rd_index = strlen("CLOSED");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_ERR_CLOSED;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QHTTPDL: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QHTTPDL: ");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_HTTP_DL;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QSSLOPEN: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QSSLOPEN: ");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_SSLOPEN;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QIFGCNT: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QIFGCNT: ");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_QIFGCNT;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QSECREAD: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QSECREAD: ");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_QSECREAD;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QSECWRITE: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QSECWRITE: ");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_QSECWRITE;
  }
  res = strstr((char *)&Rx_line_buffer[0],"CONNECT");
  if(res)
  {
    line_buffer_rd_index = strlen("CONNECT");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_CONNECT;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QSSLURC: \"recv\",");
  if(res)
  {
    line_buffer_rd_index = strlen("+QSSLURC: \"recv\",");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_RECV_SERVER_RESP;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QSSLRECV: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QSSLRECV: ");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_READ_SERVER_RESP;
  }
  res = strstr((char *)&Rx_line_buffer[0],"SHUT OK");
  if(res)
  {
    line_buffer_rd_index = strlen("SHUT OK");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_IPSHUT;
  }
  res = strstr((char *)&Rx_line_buffer[0],"DEACT OK");
  if(res)
  {
    line_buffer_rd_index = strlen("DEACT OK");
    line_buffer_counter -= line_buffer_rd_index;
    return GSM_DEACT_PDP;
  }
  res = strstr((char *)&Rx_line_buffer[0], "SEND OK");
  if(res)
  {
    line_buffer_rd_index = strlen("SEND OK");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_SEND_OK;
  }
  res = strstr((char *)&Rx_line_buffer[0],"SEND FAIL");
  if(res)
  {
    line_buffer_rd_index = strlen("SEND FAIL");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_SEND_FAIL;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+SAPBR: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+SAPBR: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_SAPBR;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CGATT: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CGATT: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CGATT;
  }
  
  res = strstr((char *) &Rx_line_buffer[0],"OK");
  if(res)
  {
    line_buffer_rd_index = strlen("OK");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_OK;
  }
  
  /* ............ General Messages .............. */
  res = strstr((char *) &Rx_line_buffer[0],"> ");
  if(res)
  {
    line_buffer_rd_index = strlen("> ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_WAITINGMSG;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CME ERROR: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CME ERROR: ");
    line_buffer_counter -= line_buffer_rd_index;
    
    // First data
    int8_t sign_data = Rx_line_buffer[line_buffer_rd_index];
    if(sign_data == '-')
    {
      line_buffer_rd_index++;
      sign_data = -1;
    }
    else
      sign_data = 1;
    
    uint64_t recv_err = 0;
    GSM_get_number_from_line_buffer('\r', (uint64_t*)&recv_err);
    recv_err *= sign_data;
    
    *error = recv_err;
    return GSM_CME_ERROR;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CMS ERROR: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CMS ERROR: ");
    line_buffer_counter -= line_buffer_rd_index;
    
    // First data
    int8_t sign_data = Rx_line_buffer[line_buffer_rd_index];
    if(sign_data == '-')
    {
      line_buffer_rd_index++;
      sign_data = -1;
    }
    else
      sign_data = 1;
    
    uint64_t recv_err = 0;
    GSM_get_number_from_line_buffer('\r', (uint64_t*)&recv_err);
    recv_err *= sign_data;
    
    *error = recv_err;
    return GSM_CMS_ERROR;
  }
  res = strstr((char *)&Rx_line_buffer[0],"ERROR");
  if(res)
  {
    line_buffer_rd_index = strlen("ERROR");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_ERROR;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CPIN: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CPIN: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CPIN;
  }
  // Network Registration Status:
  res = strstr((char *)&Rx_line_buffer[0],"+CGREG: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CGREG: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CGREG;
  }
  res = strstr((char *)&Rx_line_buffer[0],"SIM PIN");
  if(res)
  {
    line_buffer_rd_index = strlen("SIM PIN");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_SIM_PIN;
  }
  // Network Registration
  res = strstr((char *)&Rx_line_buffer[0],"+CREG: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CREG: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CREG;
  }
  // Network Provider
  res = strstr((char *)&Rx_line_buffer[0],"+COPS: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+COPS: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_COPS;
  }
  // Signal Quality Report
  res = strstr((char *)&Rx_line_buffer[0],"+CSQ: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CSQ: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CSQ;
  }
  // Incomming Call
  res = strstr((char *)&Rx_line_buffer[0],"+CLIP: \"");
  if(res)
  {
    line_buffer_rd_index = strlen("+CLIP: \"");
    line_buffer_counter -= line_buffer_rd_index;
    
//    memset(Call_phone_number, 0, 15);
//    SMS_extract_phone_number(Call_phone_number, &Call_phone_number_length);
    
    return GSM_CALL;
  }
  // Hangup Call
  res = strstr((char *)&Rx_line_buffer[0],"NO CARRIER");
  if(res)
  {
    line_buffer_rd_index = strlen("NO CARRIER");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_HANGUP;
  }
  // Call Status
  res = strstr((char *)&Rx_line_buffer[0],"+CLCC");
  if(res)
  {
    line_buffer_rd_index = strlen("+CLCC");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_CLCC;
  }
  
  /* --------------- Bluetooth --------------- */
  res = strstr((char *)&Rx_line_buffer[0],"+QBTIND: \"pair\",\"");
  if(res)
  {
    line_buffer_rd_index = strlen("+QBTIND: \"pair\",\"");
    line_buffer_counter -= line_buffer_rd_index;
    
    GSM_read_until_from_line_buffer(',');
    uint8_t bt_addr[6] = {0};
    for(uint8_t i = 0; i < 6; i++)
    {
      bt_addr[i] = (GF_convert_string_hex_number_to_integer(Rx_line_buffer[line_buffer_rd_index]) << 4)
        + GF_convert_string_hex_number_to_integer(Rx_line_buffer[line_buffer_rd_index+1]);
      line_buffer_rd_index += 2;
    }
    
    for(uint8_t i = 0; i < bluetooth_device_count; i++)
    {
      if(compareArray(bluetooth_mac_addr[i], bt_addr, BT_MAC_ADDR_SIZE) == 0)
      {
        GSM_Send_Usart("AT+QBTPAIRCNF=1\r", strlen("AT+QBTPAIRCNF=1\r"));
      }
    }
    
    return GSM_BT_PAIR_REQ;
  }
  
  /* --------------- SMS --------------- */
  res = strstr((char *)&Rx_line_buffer[0],"+CMGS: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CMGS: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CMGS;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CMGR: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CMGR: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CMGR;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CPMS: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+CPMS: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_CPMS;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CMT: \"");
  if(res)
  {
    line_buffer_rd_index = strlen("+CMT: \"");
    line_buffer_counter -= line_buffer_rd_index;
    
    //memset(SMS_phone_number, 0, 30);
    //memset(SMS_content, 0, Sim_USART_BUFFERSIZE);
    //SMS_extract_phone_number(SMS_phone_number, &SMS_phone_num_length);
    //SMS_extract_SMS_content(SMS_content, &SMS_content_length);
        
    return GSM_CMT;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+CMTI: \"");
  if(res)
  {
    line_buffer_rd_index = strlen("+CMTI: \"");
    line_buffer_counter -= line_buffer_rd_index;
    
    GSM_read_until_from_line_buffer(',');
    
    uint64_t tmp_data = 0;
    GSM_get_number_from_line_buffer('\r', &tmp_data);
    if(tmp_data > 0)
    {
      if(current_sms_index == 0)
        current_sms_index = tmp_data;
      
      system_has_short_message = SET;
      
      return GSM_CMTI;
    }
    else
    {
      return GSM_UNSPECIFIED;
    }
  }
  res = strstr((char *)&Rx_line_buffer[0],"SIM PUK");
  if(res)
  {
    line_buffer_rd_index = strlen("SIM PUK");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_SIM_PUK;
  }
  res = strstr((char *)&Rx_line_buffer[0],"SIM PIN2");
  if(res)
  {
    line_buffer_rd_index = strlen("SIM PIN2");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_SIM_PIN2;
  }
  res = strstr((char *)&Rx_line_buffer[0],"SIM PUK2");
  if(res)
  {
    line_buffer_rd_index = strlen("SIM PUK2");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_SIM_PUK2;
  }
  
  /* ............ Config ............*/
  res = strstr((char *)&Rx_line_buffer[0],"+QCFG: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QCFG: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_QCFG;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QGBAND: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QGBAND: ");
    line_buffer_counter -= line_buffer_rd_index;

    return GSM_QGBAND;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QLTS: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QLTS: ");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_QLTS;
  }
  
  if(Rx_line_buffer[0] == '\r')
  {
    line_buffer_rd_index = strlen("\r");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_LF;
  }
  
  //---------------- FTP -----------------//
  res = strstr((char *)&Rx_line_buffer[0],"+QFTPSTAT: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFTPSTAT: ");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FTP_STAT;
  }
  
  res = strstr((char *)&Rx_line_buffer[0],"+QFTPOPEN:");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFTPOPEN:");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FTP_OPEN;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QFTPPATH: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFTPPATH: ");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FTP_PATH;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QFTPNLST:");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFTPNLST:");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FTP_NLST;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QFTPSIZE:");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFTPSIZE:");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FTP_SIZE;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QFTPGET:");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFTPGET:");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FTP_GET;
  }
  res = strstr((char *)&Rx_line_buffer[0],"+QFTPCLOSE:");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFTPCLOSE:");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FTP_CLOSE;
  }
  //---------------- File System -----------------//
  res = strstr((char *)&Rx_line_buffer[0],"+QFOPEN: ");
  if(res)
  {
    line_buffer_rd_index = strlen("+QFOPEN: ");
    line_buffer_counter -= line_buffer_rd_index;
    
    return GSM_FS_OPEN;
  }
  

  return GSM_UNSPECIFIED;
}


void GSM_parse_IMEI(void)
{
  uint64_t recv_num = 0;
  
  if(GSM_get_n_number_from_line_buffer(15, &recv_num) != GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    return;
  }
  
  if( GF_calculate_number_digit_count(recv_num) == 15)
  {
    if(save_system_IMEI == SET)
    {
      save_system_IMEI = RESET;
      system_error.IMEI_error = RESET;
      system_IMEI = recv_num;
      
      SPIF_Program_Security_Register(SPIF_SECURITY_REGISTE_1_ADDR, (uint8_t*)&system_IMEI, 8);
    }
  }
  
  GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
  GSM_Parameters.stage_action = SIM_RCV_RESP;
  
  GSM_Parameters.prev_stage = SIM_STAGE_QUERY_IMEI;
  GSM_Parameters.next_stage = SIM_STAGE_QUERY_SIMCARD;
  
  gsm_pt_timeout = GSM_MSG_DEFAULT_TIMEOUT;
  
  GSM_Parameters.number_of_retries_command = 0;
}

void GSM_parse_CPIN(void)
{
  char *res;
  res = strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "READY");
  
  if(res)
  {
    GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
    GSM_Parameters.prev_stage = SIM_STAGE_QUERY_SIMCARD;
    GSM_Parameters.next_stage = SIM_STAGE_QUERY_NETWORK;
    
    GSM_Parameters.number_of_retries_command = 0;
    
    GSM_Parameters.simcard_available = 1;
  }
  else
  {
    GSM_Parameters.stage = SIM_STAGE_RESET_MODULE;
  }
  
  GSM_Parameters.stage_action = SIM_SEND_REQ;
}

void GSM_parse_CREG(void)
{
  uint64_t recv_num = 0;
  
  if(GSM_read_until_from_line_buffer(',') != GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    return;
  }
  if(GSM_get_number_from_line_buffer('\r', &recv_num) != GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    return;
  }
  switch(recv_num)
  {
  case 0:       // Not registered, Not Searching
    {
      GSM_Parameters.next_stage = SIM_STAGE_MODULE_OFF;
      break;
    }
  case 1:       // Registered, home network
    {
      GSM_Parameters.next_stage = SIM_STAGE_QUERY_NETWORK_PROVIDER;
      GSM_Parameters.number_of_retries_command = 0;
      
      GSM_Creg_two_hour_counter = 0;
      GSM_network_search_counter = 0;
      disable_any_gsm_activity = RESET;
      
      // Change LED Pattern
      if(system_current_state == SYSTEM_RUNNING)
        LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
      break;
    }
  case 2:       // Not registered, Searching
    {
      GSM_Parameters.next_stage = SIM_STAGE_QUERY_NETWORK;
      GSM_HALT_Timer = 4;
      break;
    }
  case 3:       // Registration denied
    {
      GSM_Parameters.next_stage = SIM_STAGE_MODULE_OFF;
      
      if(++GSM_network_search_counter > 5)
      {
        GSM_network_search_counter = 0;
        GSM_Creg_two_hour_counter = 1;
        disable_any_gsm_activity = SET;
      }
      
      GSM_HALT_Timer = 2;
      break;
    }
  case 4:       // Unknown
    {
      GSM_Parameters.next_stage = SIM_STAGE_MODULE_OFF;
      
      if(++GSM_network_search_counter > 5)
      {
        GSM_network_search_counter = 0;
        GSM_Creg_two_hour_counter = 1;
        disable_any_gsm_activity = SET;
      }
      
      GSM_HALT_Timer = 3;
      break;
    }
  case 5:       // Registered, roaming
    {
      GSM_Parameters.next_stage = SIM_STAGE_QUERY_NETWORK_PROVIDER;
      GSM_Parameters.number_of_retries_command = 0;
      
      GSM_Creg_two_hour_counter = 0;
      GSM_network_search_counter = 0;
      disable_any_gsm_activity = RESET;
      
      // Change LED Pattern
      if(system_current_state == SYSTEM_RUNNING)
        LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
      break;
    }
  }
  GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.prev_stage = SIM_STAGE_QUERY_NETWORK;
  
  GSM_Parameters.nw_status = (uint8_t)recv_num;
}

void GSM_parse_IMSI(void)
{
  uint64_t recv_num = 0;
  
  char imsi_buffer[5] = {0};
  memcpy(imsi_buffer, Rx_line_buffer, 5);
  recv_num = str_to_llint(imsi_buffer, 5);
  
  GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
  GSM_Parameters.prev_stage = SIM_STAGE_QUERY_NETWORK_PROVIDER;
  GSM_Parameters.next_stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
  
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.number_of_retries_command = 0;
  
  if(GSM_Parameters.nw_provider != recv_num)
  {
    GSM_Parameters.nw_provider = (uint16_t)recv_num;
    GSM_status = GSM_IDLE;
    sending_data_is_in_progress = RESET;
    
    send_command_to_GSM = SET;
    GSM_cmd_execution_code = GSM_BT_STAGE_EN_PWR;
  }
}

void GSM_parse_CSQ(void)
{
  uint64_t recv_num = 0;
  
  if(GSM_get_number_from_line_buffer(',', &recv_num) != GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    return;
  }
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.stage = SIM_STAGE_SEEK_OK;
  GSM_Parameters.prev_stage = SIM_STAGE_QUERY_SIGNAL_QUALITY;
  GSM_Parameters.next_stage = SIM_STAGE_PRE_SEND_DATA;
  
  GSM_Parameters.signal_quality = (uint16_t)recv_num;
  GSM_Parameters.number_of_retries_command = 0;
  
  // Generating An Event
  if(recv_num < 5)
  {
    Force_Produce_Event();
    Event_Flags.gsm_lost_signal = SET;
  }
}

void GSM_parse_Connection_STATUS(void)
{
  char *res;
  res = strstr((char *) &Rx_line_buffer[line_buffer_rd_index],"IP INITIAL");
  if(res)
  {
    GSM_Parameters.stage = SIM_STAGE_QUERY_FOREGROUND_CONTEXT;
    
    GSM_Parameters.number_of_retries_command = 0;
  }
  else
  {
    res = strstr((char *) &Rx_line_buffer[line_buffer_rd_index],"IP START");
    if(res)
    {
      GSM_Parameters.stage = SIM_STAGE_CONNECT_GPRS;
      
      GSM_Parameters.number_of_retries_command = 0;
    }
    else
    {
      res = strstr((char *) &Rx_line_buffer[line_buffer_rd_index],"IP CONFIG");
      if(res)
      {
        GSM_Parameters.stage = SIM_STAGE_QUERY_LOCAL_IP;
      }
      else
      {
        res = strstr((char *) &Rx_line_buffer[line_buffer_rd_index],"IP GPRSACT");
        if(res)
        {
          GSM_Parameters.stage = SIM_STAGE_QUERY_LOCAL_IP;
        }
        else
        {
          res = strstr((char *) &Rx_line_buffer[line_buffer_rd_index],"IP STATUS");
          if(res)
          {
            GSM_Parameters.stage = SIM_STAGE_CFG_SSL_VERSION;
            
            GSM_Parameters.number_of_retries_command = 0;
            
            GSM_Parameters.internet_connection = SET;
          }
          else
          {
            res = strstr((char *) &Rx_line_buffer[line_buffer_rd_index],"IP PROCESSING");
            if(res)
            {
              if(server_socket == SET)
                GSM_Parameters.stage = SIM_STAGE_QUERY_SIGNAL_QUALITY;
              else
              {
                GSM_Parameters.stage = SIM_STAGE_QUERY_RAM_CA0_FILE;
                
                if(GSM_Parameters.internet_connection == SET)
                  GSM_Parameters.stage = SIM_STAGE_OPEN_SOCKET;
              }
                
              
              GSM_Parameters.number_of_retries_command = 0;
            }
            else
            {
              res = strstr((char *) &Rx_line_buffer[line_buffer_rd_index],"PDP DEACT");
              if(res)
              {
                GSM_Parameters.stage = SIM_STAGE_CIPSHUT;
                GSM_Parameters.internet_connection = RESET;
                
                sending_data_is_in_progress = RESET;
                GSM_status = GSM_IDLE;
                sending_data_is_in_progress = RESET;
                
                // Change LED Pattern
                if(system_current_state == SYSTEM_RUNNING)
                  LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
              }
            }
          }
        }
      }
    }
  }
  
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.prev_stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
  GSM_Parameters.next_stage = 0;
}

void GSM_Send_Set_APN_cmd(void)
{
  uint8_t msg[40] = {0};
  uint8_t msg_length = 0;
  if(setting.custom_APN == RESET)
  {
    if(GSM_Parameters.nw_provider == 0)
    {
      GSM_status = GSM_INITIATING;
      GSM_Parameters.stage = SIM_STAGE_QUERY_NETWORK;
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.prev_stage = 0;
      GSM_Parameters.next_stage = 0;
      gsm_pt_timeout = 0;
      GSM_Parameters.number_of_retries_command = 0;
      return;
    }
    
    switch(GSM_Parameters.nw_provider)
    {
    case 43235:
      {
        msg_length = 26;
        memcpy(msg, "AT+QICSGP=1,\"mtnirancell\"\r", msg_length);
        break;
      }
    case 43211:
      {
        msg_length = 21;
        memcpy(msg, "AT+QICSGP=1,\"mcinet\"\r", msg_length);
        break;
      }
    case 43220:
      {
        msg_length = 22;
        memcpy(msg, "AT+QICSGP=1,\"rightel\"\r", msg_length);
        break;
      }
    case 43208:
      {
        msg_length = 27;
        memcpy(msg, "AT+QICSGP=1,\"shatelmobile\"\r", msg_length);
        break;
      }
    case 43232:
      {
        msg_length = 24;
        memcpy(msg, "AT+QICSGP=1,\"taliyanet\"\r", msg_length);
        break;
      }
    }
  }
  else
  {
    msg_length = 13;
    memcpy(msg, "AT+QICSGP=1,\"", msg_length);
    memcpy(&msg[msg_length], setting.APN, setting.APN_length);
    msg_length += setting.APN_length;
    msg[msg_length++] = '\"';
    msg[msg_length++] = '\r';
  }
  
  GSM_Send_Usart(msg, msg_length);
}

uint8_t GSM_handle_server_response(uint16_t response_length)
{
  uint8_t result = 0;
  
  result = SIP_parse_server_response(&GSM_rx_buffer[GSM_rx_rd_index], response_length);
  
  Increment_GSM_rx_rd_index(response_length + 6);     // \r\n -- (ok\r\n) are remained
  GSM_rx_counter -=  (response_length + 6);     // \r\n -- (ok\r\n) are remained
  
  return result;
}

void GSM_send_cmd_to_GSM(void)
{
  if(GSM_HALT_Timer == GSM_TIMER_DEFAULT)
  {
    if(GSM_Parameters.stage_action == SIM_SEND_REQ)
    {
      switch(GSM_cmd_execution_code)
      {
      case 0:
        {
          if(GSM_status == GSM_IDLE)
          {
            GSM_clear_RX_buffer();
            send_command_to_GSM = RESET;
            
            // Change LED Pattern
            if(system_current_state == SYSTEM_RUNNING)
              LED_change_mode(GSM_LED, LED_OFF);
          }
          break;
        }
      case SIM_STAGE_CLOSE_SOCKET:
        {
          if(GSM_status == GSM_IDLE)
          {
            SSL_Close_Socket();
            GSM_cmd_execution_code = SIM_STAGE_CIPSHUT;
            server_socket = RESET;
            GSM_HALT_Timer = 2;
          }
          break;
        }
      case SIM_STAGE_CIPSHUT:
        {
          if(GSM_status == GSM_IDLE)
          {
            SSL_CIPSHUT();
            GSM_cmd_execution_code = 0;
            GSM_HALT_Timer = 2;
            
            GSM_Parameters.stage_action = SIM_SEND_REQ;
            GSM_Parameters.stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
            GSM_Parameters.prev_stage = 0;
            GSM_Parameters.number_of_retries_command = 0;
            GSM_Parameters.internet_connection = RESET;
          }
          break;
        }
      case SIM_STAGE_QUERY_SIMCARD:
        {
          int16_t error_code = 0;
          GSM_Send_Usart("AT+CPIN?\r",sizeof("AT+CPIN?\r"));
          GSM_cmd_execution_code = 0;
          send_command_to_GSM = RESET;
          
          gsm_pt_timeout = 1000;
          uint16_t check_msg_result = GSM_WAITING;
          
          while(gsm_pt_timeout > 0)
          {
            check_msg_result = GSM_check_msg(&error_code);
            if(gsm_pt_timeout == 0 && check_msg_result == GSM_WAITING)
            {
            }
            else
            {
              if(check_msg_result == GSM_LF || check_msg_result == GSM_WAITING)
                continue;
              if(check_msg_result == GSM_CME_ERROR)
              {
                if(error_code == CMEE_NO_SIMCARD)
                {
                  GSM_Parameters.stage = SIM_STAGE_RESET_MODULE;
                  GSM_Parameters.stage_action = SIM_SEND_REQ;
                  GSM_Parameters.number_of_retries_command = 0;
                  GSM_status = GSM_INITIATING;
                  GSM_Parameters.simcard_available = 0;
                  GSM_Parameters.nw_provider = 0;
                }
              }
              else if(check_msg_result == GSM_CPIN)
              {
                char *res;
                res = strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "READY");
                
                if(res == 0)
                {
                  GSM_Parameters.stage = SIM_STAGE_RESET_MODULE;
                  GSM_Parameters.stage_action = SIM_SEND_REQ;
                  GSM_Parameters.number_of_retries_command = 0;
                  GSM_status = GSM_INITIATING;
                  GSM_Parameters.simcard_available = 0;
                  GSM_Parameters.nw_provider = 0;
                }
                else
                  break;
              }
            }
          }
          
          GSM_clear_RX_buffer();
          
          break;
        }
      case SIM_STAGE_DEL_SMS:
        {
          if(GSM_status == GSM_IDLE && GSM_Parameters.simcard_available == SET)
          {
            GSM_Send_Usart("AT+QMGDA=\"DEL ALL\"\r", sizeof("AT+QMGDA=\"DEL ALL\"\r"));
            send_command_to_GSM = RESET;
            Delay(300);
            GSM_clear_RX_buffer();
          }
          
          break;
        }
      case GSM_BT_STAGE_EN_PWR:
        {
          int16_t error_code = 0;
          BT_Enable();
          
          gsm_pt_timeout = 1000;
          uint16_t check_msg_result = GSM_WAITING;
          
          while(gsm_pt_timeout > 0)
          {
            check_msg_result = GSM_check_msg(&error_code);
            if(gsm_pt_timeout == 0 && check_msg_result == GSM_WAITING)
            {
            }
            else
            {
              if(check_msg_result == GSM_LF || check_msg_result == GSM_WAITING)
                continue;
              if(check_msg_result == GSM_CME_ERROR)
              {
                if(error_code == CMEE_NO_SIMCARD)
                {
                  GSM_Parameters.stage = SIM_STAGE_RESET_MODULE;
                  GSM_Parameters.stage_action = SIM_SEND_REQ;
                  GSM_Parameters.number_of_retries_command = 0;
                  GSM_status = GSM_INITIATING;
                  GSM_Parameters.simcard_available = 0;
                  GSM_Parameters.nw_provider = 0;
                }
              }
              else if(check_msg_result == GSM_OK)
              {
                GSM_cmd_execution_code = GSM_BT_STAGE_SET_NAME;
                GSM_HALT_Timer = 3;
                break;
              }
            }
          }
          
          GSM_clear_RX_buffer();
          
          break;
        }
      case GSM_BT_STAGE_SET_NAME:
        {
          int16_t error_code = 0;
          BT_Set_BT_Name();
          GSM_cmd_execution_code = 0;
          
          gsm_pt_timeout = 1000;
          uint16_t check_msg_result = GSM_WAITING;
          
          while(gsm_pt_timeout > 0)
          {
            check_msg_result = GSM_check_msg(&error_code);
            if(gsm_pt_timeout == 0 && check_msg_result == GSM_WAITING)
            {
            }
            else
            {
              if(check_msg_result == GSM_LF || check_msg_result == GSM_WAITING)
                continue;
              if(check_msg_result == GSM_CME_ERROR)
              { 
                if(error_code == CMEE_NO_SIMCARD)
                {
                  GSM_Parameters.stage = SIM_STAGE_RESET_MODULE;
                  GSM_Parameters.stage_action = SIM_SEND_REQ;
                  GSM_Parameters.number_of_retries_command = 0;
                  GSM_status = GSM_INITIATING;
                  GSM_Parameters.simcard_available = 0;
                  GSM_Parameters.nw_provider = 0;
                }
              }
              else if(check_msg_result == GSM_OK)
              {
                GSM_cmd_execution_code = 0;
                send_command_to_GSM = RESET;
                break;
              }
            }
          }
          
          GSM_clear_RX_buffer();
          
          break;
        }
      }
    }
  }
}