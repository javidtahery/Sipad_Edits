#include "SMS_Process.h"
#include "time.h"
#include "general_functions.h"


/*--------------------- Variables --------------------- */
int16_t GSM_HALT_SMS_CALL_Timer = GSM_TIMER_DEFAULT;

uint8_t SMS_phone_number[ADMIN_NUMBER_MAX_LENGTH] = {0};
uint8_t SMS_phone_num_length = 0;
uint8_t SMS_content[SMS_CONTEN_BUFFER_SIZE] = {0};
uint8_t SMS_content_length = 0;
uint8_t SMS_send_buffer[MAX_SEND_SMS_SIZE];
uint16_t sms_send_length;

uint8_t SMS_create_cmd_info_content(uint16_t id);
uint8_t SMS_find_CMD_code(uint8_t* cmd_string);

uint8_t current_sms_index = 0;
uint8_t system_has_short_message = RESET;

uint8_t SMS_stage_action = SIM_SEND_REQ;
uint8_t SMS_stage = 0;
uint8_t SMS_next_stage = 0;
uint8_t SMS_stage_number_of_retries = 0;


extern uint8_t firmware_downloading_process;
extern uint8_t new_firmware_available;


/*--------------------- Functions --------------------- */
void RI_EXTI_Callback(void)
{
}


void SMS_extract_phone_number(uint8_t* phone_num, uint8_t* num_length)
{
  uint8_t iteration;
  for(iteration = 0; (iteration < ADMIN_NUMBER_MAX_LENGTH)
      && (Rx_line_buffer[line_buffer_rd_index+iteration] != '"'); iteration++)
    *(phone_num+iteration) = Rx_line_buffer[line_buffer_rd_index+iteration];
  *(phone_num+iteration) = '\0';
  *num_length = iteration;

  GSM_read_until_from_line_buffer('\r');
}

uint16_t SMS_extract_SMS_content(uint8_t* msg, uint8_t* msg_length)
{
  uint8_t buffer_err = 0; 
  
  Set_zero(Rx_line_buffer, Sim_USART_BUFFERSIZE);
  line_buffer_rd_index = 0;

  line_buffer_counter = GSM_get_line((char *)Rx_line_buffer, sizeof(Rx_line_buffer), &buffer_err);
  
  if(buffer_err == GSM_OK)
  {
    if(line_buffer_counter > 0 && line_buffer_counter <= SMS_CONTEN_BUFFER_SIZE)
    {
      memcpy(msg, Rx_line_buffer, line_buffer_counter);
      *msg_length = line_buffer_counter;
      
      return GSM_OK;
    }
  }
  return GSM_ERROR;
}


void SMS_parse_short_message(uint8_t* msg, uint8_t msg_length, uint16_t* setting_id)
{
  uint8_t buffer_index = 0;
  uint8_t tmp_data_8 = 0;
  uint8_t msg_is_setting = RESET;
  char terminator = '#';
  uint8_t cmd_string[15] = {0};
  uint8_t code_id = 0;
  *setting_id = 0;
  
  // ##
  buffer_index += 2;
  
  // Finding ending '#'
  tmp_data_8 = GSM_read_until_from_buffer(msg, buffer_index, '#', 50);
  if(tmp_data_8 > 0)
  {
    // Finding any ','
    tmp_data_8 = GSM_read_until_from_buffer(msg, buffer_index, ',', 40);
    if(tmp_data_8 > 0)
    {
      msg_is_setting = SET;
      terminator = ',';
    }
  }
  
  // Seprating CMD part
  tmp_data_8 = GSM_get_string_from_buffer(msg, cmd_string, buffer_index, terminator, 15);
  if(tmp_data_8 > 0)
  {
    buffer_index += tmp_data_8;
    buffer_index++;                   // For terminator
    // Convert cmd string to upper case
    for(uint8_t idx = 0; idx < tmp_data_8; idx++)
      cmd_string[idx] = GSM_get_upper_case_char(cmd_string[idx]);
  }
  
  // Parsing CMD Code
  code_id = SMS_find_CMD_code(cmd_string);
  
  switch(msg_is_setting)
  {
  case RESET:
    {
      uint8_t cmd_result = RESET;
      cmd_result = SMS_create_cmd_info_content((uint16_t)code_id);
      if(cmd_result == SET)
        *setting_id = SMS_INFO_TYPE;
      break;
    }
  case SET:
    {
      // Parse setting
      uint8_t cmd_result = RESET;
      cmd_result = SMS_parse_CMD((uint16_t)code_id, msg, buffer_index);
      if(cmd_result == SET)
        *setting_id = (uint16_t)code_id;
      break;
    }
  }
}

