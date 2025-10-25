#include "GPS.h"
#include "general_functions.h"


GPS_DATA_typedef        GPS_data;
RMC_NMEA_message        RMC_Message;
GSA_NMEA_message        GSA_Message;
GGA_NMEA_message        GGA_Message;



uint8_t read_buff_index = 0;

uint64_t startup_latitude_average = 0;
uint64_t startup_longitude_average = 0;
uint8_t startup_fixed_counter = 0;

uint32_t last_valid_latitude = 0;
uint32_t last_valid_longitude = 0;
uint16_t last_valid_altitude = 0;
uint16_t last_valid_COG = 0;

uint32_t last_record_latitude = 0;
uint32_t last_record_longitude = 0;
uint16_t last_record_COG = 0;

uint8_t check_location_jump = RESET;
uint8_t gps_is_fixed_for_first_time = RESET;
uint8_t device_has_speed = 0;
uint8_t gps_has_data = 0;
uint16_t gps_max_speed = 0;
uint32_t last_PPS_time_unix = 0;
uint8_t moving_status = RESET;
uint8_t check_updating_timeunix = RESET;

uint8_t gps_enter_sleep = RESET;
uint8_t gps_exit_sleep = RESET;
uint8_t gps_activity = RESET;
int16_t set_gps_sleep_counter = 0;
uint8_t reconfigure_GPS = RESET;
uint8_t reconfig_gps_protection = RESET;
uint16_t reconfig_gps_protection_counter = 0;


// Prototypes
void GPS_disable_all_NMEA_msgs(void);
void GPS_enable_GSA_RMC_NMEA_msgs(void);
void GPS_enable_sync_PPS(void);
void GPS_set_position_Fix(void);
void GPS_enhance_timing_product(void);
void GPS_set_PPS_2D3D_100ms(void);
void GPS_Enable_AIC(void);
void GPS_search_GPS_GNSS_sats(void);
void GPS_set_Tracking_mode(void);
void GPS_set_Static_Navigation(void);
void GPS_set_Jamming_Detection(void);


uint8_t RTC_Months[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Not leap year */
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}	/* Leap year */
};


uint8_t GPS_read_data(uint8_t* data)
{
  if(GPS_rx_counter == 0)
    return GPS_TIMEOUT;
  
  *data = GPS_rx_buffer[GPS_rx_rd_index++];
  if(GPS_rx_rd_index == GPS_USART_RX_BUFFER_SIZE)
    GPS_rx_rd_index = 0;

  GPS_rx_counter--;
  
  return GPS_OK;
}

uint8_t GPS_getline(uint8_t* data, uint8_t* bufferd_length)
{
  uint8_t  character = 0;
  uint8_t  iterator = 0;
  uint8_t fn_result = GPS_TIMEOUT;
  
  GPS_rx_timeout = 100;
  while(GPS_rx_timeout > 0)
  {
    if( GPS_read_data(&character) == GPS_OK )
    {
      if(character == NULL)
      {
        fn_result = GPS_ERROR;
        break;
      }
      
      *(data+iterator) = character;
      iterator++;
      if(character == '\n')
      {
        *(data+iterator) = NULL;
        *bufferd_length = iterator + 1;
        fn_result = GPS_OK;
        break;
      }
    }
  }
  
  return fn_result;
}

float str_to_float(char* str, uint8_t char_count/*, uint64_t* real_num, uint64_t* float_num*/)
{
  float tmp_data1 = 0;
  float tmp_data2 = 0;
  char char_num = 0;
  uint8_t float_part = 0;
  uint32_t power = 1;
  for(uint8_t iterator = 0; iterator < char_count; iterator++)
  {
    char_num = str[iterator];
    if(char_num == '.')
    {
      float_part = 1;
      continue;
    }
    if(float_part == 0)
      tmp_data1 = tmp_data1*10 + convert_char_to_int(char_num);
    else
    {
      power *= 10;
      tmp_data2 = tmp_data2*10 + convert_char_to_int(char_num);
    }
  }
  tmp_data2 /= power;
  tmp_data1 += tmp_data2;
  return tmp_data1;
}

void read_buffer_until(uint8_t* buffer, char terminator, uint8_t lenth)
{
  uint8_t tmp_char = 0;
  
  if(lenth > 0)
  {
    for(; read_buff_index < lenth; read_buff_index++)
    {
      tmp_char = *(buffer + read_buff_index);
      if (tmp_char == terminator)
      {
        read_buff_index++;
        return;
      }
    }
  }
}

void get_n_number_from(uint8_t *buffer, uint8_t start_point, uint8_t count, uint64_t *num)
{
  char str[9];
  Set_zero((uint8_t*)str, 9);
  memcpy(str, (buffer + start_point), count);

  *num = str_to_llint(str, count);
}

uint8_t get_n_number_from_till(uint8_t *buffer, uint8_t start_point, char terminator, uint64_t* num, uint8_t length)
{
  uint8_t i;
  char c = 0;
  char str[9]={0};
  for(i = 0; i < length; i++)
  {
    c = *(buffer + start_point + i);
    if(c == terminator)
    {
      break;
    }
    str[i] = c;
  }
  
  if(i == length && c != terminator)
    return 0;
  else
    *num = str_to_llint(str, i);
  
  return i;
}

//void get_n_float_from(uint8_t *buffer, uint8_t start_point, uint8_t count, float *num)
//{
//  char str[9];
//  memcpy(str, (buffer + start_point), count);
//  
//  *num = str_to_float(str, count);
//}
//
//
//void get_n_float_from_till(uint8_t *buffer, uint8_t start_point, char terminator, float *num, uint8_t length)
//{
//  uint8_t i;
//  for(i = 0; i < length; i++)
//  {
//    char c = *(buffer + start_point + i);
//    if (c == terminator)
//    {
//      break;
//    }
//  }
//  
//  char str[9];
//  memcpy(str, (buffer + start_point), i);
//  
//  *num = str_to_float(str, i);
//}

