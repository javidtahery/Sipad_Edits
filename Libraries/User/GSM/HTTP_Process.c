#include "HTTP_Process.h"
#include "general_functions.h"

http_tmp_typedef                HTTP_Params = {0};
http_fwu_info_typedef           HTTP_FWU = {0};

uint8_t enable_http_process = RESET;
uint8_t http_has_request = RESET;

uint32_t FS_file_read_index = 0;
int32_t FS_num_bytes_read = 0;
uint32_t http_SPIF_Cert_wr_index = 2;
uint32_t http_SPIF_Fwr_wr_index = 0;

uint8_t halt_http_process = RESET;
uint16_t halt_http_process_counter = 0;
uint8_t number_of_http_process_run = 0;

uint8_t http_firmware_upgrade_ready = RESET;

uint8_t disable_http_pdp = RESET;

uint16_t api_interval_counter = 0;

uint8_t server_pack[2500] = {0};
uint16_t server_pack_length = 0;

extern record_typedef                   server_records[MAX_NUM_OF_SERVER_RECORD_PACKET];


void HTTP_preconfig_http_requirments(void)
{
  if(GSM_status == GSM_IDLE)
  {
    // Protect program from looping
    if(++number_of_http_process_run > 3)
    {
      halt_http_process = SET;
      enable_http_process = RESET;
      number_of_http_process_run = 0;
      halt_http_process_counter = 600;
      
      return;
    }
    else
    {
      GSM_status = GSM_HTTP_PROCESS;
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.stage = HTTP_STAGE_TEST_SERIAL;
      GSM_Parameters.prev_stage = 0;
      GSM_Parameters.next_stage = 0;
      GSM_Parameters.number_of_retries_command = 0;
      
      if(GSM_Parameters.internet_connection == SET)
      {
        disable_http_pdp = SET;
        GSM_Parameters.stage = HTTP_STAGE_DEACT_PDP;
        GSM_Parameters.stage_action = SIM_SEND_REQ;
      }
      
      if(HTTP_Params.pdp_activated == SET)
        GSM_Parameters.stage = HTTP_STAGE_MANAGE_FILE_PROCESS;
      
      // Change LED Pattern
      if(system_current_state == SYSTEM_RUNNING)
        LED_change_mode(GSM_LED, LED_ON_TWICE_EACH_1SEC);
    }
  }
}


void Sipaad_check_SPIF_Certs(void)
{
  HTTP_Params.dl_ca = SET;
  HTTP_Params.dl_cc = SET;
  HTTP_Params.dl_ck = SET;
  
  uint8_t cert_data[11] = {0};
  char begin_array[10] = "-----BEGIN";
  
  // Server Cert
  uint16_t cert_data_size = SPIF_read_certificate_file_size(SERVER_CERT_FILE);
  if(cert_data_size != 0 && cert_data_size != 0xFFFF)
  {
    if(cert_data_size < 2048)
    {
      SPIF_read_certificate_file(SERVER_CERT_FILE, cert_data, 10);
      {
        if(strcmp((char*)cert_data, begin_array) == 0)
        {
          HTTP_Params.dl_ca = RESET;
        }
      }
    }
    else
    {
      system_error.certificate_error = SET;
    }
  }
  
  // Client Cert
  cert_data_size = SPIF_read_certificate_file_size(CLIENT_CERT_FILE);
  if(cert_data_size != 0 && cert_data_size != 0xFFFF)
  {
    if(cert_data_size < 2048)
    {
      SPIF_read_certificate_file(CLIENT_CERT_FILE, cert_data, 10);
      {
        if(strcmp((char*)cert_data, begin_array) == 0)
        {
          HTTP_Params.dl_cc = RESET;
        }
      }
    }
    else
    {
      system_error.certificate_error = SET;
    }
  }
  
  // Client Key
  cert_data_size = SPIF_read_certificate_file_size(CLIENT_KEY_FILE);
  if(cert_data_size != 0 && cert_data_size != 0xFFFF)
  {
    if(cert_data_size < 2048)
    {
      SPIF_read_certificate_file(CLIENT_KEY_FILE, cert_data, 10);
      {
        if(strcmp((char*)cert_data, begin_array) == 0)
        {
          HTTP_Params.dl_ck = RESET;
        }
      }
    }
    else
    {
      system_error.certificate_error = SET;
    }
  }
  
  if(HTTP_Params.dl_ca == SET || HTTP_Params.dl_cc == SET || HTTP_Params.dl_ck == SET)
    system_error.no_certificate = SET;
}


uint16_t HTTP_calculate_certificate_CRC16(uint8_t cert_file)
{
  uint8_t cert_data[1024] = {0};
  uint16_t crc16 = 0;
  
  // Server Cert
  uint16_t cert_data_size = SPIF_read_certificate_file_size(cert_file);
  if(cert_data_size != 0 && cert_data_size != 0xFFFF)
  {
    if(cert_data_size < 2048)
    {
      SPIF_read_certificate_file(cert_file, cert_data, cert_data_size);
      crc16 = CRC16_CCITT(cert_data, cert_data_size);
    }
    else
    {
      system_error.certificate_error = SET;
    }
  }
  
  return crc16;
}


void HTTP_Create_Records_Post(void)
{
  server_pack_length = 0;
  Set_zero(server_pack, 2500);
  
  uint8_t record_count = server_packet_count;
  uint8_t null_record = 10 - server_packet_count;
  uint8_t record_index = 0;
  
  server_pack[server_pack_length++] = '{';
  
  for(record_index = 0; record_index < record_count; record_index++)
  {
    memcpy(&server_pack[server_pack_length], "\"record ", strlen("\"record "));
    server_pack_length += strlen("\"record ");
    if(record_index == 9)
    {
      server_pack[server_pack_length++] = '1';
      server_pack[server_pack_length++] = '0';
    }
    else
    {
      server_pack[server_pack_length++] = 0x30 + record_index + 1;
    }
    server_pack[server_pack_length++] = '\"';
    memcpy(&server_pack[server_pack_length], ": {", strlen(": {"));
    server_pack_length += strlen(": {");
    
    memcpy(&server_pack[server_pack_length], "\"device-imei\": \"", strlen("\"device-imei\": \""));
    server_pack_length += strlen("\"device-imei\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_IMEI);
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"lat\": \"", strlen("\"lat\": \""));
    server_pack_length += strlen("\"lat\": \"");
    
    uint32_t latitude = server_records[record_index].gps_elements.GPS_latitude;
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (latitude / PRECISION_POLYNOMIAL));
    server_pack[server_pack_length++] = '.';
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, ((latitude % PRECISION_POLYNOMIAL)/10));
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"long\": \"", strlen("\"long\": \""));
    server_pack_length += strlen("\"long\": \"");
    
    uint32_t longitude = server_records[record_index].gps_elements.GPS_longitude;
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (longitude / PRECISION_POLYNOMIAL));
    server_pack[server_pack_length++] = '.';
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, ((longitude % PRECISION_POLYNOMIAL)/10) );
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"hdop\": \"", strlen("\"hdop\": \""));
    server_pack_length += strlen("\"hdop\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, server_records[record_index].gps_elements.GPS_pdop);
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"pdop\": \"", strlen("\"pdop\": \""));
    server_pack_length += strlen("\"pdop\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, server_records[record_index].gps_elements.GPS_pdop);
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"svs\": \"", strlen("\"svs\": \""));
    server_pack_length += strlen("\"svs\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, server_records[record_index].gps_elements.GPS_nsat);
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"gsm-level\": \"", strlen("\"gsm-level\": \""));
    server_pack_length += strlen("\"gsm-level\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, server_records[record_index].GSM_lvl);
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"ignition\": \"", strlen("\"ignition\": \""));
    server_pack_length += strlen("\"ignition\": \"");
    
    if(server_records[record_index].io_status & (1<<DIGITAL_IO_ACC) == (1<<DIGITAL_IO_ACC))
      server_pack[server_pack_length++] = '1';
    else
      server_pack[server_pack_length++] = '0';
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"battery\": \"", strlen("\"battery\": \""));
    server_pack_length += strlen("\"battery\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, server_records[record_index].analoge_in_1);
    
    server_pack[server_pack_length++] = 'v';
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"external\": \"", strlen("\"external\": \""));
    server_pack_length += strlen("\"external\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, server_records[record_index].supply_voltage);
    
    server_pack[server_pack_length++] = 'v';
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"record-id\": \"", strlen("\"record-id\": \""));
    server_pack_length += strlen("\"record-id\": \"");
    
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, server_records[record_index].event_code);
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = ',';
    
    memcpy(&server_pack[server_pack_length], "\"date\": \"", strlen("\"date\": \""));
    server_pack_length += strlen("\"date\": \"");
    
    time_t event_time_unix = server_records[record_index].gps_elements.unixtime;
    struct tm UTC;
    
    UTC = *localtime(&event_time_unix);
    /* ------------ Year ------------ */
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_year + 1900));
    server_pack[server_pack_length++] = '-';
    
    /* ------------ Month ------------ */
    if(UTC.tm_mon < 9)  // Padding
    {
      server_pack[server_pack_length++] = '0';
    }
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_mon + 1));
    server_pack[server_pack_length++] = '-';
    
    /* ------------ Day ------------ */
    if(UTC.tm_mday < 10)  // Padding
    {
      server_pack[server_pack_length++] = '0';
    }
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_mday);
    server_pack[server_pack_length++] = ' ';
    
    /* ------------ Hour ------------ */
    if(UTC.tm_hour < 10)  // Padding
    {
      server_pack[server_pack_length++] = '0';
    }
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_hour);
    server_pack[server_pack_length++] = ':';
    /* ------------ Minutes ------------ */
    if(UTC.tm_min < 10)  // Padding
    {
      server_pack[server_pack_length++] = '0';
    }
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_min);
    server_pack[server_pack_length++] = ':';
    /* ------------ Seconds ------------ */
    if(UTC.tm_sec < 10)  // Padding
    {
      server_pack[server_pack_length++] = '0';
    }
    GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_sec);
    
    server_pack[server_pack_length++] = '\"';
    server_pack[server_pack_length++] = '}';
    
    if( (record_index + 1) < 10)
      server_pack[server_pack_length++] = ',';
  }
  
  if(null_record > 0)
  {
    while(null_record)
    {
      memcpy(&server_pack[server_pack_length], "\"record ", 8);
      server_pack_length += strlen("\"record ");
      if(record_index == 9)
      {
        server_pack[server_pack_length++] = '1';
        server_pack[server_pack_length++] = '0';
      }
      else
      {
        server_pack[server_pack_length++] = 0x30 + record_index + 1;
      }
      record_index++;
      server_pack[server_pack_length++] = '\"';
      memcpy(&server_pack[server_pack_length], ": {}", strlen(": {}"));
      server_pack_length += strlen(": {}");
      
      if(record_index < 10)
        server_pack[server_pack_length++] = ',';
      
      null_record--;
    }
  }
  server_pack[server_pack_length++] = '}';
}