uint8_t SMS_find_CMD_code(uint8_t* cmd_string)
{
  char* cmp_result = 0;
  cmp_result = strstr((char*)cmd_string, "VERSION");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_SW_HW_VERSION;

  cmp_result = strstr((char*)cmd_string, "SERVER");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_SERVER;

  cmp_result = strstr((char*)cmd_string, "APN");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_APN;

  cmp_result = strstr((char*)cmd_string, "RESET");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_REBOOT;

  cmp_result = strstr((char*)cmd_string, "TIMER");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_SAMPLE_RATE_TIME;

  cmp_result = strstr((char*)cmd_string, "DISTANCE");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_SAMPLE_RATE_METER;
  
  cmp_result = strstr((char*)cmd_string, "HBT");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_SAMPLE_RATE_HBT;

  cmp_result = strstr((char*)cmd_string, "ANGLE");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_ANGLE_THRESHOLD;

  cmp_result = strstr((char*)cmd_string, "CLEAR");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_ERASE_RECORDS;

  cmp_result = strstr((char*)cmd_string, "FACTORY");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_FACTORY_DEFAULT;
  
  cmp_result = strstr((char*)cmd_string, "HTTPSERV");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_HTTP_SERVER;
  
  cmp_result = strstr((char*)cmd_string, "DISHTTP");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_DISABLE_HTTP;

  cmp_result = strstr((char*)cmd_string, "ERRORS");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_ERRORS;
  
  cmp_result = strstr((char*)cmd_string, "DEBUG");
  if(cmp_result > 0)
    return SMS_SETTING_CODE_DEBUG_MODE;
  
  return 0;
}

uint8_t SMS_parse_CMD(uint16_t id, uint8_t* data, uint8_t data_start_index)
{
  uint8_t func_result = RESET;
  uint8_t tmp_data_8 = 0;
  uint64_t tmp_data_64 = 0;

  switch(id)
  {
  case SMS_SETTING_CODE_SERVER:
    {
      tmp_data_8 = GSM_read_until_from_buffer(data, data_start_index, ',', URL_MAX_LENGTH_SIZE);
      if(tmp_data_8 > 1)        // 1 is for '\r'
      {
        // Server
        memset(&setting.server_url, 0xFF , URL_MAX_LENGTH_SIZE);
        setting.server_url[0] = '#';
        setting.server_url_length = tmp_data_8-1;
        memcpy(&setting.server_url[1], (data+data_start_index), (tmp_data_8-1));
        data_start_index += tmp_data_8;

        // Port
        tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 7, &tmp_data_64);
        if(tmp_data_8 > 1)
        {
          // Clearing the server setting
          if(tmp_data_64 == 0)
          {
            memset(&setting.server_url, 0xFF , URL_MAX_LENGTH_SIZE);
            setting.server_url_length = 0;
            setting.server_port = 0;
          }
          else
          {
            setting.server_port = (uint16_t)tmp_data_64;
          }
          
          logined_to_the_server = RESET;
          sending_data_is_in_progress = RESET;
          GSM_Parameters.stage_action = SIM_SEND_REQ;
          func_result = SET;
        }
      }
      break;
    }
  case SMS_SETTING_CODE_APN:
    {
      tmp_data_8 = GSM_read_until_from_buffer(data, data_start_index, '#', APN_MAX_LENGTH_SIZE);
      if( tmp_data_8 == 2 && *(data+data_start_index) == '0')
      {
        memset(&setting.APN, 0 , APN_MAX_LENGTH_SIZE);
        setting.APN_length = 0;

        logined_to_the_server = RESET;
        sending_data_is_in_progress = RESET;
        func_result = SET;
      }
      else if(tmp_data_8 > 2)        // 1 is for '#'
      {
        memset(&setting.APN, 0xFF , APN_MAX_LENGTH_SIZE);
        setting.APN_length = tmp_data_8-1;
        memcpy(setting.APN, (data+data_start_index), (tmp_data_8-1));

        logined_to_the_server = RESET;
        sending_data_is_in_progress = RESET;
        func_result = SET;
      }
      break;
    }
  case SMS_SETTING_CODE_SAMPLE_RATE_TIME:
    {
      tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 4, &tmp_data_64);
      if(tmp_data_8 > 1 && tmp_data_64 != 0 && tmp_data_64 < 250)        // '#'
      {
        setting.data_sample_rate_time = (uint8_t)tmp_data_64;

        func_result = SET;
      }
      break;
    }
  case SMS_SETTING_CODE_SAMPLE_RATE_METER:
    {
      tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 6, &tmp_data_64);
      if(tmp_data_8 > 1 && tmp_data_64 != 0)        // '#'
      {
        setting.data_sample_rate_meter = (uint16_t)tmp_data_64;

        func_result = SET;
      }
      break;
    }
  case SMS_SETTING_CODE_SAMPLE_RATE_HBT:
    {
      tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 4, &tmp_data_64);
      if(tmp_data_8 > 1 && tmp_data_64 != 0 && tmp_data_64 < 250)        // '#'
      {
        setting.heartbeat_rate_time = (uint8_t)tmp_data_64;

        func_result = SET;
      }
      break;
    }
  case SMS_SETTING_CODE_ANGLE_THRESHOLD:
    {
      tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 6, &tmp_data_64);
      if(tmp_data_8 > 1 && (tmp_data_64 > 0 && tmp_data_64 <= 90) )        // '#'
      {
        setting.angle_threshold = (uint16_t)tmp_data_64;

        func_result = SET;
      }
      break;
    }
  case SMS_SETTING_CODE_ERASE_RECORDS:
    {
      tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 3, &tmp_data_64);
      if(tmp_data_8 > 1)        // '#'
      {
        if(tmp_data_64 == 1)
        {
          FCB_erase_all_records();

          func_result = SET;
        }
      }
      break;
    }
  case SMS_SETTING_CODE_HTTP_SERVER:
    {
      tmp_data_8 = GSM_read_until_from_buffer(data, data_start_index, '#', URL_MAX_LENGTH_SIZE);
      if(tmp_data_8 > 1)        // 1 is for '\r'
      {
        // Server
        memset(&setting.http_server, 0xFF , URL_MAX_LENGTH_SIZE);
        setting.http_server[0] = '#';
        setting.http_server_length = tmp_data_8-1;
        memcpy(&setting.http_server[1], (data+data_start_index), (tmp_data_8-1));
        data_start_index += tmp_data_8;

        logined_to_the_server = RESET;
        sending_data_is_in_progress = RESET;
        GSM_Parameters.stage_action = SIM_SEND_REQ;
        func_result = SET;
      }
      break;
    }
  case SMS_SETTING_CODE_DISABLE_HTTP:
    {
      tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 3, &tmp_data_64);
      if(tmp_data_8 > 1)        // '#'
      {
        if(tmp_data_64 == 1)
        {
          setting.disable_http = SET;
          func_result = SET;
        }
        else if(tmp_data_64 == 0)
        {
          setting.disable_http = RESET;
          func_result = SET;
        }
      }
      break;
    }
  case SMS_SETTING_CODE_DEBUG_MODE:
    {
      tmp_data_8 = GSM_get_number_from_buffer(data, data_start_index, '#', 3, &tmp_data_64);
      if(tmp_data_8 > 1)        // '#'
      {
        if(tmp_data_64 == 1)
        {
          if(setting.disable_http == RESET)
          {
            if(debug_mode == RESET)
            {
              uint16_t timer = 2 * 60 * 60;         // Default time - 2 Hours
              if(setting.debug_mode_time > 0)
                timer = setting.debug_mode_time * 60;
              
              debug_mode = SET;
              debug_mode_counter = timer;
              enable_send_data_to_sipaad = RESET;
            }
          }
        }
      }
      break;
    }
  }     // End of switch(id)

  if(func_result == SET)
    save_changed_setting = SET;

  return func_result;
}