void GPS_clear_rx_buffer(void)
{
  memset(GPS_rx_buffer, 0, GPS_USART_RX_BUFFER_SIZE);
  
  GPS_rx_wr_index = 0;
  GPS_rx_rd_index = 0;
  GPS_rx_counter = 0;
}


uint32_t GPS_Update_UnixTimeStamp(GPS_Date_typedef* date)
{
  uint32_t days = 0, seconds = 0;
  uint16_t i;
  uint16_t year = (uint16_t) (date->year + 2000);
  
  // At system startup
  if(date->year == 0 && date->month == 0 && date->day == 0)
    return 0;
  
  /* Year is below offset year */
  if (year < RTC_OFFSET_YEAR)
    return 0;
  
  /* Days in back years */
  for (i = RTC_OFFSET_YEAR; i < year; i++)
    days += RTC_DAYS_IN_YEAR(i);
  
  /* Days in current year */
  for (i = 1; i < date->month; i++)
    days += RTC_Months[RTC_LEAP_YEAR(year)][i - 1];
    
  /* Day starts with 1 */
  days += date->day - 1;
  seconds = days * RTC_SECONDS_PER_DAY;
  seconds += date->hour * RTC_SECONDS_PER_HOUR;
  seconds += date->minute * RTC_SECONDS_PER_MINUTE;
  seconds += date->second;

  date->unix = seconds;
  
  return seconds;
}

// Calculate Checksums: https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
/*Parsed parameters from NMEA messages:
  GGA ---> Latitude, Longitude, Altitude, Number of satellites.
  GSA ---> PDOP, HDOP, Fixed mode.
  RMC ---> Time, Date, COG, Speed(knots).
*/
void GPS_Config_Module(void)
{
  uint8_t quit_loop = RESET;
  char* cmp_result = 0;
  
  system_error.gps_error = RESET;
  GPS_DISABLE_1PPS_EXTI;
  
  // Wait for GNSS Power up
  Delay(5000);
  
  // Disable All NMEAs
  cfg_timeout_counter = 500;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_disable_all_NMEA_msgs();
    Delay(300);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTK001");
    if(cmp_result > 0)
    {
      if(GPS_rx_buffer[13] == '3')
        quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }
  
  // Enable accurate PPS timing
  quit_loop = RESET;
  cfg_timeout_counter = 400;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_enhance_timing_product();
    Delay(200);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTK001");
    if(cmp_result > 0)
    {
      if(GPS_rx_buffer[13] == '3')
        quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }
  
  // Set PPS pulse for 2D/3D with 100ms pulse width
  quit_loop = RESET;
  cfg_timeout_counter = 400;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_set_PPS_2D3D_100ms();
    Delay(200);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTK001");
    if(cmp_result > 0)
    {
      if(GPS_rx_buffer[13] == '3')
        quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }
  
  // Set Fix time interval to 500ms
  quit_loop = RESET;
  cfg_timeout_counter = 400;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_set_position_Fix();
    Delay(200);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTK001");
    if(cmp_result > 0)
    {
      if(GPS_rx_buffer[13] == '3')
        quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }
  
  // Set GPS search for GPS and GNSS
  quit_loop = RESET;
  cfg_timeout_counter = 400;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_search_GPS_GNSS_sats();
    Delay(200);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTK001");
    if(cmp_result > 0)
    {
      if(GPS_rx_buffer[13] == '3')
        quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }
  
  // Set GPS Tracking mode
  quit_loop = RESET;
  cfg_timeout_counter = 400;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_set_Tracking_mode();
    Delay(200);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTK001");
    if(cmp_result > 0)
    {
      if(GPS_rx_buffer[13] == '3')
        quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }
  
/*  // Enable Jamming Detection
  quit_loop = RESET;
  cfg_timeout_counter = 2000;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_set_Jamming_Detection();
    Delay(100);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTKSPF");
    if(cmp_result > 0)
    {
      quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }*/
  
  // Enable Related NMEAs
  quit_loop = RESET;
  cfg_timeout_counter = 400;
  while(quit_loop == RESET && cfg_timeout_counter != 0)
  {
    GPS_clear_rx_buffer();
    GPS_enable_GSA_RMC_NMEA_msgs();
    Delay(200);
    
    cmp_result = strstr((char*)&GPS_rx_buffer, "$PMTK001");
    if(cmp_result > 0)
    {
      if(GPS_rx_buffer[13] == '3')
        quit_loop = SET;
    }
  }
  if(quit_loop == RESET)
  {
    system_error.gps_error = SET;
    return;
  }
  
  // Enable PPS sync
  // Disable Static Navigation
  // Active interference cancellation function (AIC)
  
  GPS_clear_rx_buffer();
  
  GPS_ENABLE_1PPS_EXTI;
  
  // Change LED Pattern
  if(system_current_state == SYSTEM_RUNNING)
    LED_change_mode(GPS_LED, LED_ON_BLINK_500MS);
}