void HTTP_Create_Information_message(void)
{
  server_pack_length = 0;
  Set_zero(server_pack, 2500);
  
  server_pack[server_pack_length++] = '{';
  
  memcpy(&server_pack[server_pack_length], "\"device-imei\": \"", strlen("\"device-imei\": \""));
  server_pack_length += strlen("\"device-imei\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_IMEI);
  
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"fver\": \"", strlen("\"fver\": \""));
  server_pack_length += strlen("\"fver\": \"");
  
  memcpy(&server_pack[server_pack_length], Software_version, strlen(Software_version));
  server_pack_length += strlen(Software_version);
    
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"date\": \"", strlen("\"date\": \""));
  server_pack_length += strlen("\"date\": \"");
  
  time_t time_unix = system_Unixtime;
  struct tm UTC;
  
  UTC = *localtime(&time_unix);
  /* ------------ Year ------------ */
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_year + 1900));
  server_pack[server_pack_length++] = '-';
  
  /* ------------ Month ------------ */
  if(UTC.tm_mon < 9)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_mon + 1));
  server_pack[server_pack_length++] = '-';
  
  /* ------------ Day ------------ */
  if(UTC.tm_mday < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_mday);
  server_pack[server_pack_length++] = ' ';
  
  /* ------------ Hour ------------ */
  if(UTC.tm_hour < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_hour);
  server_pack[server_pack_length++] = ':';
  /* ------------ Minutes ------------ */
  if(UTC.tm_min < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_min);
  server_pack[server_pack_length++] = ':';
  /* ------------ Seconds ------------ */
  if(UTC.tm_sec < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_sec);
  
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"errors\": \"", strlen("\"errors\": \""));
  server_pack_length += strlen("\"errors\": \"");
  
  server_pack[server_pack_length++] = '0';
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"external\": \"", strlen("\"external\": \""));
  server_pack_length += strlen("\"external\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, ADC_Values.Main_power);
  
  server_pack[server_pack_length++] = 'v';
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"setting-code\": \"", strlen("\"setting-code\": \""));
  server_pack_length += strlen("\"setting-code\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, setting.api_setting_code);
  
  server_pack[server_pack_length++] = '\"';
  
  server_pack[server_pack_length++] = '}';
}


void HTTP_Create_setting_request(void)
{
  server_pack_length = 0;
  Set_zero(server_pack, 2500);
  
  server_pack[server_pack_length++] = '{';
  
  memcpy(&server_pack[server_pack_length], "\"device-imei\": \"", strlen("\"device-imei\": \""));
  server_pack_length += strlen("\"device-imei\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_IMEI);
  
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"setting\": \"1\"", strlen("\"setting\": \"1\""));
  server_pack_length += strlen("\"setting\": \"1\"");
  
  server_pack[server_pack_length++] = '}';
}


void HTTP_Create_Interval_Information(void)
{
  server_pack_length = 0;
  Set_zero(server_pack, 2500);
  
  server_pack[server_pack_length++] = '{';
  
  memcpy(&server_pack[server_pack_length], "\"device-imei\": \"", strlen("\"device-imei\": \""));
  server_pack_length += strlen("\"device-imei\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_IMEI);
  
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  // GPS Jump Count
  memcpy(&server_pack[server_pack_length], "\"gps-errors\": \"", strlen("\"gps-errors\": \""));
  server_pack_length += strlen("\"gps-errors\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, gps_jump_count);
    
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"gsm-errors\": \"", strlen("\"gsm-errors\": \""));
  server_pack_length += strlen("\"gsm-errors\": \"");
  server_pack[server_pack_length++] = '0';
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"server-error\": \"", strlen("\"server-error\": \""));
  server_pack_length += strlen("\"server-error\": \"");
  
  if(system_error.IMEI_error == SET)
    server_pack[server_pack_length++] = '1';
  else if(system_error.no_certificate == SET)
    server_pack[server_pack_length++] = '2';
  else if(system_error.certificate_error == SET)
    server_pack[server_pack_length++] = '3';
  else if(system_error.internet_error == SET)
    server_pack[server_pack_length++] = '4';
  else if(system_error.socket_error == SET)
    server_pack[server_pack_length++] = '5';
  else if(system_error.server_error == SET)
    server_pack[server_pack_length++] = '6';
  else
    server_pack[server_pack_length++] = '0';
  
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"gsm-err\": {", strlen("\"gsm-err\": {"));
  server_pack_length += strlen("\"gsm-err\": {");
  
  memcpy(&server_pack[server_pack_length], "\"net-count\": ", strlen("\"net-count\": "));
  server_pack_length += strlen("\"net-count\": ");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_error.internet_error_count);
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"socket-count\": ", strlen("\"socket-count\": "));
  server_pack_length += strlen("\"socket-count\": ");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_error.socket_error_count);
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"server-count\": ", strlen("\"server-count\": "));
  server_pack_length += strlen("\"server-count\": ");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_error.server_error_count);
  server_pack[server_pack_length++] = '}';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"hardware-error\": ", strlen("\"hardware-error\": "));
  server_pack_length += strlen("\"hardware-error\": ");
  
  // TODO
  memcpy(&server_pack[server_pack_length], "\"0,0,0,0,0,0\"", strlen("\"0,0,0,0,0,0\""));
  server_pack_length += strlen("\"0,0,0,0,0,0\"");
  
  server_pack[server_pack_length++] = ',';
  
  
  memcpy(&server_pack[server_pack_length], "\"reset-count\": \"", strlen("\"reset-count\": \""));
  server_pack_length += strlen("\"reset-count\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, reset_count);
    
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"total-distance\": \"", strlen("\"total-distance\": \""));
  server_pack_length += strlen("\"total-distance\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, vehicle_total_movement);
    
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"total-record\": \"", strlen("\"total-record\": \""));
  server_pack_length += strlen("\"total-record\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, total_generated_record);
    
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"notsend-count\": \"", strlen("\"notsend-count\": \""));
  server_pack_length += strlen("\"notsend-count\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, FCB_profile.s1_nsend_count);
    
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"external\": \"", strlen("\"external\": \""));
  server_pack_length += strlen("\"external\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, ADC_Values.Main_power);
  
  server_pack[server_pack_length++] = 'v';
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"date\": \"", strlen("\"date\": \""));
  server_pack_length += strlen("\"date\": \"");
  
  time_t time_unix = system_Unixtime;
  struct tm UTC;
  
  UTC = *localtime(&time_unix);
  /* ------------ Year ------------ */
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_year + 1900));
  server_pack[server_pack_length++] = '-';
  
  /* ------------ Month ------------ */
  if(UTC.tm_mon < 9)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_mon + 1));
  server_pack[server_pack_length++] = '-';
  
  /* ------------ Day ------------ */
  if(UTC.tm_mday < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_mday);
  server_pack[server_pack_length++] = ' ';
  
  /* ------------ Hour ------------ */
  if(UTC.tm_hour < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_hour);
  server_pack[server_pack_length++] = ':';
  /* ------------ Minutes ------------ */
  if(UTC.tm_min < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_min);
  server_pack[server_pack_length++] = ':';
  /* ------------ Seconds ------------ */
  if(UTC.tm_sec < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_sec);
  
  server_pack[server_pack_length++] = '\"';
  
  server_pack[server_pack_length++] = '}';
}


uint8_t HTTP_parse_information_response(void)
{
  uint8_t *res;
  uint8_t result = RESET;
  uint8_t tmp_data_8 = 0;
  uint64_t tmp_data_64 = 0;
  
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[0], "\"serial\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    GSM_read_until_from_line_buffer('"');
    
    tmp_data_8 = GSM_get_number_from_buffer(Rx_line_buffer, line_buffer_rd_index, '"', 11, &tmp_data_64);
    if(tmp_data_8 > 1)          // 1 for '"'
    {
      if(system_Serial != tmp_data_64)
      {
        system_Serial = tmp_data_64;
        SPIF_Program_Security_Register(SPIF_SECURITY_REGISTE_3_ADDR, (uint8_t*)&system_Serial, 8);
      }
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"sms-report\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    GSM_read_until_from_line_buffer('"');
    
    if(Rx_line_buffer[line_buffer_rd_index] == '1')
      sms_panel_send = SET;
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"setting\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    GSM_read_until_from_line_buffer('"');
    
    if(Rx_line_buffer[line_buffer_rd_index] == '1')
    {
      HTTP_Params.get_setting = SET;
      result = SET;
    }
  }
  
  return result;
}