uint8_t SMS_create_cmd_info_content(uint16_t id)
{
  uint8_t func_result = RESET;

  memset(SMS_send_buffer, MAX_SEND_SMS_SIZE, 0);
  sms_send_length = 0;

  switch(id)
  {
  case SMS_SETTING_CODE_SW_HW_VERSION:
    {
      memcpy(SMS_send_buffer, "Fwr: ", strlen("Fwr: "));
      sms_send_length += strlen("Fwr: ");
      memcpy(&SMS_send_buffer[sms_send_length], Software_version, strlen(Software_version));
      sms_send_length += strlen(Software_version);
      
      memcpy(&SMS_send_buffer[sms_send_length], "\nBL: 0x00", strlen("\nBL: 0x00"));
      sms_send_length += strlen("\nBL: 0x00");
      if(Bootloader_release_date != 0 && Bootloader_release_date != 0xFFFFFFFF)
      {
        uint32_t temp_32 = Bootloader_release_date;
        
        SMS_send_buffer[sms_send_length++] = (uint8_t)((temp_32 & 0x00F00000) >> 20) + 0x30;
        SMS_send_buffer[sms_send_length++] = (uint8_t)((temp_32 & 0x000F0000) >> 16) + 0x30;
        SMS_send_buffer[sms_send_length++] = (uint8_t)((temp_32 & 0x0000F000) >> 12) + 0x30;
        SMS_send_buffer[sms_send_length++] = (uint8_t)((temp_32 & 0x00000F00) >> 8) + 0x30;
        SMS_send_buffer[sms_send_length++] = (uint8_t)((temp_32 & 0x000000F0) >> 4) + 0x30;
        SMS_send_buffer[sms_send_length++] = (uint8_t)(temp_32 & 0x0000000F) + 0x30;
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "220202", strlen("220202"));
        sms_send_length += strlen("220202");
      }
      
      memcpy(&SMS_send_buffer[sms_send_length], "\nIM: ", strlen("\nIM: "));
      sms_send_length += strlen("\nIM: ");
      
      if( GF_calculate_number_digit_count(system_IMEI) == 15)
      {
        uint8_t device_IMEI[15] = {0};
        uint16_t temp_index = 0;
        GSM_add_char_number_to_buffer(device_IMEI, &temp_index, system_IMEI);
        
        memcpy(&SMS_send_buffer[sms_send_length], device_IMEI, 15);
        sms_send_length += 15;
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Error In IMEI", strlen("Error In IMEI"));
        sms_send_length += strlen("Error In IMEI");
      }
      
      memcpy(&SMS_send_buffer[sms_send_length], "\nSN: ", strlen("\nSN: "));
      sms_send_length += strlen("\nSN: ");
      
      if( GF_calculate_number_digit_count(system_Serial) == 10)
      {
        GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, system_Serial);
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Error In Serial", strlen("Error In Serial"));
        sms_send_length += strlen("Error In Serial");
      }
	  
      time_t event_time_unix = system_Unixtime + (210*60);
      struct tm UTC;

      memcpy(&SMS_send_buffer[sms_send_length], "\nDate: ", strlen("\nDate: "));
      sms_send_length += strlen("\nDate: ");

      UTC = *localtime(&event_time_unix);
      /* ------------ Year ------------ */
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, (UTC.tm_year + 1900));
      SMS_send_buffer[sms_send_length] = '/';
      sms_send_length++;

      /* ------------ Month ------------ */
      if(UTC.tm_mon < 9)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, (UTC.tm_mon + 1));
      SMS_send_buffer[sms_send_length] = '/';
      sms_send_length++;

      /* ------------ Day ------------ */
      if(UTC.tm_mday < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, UTC.tm_mday);
      SMS_send_buffer[sms_send_length] = ',';
      sms_send_length++;
      SMS_send_buffer[sms_send_length] = ' ';
      sms_send_length++;

      memcpy(&SMS_send_buffer[sms_send_length], "Time: ", strlen("Time: "));
      sms_send_length += strlen("Time: ");

      /* ------------ Hour ------------ */
      if(UTC.tm_hour < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, UTC.tm_hour);
      SMS_send_buffer[sms_send_length] = ':';
      sms_send_length++;
      /* ------------ Minutes ------------ */
      if(UTC.tm_min < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, UTC.tm_min);
      SMS_send_buffer[sms_send_length] = ':';
      sms_send_length++;
      /* ------------ Seconds ------------ */
      if(UTC.tm_sec < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, UTC.tm_sec);
      SMS_send_buffer[sms_send_length] = '\n';
      sms_send_length++;

      /* ------------ Speed ------------ */
      memcpy(&SMS_send_buffer[sms_send_length], "Speed: ", strlen("Speed: "));
      sms_send_length += strlen("Speed: ");

      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, GPS_data.speed_Kmh);
      SMS_send_buffer[sms_send_length] = ',';
      sms_send_length++;
      SMS_send_buffer[sms_send_length] = ' ';
      sms_send_length++;

      /* ------------ PDOP ------------ */
      memcpy(&SMS_send_buffer[sms_send_length], "PDOP: ", strlen("PDOP: "));
      sms_send_length += strlen("PDOP: ");

      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, GPS_data.pdop);
      SMS_send_buffer[sms_send_length] = '\n';
      sms_send_length++;

      /* ------------ URL ------------ */