void GPS_disable_all_NMEA_msgs(void)
{
  uint8_t msg[] = "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*34\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

/*
0:GLL, 1:RMC, 2:VTG, 3:GGA, 4:GSA, 5:GSV, 6:GRS, 7:GST, 17:ZDA
GSA ==> GP&GN
*/
void GPS_enable_GSA_RMC_NMEA_msgs(void)
{
  uint8_t msg[] = "$PMTK314,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*35\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

// This function configure the timeing of the NMEA output sentence
void GPS_set_position_Fix(void)
{
  uint8_t msg[] = "$PMTK220,1000*1F\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_enable_sync_PPS(void)
{
  uint8_t msg[] = "$PMTK255,1*2D\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_enhance_timing_product(void)
{
  uint8_t msg[] = "$PMTK256,1*2E\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_set_PPS_2D3D_100ms(void)
{
  uint8_t msg[] = "$PMTK285,3,100*3F\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_Enable_AIC(void)
{
  uint8_t msg[] = "$PMTK286,1*23\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_search_GPS_GNSS_sats(void)
{
  uint8_t msg[] = "$PMTK353,1,1,0,0,0*2B\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_set_Tracking_mode(void)
{
  uint8_t msg[] = "$PMTK886,0*28\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_set_Static_Navigation(void)
{
  uint8_t msg[] = "$PMTK386,0*23\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_set_Jamming_Detection(void)
{
  uint8_t msg[] = "$PQJAM,W,1,0*3D\r\n";
  
  GPS_Send_Usart(msg, strlen((char*)msg));
}

void GPS_superloop_call(void)
{
  if(gps_enter_sleep == SET)
  {
    GPS_SET_OFF;
    gps_activity = RESET;
    
    gps_enter_sleep = RESET;
    
    // Change LED Pattern
    if(system_current_state == SYSTEM_RUNNING)
      LED_change_mode(GPS_LED, LED_OFF);
  }
  if(gps_exit_sleep == SET)
  {
    gps_exit_sleep = RESET;
    if(gps_activity == RESET)
    {
      GPS_SET_ON;
      gps_activity = SET;
      
      GPS_clear_rx_buffer();
      GPS_Config_Module();
    }
  }
  if(reconfigure_GPS == SET)
  {
    reconfigure_GPS = RESET;
    
    IWDG_ReloadCounter();
    GPS_SET_OFF;
    Delay(5000);
    GPS_SET_ON;
    gps_activity = SET;
    
    IWDG_ReloadCounter();
    GPS_clear_rx_buffer();
    GPS_Config_Module();
  }
}

uint8_t GPS_validate_NMEA_messages(uint8_t* message, uint8_t message_length)
{
  uint8_t fn_result = GPS_ERROR;
  uint16_t star_index = 0;
  uint16_t dollar_index = 0;
  
  if( GF_find_cell_in_array(message, message_length, '*', &star_index) == SET)
  {
    uint8_t message_checksum =  GF_convert_string_hex_number_to_integer(message[star_index+1]) << 4;
    message_checksum += GF_convert_string_hex_number_to_integer(message[star_index+2]);
    
    if( GF_find_cell_in_array(message, message_length, '$', &dollar_index) == SET)
    {
      uint8_t calculated_checksum = GF_calculate_checksum_8_XOR(&message[dollar_index+1], (star_index - dollar_index - 1) );
      
      if(calculated_checksum == message_checksum)
        fn_result = GPS_OK;
    }
  }
  
  return fn_result;
}

/*
  uint8_t GGA_msg[] = "$GNGGA,091926.000,3113.3166,N,12121.2682,E,1,09,0.9,36.9,M,7.9,M,,0000*56\r\n";
  uint8_t GSA_msg[] = "$GPGSA,A,3,07,02,26,27,09,04,15,,,,,,1.8,1.0,1.5*33\r\n";
  uint8_t GSB_msg[] = "$GLGSA,A,3,07,02,26,27,09,04,15,,,,,,1.8,1.0,1.5*33<\r\n";
  uint8_t RMC_msg[] = "$GNRMC,094330.000,A,3113.3156,N,12121.2686,E,0.51,193.93,171210,,,A*68\r\n";
  uint8_t SPF_msg[] = "$PMTKSPF,1*5A\r\n"
*/

void GPS_NMEA_Parser(uint8_t* msg, uint8_t msg_size)
{
  uint8_t nmea_case = 20;       // Sth out of NMEA codes.
  char* result = 0;
  uint64_t tmp_data = 0;
  read_buff_index = 0;          // Reset the index

  result = strstr((char*)msg, "GGA");
  if(result > 0)
  {
    nmea_case = GPS_NMEA_GGA;
  }
  else
  {
    result = strstr((char*)msg, "PMTKSPF");
    if(result > 0)
    {
      nmea_case = GPS_JAMMING_DETECTION;
    }
    else
    {
      result = strstr((char*)msg, "RMC");
      if(result > 0)
      {
        nmea_case = GPS_NMEA_RMC;
        
        last_PPS_time_unix = system_Unixtime;
        
        // Change LED Pattern
        if(system_current_state == SYSTEM_RUNNING)
          LED_change_mode(GPS_LED, LED_ON_STEADY);
      }
      else
      {
        result = strstr((char*)msg, "GPGSA");
        if(result > 0)
        {
          nmea_case = GPS_NMEA_GSA;
        }
        else
        {
          result = strstr((char*)msg, "GLGSA");   // Garbage NMEA
          if(result > 0)
          {
            nmea_case = 20;
          }
        }
      }
    }
  }
  
  switch(nmea_case)
  {
  case GPS_NMEA_GGA:
    {
      /*
      uint8_t degree_tmp = 0;
      uint8_t minutes_tmp = 0;
      uint8_t count_i = 0;
      uint32_t seconds_division = 1;
      uint64_t seconds_tmp = 0;
      */
      
      uint8_t NMEA_field[15] = {0};
      uint8_t NMEA_field_length = 0;
      uint8_t NMEA_field_rd_index = 0;
      
      GGA_Message.process_error = RESET;
      
      
      read_buffer_until(msg, ',', msg_size);                                    // $GNGGA,
      
      read_buffer_until(msg, ',', msg_size);                                    // UTC,
      
      // Latitude
      /*
      // Parse the Latitude
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 13);
      if(NMEA_field_length >= 9)
      {
        uint16_t tmp_index = 0;
        if( GF_find_cell_in_array(NMEA_field, NMEA_field_length, '.', &tmp_index) == SET )
        {
          if(tmp_index == 4)
          {
            NMEA_field[NMEA_field_length] = ',';
            
            degree_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            minutes_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            minutes_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            NMEA_field_rd_index++;                      // for '.'
            
            count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &seconds_tmp, 8);       // second
            seconds_division = 1;
            for(uint8_t k = 0; k < count_i; k++)
              seconds_division *= 10;
            
            GGA_Message.latitude = (degree_tmp*PRECISION_POLYNOMIAL) + (minutes_tmp*PRECISION_POLYNOMIAL/60) + (seconds_tmp*PRECISION_POLYNOMIAL*60 / (3600*seconds_division));
          }
        }
      }
      else
      {
        GGA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      */
      read_buffer_until(msg, ',', msg_size);                                    // Latitude,
      
      /*
      // N/S Indicator
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        if( NMEA_field[NMEA_field_rd_index] == 'N')
          GGA_Message.S_N = 1;
        else if(NMEA_field[NMEA_field_rd_index] == 'S')
          GGA_Message.S_N = 0;
      }
      else
      {
        GGA_Message.S_N = 0;
        GGA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      */
      read_buffer_until(msg, ',', msg_size);                                    // N/S Indicator,
      
      /*
      // Parse the Longitude
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 14);
      if(NMEA_field_length >= 10)
      {
        uint16_t tmp_index = 0;
        if( GF_find_cell_in_array(NMEA_field, NMEA_field_length, '.', &tmp_index) == SET )
        {
          if(tmp_index == 5)
          {
            NMEA_field[NMEA_field_length] = ',';
            
            degree_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 100;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            minutes_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            minutes_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            NMEA_field_rd_index++;                      // for '.'
            
            count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &seconds_tmp, 8);       // second
            seconds_division = 1;
            for(uint8_t k = 0; k < count_i; k++)
              seconds_division *= 10;
            GGA_Message.longitude = (degree_tmp*PRECISION_POLYNOMIAL) + (minutes_tmp*PRECISION_POLYNOMIAL/60) + (seconds_tmp*PRECISION_POLYNOMIAL*60 / (3600*seconds_division));
          }
          else if(tmp_index == 4)
          {
            NMEA_field[NMEA_field_length] = ',';
            
            degree_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            minutes_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            minutes_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            NMEA_field_rd_index++;                      // for '.'
            
            count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &seconds_tmp, 8);       // second
            seconds_division = 1;
            for(uint8_t k = 0; k < count_i; k++)
              seconds_division *= 10;
            GGA_Message.longitude = (degree_tmp*PRECISION_POLYNOMIAL) + (minutes_tmp*PRECISION_POLYNOMIAL/60) + (seconds_tmp*PRECISION_POLYNOMIAL*60 / (3600*seconds_division));
          }
        }
      }
      else
      {
         GGA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      */
      read_buffer_until(msg, ',', msg_size);                                    // Longitude,
      
      /*
      // E/W Indicator
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        if( NMEA_field[NMEA_field_rd_index] == 'W')
          GGA_Message.E_W = 1;
        else if(NMEA_field[NMEA_field_rd_index] == 'E')
          GGA_Message.E_W = 0;
      }
      else
      {
        GGA_Message.E_W = 0;
        GGA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      */
      read_buffer_until(msg, ',', msg_size);                                    // E/W Indicator,
      
      /*
      // Position Fix Indicator
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        GGA_Message.fix_indicator = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index]);
      }
      else
      {
        GGA_Message.fix_indicator = 0;
        GGA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      */
      read_buffer_until(msg, ',', msg_size);                                    // Position Fix Indicator,
      
      
      // Satellites Used
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
      if(NMEA_field_length > 0)
      {
        NMEA_field[NMEA_field_length] = ',';
        
        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
        GGA_Message.satellites = tmp_data;
      }
      else
      {
        GGA_Message.satellites = 0;
        GGA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      /*
      // HDOP
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 7);
      if(NMEA_field_length >= 3)
      {
        count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, '.', &tmp_data, 3);
        NMEA_field_rd_index += count_i;
        NMEA_field_rd_index++;          // '.'
        
        GGA_Message.HDOP = (uint16_t)tmp_data*10;
        GGA_Message.HDOP += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index]);
      }
      else
      {
        GGA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      */
      read_buffer_until(msg, ',', msg_size);                                    // HDOP,
      
      // Parse the Altitude
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 11);
      if(NMEA_field_length >= 2)
      {
        uint16_t tmp_index = 0;
        if( GF_find_cell_in_array(NMEA_field, NMEA_field_length, '.', &tmp_index) == SET )
        {
          int16_t minus = 1;
          if(NMEA_field[NMEA_field_rd_index] == '-')
          {
            minus = -1;
            NMEA_field_rd_index++;
            tmp_index--;
          }
          get_n_number_from_till(NMEA_field, NMEA_field_rd_index, '.', &tmp_data, tmp_index+1);
          
          GGA_Message.altitude = (uint16_t)tmp_data;
          GGA_Message.altitude *= minus;
        }
      }
      else
      {
        GGA_Message.process_error = SET;
      }
      
      
      if(GGA_Message.process_error == RESET)
      {
        //GPS_data.latitude = GGA_Message.latitude;
        //GPS_data.longitude = GGA_Message.longitude;
        //GPS_data.S_N = GGA_Message.S_N;
        //GPS_data.E_W = GGA_Message.E_W;
        GPS_data.altitude = GGA_Message.altitude;
        GPS_data.sats_in_view = GGA_Message.satellites;
      }
      
      break;
    }
  case GPS_NMEA_GSA:
    {
      uint8_t count_i = 0;
      
      uint8_t NMEA_field[7] = {0};
      uint8_t NMEA_field_length = 0;
      uint8_t NMEA_field_rd_index = 0;
      
      GSA_Message.process_error = RESET;
      
      read_buffer_until(msg, ',', msg_size);                                    // $GPGSA,
      
      
      // Mode 1
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        GSA_Message.mode_1 = NMEA_field[NMEA_field_rd_index];
      }
      else
      {
        GSA_Message.mode_1 = 0;
        GSA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      
      // Mode 2 (Fix status)
      memset(NMEA_field, 0, 7);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        GSA_Message.mode_2 = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index]);
      }
      else
      {
        GSA_Message.mode_2 = 0;
        GSA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      
      // Satellites
      for(uint8_t i = 0; i < 12; i++)
        read_buffer_until(msg, ',', msg_size);
      
//      // SV On Channel 1
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_1 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_1 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 2
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_2 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_2 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 3
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_3 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_3 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 4
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_4 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_4 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 5
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_5 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_5 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 6
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_6 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_6 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 7
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_7 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_7 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 8
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_8 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_8 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 9
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_9 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_9 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 10
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_10 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_10 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 11
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_11 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_11 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // SV On Channel 12
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 4);
//      if(NMEA_field_length > 0)
//      {
//        NMEA_field[NMEA_field_length] = ',';
//        
//        get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, (NMEA_field_length+1));
//        GSA_Message.SV_channels.chan_12 = tmp_data;
//      }
//      else
//      {
//        GSA_Message.SV_channels.chan_12 = 0;
//      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
      
      
      // Parse the PDOP
      memset(NMEA_field, 0, 7);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 7);
      if(NMEA_field_length >= 3)
      {
        count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, '.', &tmp_data, 3);
        NMEA_field_rd_index += count_i;
        NMEA_field_rd_index++;          // '.'
        
        GSA_Message.PDOP = (uint16_t)tmp_data*10;
        GSA_Message.PDOP += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index]);
      }
      else
      {
        GSA_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      // Parse the HDOP
      memset(NMEA_field, 0, 7);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 7);
      if(NMEA_field_length >= 3)
      {
        count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, '.', &tmp_data, 3);
        NMEA_field_rd_index += count_i;
        NMEA_field_rd_index++;          // '.'
        
        GSA_Message.HDOP = (uint16_t)tmp_data*10;
        GSA_Message.HDOP += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index]);
      }
      else
      {
        GSA_Message.process_error = SET;
      }
//      read_buff_index += NMEA_field_length;
//      read_buff_index++;                                // ','
//      
//      // Parse the VDOP
//      memset(NMEA_field, 0, 7);
//      NMEA_field_length = 0;
//      NMEA_field_rd_index = 0;
//      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 7);
//      if(NMEA_field_length >= 3)
//      {
//        count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, '.', &tmp_data, 3);
//        NMEA_field_rd_index += count_i;
//        NMEA_field_rd_index++;          // '.'
//        
//        GSA_Message.VDOP = (uint16_t)tmp_data*10;
//        GSA_Message.VDOP += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index]);
//      }
//      else
//      {
//        GSA_Message.process_error = SET;
//      }
      
      if(GSA_Message.process_error == RESET)
      {
        GPS_data.fix_mode = GSA_Message.mode_2;
        GPS_data.pdop = GSA_Message.PDOP;
        GPS_data.hdop = GSA_Message.HDOP;
      }
      else
      {
        GPS_data.fix_mode = GPS_NOT_FIX;
        GPS_data.pdop = 0;
        GPS_data.hdop = 0;
      }

      break;
    }
  case GPS_NMEA_RMC:
    {
      uint8_t degree_tmp = 0;
      uint8_t minutes_tmp = 0;
      uint8_t count_i = 0;
      uint32_t seconds_division = 1;
      uint64_t seconds_tmp = 0;
      
      uint8_t NMEA_field[15] = {0};
      uint8_t NMEA_field_length = 0;
      uint8_t NMEA_field_rd_index = 0;
      
      RMC_Message.process_error = RESET;
      
      read_buffer_until(msg, ',', msg_size);                                    // $GNRMC,
      
      
      // Parse the UTC
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 12);
      if(NMEA_field_length >= 7)
      {
        // Hour
        RMC_Message.time_date.hour = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
        RMC_Message.time_date.hour += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
        
        // Minute
        RMC_Message.time_date.minute = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
        RMC_Message.time_date.minute += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
        
        // Second
        RMC_Message.time_date.second = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
        RMC_Message.time_date.second += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
      }
      else
      {
        RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      

      // Status
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        RMC_Message.status = NMEA_field[NMEA_field_rd_index];
      }
      else
      {
        RMC_Message.status = 0;
        RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      
      // Latitude
      // Parse the Latitude
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 13);
      if(NMEA_field_length >= 9)
      {
        uint16_t tmp_index = 0;
        if( GF_find_cell_in_array(NMEA_field, NMEA_field_length, '.', &tmp_index) == SET )
        {
          if(tmp_index == 4)
          {
            NMEA_field[NMEA_field_length] = ',';
            
            degree_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            minutes_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            minutes_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            NMEA_field_rd_index++;                      // for '.'
            
            count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &seconds_tmp, 8);       // second
            seconds_division = 1;
            for(uint8_t k = 0; k < count_i; k++)
              seconds_division *= 10;
            
            RMC_Message.latitude = (degree_tmp*PRECISION_POLYNOMIAL) + (minutes_tmp*PRECISION_POLYNOMIAL/60) + (seconds_tmp*PRECISION_POLYNOMIAL*60 / (3600*seconds_division));
          }
        }
      }
      else
      {
        RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      
      // N/S Indicator
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        if( NMEA_field[NMEA_field_rd_index] == 'N')
          RMC_Message.S_N = 1;
        else if(NMEA_field[NMEA_field_rd_index] == 'S')
          RMC_Message.S_N = 0;
      }
      else
      {
        RMC_Message.S_N = 0;
        RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      
      // Parse the Longitude
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 14);
      if(NMEA_field_length >= 10)
      {
        uint16_t tmp_index = 0;
        if( GF_find_cell_in_array(NMEA_field, NMEA_field_length, '.', &tmp_index) == SET )
        {
          if(tmp_index == 5)
          {
            NMEA_field[NMEA_field_length] = ',';
            
            degree_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 100;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            minutes_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            minutes_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            NMEA_field_rd_index++;                      // for '.'
            
            count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &seconds_tmp, 8);       // second
            seconds_division = 1;
            for(uint8_t k = 0; k < count_i; k++)
              seconds_division *= 10;
            RMC_Message.longitude = (degree_tmp*PRECISION_POLYNOMIAL) + (minutes_tmp*PRECISION_POLYNOMIAL/60) + (seconds_tmp*PRECISION_POLYNOMIAL*60 / (3600*seconds_division));
          }
          else if(tmp_index == 4)
          {
            NMEA_field[NMEA_field_length] = ',';
            
            degree_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            degree_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            minutes_tmp = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
            minutes_tmp += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
            
            NMEA_field_rd_index++;                      // for '.'
            
            count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &seconds_tmp, 8);       // second
            seconds_division = 1;
            for(uint8_t k = 0; k < count_i; k++)
              seconds_division *= 10;
            RMC_Message.longitude = (degree_tmp*PRECISION_POLYNOMIAL) + (minutes_tmp*PRECISION_POLYNOMIAL/60) + (seconds_tmp*PRECISION_POLYNOMIAL*60 / (3600*seconds_division));
          }
        }
      }
      else
      {
         RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      
      // E/W indicator
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 3);
      if(NMEA_field_length == 1)
      {
        if( NMEA_field[NMEA_field_rd_index] == 'W')
          RMC_Message.E_W = 1;
        else if(NMEA_field[NMEA_field_rd_index] == 'E')
          RMC_Message.E_W = 0;
      }
      else
      {
        RMC_Message.E_W = 0;
        RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;                                // ','
      
      
      // Parse the Speed
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 8);
      if(NMEA_field_length >= 3)
      {
        float tmp_flt = 0;
        uint32_t precision_division = 1;
        
        NMEA_field[NMEA_field_length] = ',';
        
        count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, '.', &tmp_data, 5);
        NMEA_field_rd_index += count_i;
        NMEA_field_rd_index++;                          // for '.'
        
        tmp_flt = (float)tmp_data*1.85;
        
        count_i = get_n_number_from_till(NMEA_field, NMEA_field_rd_index, ',', &tmp_data, 4);
        for(uint8_t k = 0; k < count_i; k++)
          precision_division *= 10;
        
        tmp_flt += (float)tmp_data*1.85/precision_division;
        
        RMC_Message.speed = (uint16_t)tmp_flt;
      }
      else
      {
        RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;
      
      
      // Parse the COG
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 8);
      if(NMEA_field_length >= 3)
      {
        count_i = get_n_number_from_till(NMEA_field, NMEA_field_length, '.', &tmp_data, 5);
        RMC_Message.cog = (uint16_t)tmp_data;
      }
      else
      {
        RMC_Message.process_error = SET;
      }
      read_buff_index += NMEA_field_length;
      read_buff_index++;
      
      
      // Parse the Date
      memset(NMEA_field, 0, 15);
      NMEA_field_length = 0;
      NMEA_field_rd_index = 0;
      NMEA_field_length = GF_extract_SubArray_from_array(msg, NMEA_field, read_buff_index, ',', 8);
      if(NMEA_field_length == 6)
      {
        // Day
        RMC_Message.time_date.day = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
        RMC_Message.time_date.day += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
        
        // Minute
        RMC_Message.time_date.month = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
        RMC_Message.time_date.month += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
        
        // Second
        RMC_Message.time_date.year = GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]) * 10;
        RMC_Message.time_date.year += GF_convert_string_hex_number_to_integer(NMEA_field[NMEA_field_rd_index++]);
      }
      else
      {
        RMC_Message.process_error = SET;
      }
      
      if(RMC_Message.process_error == RESET)
      {
        memcpy((uint8_t*)&GPS_data.GPS_UTC, (uint8_t*)&RMC_Message.time_date, sizeof(RMC_Message.time_date) - 4);               // 4: Timeunix
        GPS_data.latitude = RMC_Message.latitude;
        GPS_data.longitude = RMC_Message.longitude;
        GPS_data.bearing = RMC_Message.cog;
        GPS_data.speed_Kmh = RMC_Message.speed;
        GPS_data.S_N = RMC_Message.S_N;
        GPS_data.E_W = RMC_Message.E_W;
        
        if(check_updating_timeunix == SET)
        {
          check_updating_timeunix = RESET;
          
          // Update Unixtime
          GPS_Update_UnixTimeStamp(&GPS_data.GPS_UTC);
          system_Unixtime = GPS_data.GPS_UTC.unix;
          last_PPS_time_unix = system_Unixtime;
        }
      }
      break;
    }
  case GPS_JAMMING_DETECTION:
    {
      read_buffer_until(msg, ',', msg_size);                                    // $PMTKSPF,
      
      if(msg[read_buff_index] == '1')
      {
        GPS_data.jamming = 1;
      }
      else if(msg[read_buff_index] == '2')
      {
        GPS_data.jamming = 2;
      }
      else if(msg[read_buff_index] == '3')
      {
        GPS_data.jamming = 3;
      }
      break;
    }
  }
}