void HTTP_parse_setting_response(void)
{
  uint8_t *res;
  uint8_t tmp_data_8 = 0;
  uint64_t tmp_data_64 = 0;
  
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[0], "\"sms-panel\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    GSM_read_until_from_line_buffer('"');
    
    tmp_data_8 = GSM_read_until_from_buffer(Rx_line_buffer, line_buffer_rd_index, '"', SMS_PANEL_MAX_LENGTH_SIZE);
    if(tmp_data_8 > 1)        // 1 is for '"'
    {
      memset(&setting.sms_panel, 0 , SMS_PANEL_MAX_LENGTH_SIZE);
      setting.sms_panel_length = tmp_data_8-1;
      memcpy(setting.sms_panel, (Rx_line_buffer+line_buffer_rd_index), (tmp_data_8-1));
      line_buffer_rd_index += tmp_data_8;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"http-server\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    GSM_read_until_from_line_buffer('"');
    
    tmp_data_8 = GSM_read_until_from_buffer(Rx_line_buffer, line_buffer_rd_index, '"', URL_MAX_LENGTH_SIZE);
    if(tmp_data_8 > 1)        // 1 is for '"'
    {
      memset(&setting.http_server, 0 , URL_MAX_LENGTH_SIZE);
      setting.http_server[0] = '#';
      setting.http_server_length = tmp_data_8-1;
      memcpy(&setting.http_server[1], (Rx_line_buffer+line_buffer_rd_index), (tmp_data_8-1));
      line_buffer_rd_index += tmp_data_8;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"api-interval\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    tmp_data_8 = GSM_get_number_from_buffer(Rx_line_buffer, line_buffer_rd_index, ',', 6, &tmp_data_64);
    if(tmp_data_8 > 1)          // 1 for ','
    {
      if(tmp_data_64 == 0)
        setting.api_interval_disable = SET;
      else
        setting.api_interval_time = tmp_data_64;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"time-interval\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    tmp_data_8 = GSM_get_number_from_buffer(Rx_line_buffer, line_buffer_rd_index, ',', 6, &tmp_data_64);
    if(tmp_data_8 > 1)          // 1 for ','
    {
      if(tmp_data_64 >= 10)
        setting.data_sample_rate_time = tmp_data_64;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"hash-ncount\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    tmp_data_8 = GSM_get_number_from_buffer(Rx_line_buffer, line_buffer_rd_index, ',', 7, &tmp_data_64);
    if(tmp_data_8 > 1)          // 1 for ','
    {
      setting.hash_nsend_count = tmp_data_64;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"debug-mode\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    if(Rx_line_buffer[line_buffer_rd_index] == '1')
    {
      uint16_t timer = 2 * 60 * 60;         // Default time - 2 Hours
      if(setting.debug_mode_time > 0)
        timer = setting.debug_mode_time * 60;
      
      debug_mode = SET;
      debug_mode_counter = timer;
      enable_send_data_to_sipaad = RESET;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"debug-time\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    tmp_data_8 = GSM_get_number_from_buffer(Rx_line_buffer, line_buffer_rd_index, ',', 6, &tmp_data_64);
    if(tmp_data_8 > 1)          // 1 for '"'
    {
      if(tmp_data_64 > 0)
        setting.debug_mode_time = tmp_data_64;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"force-fota\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    if(Rx_line_buffer[line_buffer_rd_index] == '1')
    {
      setting.force_FOTA = SET;
      HTTP_Params.dl_boot = SET;
      HTTP_Params.dl_app = SET;
      HTTP_Params.dl_fwr_info = SET;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"recert\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    if(Rx_line_buffer[line_buffer_rd_index] == '1')
    {
      SPIF_Erase_certificates();
      
      HTTP_Params.dl_ca = SET;
      HTTP_Params.dl_cc = SET;
      HTTP_Params.dl_ck = SET;
      
      system_error.no_certificate = SET;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"custom-apn\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    if(Rx_line_buffer[line_buffer_rd_index] == '1')
      setting.custom_APN = SET;
    else
      setting.custom_APN = RESET;
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"apn-name\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    GSM_read_until_from_line_buffer('"');
    
    tmp_data_8 = GSM_read_until_from_buffer(Rx_line_buffer, line_buffer_rd_index, '"', APN_MAX_LENGTH_SIZE);
    if(tmp_data_8 > 1)        // 1 is for '"'
    {
      memset(&setting.APN, 0 , APN_MAX_LENGTH_SIZE);
      setting.APN_length = tmp_data_8-1;
      memcpy(setting.APN, (Rx_line_buffer+line_buffer_rd_index), (tmp_data_8-1));
      line_buffer_rd_index += tmp_data_8;
    }
  }
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"setting-code\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    GSM_read_until_from_line_buffer('"');
    
    tmp_data_8 = GSM_get_number_from_buffer(Rx_line_buffer, line_buffer_rd_index, '"', 8, &tmp_data_64);
    if(tmp_data_8 > 1)          // 1 for '"'
    {
      if(setting.api_setting_code != tmp_data_64)
      {
        setting.api_setting_code = tmp_data_64;
        save_changed_setting = SET;
      }
    }
  }
}


void HTTP_Create_Hash_message(void)
{
  server_pack_length = 0;
  Set_zero(server_pack, 2500);
  
  server_pack[server_pack_length++] = '{';
  
  memcpy(&server_pack[server_pack_length], "\"device-imei\": \"", strlen("\"device-imei\": \""));
  server_pack_length += strlen("\"device-imei\": \"");
  
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, system_IMEI);
  
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"date\": \"", strlen("\"date\": \""));
  server_pack_length += strlen("\"date\": \"");
  
  time_t time_unix = system_Unixtime;
  struct tm UTC;
  
  UTC = *localtime(&time_unix);
  /* ------------ Year ------------ */
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_year + 1900));
  server_pack[server_pack_length++] = '-';
  
  /* ------------ Month ------------ */
  if(UTC.tm_mon < 9)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, (UTC.tm_mon + 1));
  server_pack[server_pack_length++] = '-';
  
  /* ------------ Day ------------ */
  if(UTC.tm_mday < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_mday);
  server_pack[server_pack_length++] = ' ';
  
  /* ------------ Hour ------------ */
  if(UTC.tm_hour < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_hour);
  server_pack[server_pack_length++] = ':';
  /* ------------ Minutes ------------ */
  if(UTC.tm_min < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_min);
  server_pack[server_pack_length++] = ':';
  /* ------------ Seconds ------------ */
  if(UTC.tm_sec < 10)  // Padding
  {
    server_pack[server_pack_length++] = '0';
  }
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, UTC.tm_sec);
  
  server_pack[server_pack_length++] = '\"';
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"crc\": {", strlen("\"crc\": {"));
  server_pack_length += strlen("\"crc\": {");
  
  memcpy(&server_pack[server_pack_length], "\"root\": ", strlen("\"root\": "));
  server_pack_length += strlen("\"root\": ");
  
  uint16_t crc16 = HTTP_calculate_certificate_CRC16(SERVER_CERT_FILE);
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, crc16);
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"cc\": ", strlen("\"cc\": "));
  server_pack_length += strlen("\"cc\": ");
  
  crc16 = HTTP_calculate_certificate_CRC16(CLIENT_CERT_FILE);
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, crc16);
  server_pack[server_pack_length++] = ',';
  
  memcpy(&server_pack[server_pack_length], "\"ck\": ", strlen("\"ck\": "));
  server_pack_length += strlen("\"ck\": ");
  
  crc16 = HTTP_calculate_certificate_CRC16(CLIENT_KEY_FILE);
  GSM_add_char_number_to_buffer(server_pack, &server_pack_length, crc16);
  server_pack[server_pack_length++] = '}';
  
  server_pack[server_pack_length++] = '}';
}


uint8_t HTTP_parse_Hash_response(void)
{
  uint8_t *res;
  uint8_t result = RESET;
  
  res = (uint8_t *)strstr((char *)&Rx_line_buffer[line_buffer_rd_index], "\"recert\"");
  if(res)
  {
    GSM_read_until_from_line_buffer(':');
    
    if(Rx_line_buffer[line_buffer_rd_index] == '1')
    {
      HTTP_Params.dl_ca = SET;
      HTTP_Params.dl_cc = SET;
      HTTP_Params.dl_ck = SET;
      result = SET;
    }
  }
  
  return result;
}


void HTTP_analyze_FWU_file(uint8_t* data, uint8_t data_size)
{
  uint8_t data_idx = 0;
  
  // Data CRC16
  uint16_t data_crc16, calc_crc16;
  Little_endian_data(&data[data_idx], (uint8_t*)&data_crc16, 2);
  data_idx += 2;
  
  calc_crc16 = CRC16_CCITT(&data[data_idx], (data_size-2));
  
  if(calc_crc16 == data_crc16)
  {
    // Boot Size
    Little_endian_data(&data[data_idx], (uint8_t*)&HTTP_FWU.boot_size, 4);
    data_idx += 4;
    
    // Boot Chunk Count
    HTTP_FWU.boot_chunk_count = data[data_idx++];
    
    // Boot Release Date
    Little_endian_data(&data[data_idx], (uint8_t*)&HTTP_FWU.boot_release_date, 4);
    data_idx += 4;
    
    // Boot MD5
    memcpy(HTTP_FWU.boot_md5, &data[data_idx], 16);
    data_idx += 16;
    
    // App Size
    Little_endian_data(&data[data_idx], (uint8_t*)&HTTP_FWU.app_size, 4);
    data_idx += 4;
    
    // App Chunk Count
    HTTP_FWU.app_chunk_count = data[data_idx++];
    
    // App Release Date
    Little_endian_data(&data[data_idx], (uint8_t*)&HTTP_FWU.app_release_date, 4);
    data_idx += 4;
    
    // App MD5
    memcpy(HTTP_FWU.app_md5, &data[data_idx], 16);
    data_idx += 16;
    
    if(HTTP_FWU.boot_release_date > Bootloader_release_date || setting.force_FOTA == SET)
    {
      HTTP_Params.dl_boot = SET;
      
      HTTP_Params.checksum_retry = 0;
      HTTP_Params.checksum_file_dl_retry = 0;
      HTTP_Params.file_chunk_idx = 1;
      http_SPIF_Fwr_wr_index = 0;
    }
    
    if(HTTP_FWU.app_release_date > APP_RELEASE_DATE || setting.force_FOTA == SET)
    {
      HTTP_Params.dl_app = SET;
      
      HTTP_Params.checksum_retry = 0;
      HTTP_Params.checksum_file_dl_retry = 0;
      HTTP_Params.file_chunk_idx = 1;
      http_SPIF_Fwr_wr_index = 0;
    }
  }
}


void HTTP_analyze_chunk_header(uint8_t* data)
{
  uint8_t data_idx = 0;
  
  // Chunk Size
  uint16_t chunk_size;
  Little_endian_data(&data[data_idx], (uint8_t*)&chunk_size, 2);
  data_idx += 2;
  
  // Chunk CRC32
  uint32_t chunk_crc;
  HTTP_FWU.chunk_crc32 = 0;
  Little_endian_data(&data[data_idx], (uint8_t*)&chunk_crc, 4);
  data_idx += 4;
  
  // Chunk Index
  uint8_t chunk_index = data[data_idx++];
  
  if(chunk_index == HTTP_Params.file_chunk_idx && chunk_size == (HTTP_Params.file_size-7))
  {
    HTTP_FWU.chunk_crc32 = chunk_crc;
  }
}


uint32_t CRC32_Reversed(uint8_t *data, uint32_t data_length)
{
  uint32_t crc = 0xFFFFFFFF;
  
  for(uint32_t i = 0; i < data_length; i++)
  {
    char ch = data[i];
    for(uint8_t j = 0; j < 8; j++)
    {
      uint32_t b = (ch ^ crc) & 1;
      crc >>= 1;
      if(b)
        crc = crc ^ CRC32_POLYNOMIAL;
      ch >>= 1;
    }
  }
  
  return ~crc;
}