//      memcpy(&SMS_send_buffer[sms_send_length], "URL:", strlen("URL:"));
//      sms_send_length += strlen("URL:");
//      memcpy(&SMS_send_buffer[sms_send_length], "http://maps.google.com/?q=", strlen("http://maps.google.com/?q="));
//      sms_send_length += strlen("http://maps.google.com/?q=");
//      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, (last_valid_latitude / PRECISION_POLYNOMIAL));
//      SMS_send_buffer[sms_send_length] = '.';
//      sms_send_length++;
//
//      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, ((last_valid_latitude % PRECISION_POLYNOMIAL)/10));
//      SMS_send_buffer[sms_send_length] = ',';
//      sms_send_length++;
//
//      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, (last_valid_longitude / PRECISION_POLYNOMIAL));
//      SMS_send_buffer[sms_send_length] = '.';
//      sms_send_length++;
//
//      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, ((last_valid_longitude % PRECISION_POLYNOMIAL)/10) );
//      memcpy(&SMS_send_buffer[sms_send_length], "&om=1speed:", strlen("&om=1speed:"));
//      sms_send_length += strlen("&om=1speed:");
//
//      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, GPS_data.speed_Kmh);
//
//      SMS_send_buffer[sms_send_length++] = '\n';
      
      // MCU Temprature