// https://www.movable-type.co.uk/scripts/latlong.html
uint16_t GPS_calculate_Bearing(uint32_t lat1, uint32_t long1, uint32_t lat2, uint32_t long2)
{
  float real_lat1, real_long1, real_lat2, real_long2;
  real_lat1 = (float)lat1/PRECISION_POLYNOMIAL;
  real_long1 = (float)long1/PRECISION_POLYNOMIAL;
  real_lat2 = (float)lat2/PRECISION_POLYNOMIAL;
  real_long2 = (float)long2/PRECISION_POLYNOMIAL;
  
  float rad_lat1, rad_lat2, rad_d_long;
  rad_lat1 = GPS_DEGREES2RADIANS(real_lat1);
  rad_lat2 = GPS_DEGREES2RADIANS(real_lat2);
  rad_d_long = GPS_DEGREES2RADIANS(real_long2 - real_long1);
  
  float X,Y;
  X = cos(rad_lat2) * sin(rad_d_long);
  Y = (cos(rad_lat1)*sin(rad_lat2)) - (sin(rad_lat1)*cos(rad_lat2)*cos(rad_d_long));
  
  float rad_bearing = atan2(X, Y);
  
  rad_bearing = rad_bearing * 180.0 / 3.1415;
  
  uint16_t heading = (uint16_t)rad_bearing;
  heading = (heading + 360) % 360;
  
  return heading;
}