uint32_t HTTP_compute_chunk_CRC32(uint32_t chunk_size, uint8_t file_type, uint32_t start_addr)
{
  uint32_t crc = 0xFFFFFFFF;
  uint8_t data_array[SPIF_PAGE_SIZE] = {0};
  
  uint16_t sector_index = 0;
  if(file_type == BOOTLOADER_FWR)
    sector_index = SPIF_BOOT_FW_FILE_START_SECTOR;
  else if(file_type == APPLICATION_FWR)
    sector_index = SPIF_APP_FW_FILE_START_SECTOR;
  
  uint32_t SPIF_data_address = sector_index * SPIF_SECTOR_SIZE + start_addr;
  uint16_t page_count = chunk_size / SPIF_PAGE_SIZE;
  uint16_t last_page_remain_bytes = chunk_size % SPIF_PAGE_SIZE;
  
  
  // Reading entire sector
  for(uint16_t SPIF_sector_itr = 0; SPIF_sector_itr < page_count; SPIF_sector_itr++)
  {
    SPIF_Read(data_array, SPIF_data_address, SPIF_PAGE_SIZE);
    
    Delay(50);
    
    for(uint16_t iterator = 0; iterator < SPIF_PAGE_SIZE; iterator++)
    {
      uint8_t ch = data_array[iterator];
      for(uint8_t j = 0; j < 8; j++)
      {
        uint32_t b = (ch ^ crc) & 1;
        crc >>= 1;
        if(b)
          crc = crc ^ CRC32_POLYNOMIAL;
        ch >>= 1;
      }
    }
    
    SPIF_data_address += SPIF_PAGE_SIZE;
    IWDG_ReloadCounter();
  }
  
  // Reading the last sector
  if(last_page_remain_bytes > 0)
  {
    SPIF_Read(data_array, SPIF_data_address, last_page_remain_bytes);
    
    Delay(300);
    
    for(uint16_t iterator = 0; iterator < last_page_remain_bytes; iterator++)
    {
      uint8_t ch = data_array[iterator];
      for(uint8_t j = 0; j < 8; j++)
      {
        uint32_t b = (ch ^ crc) & 1;
        crc >>= 1;
        if(b)
          crc = crc ^ CRC32_POLYNOMIAL;
        ch >>= 1;
      }
    }
  }
  
  return ~crc;
}


// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

const uint8_t http_stage_timeout[] = {2,         // Reset Module
                                      2,         // Test Serial
                                      2,         // Cfg Foreground context
                                      2,         // Cfg APN
                                      120,       // Register TCP Stack
                                      120,       // Activate FGCNT
                                      30,        // Query Local IP
                                      2,         // Manage File Process
                                      2,         // Enter URL length
                                      2,         // Enter URL
                                      10,       // Post Data
                                      20,        // Send Post Data
                                      30,       // Read Post Data
                                      2,         // Delete RAM File
                                      60,        // HTTP GET
                                      65,        // Download File
                                      2,         // Open File
                                      2,         // FS Set Position
                                      2,         // FS Read File
                                      2,         // FS Close File
                                      30,        // Deact PDP
                                      2,         // Seek OK
};

const uint8_t http_stage_max_num_of_retries[] = {3,              // Reset Module
                                                  3,              // Test Serial
                                                  2,              // Cfg Foreground context
                                                  2,              // cfg APN
                                                  2,              // Register TCP Stack
                                                  2,              // Activate FGCNT
                                                  2,              // Query Local IP
                                                  2,              // Manage File Process
                                                  2,              // Enter URL length
                                                  2,              // Enter URL
                                                  2,            // Post Data
                                                  2,            // Send Post Data
                                                  2,            // Read Post Data
                                                  2,              // Delete RAM File
                                                  2,              // HTTP GET
                                                  4,              // Download File
                                                  2,              // Open File
                                                  2,              // FS Set Position
                                                  2,              // FS Read File
                                                  2,              // FS Close File
                                                  2,              // Deact PDP
                                                  1,              // Seek OK
};

const uint8_t http_stage_retries_redirect[] = {GSM_STAGE_DIS_POWER,               // Reset Module
                                                GSM_STAGE_DIS_POWER,               // Test Serial
                                                HTTP_STAGE_RESET_MODULE,           // Cfg Foreground context
                                                HTTP_STAGE_RESET_MODULE,           // cfg APN
                                                HTTP_STAGE_RESET_MODULE,           // Register TCP Stack
                                                HTTP_STAGE_RESET_MODULE,           // Activate FGCNT
                                                HTTP_STAGE_RESET_MODULE,           // Query Local IP
                                                HTTP_STAGE_MANAGE_FILE_PROCESS,    // Manage File Process
                                                HTTP_STAGE_RESET_MODULE,           // Enter URL length
                                                HTTP_STAGE_RESET_MODULE,           // Enter URL
                                                HTTP_STAGE_RESET_MODULE,          // Post Data
                                                HTTP_STAGE_RESET_MODULE,                // Send Post Data
                                                HTTP_STAGE_RESET_MODULE,                // Read Post Data
                                                HTTP_STAGE_RESET_MODULE,           // Delete RAM File
                                                HTTP_STAGE_RESET_MODULE,           // HTTP GET
                                                HTTP_STAGE_RESET_MODULE,           // Download File
                                                HTTP_STAGE_RESET_MODULE,           // Open File
                                                HTTP_STAGE_RESET_MODULE,           // FS Set Position
                                                HTTP_STAGE_RESET_MODULE,           // FS Read File
                                                HTTP_STAGE_RESET_MODULE,           // FS Close File
                                                HTTP_STAGE_RESET_MODULE,           // Deact PDP
                                                HTTP_STAGE_TEST_SERIAL             // Seek OK
};

const uint8_t http_stage_TO_CME_Error[] = {SIM_STAGE_MODULE_OFF,                 // Reset Module
                                            HTTP_STAGE_TEST_SERIAL,                // Test Serial
                                            HTTP_STAGE_CFG_FOREGROUND_CNT,         // Cfg Foreground context
                                            HTTP_STAGE_CFG_APN,                    // cfg APN
                                            HTTP_STAGE_REG_TCP_STACK,              // Register TCP Stack
                                            HTTP_STAGE_ACTIVATE_FGCNT,             // Activate FGCNT
                                            HTTP_STAGE_QUERY_LOCAL_IP,             // Query Local IP
                                            HTTP_STAGE_MANAGE_FILE_PROCESS,        // Manage File Process
                                            HTTP_STAGE_ENTER_URL_LENGTH,          // Enter URL length
                                            HTTP_STAGE_ENTER_URL,                 // Enter URL
                                            HTTP_STAGE_POST_DATA,               // Post Data
                                            HTTP_STAGE_POST_DATA,               // Send Post Data
                                            HTTP_STAGE_READ_POST_DATA,          // Read Post Data
                                            HTTP_STAGE_DELETE_RAM_FILES,               // Delete File
                                            HTTP_STAGE_GET,                   // Set Path
                                            HTTP_STAGE_DL_FILE,                 // Query NLST
                                            HTTP_STAGE_FS_OPEN_FILE,               // Open File
                                            HTTP_STAGE_FS_SET_POSITION,            // FS Set Position
                                            HTTP_STAGE_FS_READ_FILE,               // FS Read File
                                            HTTP_STAGE_FS_CLOSE_FILE,              // FS Close File
                                            HTTP_STAGE_DEACT_PDP,                  // Deact PDP
                                            HTTP_STAGE_SEEK_OK                     // Seek OK
};


void HTTP_reset_module(void)
{
  GSM_Send_Usart("AT+CFUN=1,1\r", strlen("AT+CFUN=1,1\r"));
  
  GSM_clear_RX_buffer();
  
  GSM_Parameters.nw_provider = 0;
  GSM_Parameters.nw_status = 0;
  GSM_Parameters.signal_quality = 0;
  GSM_Parameters.simcard_available = 0;
  GSM_Parameters.internet_connection = RESET;
  
  HTTP_Params.pdp_activated = RESET;
  
  // Change LED Pattern
  if(system_current_state == SYSTEM_RUNNING)
    LED_change_mode(GSM_LED, LED_ON_BLINK_500MS);
  
  GSM_HALT_Timer = 6;
}


void HTTP_reset_module_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_TEST_SERIAL;
    
    GSM_HALT_Timer = 5;
  }
}


void HTTP_test_serial_port(void)
{
  uint8_t msg[1] = {0x1B};
  GSM_Send_Usart(msg, 1);
  Delay(200);
  GSM_Send_Usart("AT\r", strlen("AT\r"));
}


void HTTP_test_serial_port_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.number_of_retries_command = 0;
    
    if(GSM_Parameters.next_stage == 0)
      GSM_Parameters.stage = HTTP_STAGE_CFG_FOREGROUND_CNT;
    else
    {
      GSM_Parameters.stage = GSM_Parameters.next_stage;     // When the buffer has error and we want to check the serial port.
      GSM_Parameters.next_stage = 0;
    }
  }
}


void HTTP_cfg_foreground_context(void)
{
  GSM_Send_Usart("AT+QIFGCNT=0\r", strlen("AT+QIFGCNT=0\r"));
}


void HTTP_cfg_foreground_context_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_CFG_APN;
  }
}


void HTTP_cfg_APN(void)
{
  GSM_Send_Set_APN_cmd();
}


void HTTP_cfg_APN_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_REG_TCP_STACK;
    GSM_Parameters.prev_stage = HTTP_STAGE_CFG_APN;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void HTTP_register_TCP_stack(void)
{
  GSM_Send_Usart("AT+QIREGAPP\r", strlen("AT+QIREGAPP\r"));
}


void HTTP_register_TCP_stack_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_ACTIVATE_FGCNT;
    GSM_Parameters.prev_stage = HTTP_STAGE_REG_TCP_STACK;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void HTTP_Activate_FGCNT(void)
{
  GSM_Send_Usart("AT+QIACT\r", strlen("AT+QIACT\r"));
}


void HTTP_Activate_FGCNT_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_QUERY_LOCAL_IP;
    GSM_Parameters.prev_stage = HTTP_STAGE_ACTIVATE_FGCNT;
    GSM_Parameters.number_of_retries_command = 0;
    
    GSM_HALT_Timer = 2;
  }
  else if(*resp_msg == GSM_ERROR)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_ACTIVATE_FGCNT;
    GSM_Parameters.prev_stage = HTTP_STAGE_ACTIVATE_FGCNT;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void HTTP_query_Local_IP(void)
{
  GSM_Send_Usart("AT+QILOCIP\r", strlen("AT+QILOCIP\r"));
}


void HTTP_query_Local_IP_resp(uint16_t* resp_msg, int16_t error)
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
      
//      GSM_Parameters.internet_connection = SET;
      GSM_Parameters.stage = HTTP_STAGE_MANAGE_FILE_PROCESS;
      GSM_Parameters.prev_stage = HTTP_STAGE_QUERY_LOCAL_IP;
      GSM_Parameters.number_of_retries_command = 0;
      
      HTTP_Params.pdp_activated = SET;
      
      GSM_HALT_Timer = 2;
    }
    
    GSM_Parameters.stage_action = SIM_SEND_REQ;
  }
}