//      memcpy(&SMS_send_buffer[sms_send_length], "Temp: ", strlen("Temp: "));
//      sms_send_length += strlen("Temp: ");
//      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, ADC_Values.Average_Temperture);
//      SMS_send_buffer[sms_send_length++] = '\n';

      // Count of not sent record
      memcpy(&SMS_send_buffer[sms_send_length], "Not sent: ", strlen("Not sent: "));
      sms_send_length += strlen("Not sent: ");
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, FCB_profile.s1_nsend_count);
      SMS_send_buffer[sms_send_length++] = '\n';

      // Uptime
      memcpy(&SMS_send_buffer[sms_send_length], "Up: ", strlen("Up: "));
      sms_send_length += strlen("Up: ");
      uint16_t days = 0;
      uint8_t hours = 0;
      uint8_t minutes = 0;
      uint8_t seconds = 0;
      uint32_t remains_seconds = 0;
      days = (system_uptime / 86400);
      remains_seconds  = system_uptime - (days*86400);
      hours = (remains_seconds/3600);
      remains_seconds -= (hours*3600);
      minutes = (remains_seconds/60);
      seconds = remains_seconds - (minutes * 60);
      /* ------------ Days ------------ */
      if(days < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, days);
      SMS_send_buffer[sms_send_length] = ':';
      sms_send_length++;
      /* ------------ Hour ------------ */
      if(hours < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, hours);
      SMS_send_buffer[sms_send_length] = ':';
      sms_send_length++;
      /* ------------ Minutes ------------ */
      if(minutes < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, minutes);
      SMS_send_buffer[sms_send_length] = ':';
      sms_send_length++;
      /* ------------ Seconds ------------ */
      if(seconds < 10)  // Padding
      {
        SMS_send_buffer[sms_send_length] = '0';
        sms_send_length++;
      }
      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, seconds);

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_SERVER:
    {
      memcpy(SMS_send_buffer, "Server: ", strlen("Server: "));
      sms_send_length += strlen("Server: ");

      if(setting.server_url_length)
      {
        memcpy(&SMS_send_buffer[sms_send_length], &setting.server_url[1], setting.server_url_length);
        sms_send_length += setting.server_url_length;
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Not set", strlen("Not set"));
        sms_send_length += strlen("Not set");
      }
      SMS_send_buffer[sms_send_length++] = '\n';

      memcpy(&SMS_send_buffer[sms_send_length], "Port: ", strlen("Port: "));
      sms_send_length += strlen("Port: ");

      if(setting.server_port)
        GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, setting.server_port);
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Not set", strlen("Not set"));
        sms_send_length += strlen("Not set");
      }

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_APN:
    {
      memcpy(SMS_send_buffer, "APN: ", strlen("APN: "));
      sms_send_length += strlen("APN: ");

      if(setting.APN_length)
      {
        memcpy(&SMS_send_buffer[sms_send_length], setting.APN, setting.APN_length);
        sms_send_length += setting.APN_length;
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Not set", strlen("Not set"));
        sms_send_length += strlen("Not set");
      }

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_SAMPLE_RATE_TIME:
    {
      memcpy(SMS_send_buffer, "Record Generation Time Interval: ", strlen("Record Generation Time Interval: "));
      sms_send_length += strlen("Record Generation Time Interval: ");

      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, setting.data_sample_rate_time);

      memcpy(&SMS_send_buffer[sms_send_length], " seconds", strlen(" seconds"));
      sms_send_length += strlen(" seconds");

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_SAMPLE_RATE_METER:
    {
      memcpy(SMS_send_buffer, "Record Generation Distance Interval: ", strlen("Record Generation Distance Interval: "));
      sms_send_length += strlen("Record Generation Distance Interval: ");

      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, setting.data_sample_rate_meter);

      memcpy(&SMS_send_buffer[sms_send_length], " meters", strlen(" meters"));
      sms_send_length += strlen(" meters");

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_SAMPLE_RATE_HBT:
    {
      memcpy(SMS_send_buffer, "Heartbeat Time Interval: ", strlen("Heartbeat Time Interval: "));
      sms_send_length += strlen("Heartbeat Time Interval: ");

      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, setting.heartbeat_rate_time);

      memcpy(&SMS_send_buffer[sms_send_length], " minutes", strlen(" minutes"));
      sms_send_length += strlen(" minutes");

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_ANGLE_THRESHOLD:
    {
      memcpy(SMS_send_buffer, "Angle Threshold: ", strlen("Angle Threshold: "));
      sms_send_length += strlen("Angle Threshold: ");

      GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, setting.angle_threshold);

      memcpy(&SMS_send_buffer[sms_send_length], " degree", strlen(" degree"));
      sms_send_length += strlen(" degree");

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_REBOOT:
    {
      __NVIC_SystemReset();
    }
  case SMS_SETTING_CODE_FACTORY_DEFAULT:
    {
      memcpy(SMS_send_buffer, "OK", 2);
      sms_send_length += 2;

      AVL_Initiate_Setting();
      save_changed_setting = SET;
      logined_to_the_server = RESET;
      sending_data_is_in_progress = RESET;
      system_temp_params.device_version = INITIAL_DEVICE_VERSION;
      SPIF_Write_sys_temp_params();  

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_HTTP_SERVER:
    {
      memcpy(SMS_send_buffer, "HTTP_Server: ", strlen("HTTP_Server: "));
      sms_send_length += strlen("HTTP_Server: ");

      if(setting.http_server_length)
      {
        memcpy(&SMS_send_buffer[sms_send_length], &setting.http_server[1], setting.http_server_length);
        sms_send_length += setting.http_server_length;
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Not set", strlen("Not set"));
        sms_send_length += strlen("Not set");
      }

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_DISABLE_HTTP:
    {
      memcpy(SMS_send_buffer, "HTTP_Server is: ", strlen("HTTP_Server is: "));
      sms_send_length += strlen("HTTP_Server is: ");

      if(setting.disable_http)
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Disable", strlen("Disable"));
        sms_send_length += strlen("Disable");
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Enable", strlen("Enable"));
        sms_send_length += strlen("Enable");
      }

      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_ERRORS:
    {
      uint8_t has_error = RESET;
      
      if(system_error.IMEI_error == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], " No IMEI\n", strlen("No IMEI\n"));
        sms_send_length += strlen("No IMEI\n");
        
        system_error.IMEI_error = RESET;
      }
      if(system_error.server_error == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], "Server error\n", strlen("Server error\n"));
        sms_send_length += strlen("Server error\n");
        
        system_error.server_error = RESET;
      }
      if(system_error.certificate_error == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], "Wrong Cert\n", strlen("Wrong Cert\n"));
        sms_send_length += strlen("Wrong Cert\n");
        
        system_error.certificate_error = RESET;
      }
      if(system_error.no_certificate == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], "No Cert\n", strlen("No Cert\n"));
        sms_send_length += strlen("No Cert\n");
        
        system_error.no_certificate = RESET;
      }
      if(system_error.socket_error == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], "Server socket\n", strlen("Server socket\n"));
        sms_send_length += strlen("Server socket\n");
        
        system_error.socket_error = RESET;
      }
      if(system_error.accel_error == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], "Accel error\n", strlen("Accel error\n"));
        sms_send_length += strlen("Accel error\n");
        
        system_error.accel_error = RESET;
      }
      if(system_error.gps_error == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], "GPS error\n", strlen("GPS error\n"));
        sms_send_length += strlen("GPS error\n");
        
        system_error.gps_error = RESET;
      }
      if(system_error.internet_error == SET)
      {
        has_error = SET;
        memcpy(&SMS_send_buffer[sms_send_length], "Internet error\n", strlen("Internet error\n"));
        sms_send_length += strlen("Internet error\n");
        
        system_error.internet_error = RESET;
      }
      
      
      
      if(has_error == RESET)
      {
        memcpy(&SMS_send_buffer[sms_send_length], "There is no error", strlen("There is no error"));
        sms_send_length += strlen("There is no error");
      }
      
      func_result = SET;
      break;
    }
  case SMS_SETTING_CODE_DEBUG_MODE:
    {
      memcpy(SMS_send_buffer, "Debug mode is: ", strlen("Debug mode is: "));
      sms_send_length += strlen("Debug mode is: ");

      if(debug_mode == SET)
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Enable", strlen("Enable"));
        sms_send_length += strlen("Enable");
      }
      else
      {
        memcpy(&SMS_send_buffer[sms_send_length], "Disable", strlen("Disable"));
        sms_send_length += strlen("Disable");
      }

      func_result = SET;
      break;
    }
  }     // End of switch(id)

  return func_result;
}