uint32_t GPS_get_distance(uint32_t lat1, uint32_t long1, uint32_t lat2, uint32_t long2)
{
  float real_lat1, real_long1, real_lat2, real_long2;
  real_lat1 = (float)lat1/PRECISION_POLYNOMIAL;
  real_long1 = (float)long1/PRECISION_POLYNOMIAL;
  real_lat2 = (float)lat2/PRECISION_POLYNOMIAL;
  real_long2 = (float)long2/PRECISION_POLYNOMIAL;
  
  float rad_lat1, rad_lat2, rad_d_lat, rad_d_long, distance;
  rad_lat1 = GPS_DEGREES2RADIANS(real_lat1);
  rad_lat2 = GPS_DEGREES2RADIANS(real_lat2);
  rad_d_lat = GPS_DEGREES2RADIANS(real_lat2 - real_lat1);
  rad_d_long = GPS_DEGREES2RADIANS(real_long2 - real_long1);

  /* Calculate distance between 2 pointes */
  distance = sin(rad_d_lat * (float)0.5) * sin(rad_d_lat * (float)0.5) + cos(rad_lat1) * cos(rad_lat2) * sin(rad_d_long * (float)0.5) * sin(rad_d_long * (float)0.5);
  
  /* Get distance in meters */
  distance = GPS_EARTH_RADIUS * 2 * atan2(sqrt(distance), sqrt(1 - distance)) * 1000;
  
  return (uint32_t)distance;
}