void HTTP_Manage_File_Process(void)
{
  uint8_t http_request = RESET;
  HTTP_Params.request_type = NO_REQ;
  
  if(HTTP_Params.dl_fwr_info == SET)
  {
    HTTP_Params.request_type = FW_UPGRADE_INFO;
    http_request = SET;
  }
  else if(HTTP_Params.dl_boot == SET)
  {
    HTTP_Params.request_type = BOOTLOADER_FWR;
    http_request = SET;
  }
  else if(HTTP_Params.dl_app == SET)
  {
    HTTP_Params.request_type = APPLICATION_FWR;
    http_request = SET;
  }
  else if(HTTP_Params.send_record == SET)
  {
    if(server_packet_count > 0)
    {
      HTTP_Params.request_type = SEND_RECORD;
      http_request = SET;
    }
  }
  else if(HTTP_Params.send_information == SET)
  {
    HTTP_Params.request_type = SEND_INFORMATION;
    http_request = SET;
  }
  else if(HTTP_Params.get_setting == SET)
  {
    HTTP_Params.request_type = GET_SETTING;
    http_request = SET;
  }
  else if(HTTP_Params.send_interval == SET)
  {
    HTTP_Params.request_type = SEND_INTERVAL;
    http_request = SET;
  }
  else if(HTTP_Params.send_hash == SET)
  {
    HTTP_Params.request_type = SEND_HASH;
    http_request = SET;
  }
  else if(HTTP_Params.dl_ca == SET)
  {
    HTTP_Params.request_type = SERVER_CERT_FILE;
    http_request = SET;
  }
  else if(HTTP_Params.dl_cc == SET)
  {
    HTTP_Params.request_type = CLIENT_CERT_FILE;
    http_request = SET;
  }
  else if(HTTP_Params.dl_ck == SET)
  {
    HTTP_Params.request_type = CLIENT_KEY_FILE;
    http_request = SET;
  }
  
  if(http_request == SET)
    GSM_Parameters.stage = HTTP_STAGE_ENTER_URL_LENGTH;
  else
    GSM_Parameters.stage = HTTP_STAGE_DEACT_PDP;
  
  GSM_Parameters.stage_action = SIM_SEND_REQ;
  GSM_Parameters.number_of_retries_command = 0;
}


void HTTP_Manage_File_Process_resp(uint16_t* resp_msg, int16_t error)
{
  asm("nop");
}


void HTTP_enter_URL_length(void)
{
  uint8_t msg[128] = {0};
  uint16_t msg_length = 0;
  
  msg_length = strlen("AT+QHTTPURL=");
  memcpy(msg, "AT+QHTTPURL=", msg_length);
  
  if(setting.http_server_length != 0 && setting.http_server_length != 0xFF)
  {
    uint16_t url_length = 0;
    if(HTTP_Params.request_type == BOOTLOADER_FWR || HTTP_Params.request_type == APPLICATION_FWR ||
       HTTP_Params.request_type == FW_UPGRADE_INFO)
    {
      url_length = 7;                   // http://
      url_length += setting.http_server_length;
      url_length += strlen("/Tracker/S500/");
      
      url_length += 4;                  // Fwr/
      if(HTTP_Params.request_type == FW_UPGRADE_INFO)
      {
        url_length += 13;               // S500-fwux.bin
      }
      else if(HTTP_Params.request_type == BOOTLOADER_FWR)
      {
        if(HTTP_Params.file_chunk_idx > 9)
          url_length += 17;            // S500-Boot_xx.bin
        else
          url_length += 16;             // S500-Boot_x.bin
      }
      else if(HTTP_Params.request_type == APPLICATION_FWR)
      {
        if(HTTP_Params.file_chunk_idx > 9)
          url_length += 16;             // S500-App_xx.bin
        else
          url_length += 15;             // S500-App_x.bin
      }
      
      GSM_add_char_number_to_buffer(msg, &msg_length, url_length);
      
      msg[msg_length++] = ',';
      msg[msg_length++] = '3';
      msg[msg_length++] = '0';
      msg[msg_length++] = '\r';
    }
    else if(HTTP_Params.request_type == SERVER_CERT_FILE || HTTP_Params.request_type == CLIENT_CERT_FILE ||
            HTTP_Params.request_type == CLIENT_KEY_FILE)
    {
      url_length = 7;                   // http://
      url_length += setting.http_server_length;
      url_length += strlen("/Tracker/S500/");
      
      url_length += 6;                  // Certs/
      url_length += 16;                 // IMEI + '/'
      
      if(HTTP_Params.request_type == SERVER_CERT_FILE)
        url_length += 8;                // Root.cer
      else if(HTTP_Params.request_type == CLIENT_CERT_FILE || HTTP_Params.request_type == CLIENT_KEY_FILE )
        url_length += 19;               // IMEI.cer or IMEI.pem
      
      GSM_add_char_number_to_buffer(msg, &msg_length, url_length);
      
      msg[msg_length++] = ',';
      msg[msg_length++] = '3';
      msg[msg_length++] = '0';
      msg[msg_length++] = '\r';
    }
    else if (HTTP_Params.request_type == SEND_RECORD || HTTP_Params.request_type == SEND_INFORMATION ||
             HTTP_Params.request_type == GET_SETTING || HTTP_Params.request_type == SEND_INTERVAL || 
               HTTP_Params.request_type == SEND_HASH)
    {
      url_length = strlen("http://sipdata.src-co.ir/api/data/");
      GSM_add_char_number_to_buffer(msg, &msg_length, url_length);
      
      msg[msg_length++] = ',';
      msg[msg_length++] = '3';
      msg[msg_length++] = '0';
      msg[msg_length++] = '\r';
    }
  }
  
  GSM_Send_Usart(msg, msg_length);
}


void HTTP_enter_URL_length_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_CONNECT)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_ENTER_URL;
    GSM_Parameters.prev_stage = HTTP_STAGE_ENTER_URL_LENGTH;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void HTTP_enter_URL(void)
{
  uint8_t msg[164] = {0};
  uint16_t msg_length = 0;
  
  if(setting.http_server_length != 0 && setting.http_server_length != 0xFF)
  {
    if(HTTP_Params.request_type == BOOTLOADER_FWR || HTTP_Params.request_type == APPLICATION_FWR ||
       HTTP_Params.request_type == FW_UPGRADE_INFO)
    {
      msg_length = strlen("http://");
      memcpy(msg, "http://", msg_length);
      
      if(setting.http_server_length != 0 && setting.http_server_length != 0xFF)
      {
        memcpy(&msg[msg_length], &setting.http_server[1], setting.http_server_length);
        msg_length += setting.http_server_length;
      }
      
      memcpy(&msg[msg_length], "/Tracker/S500/", strlen("/Tracker/S500/"));
      msg_length += strlen("/Tracker/S500/");
      
      memcpy(&msg[msg_length], "Fwr/", 4);
      msg_length += 4;
      
      if(HTTP_Params.request_type == FW_UPGRADE_INFO)
      {
        memcpy(&msg[msg_length], "S500-fwu", strlen("S500-fwu"));
        msg_length += strlen("S500-fwu");
        
        msg[msg_length++] = SIPAAD_2G_FW_IDX + 0x30;
        
        memcpy(&msg[msg_length], ".bin", strlen(".bin"));
        msg_length += strlen(".bin");
      }
      else if(HTTP_Params.request_type == BOOTLOADER_FWR)
      {
        memcpy(&msg[msg_length], "S500-Boot", strlen("S500-Boot"));
        msg_length += strlen("S500-Boot");
        
        msg[msg_length++] = SIPAAD_2G_FW_IDX + 0x30;
        msg[msg_length++] = '_';
        
        msg[msg_length++] = HTTP_Params.file_chunk_idx + 0x30;
        
        memcpy(&msg[msg_length], ".bin", strlen(".bin"));
        msg_length += strlen(".bin");
      }
      else if(HTTP_Params.request_type == APPLICATION_FWR)
      {
        memcpy(&msg[msg_length], "S500-App", strlen("S500-App"));
        msg_length += strlen("S500-App");
        
        msg[msg_length++] = SIPAAD_2G_FW_IDX + 0x30;
        msg[msg_length++] = '_';
        
        if(HTTP_Params.file_chunk_idx < 10)
          msg[msg_length++] = HTTP_Params.file_chunk_idx + 0x30;
        else
        {
          uint8_t divid = HTTP_Params.file_chunk_idx / 10;
          msg[msg_length++] = divid + 0x30;
          
          uint8_t remain = HTTP_Params.file_chunk_idx % 10;
          msg[msg_length++] = remain + 0x30;
        }
        
        memcpy(&msg[msg_length], ".bin", strlen(".bin"));
        msg_length += strlen(".bin");
      }
    }
    else if(HTTP_Params.request_type == SERVER_CERT_FILE || HTTP_Params.request_type == CLIENT_CERT_FILE ||
            HTTP_Params.request_type == CLIENT_KEY_FILE)
    {
      msg_length = strlen("http://");
      memcpy(msg, "http://", msg_length);
      
      if(setting.http_server_length != 0 && setting.http_server_length != 0xFF)
      {
        memcpy(&msg[msg_length], &setting.http_server[1], setting.http_server_length);
        msg_length += setting.http_server_length;
      }
      
      memcpy(&msg[msg_length], "/Tracker/S500/", strlen("/Tracker/S500/"));
      msg_length += strlen("/Tracker/S500/");
      
      memcpy(&msg[msg_length], "Certs/", 6);
      msg_length += 6;
      
      GSM_add_char_number_to_buffer(msg, &msg_length, system_IMEI);
      
      msg[msg_length++] = '/';
      
      if(HTTP_Params.request_type == SERVER_CERT_FILE)
      {
        memcpy(&msg[msg_length], "Root.cer", 8);
        msg_length += 8;
      }
      else if(HTTP_Params.request_type == CLIENT_CERT_FILE)
      {
        GSM_add_char_number_to_buffer(msg, &msg_length, system_IMEI);
        
        memcpy(&msg[msg_length], ".cer", 4);
        msg_length += 4;
      }
      else if(HTTP_Params.request_type == CLIENT_KEY_FILE )
      {
        GSM_add_char_number_to_buffer(msg, &msg_length, system_IMEI);
        
        memcpy(&msg[msg_length], ".pem", 4);
        msg_length += 4;
      }
    }
    else if (HTTP_Params.request_type == SEND_RECORD || HTTP_Params.request_type == SEND_INFORMATION ||
             HTTP_Params.request_type == GET_SETTING || HTTP_Params.request_type == SEND_INTERVAL || 
               HTTP_Params.request_type == SEND_HASH)
    {
      memcpy(&msg[msg_length], "http://sipdata.src-co.ir/api/data/", strlen("http://sipdata.src-co.ir/api/data/"));
      msg_length += strlen("http://sipdata.src-co.ir/api/data/");
    }
  }
  
  GSM_Send_Usart(msg, msg_length);
}


void HTTP_enter_URL_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.prev_stage = HTTP_STAGE_ENTER_URL;
    GSM_Parameters.number_of_retries_command = 0;
    
    if(HTTP_Params.request_type == SEND_RECORD || HTTP_Params.request_type == SEND_INFORMATION ||
       HTTP_Params.request_type == GET_SETTING || HTTP_Params.request_type == SEND_INTERVAL ||
         HTTP_Params.request_type == SEND_HASH)
      GSM_Parameters.stage = HTTP_STAGE_POST_DATA;
    else
      GSM_Parameters.stage = HTTP_STAGE_DELETE_RAM_FILES;
  }
}