const uint8_t sim_SMS_stage_timeout_timer[] = {2,       // Read Message
                                               2,       // Validate Message
                                               2,       // Set Format
                                               3,       // Pre send
                                               10,       // Send SMS content
                                               2,       // Delete Message
                                               2,       // Check unread messages
                                               3,
                                               1,       // Seek OK
                                        };


const uint8_t sim_SMS_stage_cme_error[] = {SMS_READ_MESSAGE,            // Read Message
                                           SMS_VALIDATE_MESSAGE,        // Validate Message
                                           SMS_SET_FORMAT,              // Set Format
                                           SMS_PRE_SEND,                // Pre send
                                           SMS_SEND_CONTENT,            // Send SMS content
                                           SMS_DELETE_MESSAGE,          // Delete Message
                                           SMS_CHECK_UNREAD_MSG,        // Check unread messages
                                           SMS_DEL_ALL_MSG,
                                           SMS_STAGE_SEEK_OK            // Seek OK
                                          };


const uint8_t sim_SMS_stage_TO[] = {SMS_READ_MESSAGE,           // Read Message
                                    SMS_VALIDATE_MESSAGE,       // Validate Message
                                    SMS_SET_FORMAT,             // Set Format
                                    SMS_PRE_SEND,               // Pre send
                                    SMS_SEND_CONTENT,           // Send SMS content
                                    SMS_DELETE_MESSAGE,         // Delete Message
                                    SMS_CHECK_UNREAD_MSG,       // Check unread messages
                                    SMS_DEL_ALL_MSG,
                                    SMS_STAGE_SEEK_OK           // Seek OK
                                    };


void SMS_increment_read_index(void)
{
  if(++current_sms_index > 100)
    current_sms_index = 1;
}


void SMS_read_message(void)
{
  uint8_t tmp_msg[15] = {0};
  uint16_t tmp_length = 0;
  
  tmp_length = 8;
  memcpy(tmp_msg, "AT+CMGR=", tmp_length);
  
  GSM_add_char_number_to_buffer(tmp_msg, &tmp_length, current_sms_index);
  
  tmp_msg[tmp_length++] = '\r';
  
  GSM_clear_RX_buffer();
  
  GSM_Send_Usart(tmp_msg, tmp_length);
  
  memset(SMS_phone_number, 0, ADMIN_NUMBER_MAX_LENGTH);
  memset(SMS_content, 0, SMS_CONTEN_BUFFER_SIZE);
}


void SMS_read_message_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_CMGR)
  {
    uint8_t read_status[12] = {0};
    
    GSM_read_until_from_line_buffer('\"');
    
    // REC READ
    uint8_t temp_8 = GSM_get_string_from_buffer(Rx_line_buffer, read_status, line_buffer_rd_index, '\"', 12);
    line_buffer_rd_index +=temp_8 + 1;
    
    char *res;
    res = strstr((char *)read_status, "REC UNREAD");
    if(res)
    {
      GSM_read_until_from_line_buffer('\"');
      
      SMS_extract_phone_number(SMS_phone_number, &SMS_phone_num_length);
      SMS_extract_SMS_content(SMS_content, &SMS_content_length);
      
      gsm_pt_timeout = 1*GSM_MSG_DEFAULT_TIMEOUT;
    }
    else
    {
      // Todo:
    }
    
    SMS_stage_action = SIM_RCV_RESP;
    SMS_stage_number_of_retries = 0;
  }
  if(*resp_msg == GSM_OK)
  {
    // If there is no message the sms content buffer is empty    
    SMS_stage_action = SIM_SEND_REQ;
    SMS_stage = SMS_VALIDATE_MESSAGE;
    SMS_stage_number_of_retries = 0;
  }
}