void OnePPS_EXTI_Callback(void) //void GPS_1PPS_SW_EXTI_Callback(void)
{
  uint8_t nmea[GPS_NMEA_BUFFERSIZE];
  uint8_t nmea_data_size = 0;
  
  while(GPS_rx_counter > 0)									
  {
    memset(nmea, 0, GPS_NMEA_BUFFERSIZE);
    nmea_data_size = 0;
    if( GPS_getline(nmea, &nmea_data_size) == GPS_OK )
    {
      if( GPS_validate_NMEA_messages(nmea, nmea_data_size) == GPS_OK )
      {
        GPS_NMEA_Parser(nmea, nmea_data_size);
      }
    }
  }
  GPS_clear_rx_buffer();
  
  // For non jamming detection modules
  GPS_data.jamming = 1;
  
  if(GPS_data.jamming == 1)
  {
    if(GPS_data.GPS_UTC.unix > 1599880000 && GPS_data.GPS_UTC.unix < 2208988800)  //2208988800: 01/01/2040-12:00 AM
    {
      if(GPS_data.fix_mode == GPS_FIX_2D || GPS_data.fix_mode == GPS_FIX_3D)
      {
        if(gps_is_fixed_for_first_time == SET)
        {
          if(GPS_data.latitude != 0 && GPS_data.longitude != 0)
          {
            uint32_t tmp_distance = 0;
            
            uint32_t tmp_last_valid_latitude = last_valid_latitude;
            uint32_t tmp_last_valid_longitude = last_valid_longitude;
            
            if(GPS_data.speed_Kmh >= MOVING_THRESHOLD_SPEED)
            {
              if(device_has_speed < 2)
                device_has_speed++;
            }
            else
            {
              device_has_speed = 0;
              
              if(vehicle_idle_state == RESET && check_vehicle_idle == RESET)
              {
                check_vehicle_idle = SET;
              }
            }
            
            // Location Jump
            if(check_location_jump == SET)
            {
              check_location_jump = RESET;
              
              tmp_distance = GPS_get_distance(last_record_latitude, last_record_longitude, GPS_data.latitude, GPS_data.longitude);
              if(tmp_distance >= VEHICLE_LOCATION_JUMP_THRESHOLD)
              {
                last_valid_latitude = GPS_data.latitude;
                last_valid_longitude = GPS_data.longitude;
                last_valid_altitude = GPS_data.altitude;
                
                GPS_traveled_distance += tmp_distance;
                
                Force_Produce_Event();
                Event_Flags.gnss_loc_jump = ENABLE;
              }
            }
            
            moving_status = KX023.vehicle_moving;
            
            if(device_has_speed >= 2 || moving_status == SET)
            {
              last_valid_latitude = GPS_data.latitude;
              last_valid_longitude = GPS_data.longitude;
              last_valid_altitude = GPS_data.altitude;
              
              // Vehicle Idle
              if(check_vehicle_idle == SET)
              {
                check_vehicle_idle = RESET;
                idle_started_counter = 0;
              }
              if(vehicle_idle_state == SET)
              {
                vehicle_idle_state = RESET;
                
                if(IO_Digital.ignition == ON)
                {
                  Force_Produce_Event();
                  Event_Flags.idle_end = ENABLE;
                }
              }
              
              // Maximum record Speed
              if(GPS_data.speed_Kmh > GPS_max_speed)
                GPS_max_speed = GPS_data.speed_Kmh;
              
              // Overspeeding
              if(GPS_data.speed_Kmh >= setting.illegal_speed)
              {
                if(over_speed_happened == RESET)
                {
                  over_speed_happened = SET;
                  Force_Produce_Event();
                  Event_Flags.unallowed_speed = ENABLE;
                }
              }
              else
              {
                if(over_speed_happened == SET)
                  over_speed_happened = RESET;
              }
            }
            
            /* -------------- Bearing and Distance -------------- */
            if(device_has_speed >= 2)
            {
              // ---------- Bearing ----------
              if(tmp_last_valid_latitude != GPS_data.latitude && tmp_last_valid_longitude != GPS_data.longitude)
              {
                uint16_t tmp_angle_1, tmp_angle_2;
                
                tmp_angle_1 = last_record_COG;
                tmp_angle_2 = GPS_calculate_Bearing(tmp_last_valid_latitude, tmp_last_valid_longitude, GPS_data.latitude, GPS_data.longitude);
                
                last_valid_COG = tmp_angle_2;
                
                if(tmp_angle_2 >= (360-setting.angle_threshold) && tmp_angle_1 <= setting.angle_threshold)
                  tmp_angle_1 += 360;
                else if(tmp_angle_2 <= setting.angle_threshold && tmp_angle_1 >= (360-setting.angle_threshold))
                  tmp_angle_2 += 360;
                if(abs(tmp_angle_2 - tmp_angle_1) >= setting.angle_threshold)
                {
                  if(IO_Digital.ignition == ON)
                  {
                    Force_Produce_Event();
                    Event_Flags.change_direction = ENABLE;
                  }
                }
              }
              
              // Total traveled Distance between two records
              tmp_distance = GPS_get_distance(tmp_last_valid_latitude, tmp_last_valid_longitude, last_valid_latitude, last_valid_longitude);
              GPS_traveled_distance += tmp_distance;
            }
            
            // ---------- Distance ----------
            tmp_distance = 0;
            tmp_distance = GPS_get_distance(last_record_latitude, last_record_longitude, GPS_data.latitude, GPS_data.longitude);
            if(tmp_distance >= setting.data_sample_rate_meter && tmp_distance < VEHICLE_LOCATION_JUMP_THRESHOLD)
            {
              create_normal_event = SET;
            }
            if(tmp_distance >= VEHICLE_LOCATION_JUMP_THRESHOLD)
            {
				GPS_traveled_distance += tmp_distance;
              Force_Produce_Event();
              Event_Flags.gnss_loc_jump = ENABLE;
            }
          }
          else
          {
            device_has_speed = 0;
          }
        }         // End of if(gps_is_fixed_for_first_time == SET)
        else
        {
          // Vehicle is moving
          if(GPS_data.speed_Kmh >= MOVING_THRESHOLD_SPEED)
          {
            if(device_has_speed < 2)
              device_has_speed++;
            
            if(GPS_data.latitude != 0 && GPS_data.longitude != 0 && GPS_data.pdop < GOOD_PDOP_THRESHOLD)
            {
              last_valid_latitude = GPS_data.latitude;
              last_valid_longitude = GPS_data.longitude;
              last_valid_altitude = GPS_data.altitude;
              
              system_Unixtime = GPS_data.GPS_UTC.unix;
              last_PPS_time_unix = system_Unixtime;
              gps_is_fixed_for_first_time = SET;
            }
          }
          // Vehicle is stationary
          else
          {
            device_has_speed = 0;
            
            if(GPS_data.latitude != 0 && GPS_data.longitude != 0 && GPS_data.pdop < GOOD_PDOP_THRESHOLD)
            {
              startup_latitude_average += GPS_data.latitude;
              startup_longitude_average += GPS_data.longitude;
              startup_fixed_counter++;
            }
            if(startup_fixed_counter == 10)
            {
              startup_latitude_average /= 10;
              startup_longitude_average /= 10;
              startup_fixed_counter = 0;
              
              last_valid_latitude = startup_latitude_average;
              last_valid_longitude = startup_longitude_average;
              last_valid_altitude = GPS_data.altitude;
              
              system_Unixtime = GPS_data.GPS_UTC.unix;
              last_PPS_time_unix = system_Unixtime;
              gps_is_fixed_for_first_time = SET;
            }
          }
        }
        
        return;
      }           // End of if(GPS_data.fix_mode == GPS_FIX_2D || GPS_data.fix_mode == GPS_FIX_3D)
    }             // End of if(GPS_data.GPS_UTC.unix > 1599880000 && GPS_data.GPS_UTC.unix < 1893456000)
  }               // Jamming Detection
  else
  {
    //    GPS_data.fix_mode = GPS_NOT_FIX;
    device_has_speed = 0;
    
    Force_Produce_Event();
    Event_Flags.gps_jamming = ENABLE;
  }
  
  GPS_data.fix_mode = GPS_NOT_FIX;
}