void HTTP_Post_data(void)
{
  uint8_t msg[50] = {0};
  uint16_t msg_length = 0;
  
  if(HTTP_Params.request_type == SEND_RECORD)
    HTTP_Create_Records_Post();
  else if(HTTP_Params.request_type == SEND_INFORMATION)
  {
    HTTP_Create_Information_message();
    
    if(FCB_profile.s1_nsend_count > setting.hash_nsend_count)
      HTTP_Params.send_hash = SET;
  }
  else if(HTTP_Params.request_type == GET_SETTING)
    HTTP_Create_setting_request();
  else if(HTTP_Params.request_type == SEND_INTERVAL)
    HTTP_Create_Interval_Information();
  else if(HTTP_Params.request_type == SEND_HASH)
    HTTP_Create_Hash_message();
  
  msg_length = strlen("AT+QHTTPPOST=");
  memcpy(msg, "AT+QHTTPPOST=", msg_length);
  
  GSM_add_char_number_to_buffer(msg, &msg_length, server_pack_length);
  
  memcpy(&msg[msg_length], ",50,20\r", 7);
  msg_length += 7;
  
  GSM_Send_Usart(msg, msg_length);
}


void HTTP_Post_data_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_CONNECT)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_SEND_POST_DATA;
    GSM_Parameters.prev_stage = HTTP_STAGE_ENTER_URL_LENGTH;
  }
}


void HTTP_send_Post_data(void)
{
  GSM_Send_Usart(server_pack, server_pack_length);
}


void HTTP_send_Post_data_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_READ_POST_DATA;
    GSM_Parameters.prev_stage = HTTP_STAGE_ENTER_URL_LENGTH;
    GSM_Parameters.number_of_retries_command = 0;
  }
}

void HTTP_read_Post_data(void)
{
  uint8_t msg[50] = {0};
  uint16_t msg_length = 0;
  
  msg_length = strlen("AT+QHTTPREAD=50\r");
  memcpy(msg, "AT+QHTTPREAD=50\r", msg_length);
  
  GSM_clear_RX_buffer();
  
  GSM_Send_Usart(msg, msg_length);
  
  GSM_HALT_Timer = 2;
}


void HTTP_read_Post_data_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_CONNECT)
  {
    Delay(500);
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_MANAGE_FILE_PROCESS;
    GSM_Parameters.prev_stage = HTTP_STAGE_ENTER_URL_LENGTH;
    GSM_Parameters.number_of_retries_command = 0;

    if(HTTP_Params.request_type == SEND_RECORD)
    {
      char *res;
      res = strstr((char *)&GSM_rx_buffer[0],"[\"OK\"]");
      if(res)
      {
        HTTP_Params.send_record = RESET;
        *resp_msg = GSM_HTTP_CHECK_NEXT_REQUEST;
      }
    }
    else if(HTTP_Params.request_type == SEND_INFORMATION)
    {
      int16_t error_code = 0;
      GSM_check_msg(&error_code);
      
      HTTP_parse_information_response();
      
      http_has_request = SET;
      HTTP_Params.send_information = RESET;
      *resp_msg = GSM_HTTP_DONE_PROCESS;
    }
    else if(HTTP_Params.request_type == GET_SETTING)
    {
      int16_t error_code = 0;
      GSM_check_msg(&error_code);
      
      HTTP_parse_setting_response();
      
      http_has_request = SET;
      HTTP_Params.get_setting = RESET;
      *resp_msg = GSM_HTTP_DONE_PROCESS;
    }
    else if(HTTP_Params.request_type == SEND_INTERVAL)
    {
      char *res;
      res = strstr((char *)&GSM_rx_buffer[0],"[\"OK\"]");
      if(res)
      {
        http_has_request = SET;
        HTTP_Params.send_interval = RESET;
        *resp_msg = GSM_HTTP_DONE_PROCESS;
      }
    }
    else if(HTTP_Params.request_type == SEND_HASH)
    {
      int16_t error_code = 0;
      GSM_check_msg(&error_code);
      
      HTTP_parse_Hash_response();
      
      http_has_request = SET;
      HTTP_Params.send_hash = RESET;
      *resp_msg = GSM_HTTP_DONE_PROCESS;
    }
  }
  else if(*resp_msg == GSM_CME_ERROR)
  {
    if(error == CMEE_HTTP_GET_NO_REQ)
    {
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.stage = HTTP_STAGE_DEACT_PDP;
      GSM_Parameters.prev_stage = HTTP_STAGE_READ_POST_DATA;
      GSM_Parameters.number_of_retries_command = 0;
      
      *resp_msg = GSM_OK;
    }
  }
}


void HTTP_del_FS_RAM_file(void)
{
  GSM_Send_Usart("AT+QFDEL=\"RAM:*\"\r", strlen("AT+QFDEL=\"RAM:*\"\r"));
}


void HTTP_del_FS_RAM_file_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_GET;
    GSM_Parameters.prev_stage = HTTP_STAGE_DELETE_RAM_FILES;
    GSM_Parameters.number_of_retries_command = 0;
  }
  else if(*resp_msg == GSM_CME_ERROR)
  {
    // There is no file in RAM to delete
    if(error == CMEE_INVALID_PARAM)
    {
      *resp_msg = GSM_OK;
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.stage = HTTP_STAGE_GET;
      GSM_Parameters.prev_stage = HTTP_STAGE_DELETE_RAM_FILES;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}


void HTTP_GET(void)
{
  GSM_Send_Usart("AT+QHTTPGET=60\r", strlen("AT+QHTTPGET=60\r"));
}


void HTTP_GET_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_DL_FILE;
    GSM_Parameters.prev_stage = HTTP_STAGE_GET;
    GSM_Parameters.number_of_retries_command = 0;
  }
  else if(*resp_msg == GSM_CME_ERROR)
  {
    // There is no file in the directory or there is no directory
    //    if(error == CMEE_HTTP_RESP_FAILED || error == CMEE_HTTP_DNS_ERROR)
    //    {
    *resp_msg = GSM_OK;
    
    if(HTTP_Params.request_type == SERVER_CERT_FILE || HTTP_Params.request_type == CLIENT_CERT_FILE || HTTP_Params.request_type == CLIENT_KEY_FILE)
    {
      if(HTTP_Params.request_type == SERVER_CERT_FILE)
      {
        HTTP_Params.dl_ca = RESET;
      }
      else if(HTTP_Params.request_type == CLIENT_CERT_FILE)
      {
        HTTP_Params.dl_cc = RESET;
      }
      else if(HTTP_Params.request_type == CLIENT_KEY_FILE)
      {
        HTTP_Params.dl_ck = RESET;
      }
      
      GSM_Parameters.stage = HTTP_STAGE_MANAGE_FILE_PROCESS;
    }
    else
    {
      if(HTTP_Params.request_type == BOOTLOADER_FWR)
      {
        HTTP_Params.dl_boot = RESET;
      }
      else if(HTTP_Params.request_type == APPLICATION_FWR)
      {
        HTTP_Params.dl_app = RESET;
      }
      else if(HTTP_Params.request_type == FW_UPGRADE_INFO)
      {
        HTTP_Params.dl_fwr_info = RESET;
      }
      
      GSM_Parameters.stage = HTTP_STAGE_MANAGE_FILE_PROCESS;
    }
    
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.prev_stage = HTTP_STAGE_GET;
    GSM_Parameters.number_of_retries_command = 0;
    //    }
  }
}


void HTTP_Download_File(void)
{
  uint8_t msg[50] = {0};
  uint16_t msg_length = 0;
  
  msg_length = strlen("AT+QHTTPDL=\"RAM:");
  memcpy(msg, "AT+QHTTPDL=\"RAM:", msg_length);
  
  if(HTTP_Params.request_type == BOOTLOADER_FWR || HTTP_Params.request_type == APPLICATION_FWR || HTTP_Params.request_type == FW_UPGRADE_INFO)
  {
    if(HTTP_Params.request_type == FW_UPGRADE_INFO)
    {
      memcpy(&msg[msg_length], "fwu.info\"", 9);
      msg_length+= 9;
    }
    else if(HTTP_Params.request_type == BOOTLOADER_FWR)
    {
      memcpy(&msg[msg_length], "boot.bin\"", 9);
      msg_length+= 9;
    }
    else if(HTTP_Params.request_type == APPLICATION_FWR)
    {
      memcpy(&msg[msg_length], "app.bin\"", 8);
      msg_length+= 8;
    }
    
    memcpy(&msg[msg_length], ",9223\r", 6);     // 9223 = (9 * 1024) + 7 bytes header
    msg_length+= 6;
  }
  else if(HTTP_Params.request_type == SERVER_CERT_FILE || HTTP_Params.request_type == CLIENT_CERT_FILE || HTTP_Params.request_type == CLIENT_KEY_FILE)
  {
    if(HTTP_Params.request_type == SERVER_CERT_FILE)
    {
      memcpy(&msg[msg_length], "Root.cer\"", 9);
      msg_length+= 9;
    }
    else if(HTTP_Params.request_type == CLIENT_CERT_FILE)
    {
      GSM_add_char_number_to_buffer(msg, &msg_length, system_IMEI);
      
      memcpy(&msg[msg_length], ".cer\"", 5);
      msg_length+= 5;
    }
    else if(HTTP_Params.request_type == CLIENT_KEY_FILE)
    {
      GSM_add_char_number_to_buffer(msg, &msg_length, system_IMEI);
      
      memcpy(&msg[msg_length], ".pem\"", 5);
      msg_length+= 5;
    }
    
    memcpy(&msg[msg_length], ",1024\r", 6);
    msg_length+= 6;
  }
  
  GSM_Send_Usart(msg, msg_length);
}