void SMS_Validate_Message(void)
{
  uint8_t parse_sms = RESET;

  char* res;
  res = strstr((char *)SMS_content, "##");
  if(res)
  {
    parse_sms = SET;
  }

  if(parse_sms == SET)
  {
    uint16_t setting_id = 0;
    
    SMS_parse_short_message(SMS_content, SMS_content_length, &setting_id);
    if(setting_id != 0)
    {
      if(setting_id == SMS_INFO_TYPE)
      {
        asm("nop");
      }
      else
      {
        memset(SMS_send_buffer, 0, MAX_SEND_SMS_SIZE);
        sms_send_length = 0;
        
        memcpy(SMS_send_buffer, "OK", 2);
        sms_send_length += 2;
      }
      
      SMS_stage = SMS_SET_FORMAT;
      SMS_stage_action = SIM_SEND_REQ;
      SMS_stage_number_of_retries = 0;
      
      return;
    }
  }

  SMS_stage = SMS_DELETE_MESSAGE;
  SMS_stage_action = SIM_SEND_REQ;
  SMS_stage_number_of_retries = 0;
}


void SMS_Validate_Message_resp(uint16_t* resp_msg, int16_t error)
{
  asm("nop");
}

// https://www.smssolutions.net/tutorials/gsm/sendsmsat/
void SMS_set_Format(void)
{
//  if(SMS_has_unicode_letters == SET)
//  {
//    GSM_Send_Usart("AT+CSMP=17,167,0,8\r", strlen("AT+CSMP=17,167,0,8\r"));
//
//    SMS_has_unicode_letters = RESET;
//  }
//  else
//  {
//    GSM_Send_Usart("AT+CSMP=17,167,0,0\r", strlen("AT+CSMP=17,167,0,0\r"));
//  }
  
  GSM_Send_Usart("AT+CSMP=17,167,0,0\r", strlen("AT+CSMP=17,167,0,0\r"));
}


void SMS_set_Format_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    SMS_stage = SMS_PRE_SEND;
    SMS_stage_action = SIM_SEND_REQ;
    SMS_stage_number_of_retries = 0;
  }
}


void SMS_pre_send(void)
{
  uint8_t tmp_msg[ADMIN_NUMBER_MAX_LENGTH+15] = {0};
  uint8_t tmp_length = 0;

  tmp_length = 9;
  memcpy(tmp_msg, "AT+CMGS=\"", tmp_length);

  memcpy(&tmp_msg[tmp_length], SMS_phone_number, SMS_phone_num_length);
  tmp_length += SMS_phone_num_length;

  tmp_msg[tmp_length] = '\"';
  tmp_length++;
  tmp_msg[tmp_length] = '\r';
  tmp_length++;

  GSM_Send_Usart(tmp_msg, tmp_length);
}


void SMS_pre_send_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_DATA_MODE)
  {
    SMS_stage = SMS_SEND_CONTENT;
    SMS_stage_action = SIM_SEND_REQ;
    SMS_stage_number_of_retries = 0;
  }
}


void SMS_send_content(void)
{
  SMS_send_buffer[sms_send_length++] = 0x1A;

  GSM_Send_Usart(SMS_send_buffer, sms_send_length);
}


void SMS_send_content_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_CMGS)
  {
    SMS_stage = SMS_STAGE_SEEK_OK;
    SMS_next_stage = SMS_DELETE_MESSAGE;

    SMS_stage_action = SIM_SEND_REQ;
    SMS_stage_number_of_retries = 0;
  }
}


void SMS_delete_message(void)
{
  uint8_t tmp_msg[15] = {0};
  uint16_t tmp_length = 0;

  tmp_length = 8;
  memcpy(tmp_msg, "AT+CMGD=", tmp_length);

  GSM_add_char_number_to_buffer(tmp_msg, &tmp_length, current_sms_index);
  
  tmp_msg[tmp_length++] = ',';
  tmp_msg[tmp_length++] = '3';          // delete all read messages
  tmp_msg[tmp_length++] = '\r';

  GSM_Send_Usart(tmp_msg, tmp_length);
}


void SMS_delete_message_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    SMS_stage = SMS_CHECK_UNREAD_MSG;
    SMS_stage_action = SIM_SEND_REQ;
    SMS_stage_number_of_retries = 0;
  }
}


void SMS_check_unread_message(void)
{
  GSM_Send_Usart("AT+CPMS?\r", strlen("AT+CPMS?\r"));
}