void HTTP_Download_File_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_HTTP_DL)
  {
    int64_t recv_num = 0;
    int16_t downloaded_size = 0;
    int16_t content_length = 0;
    int16_t error_code = 0;
    
    // First data
    int8_t sign_data = Rx_line_buffer[line_buffer_rd_index];
    if(sign_data == '-')
    {
      line_buffer_rd_index++;
      sign_data = -1;
    }
    else
      sign_data = 1;
    
    // dl_size
    GSM_get_number_from_line_buffer(',', (uint64_t*)&recv_num);
    recv_num *= sign_data;
    downloaded_size = recv_num;
    HTTP_Params.file_size = downloaded_size;
    
    // content-length
    sign_data = Rx_line_buffer[line_buffer_rd_index];
    if(sign_data == '-')
    {
      line_buffer_rd_index++;
      sign_data = -1;
    }
    else
      sign_data = 1;
    GSM_get_number_from_line_buffer(',', (uint64_t*)&recv_num);
    recv_num *= sign_data;
    content_length = recv_num;
    
    // err-code
    sign_data = Rx_line_buffer[line_buffer_rd_index];
    if(sign_data == '-')
    {
      line_buffer_rd_index++;
      sign_data = -1;
    }
    else
      sign_data = 1;
    GSM_get_number_from_line_buffer('\r', (uint64_t*)&recv_num);
    recv_num *= sign_data;
    error_code = recv_num;
    
    if(error_code == 0 && content_length == downloaded_size)
    {
      HTTP_Params.file_size = downloaded_size;
      GSM_Parameters.stage = HTTP_STAGE_FS_OPEN_FILE;
      
      HTTP_Params.checksum_file_dl_retry++;
    }
    else if(error_code == CMEE_FILE_TOO_LARGE)
    {
      // Downloaded size is less than file size (content_length)
      GSM_Parameters.stage = HTTP_STAGE_DL_FILE;
    }
    
    
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.prev_stage = HTTP_STAGE_DL_FILE;
    GSM_Parameters.next_stage = 0;
    GSM_Parameters.number_of_retries_command = 0;
  }
  else if(*resp_msg == GSM_CME_ERROR)
  {
    if(error == CMEE_INVALID_INPUT_VAL)
    {
      *resp_msg = GSM_OK;
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.stage = HTTP_STAGE_DL_FILE;
      GSM_Parameters.prev_stage = HTTP_STAGE_DL_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
    if(error == CMEE_HTTP_GET_NO_REQ)
    {
      *resp_msg = GSM_OK;
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.stage = HTTP_STAGE_GET;
      GSM_Parameters.prev_stage = HTTP_STAGE_DL_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
  }
}


void HTTP_FS_Open_File(void)
{
  uint8_t msg[50] = {0};
  uint16_t msg_length = 0;
  
  msg_length = strlen("AT+QFOPEN=\"RAM:");
  memcpy(msg, "AT+QFOPEN=\"RAM:", msg_length);
  
  if(HTTP_Params.request_type == BOOTLOADER_FWR || HTTP_Params.request_type == APPLICATION_FWR || HTTP_Params.request_type == FW_UPGRADE_INFO)
  {
    if(HTTP_Params.request_type == FW_UPGRADE_INFO)
    {
      memcpy(&msg[msg_length], "fwu.info\",2\r", 12);
      msg_length+= 12;
    }
    else if(HTTP_Params.request_type == BOOTLOADER_FWR)
    {
      memcpy(&msg[msg_length], "boot.bin\",2\r", 12);
      msg_length+= 12;
    }
    else if(HTTP_Params.request_type == APPLICATION_FWR)
    {
      memcpy(&msg[msg_length], "app.bin\",2\r", 11);
      msg_length+= 11;
    }
  }
  else if(HTTP_Params.request_type == SERVER_CERT_FILE || HTTP_Params.request_type == CLIENT_CERT_FILE || HTTP_Params.request_type == CLIENT_KEY_FILE)
  {
    http_SPIF_Cert_wr_index = 2;
    if(HTTP_Params.request_type == SERVER_CERT_FILE)
    {
      memcpy(&msg[msg_length], "Root.cer\",2\r", 12);
      msg_length+= 12;
    }
    else if(HTTP_Params.request_type == CLIENT_CERT_FILE)
    {
      GSM_add_char_number_to_buffer(msg, &msg_length, system_IMEI);
      
      memcpy(&msg[msg_length], ".cer\",2\r", 8);
      msg_length+= 8;
    }
    else if(HTTP_Params.request_type == CLIENT_KEY_FILE)
    {
      GSM_add_char_number_to_buffer(msg, &msg_length, system_IMEI);
      
      memcpy(&msg[msg_length], ".pem\",2\r", 8);
      msg_length+= 8;
    }
  }
  
  GSM_Send_Usart(msg, msg_length);
}


void HTTP_FS_Open_File_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_FS_OPEN)
  {
    int64_t recv_num = 0;
    
    // First data
    int8_t sign_data = Rx_line_buffer[line_buffer_rd_index];
    if(sign_data == '-')
    {
      line_buffer_rd_index++;
      sign_data = -1;
    }
    else
    {
      sign_data = 1;
    }
    
    if(GSM_get_number_from_line_buffer('\r', (uint64_t*)&recv_num) == GSM_OK)
    {
      recv_num *= sign_data;
      if(recv_num > 0)
      {
        HTTP_Params.file_handle = recv_num;
        FS_file_read_index = 0;
        
        GSM_Parameters.number_of_retries_command = 0;
        GSM_Parameters.next_stage = HTTP_STAGE_FS_SET_POSITION;
      }
      else
      {
        GSM_Parameters.next_stage = HTTP_STAGE_FS_OPEN_FILE;
      }
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.stage = HTTP_STAGE_SEEK_OK;
      GSM_Parameters.prev_stage = HTTP_STAGE_FS_OPEN_FILE;
    }
  }
  else if(*resp_msg == GSM_CME_ERROR)
  {
    if(error == CMEE_FILE_NOT_FOUND)
    {
      *resp_msg = GSM_OK;
      
      GSM_Parameters.stage_action = SIM_SEND_REQ;
      GSM_Parameters.stage = HTTP_STAGE_FS_OPEN_FILE;
      GSM_Parameters.prev_stage = HTTP_STAGE_FS_OPEN_FILE;
      GSM_Parameters.number_of_retries_command = 0;
    }
    else if(error == CMEE_ACCESS_DENIED)
    {
      // File is already opened and you can not find the file-handle
      // reset the module
    }
  }
}


void HTTP_FS_Set_Position(void)
{
  uint8_t msg[50] = {0};
  uint16_t msg_length = 0;
  
  msg_length = strlen("AT+QFSEEK=");
  memcpy(msg, "AT+QFSEEK=", msg_length);
  GSM_add_char_number_to_buffer(msg, &msg_length, HTTP_Params.file_handle);
  
  msg[msg_length++] = ',';
  
  GSM_add_char_number_to_buffer(msg, &msg_length, FS_file_read_index);
  
  msg[msg_length++] = '\r';
  
  GSM_Send_Usart(msg, msg_length);
}


void HTTP_FS_Set_Position_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_FS_READ_FILE;
    GSM_Parameters.prev_stage = HTTP_STAGE_FS_SET_POSITION;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void HTTP_FS_Read_File(void)
{
  uint8_t msg[50] = {0};
  uint16_t msg_length = 0;
  
  FS_num_bytes_read = HTTP_Params.file_size - FS_file_read_index;
  if(FS_num_bytes_read > 0)
  {
    if(FS_num_bytes_read > MAX_FS_FILE_READ_SIZE)
      FS_num_bytes_read = MAX_FS_FILE_READ_SIZE;
  }
  
  msg_length = strlen("AT+QFREAD=");
  memcpy(msg, "AT+QFREAD=", msg_length);
  GSM_add_char_number_to_buffer(msg, &msg_length, HTTP_Params.file_handle);
  
  msg[msg_length++] = ',';
  
  GSM_add_char_number_to_buffer(msg, &msg_length, FS_num_bytes_read);
  
  msg[msg_length++] = '\r';
  
  // Clear the gsm serial buffer
  GSM_clear_RX_buffer();
  
  GSM_Send_Usart(msg, msg_length);
  
  GSM_HALT_Timer = 2;
}


void HTTP_FS_Read_File_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_CONNECT)
  {
    uint64_t recv_num = 0;
    line_buffer_counter--;
    line_buffer_rd_index++;
    
    if(GSM_get_number_from_line_buffer('\r', (uint64_t*)&recv_num) == GSM_OK)
    {
      if(recv_num == FS_num_bytes_read)
      {         // Certificates Files
        if(HTTP_Params.request_type == SERVER_CERT_FILE || HTTP_Params.request_type == CLIENT_CERT_FILE || HTTP_Params.request_type == CLIENT_KEY_FILE)
        {
          SPIF_save_HTTP_file_chunk(&GSM_rx_buffer[GSM_rx_rd_index], FS_num_bytes_read, HTTP_Params.request_type, &http_SPIF_Cert_wr_index);
          FS_file_read_index += FS_num_bytes_read;
          GSM_clear_RX_buffer();
          
          if((HTTP_Params.file_size - FS_file_read_index) == 0)
          {
            SPIF_Save_Cert_size(HTTP_Params.file_size, HTTP_Params.request_type);
            
            if(HTTP_Params.request_type == SERVER_CERT_FILE)
            {
              HTTP_Params.dl_ca = RESET;
            }
            else if(HTTP_Params.request_type == CLIENT_CERT_FILE)
            {
              HTTP_Params.dl_cc = RESET;
            }
            else if(HTTP_Params.request_type == CLIENT_KEY_FILE)
            {
              HTTP_Params.dl_ck = RESET;
            }
            
            GSM_Parameters.stage = HTTP_STAGE_FS_CLOSE_FILE;
          }
          else
          {
            GSM_Parameters.stage = HTTP_STAGE_FS_SET_POSITION;
          }
        }
        else
        {       // Firmware Files
          if(HTTP_Params.request_type == FW_UPGRADE_INFO)
          {
            HTTP_analyze_FWU_file(&GSM_rx_buffer[GSM_rx_rd_index], FS_num_bytes_read);
            
            HTTP_Params.checksum_file_dl_retry = 0;
            HTTP_Params.dl_fwr_info = RESET;
            HTTP_Params.file_chunk_idx = 1;
            GSM_Parameters.stage = HTTP_STAGE_FS_CLOSE_FILE;
          }
          else if(HTTP_Params.request_type == BOOTLOADER_FWR || HTTP_Params.request_type == APPLICATION_FWR)
          {
            if(FS_file_read_index == 0)
            {
              HTTP_analyze_chunk_header(&GSM_rx_buffer[GSM_rx_rd_index]);
              GSM_rx_rd_index += 7;
              FS_file_read_index += 7;
              FS_num_bytes_read -= 7;
            }
            SPIF_save_HTTP_file_chunk(&GSM_rx_buffer[GSM_rx_rd_index], FS_num_bytes_read, HTTP_Params.request_type, &http_SPIF_Fwr_wr_index);
            
            FS_file_read_index += FS_num_bytes_read;
            GSM_clear_RX_buffer();
            
            if((HTTP_Params.file_size - FS_file_read_index) == 0)
            {
              uint32_t calc_crc = 0;
              uint8_t checksum_result = RESET;
              
              while(HTTP_Params.checksum_retry < 4)
              {
                HTTP_Params.checksum_retry++;
                
                // Checking chunk CRC-32 validation
                calc_crc = HTTP_compute_chunk_CRC32(HTTP_Params.file_size - 7, HTTP_Params.request_type, (http_SPIF_Fwr_wr_index - (HTTP_Params.file_size - 7)));
                Delay(500);
                
                if(calc_crc == HTTP_FWU.chunk_crc32)
                {
                  checksum_result = SET;
                  HTTP_Params.checksum_retry = 0;
                  
                  break;
                }
              }
              
              if(checksum_result == SET)
              {
                HTTP_Params.checksum_file_dl_retry = 0;
                if(HTTP_Params.request_type == BOOTLOADER_FWR)
                {
                  if(++HTTP_Params.file_chunk_idx > HTTP_FWU.boot_chunk_count)
                  {
                    HTTP_Params.dl_boot = RESET;
                    
                    // Checking MD5 Hash
                    uint8_t fw_md5[16] = {0};
                    error_t error;
                    error = PT_generate_firmware_MD5(fw_md5, BOOTLOADER_FWR, HTTP_FWU.boot_size);
                    if(error == NO_ERROR)
                    {
                      // MD5  Matches
                      int cmp_result = 0;
                      cmp_result = memcmp(fw_md5, HTTP_FWU.boot_md5, 16);
                      if(cmp_result == 0)
                      {
                        // MD5 Match
                        Flash_Execute_FirmwareUpgrade(HTTP_FWU.boot_size);
                      }
                      else
                      {
                        // TODO:
                      }
                    }
                    // Reset download parameters
                    HTTP_Params.file_chunk_idx = 1;
                    http_SPIF_Fwr_wr_index = 0;
                  }
                }
                else if(HTTP_Params.request_type == APPLICATION_FWR)
                {
                  if(++HTTP_Params.file_chunk_idx > HTTP_FWU.app_chunk_count)
                  {
                    HTTP_Params.dl_app = RESET;
                    
                    // Checking MD5 Hash
                    uint8_t fw_md5[16] = {0};
                    error_t error;
                    error = PT_generate_firmware_MD5(fw_md5, APPLICATION_FWR, HTTP_FWU.app_size);
                    if(error == NO_ERROR)
                    {
                      // MD5  Matches
                      int cmp_result = 0;
                      cmp_result = memcmp(fw_md5, HTTP_FWU.app_md5, 16);
                      if(cmp_result == 0)
                      {
                        // MD5 Match
                        system_temp_params.firmware_CRC_value = PT_compute_Firmware_CRC16(HTTP_FWU.app_size, APPLICATION_FWR, 0);
                        system_temp_params.firmware_size = HTTP_FWU.app_size;
                        system_temp_params.firmware_available = SET;
                        system_temp_params.firmware_source = SRC_SERVER;
                        
                        system_temp_params.device_version_temp = system_temp_params.device_version;
                        system_temp_params.upgrade_done = 0;
                        SPIF_Write_sys_temp_params();
                        
                        http_firmware_upgrade_ready = SET;
                      }
                      else
                      {
                        // TODO:
                      }
                    }
                    // Reset download parameters
                    HTTP_Params.file_chunk_idx = 1;
                    http_SPIF_Fwr_wr_index = 0;
                  }
                }
              }
              else
              {
                HTTP_Params.checksum_retry = 0;
                http_SPIF_Fwr_wr_index -= HTTP_Params.file_size - 7;
                
                if(HTTP_Params.checksum_file_dl_retry > 2)
                {
                  if(HTTP_Params.request_type == BOOTLOADER_FWR)
                    HTTP_Params.dl_boot = RESET;
                  else if(HTTP_Params.request_type == APPLICATION_FWR)
                    HTTP_Params.dl_app = RESET;
                  
                  // Reset download parameters
                  HTTP_Params.checksum_file_dl_retry = 0;
                  HTTP_Params.file_chunk_idx = 1;
                  http_SPIF_Fwr_wr_index = 0;
                }
              }
              
              FS_file_read_index = 0;
              GSM_Parameters.stage = HTTP_STAGE_FS_CLOSE_FILE;
            }
            else
            {
              GSM_Parameters.stage = HTTP_STAGE_FS_SET_POSITION;
            }
          }
        }
      }
    }
    
    
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.prev_stage = HTTP_STAGE_FS_READ_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void HTTP_FS_Close_File(void)
{
  uint8_t msg[50] = {0};
  uint16_t msg_length = 0;
  
  msg_length = strlen("AT+QFCLOSE=");
  memcpy(msg, "AT+QFCLOSE=", msg_length);
  GSM_add_char_number_to_buffer(msg, &msg_length, HTTP_Params.file_handle);
  
  msg[msg_length++] = '\r';
  
  GSM_Send_Usart(msg, msg_length);
}


void HTTP_FS_Close_File_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = HTTP_STAGE_MANAGE_FILE_PROCESS;
    GSM_Parameters.prev_stage = HTTP_STAGE_FS_CLOSE_FILE;
    GSM_Parameters.number_of_retries_command = 0;
  }
}


void HTTP_Deact_PDP(void)
{
  GSM_Send_Usart("AT+QIDEACT\r", strlen("AT+QIDEACT\r"));
}


void HTTP_Deact_PDP_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_DEACT_PDP)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = SIM_STAGE_TEST_SERIAL;
    GSM_Parameters.prev_stage = HTTP_STAGE_DEACT_PDP;
    GSM_Parameters.next_stage = SIM_STAGE_QUERY_CURRNET_CONNECTION_STATUS;
    GSM_Parameters.number_of_retries_command = 0;
    
    HTTP_Params.pdp_activated = RESET;
    GSM_Parameters.internet_connection = RESET;
    number_of_http_process_run = 0;
    
    http_has_request = RESET;
    
    if(disable_http_pdp == SET)
    {
      disable_http_pdp = RESET;
      GSM_Parameters.stage = HTTP_STAGE_TEST_SERIAL;
      http_has_request = SET;
    }
    
    *resp_msg = GSM_HTTP_DONE_PROCESS;
  }
}


void HTTP_seek_OK(void)
{
  asm("nop");
}


void HTTP_seek_OK_resp(uint16_t* resp_msg, int16_t error)
{
  if(*resp_msg == GSM_OK)
  {
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_Parameters.stage = GSM_Parameters.next_stage;
    GSM_Parameters.next_stage = 0;
    //    GSM_Parameters.prev_stage = 0;
  }
}


void (*HTTP_Request_ptr[])(void) = {&HTTP_reset_module,
                                    &HTTP_test_serial_port,
                                    &HTTP_cfg_foreground_context,
                                    &HTTP_cfg_APN,
                                    &HTTP_register_TCP_stack,
                                    &HTTP_Activate_FGCNT,
                                    &HTTP_query_Local_IP,
                                    &HTTP_Manage_File_Process,
                                    &HTTP_enter_URL_length,
                                    &HTTP_enter_URL,
                                    &HTTP_Post_data,
                                    &HTTP_send_Post_data,
                                    &HTTP_read_Post_data,
                                    &HTTP_del_FS_RAM_file,
                                    &HTTP_GET,
                                    &HTTP_Download_File,
                                    &HTTP_FS_Open_File,
                                    &HTTP_FS_Set_Position,
                                    &HTTP_FS_Read_File,
                                    &HTTP_FS_Close_File,
                                    &HTTP_Deact_PDP,
                                    &HTTP_seek_OK
};


void (*HTTP_Response_ptr[])(uint16_t*, int16_t) = {&HTTP_reset_module_resp,
                                                    &HTTP_test_serial_port_resp,
                                                    &HTTP_cfg_foreground_context_resp,
                                                    &HTTP_cfg_APN_resp,
                                                    &HTTP_register_TCP_stack_resp,
                                                    &HTTP_Activate_FGCNT_resp,
                                                    &HTTP_query_Local_IP_resp,
                                                    &HTTP_Manage_File_Process_resp,
                                                    &HTTP_enter_URL_length_resp,
                                                    &HTTP_enter_URL_resp,
                                                    &HTTP_Post_data_resp,
                                                    &HTTP_send_Post_data_resp,
                                                    &HTTP_read_Post_data_resp,
                                                    &HTTP_del_FS_RAM_file_resp,
                                                    &HTTP_GET_resp,
                                                    &HTTP_Download_File_resp,
                                                    &HTTP_FS_Open_File_resp,
                                                    &HTTP_FS_Set_Position_resp,
                                                    &HTTP_FS_Read_File_resp,
                                                    &HTTP_FS_Close_File_resp,
                                                    &HTTP_Deact_PDP_resp,
                                                    &HTTP_seek_OK_resp
};


uint16_t GSM_HTTP_routine_pro(uint8_t function_index)
{
  uint16_t check_msg_result = GSM_WAITING;
  
  if(GSM_HALT_Timer == GSM_TIMER_DEFAULT)
  {
    switch(GSM_Parameters.stage_action)
    {
    case SIM_SEND_REQ:
      {
        GSM_Parameters.number_of_retries_command++;
        
        if(GSM_Parameters.number_of_retries_command > http_stage_max_num_of_retries[function_index])
        {
          if(function_index == HTTP_STAGE_ACTIVATE_FGCNT)
          {
            enable_http_process = RESET;
            
            server_is_unreachable = SET;
            reset_server_unreachable_counter = 1800;   // 0.5 Hours
          }
          
          GSM_Parameters.stage = http_stage_retries_redirect[function_index];
          GSM_Parameters.number_of_retries_command = 0;
          break;
        }
        
        GSM_Parameters.stage_action = SIM_RCV_RESP;
        gsm_pt_timeout = http_stage_timeout[GSM_Parameters.stage]*GSM_MSG_DEFAULT_TIMEOUT;
        (*HTTP_Request_ptr[function_index])();
        break;
      }
    case SIM_RCV_RESP:
      {
        int16_t msg_err = 0;
        check_msg_result = GSM_check_msg(&msg_err);
        if(gsm_pt_timeout == 0 && check_msg_result == GSM_WAITING)
        {
          GSM_Parameters.stage_action = SIM_SEND_REQ;
          GSM_Parameters.stage = http_stage_TO_CME_Error[function_index];
          return GSM_TIMEOUT;
        }
        else
        {
          if(check_msg_result == GSM_LF || check_msg_result == GSM_WAITING)
            break;
          else
          {
            (*HTTP_Response_ptr[function_index])(&check_msg_result, msg_err);
            
            if(check_msg_result == GSM_CME_ERROR)
            {
              GSM_Parameters.stage_action = SIM_SEND_REQ;
              
              if(msg_err == CMEE_NO_SIMCARD)
              {
                GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
                GSM_Parameters.simcard_available = RESET;
                GSM_Parameters.nw_provider = 0;
                GSM_status = GSM_INITIATING;
              }
              else
                GSM_Parameters.stage = http_stage_TO_CME_Error[function_index];
            }
          }
        }
        break;
      }
    } /* end of switch(GSM_Parameters.stage_action) */
  }
  else
    check_msg_result = GSM_HALT;
  
  return check_msg_result;
}