void SMS_check_unread_message_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_CPMS)
  {
    uint64_t tmp_data = 0;
    
    GSM_read_until_from_line_buffer(',');
    GSM_get_number_from_line_buffer(',', &tmp_data);
    
    if(tmp_data == 0)
    {
      current_sms_index = 0;
      system_has_short_message = RESET;
      GSM_status = GSM_IDLE;
    }
    else
    {
      SMS_increment_read_index();
      if(current_sms_index == 1)
        SMS_stage = SMS_DEL_ALL_MSG;
      else
        SMS_stage = SMS_READ_MESSAGE;
    }
    
    SMS_stage_action = SIM_SEND_REQ;
    SMS_stage_number_of_retries = 0;
  }
}

void SMS_delete_all_message(void)
{
  GSM_Send_Usart("AT+QMGDA=\"DEL ALL\"\r", sizeof("AT+QMGDA=\"DEL ALL\"\r"));
}

void SMS_delete_all_message_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    current_sms_index = 0;
    system_has_short_message = RESET;
    GSM_status = GSM_IDLE;
  }
}

void SMS_seek_OK(void)
{
  asm("nop");
}


void SMS_seek_OK_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    SMS_stage = SMS_next_stage;
    SMS_next_stage = 0;
    SMS_stage_action = SIM_SEND_REQ;
  }
}


void (*SMS_Request_ptr[])(void) = {&SMS_read_message,
                                   &SMS_Validate_Message,
                                   &SMS_set_Format,
                                   &SMS_pre_send,
                                   &SMS_send_content,
                                   &SMS_delete_message,
                                   &SMS_check_unread_message,
                                   &SMS_delete_all_message,
                                   &SMS_seek_OK
                                  };


void (*SMS_Response_ptr[])(uint16_t*, int16_t) = {&SMS_read_message_resp,
                                                  &SMS_Validate_Message_resp,
                                                  &SMS_set_Format_resp,
                                                  &SMS_pre_send_resp,
                                                  &SMS_send_content_resp,
                                                  &SMS_delete_message_resp,
                                                  &SMS_check_unread_message_resp,
                                                  &SMS_delete_all_message_resp,
                                                  &SMS_seek_OK_resp
                                                  };



uint16_t GSM_send_SMS_routine_pro(uint8_t timeout, uint8_t function_index)
{
  uint16_t check_msg_result = GSM_WAITING;

  if(GSM_HALT_Timer == GSM_TIMER_DEFAULT)
  {
    switch(SMS_stage_action)
    {
    case SIM_SEND_REQ:
      {
        if(SMS_stage_number_of_retries == 3)
        {
          if(SMS_stage == SMS_READ_MESSAGE)
          {
            SMS_stage_action = SIM_SEND_REQ;
            SMS_stage = SMS_DELETE_MESSAGE;
            SMS_stage_number_of_retries = 0;
            break;
          }
          else
          {
            GSM_status = GSM_INITIATING;
            sending_data_is_in_progress = RESET;
            GSM_Parameters.stage = SIM_STAGE_RESET_MODULE;
            GSM_Parameters.simcard_available = 0;
            GSM_Parameters.nw_provider = 0;
            GSM_Parameters.number_of_retries_command = 0;
            SMS_stage_number_of_retries = 0;
          }
          break;
        }

        SMS_stage_action = SIM_RCV_RESP;
        SMS_stage_number_of_retries++;
        gsm_pt_timeout = timeout*GSM_MSG_DEFAULT_TIMEOUT;
        (*SMS_Request_ptr[function_index])();
        break;
      }
    case SIM_RCV_RESP:
      {
        int16_t error_code = 0;
        check_msg_result = GSM_check_msg(&error_code);
        if(gsm_pt_timeout == 0 && check_msg_result == GSM_WAITING)
        {
          SMS_stage_action = SIM_SEND_REQ;
          SMS_stage = sim_SMS_stage_TO[function_index];
          return GSM_TIMEOUT;
        }
        else
        {
          if(check_msg_result == GSM_LF || check_msg_result == GSM_WAITING)
            break;
          if(check_msg_result == GSM_CME_ERROR)
          {
            SMS_stage_action = SIM_SEND_REQ;
            GSM_Parameters.stage_action = SIM_SEND_REQ;

            if(error_code == CMEE_NO_SIMCARD)
            {
              GSM_status = GSM_INITIATING;
              sending_data_is_in_progress = RESET;
              GSM_Parameters.stage = SIM_STAGE_RESET_MODULE;
              GSM_Parameters.simcard_available = 0;
              GSM_Parameters.nw_provider = 0;
              GSM_Parameters.number_of_retries_command = 0;
              SMS_stage_number_of_retries = 0;
            }
            else
              SMS_stage = sim_SMS_stage_cme_error[function_index];
          }
          else if(check_msg_result == GSM_CMS_ERROR)
          {
            if(error_code == CMSE_INVALID_PARAM)
            {
              if(SMS_stage == SMS_READ_MESSAGE)
              {
                SMS_stage_action = SIM_SEND_REQ;
                SMS_stage = SMS_DELETE_MESSAGE;
                SMS_stage_number_of_retries = 0;
              }
            }
          }
          else
            (*SMS_Response_ptr[function_index])(&check_msg_result, error_code);
        }
        break;
      }
    } /* end of switch(SMS_stage_action) */
  }
  else
    check_msg_result = GSM_HALT;

  return check_msg_result;
}