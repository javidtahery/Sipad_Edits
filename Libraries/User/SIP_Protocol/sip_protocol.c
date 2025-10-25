#include "sip_protocol.h"
#include "time.h"
#include "hash/md5.h"

/* ------------------ Variables ------------------ */

// --------------- Structures ---------------
setting_typeDef                         setting = {0};

Event_production_flags_typedef          Event_Flags = {0};
record_typedef                          server_records[MAX_NUM_OF_SERVER_RECORD_PACKET] = {0};
server_request_typedef                  server_requests;
server_errors_typedef                   server_errors = {0};

// --------------- General variables of the protocol ---------------
int create_normal_event_counter = 0;
//int create_Heartbeat_counter = 0;
int data_send_rate_counter = 0;
uint8_t send_data_less_than_10_records = RESET;
uint8_t halt_event_production_counter = HALT_EVENT_PRODUCTION_TIMER_DEFAULT;
uint8_t create_normal_event = RESET;
uint8_t force_create_event = RESET;
//uint8_t create_Heartbeat = RESET;
uint32_t vehicle_total_movement = 0;
uint16_t total_generated_record = 0;

uint8_t activate_record_production = ENABLE;
uint8_t change_record_production = RESET;
uint8_t enable_disable_record_production = RESET;
uint8_t enable_send_data_to_sipaad = RESET;

uint8_t reconfig_accelerometer = RESET;

uint8_t save_changed_setting = RESET;
uint8_t Disable_Relay_process = RESET;
uint8_t GPS_max_speed = 0;

uint8_t Odo_max_speed = 0;
float Current_ODO_Speed_KMh = 0;
float Odo_travelled_distance = 0;

uint8_t over_speed_happened = RESET;
uint32_t GPS_traveled_distance = 0;
uint32_t Sensor_traveled_distance = 0;
uint8_t low_battery_detection = RESET;
uint8_t gps_notfix_event_counter = 0;

// Redundancies
uint16_t repetition_odometer_gnss_speed_difference_counter = 0;
uint16_t repetition_low_battery_counter = 0;
uint16_t repetition_accident_detection_counter = 0;

uint32_t last_record_unixtime = 0;
uint64_t system_IMEI = 0;
uint64_t system_Serial = 0;
uint8_t save_system_IMEI = RESET;
uint8_t read_system_IMEI = RESET;

// --------------- GSM-GPS ---------------
gsm_status GSM_status = GSM_INITIATING;

uint8_t keep_GSM_module_awake = SET;
uint8_t gsm_power_status = RESET;
uint8_t turn_off_gsm = RESET;
uint8_t turn_on_gsm = RESET;
uint8_t GSM_cmd_execution_code = 0;
uint8_t send_command_to_GSM = RESET;
uint8_t GSM_check_simcard_counter = 0;
uint8_t sms_panel_send = RESET;

// --------------- Server variables of the protocol ---------------
uint8_t server_packet_count = 0;
uint8_t data_packet[MAX_SIZE_OF_DATA_PACKET];
uint16_t data_packet_length = 0;
uint8_t logined_to_the_server = RESET;
uint8_t server_socket = RESET;
uint8_t server_packet_tag = 0;
uint8_t sending_data_is_in_progress = 0;
uint8_t send_heart_beat = 0;
uint8_t server_packet_type = 0;
uint8_t packet_tag = 0;
uint32_t GSM_local_IP = 0;
uint16_t idle_started_counter = 0;
uint8_t check_vehicle_idle = RESET;
uint8_t vehicle_idle_state = RESET;
uint32_t last_sipaad_logined_unixtime = 0;
uint8_t sipaad_server_is_ok = RESET;
uint8_t check_server_ok_after_gps_fix = RESET;

uint8_t server_security_code[16] = {0};
uint8_t send_request_to_server = RESET;
uint8_t server_ongoing_request_code = 0;
uint8_t check_server_certificates_and_fwu = RESET;

uint8_t server_is_unreachable = RESET;
uint8_t server_fault_count = 0;
uint16_t reset_server_unreachable_counter = 0;

uint8_t PT_test_server_sipaad_login = RESET;
uint8_t PT_test_login_packet_is_created = RESET;
// --------------- Firmware Upgrade ---------------
uint8_t new_firmware_available = RESET;
uint8_t file_header_is_read = RESET;
uint8_t firmware_downloading_process = RESET;
uint32_t fota_file_version = 0;
uint32_t firmware_base_address = 0;
uint16_t firmware_chunk_size = 0;
uint32_t firmware_downloaded_size = 0;
uint16_t firmware_chunk_index = 0;

// --------------- Bluetooth ---------------
uint8_t bluetooth_device_count = 0;
uint8_t bluetooth_mac_addr[MAX_BT_DEVICE_COUNT][BT_MAC_ADDR_SIZE] = {0};

// --------------- Debug Mode ---------------
uint8_t debug_mode = RESET;
uint16_t debug_mode_counter = 0;

uint16_t mainpower_disconnection_counter = 0;
uint16_t mainpower_connection_counter = 0;
uint16_t gps_jump_count = 0;

void SIP_Create_Server_Packet(uint8_t packet_type, uint8_t request_code);
error_t PT_generate_firmware_MD5(uint8_t *digest, uint8_t file_type, uint32_t file_size);

extern void SSL_Close_Socket(void);
extern void SSL_CIPSHUT(void);


/* ------------------ Functions ------------------ */

void System_Startup_Initiate(void)
{
  system_is_initiating = RESET;
  halt_event_production_counter = HALT_STARTUP_EVENT_PRODUCTION;
  systick_check_IO_counter = SYSTICK_CHECK_IO_INTERVAL;
  check_server_certificates_and_fwu = SET;
  PT_test_server_sipaad_login = SET;
  
  /* -------- Initiate GSM Routine -------- */
  GSM_Parameters.stage = GSM_STAGE_DIS_POWER;
  GSM_Parameters.stage_action = SIM_SEND_REQ;
}


void AVL_Initiate_Setting(void)
{
  /* --------------- Setting --------------- */
  setting.setting_metadata = SETTING_META_DATA;
  setting.data_send_rate_time = DEFAULT_SEND_DATA_TIME_INTERVAL_STARTUP;
  setting.data_sample_rate_meter = DEFAULT_NORMAL_POINT_DISTANCE_STARTUP;
  setting.data_sample_rate_time = DEFAULT_NORMAL_POINT_TIME_INTERVAL_STARTUP;
  setting.heartbeat_rate_time = DEFAULT_HEARTBEAT_RATE_TIME;
  setting.disable_relay = RESET;
  setting.ignition_volt_lvl = DEFAULT_IGNITION_VOLTAGE_LVL;
  setting.movement_acceleration = DEFAULT_MOVEMENT_ACCELERATION;
  setting.static_acceleration = DEFAULT_STATIC_ACCELERATION;
  setting.angle_threshold = DEFAULT_ANGLE;
  setting.illegal_speed = DEFAULT_ILLEGAL_SPEED;
  setting.odometer_k = DEFAULT_ODOMETER_K_FACTOR;
  setting.hash_nsend_count = 10000;
  setting.server_url_length = 0;
  setting.server_port = 0;
  setting.APN_length = 0;
  setting.custom_APN = RESET;
  setting.api_setting_code = 0;
  setting.force_FOTA = RESET;
  memset(setting.server_url, 0, URL_MAX_LENGTH_SIZE);
  memset(setting.http_server, 0, URL_MAX_LENGTH_SIZE);
  memset(setting.APN, 0, APN_MAX_LENGTH_SIZE);
  memset(setting.sms_panel, 0, SMS_PANEL_MAX_LENGTH_SIZE);
  
  // Server
  memcpy(setting.server_url, "#gps.sipaad.ir", 14);
  setting.server_url_length = 13;
  setting.server_port = 11000;
  
  // HTTP
  memcpy(setting.http_server, "#sipdata.src-co.ir", 18);
  setting.http_server_length = 17;
  setting.disable_http = RESET;
  
  memcpy(setting.sms_panel , "+9810008000700070", 17);
  setting.sms_panel_length = 17;
  
  setting.debug_mode_time = 120;                // Default: 2 Hours
  setting.api_interval_disable = RESET;
  setting.api_interval_time = 360;              // Default: 6 Hours
}


void AVL_check_setting_values(void)
{
  uint8_t save_setting = RESET;
  
  if(setting.heartbeat_rate_time == 0 || setting.heartbeat_rate_time == 0xFF)
  {
    save_setting = SET;
    setting.heartbeat_rate_time = DEFAULT_HEARTBEAT_RATE_TIME;
  }
  
  if(setting.http_server_length == 0 || setting.http_server_length == 0xFF)
  {
    save_setting = SET;
    
    memcpy(setting.http_server, "#sipdata.src-co.ir", 18);
    setting.http_server_length = 17;
  }
  
  if(setting.server_url_length == 0 || setting.server_url_length == 0xFF)
  {
    AVL_Initiate_Setting();
    
    save_setting = SET;
  }
  
  
  if(save_setting == SET)
  {
    SPIF_write_setting();
  }
}


void check_startup_server_ok(void)
{
  uint32_t memory_last_unix_time = 0;
  sipaad_server_is_ok = RESET;
  
  SPIF_Read_Security_Register(SPIF_SECURITY_REGISTE_2_ADDR, (uint8_t*)&memory_last_unix_time, 4);
  
  if(memory_last_unix_time > 0 && memory_last_unix_time != UINT32_MAX)
  {
    if(gps_is_fixed_for_first_time == RESET)
    {
      check_server_ok_after_gps_fix = SET;
      last_sipaad_logined_unixtime = memory_last_unix_time;
    }
    else
    {
      uint32_t temp_32 = system_Unixtime - memory_last_unix_time;
      
      if(temp_32 < (3 * RTC_SECONDS_PER_DAY))
        sipaad_server_is_ok = SET;
    }
  }
}


void save_server_ok(void)
{
  if(sipaad_server_is_ok == SET)
  {
    if(last_sipaad_logined_unixtime > RTC_SECONDS_PER_DAY)
    {
      uint32_t memory_last_unix_time = 0;      
      SPIF_Read_Security_Register(SPIF_SECURITY_REGISTE_2_ADDR, (uint8_t*)&memory_last_unix_time, 4);
      
      if(memory_last_unix_time == UINT32_MAX)
        memory_last_unix_time = 0;
      
      if(last_sipaad_logined_unixtime > memory_last_unix_time)
        SPIF_Program_Security_Register(SPIF_SECURITY_REGISTE_2_ADDR, (uint8_t*)&last_sipaad_logined_unixtime, 4);
    }
  }
}


void Force_Produce_Event(void)
{
  force_create_event = SET;
  
  if(activate_record_production == RESET)
  {
    change_record_production = SET;
    enable_disable_record_production = SET;
  }
}


void Check_disable_record_production(void)
{
  if(IO_Digital.vcc_digital == ON && IO_Digital.ignition == OFF)
  {
    change_record_production = SET;
    enable_disable_record_production = RESET;
  }
}


void Record_Production(void)
{
  if(activate_record_production == ENABLE)
  {
    if(gps_is_fixed_for_first_time == SET)
    {
      if(force_create_event == ENABLE || create_normal_event == ENABLE)
      {
        record_typedef                  SIP_Packet;
        Set_zero((uint8_t*)&SIP_Packet, SIZEOF_RECORD);
        
        SIP_Packet.gps_elements.unixtime = system_Unixtime;
        
        if(SIP_Packet.gps_elements.unixtime <= last_record_unixtime)
        {
          SIP_Packet.gps_elements.unixtime = ++last_record_unixtime;
        }
        
        last_record_unixtime = SIP_Packet.gps_elements.unixtime;
        
        /* ------------- GPS Datas -------------*/
        SIP_Packet.gps_elements.GPS_latitude = last_valid_latitude;
        SIP_Packet.gps_elements.GPS_longitude = last_valid_longitude;
        SIP_Packet.gps_elements.GPS_altitude = last_valid_altitude;
        SIP_Packet.gps_elements.GPS_bearing = last_valid_COG;
        SIP_Packet.gps_elements.GPS_nsat = GPS_data.sats_in_view;
        SIP_Packet.gps_elements.GPS_pdop = GPS_data.pdop;
        SIP_Packet.gps_elements.GPS_speed = GPS_data.speed_Kmh;
        SIP_Packet.gps_elements.S_N = GPS_data.S_N;
        SIP_Packet.gps_elements.E_W = GPS_data.E_W;
        
        if(GPS_data.fix_mode == GPS_NOT_FIX)
          SIP_Packet.gps_elements.GPS_fixMode = 0x01;
        else if(GPS_data.fix_mode == GPS_FIX_2D)
          SIP_Packet.gps_elements.GPS_fixMode = 0x02;
        else if(GPS_data.fix_mode == GPS_FIX_3D)
          SIP_Packet.gps_elements.GPS_fixMode = 0x00;
        
        if(GPS_data.fix_mode == GPS_NOT_FIX)
        {
          SIP_Packet.gps_elements.GPS_bearing = 0;
          SIP_Packet.gps_elements.GPS_nsat = 0;
          SIP_Packet.gps_elements.GPS_pdop = 0;
          SIP_Packet.gps_elements.GPS_speed = 0;
        }
        
        // IO Elements
        SIP_Packet.io_status = (IO_Digital.ignition * (1<<DIGITAL_IO_ACC))
          + (IO_Digital.vcc_digital * (1<<DIGITAL_IO_MAIN_PWR))
            + (IO_Digital.ignition * (1<<DIGITAL_IO_D_IN_1))
              + (IO_Digital.vcc_digital * (1<<DIGITAL_IO_D_IN_2))
                + (IO_Digital.digital_out * (1<<DIGITAL_IO_D_OUT_1));
        // GSM Antenna Level
        if(GSM_Parameters.signal_quality <= 6)
          SIP_Packet.GSM_lvl = 0x00;
        else if(GSM_Parameters.signal_quality > 6 || GSM_Parameters.signal_quality <= 12)
          SIP_Packet.GSM_lvl = 0x01;
        else if(GSM_Parameters.signal_quality > 12 || GSM_Parameters.signal_quality <= 19)
          SIP_Packet.GSM_lvl = 0x02;
        else if(GSM_Parameters.signal_quality > 19 || GSM_Parameters.signal_quality <= 25)
          SIP_Packet.GSM_lvl = 0x03;
        else if(GSM_Parameters.signal_quality > 25 || GSM_Parameters.signal_quality <= 32)
          SIP_Packet.GSM_lvl = 0x04;
        
        // Extra enries
        SIP_Packet.GPS_max_speed = GPS_max_speed;
        SIP_Packet.GPS_travel_dist = GPS_traveled_distance;
        SIP_Packet.sensor_speed = (uint8_t)Current_ODO_Speed_KMh;
        SIP_Packet.sensor_max_speed = Odo_max_speed;
        SIP_Packet.sensor_travel_dist = (uint32_t)Odo_travelled_distance;
        SIP_Packet.supply_voltage = ((uint16_t)ADC_Values.Average_Main_Power_Voltage) / 100;
        SIP_Packet.analoge_in_1 = ((uint16_t)ADC_Values.Average_Internal_BAT) / 100;
        SIP_Packet.analoge_in_2 = 0;
        SIP_Packet.g_sensor_value = 0;
        
        vehicle_total_movement += GPS_traveled_distance;
        total_generated_record++;
        
        // save related record odometer parameter
        last_record_latitude = last_valid_latitude;
        last_record_longitude = last_valid_longitude;
        last_record_COG = last_valid_COG;
        
        GPS_max_speed = 0;
        Odo_max_speed = 0;
        GPS_traveled_distance = 0;
        Sensor_traveled_distance = 0;
        Odo_travelled_distance = 0;
        
        
        /* ------------------------------------------------- */
        /* ----------------- Normal Point ------------------ */
        /* ------------------------------------------------- */
        if(create_normal_event == ENABLE)
        {
          // GPS Jamming
          if(Event_Flags.gps_jamming == ENABLE)
          {
            SIP_Packet.event_code = GPS_JAMMING;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.gps_jamming = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          else
          {
            SIP_Packet.event_code = NORMAL_POINT;
            SIP_Packet.event_data = RESET;
            FCB_write_records(&SIP_Packet);
            create_normal_event = DISABLE;
          }
          
          return;
        }
        
        /* ------------------------------------------------- */
        /* --------------------- Event --------------------- */
        /* ------------------------------------------------- */
        if(force_create_event == ENABLE)
        {
          /* -------------------------------------------------------- */
          /* ----------------- Main Power Connected ----------------- */
          /* -------------------------------------------------------- */
          if(Event_Flags.main_pwr_connected == ENABLE)
          {
            if(mainpower_connection_counter == 0)
            {
              mainpower_connection_counter = 10*60;             // 10 Minutes
              
              SIP_Packet.event_code = MAIN_PWR_CONNECT;
              SIP_Packet.event_data = RESET;
              
              FCB_write_records(&SIP_Packet);
            }
            
            Event_Flags.main_pwr_connected = DISABLE;
            
            return;
          }
          
          /* ------------------------------------------------------------- */
          /* ----------------- Main Power Disconnected  ------------------ */
          /* ------------------------------------------------------------- */
          if(Event_Flags.main_pwr_disconnected == ENABLE)
          {
            if(mainpower_disconnection_counter == 0)
            {
              mainpower_disconnection_counter = 10*60;          // 10 Minutes
              
              SIP_Packet.event_code = MAIN_PWR_DISCONNECT;
              SIP_Packet.event_data = SET;
              
              FCB_write_records(&SIP_Packet);
            }
            
            Event_Flags.main_pwr_disconnected = DISABLE;
            
            return;
          }
          
          /* ------------------------------------------------- */
          /* ----------------- ACC Off ------------------ */
          /* ------------------------------------------------- */
          if(Event_Flags.ignition_low == ENABLE)
          {
            SIP_Packet.event_code = IGNITION_OFF;
            SIP_Packet.event_data = RESET;
            SIP_Packet.gps_elements.GPS_speed = 0;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.ignition_low = DISABLE;
            
            if(IO_Digital.vcc_digital == ON)
            {
              change_record_production = SET;
              enable_disable_record_production = RESET;
            }
            
            return;
          }
          
          /* ------------------------------------------------- */
          /* ----------------- ACC On ----------------- */
          /* ------------------------------------------------- */
          if(Event_Flags.ignition_high == ENABLE)
          {
            SIP_Packet.event_code = IGNITION_ON;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.ignition_high = DISABLE;
            
            return;
          }
          
          /* ------------------------------------------------------------- */
          /* -------------------- G Sensor Activated  -------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.g_sensor_activated == ENABLE)
          {
            SIP_Packet.event_code = CRASH_HAPPEND;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.g_sensor_activated = DISABLE;
            Last_Time_of_Device_detached_Flag_has_been_activated = 1;        // Enable the counter
            
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* --------------------- Change direction  --------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.change_direction == ENABLE)
          {
            SIP_Packet.event_code = CHANGE_DIRECTION;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.change_direction = DISABLE;
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* ------------------- Low Internal Battery  ------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.low_internal_battery == ENABLE)
          {
            SIP_Packet.event_code = LOW_INTERNAL_BATTERY;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.low_internal_battery = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* ----------------- GNSS Odometer Speed Diff  ----------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.gnss_odometer_speed_dif == ENABLE)
          {
            SIP_Packet.event_code = GNSS_ODOMETER_SPEED_DIF;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.gnss_odometer_speed_dif = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* --------------------- Unallowed Speed  ---------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.unallowed_speed == ENABLE)
          {
            SIP_Packet.event_code = UNALLOWED_DRIVING_SPEED;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.unallowed_speed = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* --------------------- Device Attached  ---------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.device_attached == ENABLE)
          {
            SIP_Packet.event_code = DEVICE_ATTACHED;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.device_attached = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* -------------------- Device Deattached  --------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.device_deattached == ENABLE)
          {
            SIP_Packet.event_code = DEVICE_DEATTACHED;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.device_deattached = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* --------------------- Idle Started  ---------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.idle_begin == ENABLE)
          {
            SIP_Packet.event_code = STOP_STARTED;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.idle_begin = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* -------------------- Idle Ended  --------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.idle_end == ENABLE)
          {
            SIP_Packet.event_code = STOP_ENDED;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.idle_end = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* ----------------------- Location Jump  ---------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.gnss_loc_jump == ENABLE)
          {
            SIP_Packet.event_code = GPS_LOCATION_JUMP;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.gnss_loc_jump = DISABLE;
            Check_disable_record_production();
            
            gps_jump_count++;
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* --------------------- GNSS Signal Lost  --------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.gnss_lost_signal == ENABLE)
          {
            SIP_Packet.event_code = GNSS_SIGNAL_LOST;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.gnss_lost_signal = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          /* ------------------------------------------------------------- */
          /* --------------------- GSM Signal Lost  ---------------------- */
          /* ------------------------------------------------------------- */
          if(Event_Flags.gsm_lost_signal == ENABLE)
          {
            SIP_Packet.event_code = GSM_SIGNAL_LOST;
            SIP_Packet.event_data = SET;
            
            FCB_write_records(&SIP_Packet);
            
            Event_Flags.gsm_lost_signal = DISABLE;
            Check_disable_record_production();
            
            return;
          }
          
          force_create_event = DISABLE;
        }
      }
    }
  }
}


void SIP_Manage_Requests_Records(void)
{
  if(GSM_status == GSM_IDLE)
  {
    if(FCB_profile.s1_nsend_count >= 10 || send_data_less_than_10_records == SET)
    {
      if(send_data_less_than_10_records == SET)
        send_data_less_than_10_records = RESET;

      if(server_packet_count == 0)
        FCB_read_records(&server_packet_count);
    }
    
    if(debug_mode == SET)
    {
      if(halt_http_process == RESET)
      {
        if(server_packet_count > 0)
        {
          HTTP_Params.send_record = SET;
          enable_http_process = SET;
        }
        else
          return;
      }
    }
    else
    {
      if(enable_send_data_to_sipaad == SET)
      {
        if(logined_to_the_server == SET)
        {
          /* Server Request */
          if(send_request_to_server == SET)
          {
            sending_data_is_in_progress = SET;
            GSM_status = GSM_SENDING_PACKET;
            server_packet_type = REQUEST_PACKET;
            
            if(server_requests.login == SET)
            {
              SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_LOGIN);
            }
            else if(server_requests.get_settings == SET)
            {
              SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_GET_SEETINGS);
            }
            else if(server_requests.get_firware_chunk == SET)
            {
              SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_GET_FOTA_CHUNK);
            }
            else if(server_requests.get_geo_list == SET)
            {
              SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_GET_GEO_LIST);
            }
            else if(server_requests.get_geofence == SET)
            {
              SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_GET_GEO);
            }
            else if(server_requests.get_bt_list == SET)
            {
              SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_GET_BT_LIST);
            }
            else if(server_requests.send_bt_status == SET)
            {
              SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_SEND_SMARTCARD);
            }
            else
            {
              send_request_to_server = RESET;
              GSM_status = GSM_IDLE;
              sending_data_is_in_progress = RESET;
              GSM_Parameters.stage_action = SIM_SEND_REQ;
            }
          }
          
          /* Server Record and Heartbeat Packet */
          if(sending_data_is_in_progress == RESET)
          {
//            if(create_Heartbeat == SET)
//            {
//              create_Heartbeat = RESET;
//              
//              // Send heartbeat after first server packet
//              if(server_packet_count > 0)
//              {
//                sending_data_is_in_progress = SET;
//                server_packet_type = HEARTBEAT_PACKET;
//                
//                GSM_status = GSM_SENDING_PACKET;
//                SIP_Create_Server_Packet(SERVER_HEARTBEAT, NO_REQUEST);
//              }
//            }
            if(server_packet_count > 0)
            {
              sending_data_is_in_progress = SET;
              server_packet_type = LOCATION_PACKET;
              
              GSM_status = GSM_SENDING_PACKET;
              
              SIP_Create_Server_Packet(SERVER_EVENT, NO_REQUEST);
            }
            else
              return;
          }
        }
        /* Login Packet */
        else if(FCB_profile.s1_nsend_count > 0)// || create_Heartbeat == SET)
        {
          sending_data_is_in_progress = SET;
          server_packet_type = LOGIN_PACKET;
          
          GSM_status = GSM_LOGIN;
          
          SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_LOGIN);
        }
      }
    }
  }
}

void SIP_Sending_Packet_to_the_server_process(void)
{
  if(server_is_unreachable == RESET)
  {
    // ---------- Test server login ----------
    if(PT_test_server_sipaad_login == SET)
    {
      if(GSM_Parameters.nw_provider != 0 && (GSM_status == GSM_IDLE || GSM_status == GSM_SERVER_TEST))
      {
        if(PT_test_login_packet_is_created == RESET)
        {
          PT_test_login_packet_is_created = SET;
          SIP_Create_Server_Packet(SERVER_REQUEST, REQUEST_LOGIN);
        }
        else
        {
          GSM_status = GSM_SERVER_TEST;
          uint16_t resp = GSM_SSL_routine_pro(GSM_Parameters.stage);
          if(server_is_unreachable == SET)
          {
            GSM_status = GSM_IDLE;
            sending_data_is_in_progress = RESET;
            PT_test_login_packet_is_created = RESET;
          }
          if(resp == GSM_SEND_NEXT_PCKT)
          {
            PT_test_server_sipaad_login = RESET;
            PT_test_login_packet_is_created = RESET;
            sending_data_is_in_progress = RESET;
            
            GSM_status = GSM_IDLE;
            if(debug_mode == RESET)
              enable_send_data_to_sipaad = SET;
          }
          
          Delay(50);
        }
      }
    }
    else
    {
      /* Send data to the server */
      if(sending_data_is_in_progress == SET)
      {
        if( GSM_status == GSM_LOGIN || GSM_status == GSM_SENDING_PACKET )
        {
          uint16_t resp = GSM_SSL_routine_pro(GSM_Parameters.stage);
          if(server_is_unreachable == SET)
          {
            GSM_status = GSM_IDLE;
            sending_data_is_in_progress = RESET;
          }
          if(resp == GSM_SEND_NEXT_PCKT)
          {
            sending_data_is_in_progress = RESET;
            
            if(server_packet_type == LOGIN_PACKET)
            {
              GSM_status = GSM_IDLE;
              return;
            }
            if(server_packet_type == HEARTBEAT_PACKET)
            {
              GSM_status = GSM_IDLE;
            }
            if(server_packet_type == LOCATION_PACKET)
            {
              FCB_modify_sent_records();
              server_packet_count = 0;
              GSM_status = GSM_IDLE;
            }
            
            if(server_packet_type == REQUEST_PACKET)
              GSM_status = GSM_IDLE;
            
            if(activate_record_production == DISABLE && GSM_Parameters.internet_connection == SET)// && create_Heartbeat == RESET)
            {
              if(FCB_profile.s1_nsend_count == 0)
              {
                if(new_firmware_available == SET && IO_Digital.vcc_digital == SET)
                {
                  firmware_downloading_process = SET;
                }
                else
                {
                  send_command_to_GSM = SET;
                  GSM_cmd_execution_code = SIM_STAGE_CLOSE_SOCKET;
                }
              }
            }
          }
          else if(resp == GSM_RESEND_PCKT)
          {
            GSM_status = GSM_IDLE;
            sending_data_is_in_progress = RESET;
          }
          Delay(50);
        }
      }
    }
  }
}

uint16_t SIP_Check_Digit(uint16_t data_length)
{
  uint16_t sumation_result = 0;
  
  for(uint16_t iterator = 0; iterator < data_length; iterator++)
    sumation_result += data_packet[16 + iterator];
  
  return (sumation_result % server_security_code[0]);
}

void SIP_Create_Server_Packet(uint8_t packet_type, uint8_t request_code)
{
  uint16_t data_array_index = 0;
  memset(data_packet, 0, MAX_SIZE_OF_DATA_PACKET);
  
  // Sign
  data_packet[data_array_index++] = packet_type;
  
  // IMEI
  if(system_IMEI == 0)
  {
    GSM_status = GSM_IDLE;
    sending_data_is_in_progress = RESET;
    GSM_Parameters.number_of_retries_command = 0;
    GSM_Parameters.stage = GSM_STAGE_DIS_POWER;
    GSM_Parameters.stage_action = SIM_SEND_REQ;
    GSM_module_is_configured = RESET;
    
    system_error.IMEI_error = SET;
    read_system_IMEI = SET;
    
    reset_server_unreachable_counter = 3600;
    server_is_unreachable = SET;
    return;
  }
  Little_endian_data((uint8_t*)&system_IMEI, &data_packet[data_array_index], 8);
  data_array_index += 8;
  
  // Packet tag
  data_packet[data_array_index++] = packet_tag;
  packet_tag++;
  
  
  switch(packet_type)
  {
  case SERVER_REQUEST:
    {
      uint16_t tmp_var_16 = 0;
      uint32_t tmp_var_32 = 0;
      
      // Request Code
      data_packet[data_array_index++] = request_code;
      
      // Version
      data_packet[data_array_index++] = 0x01;
      
      if(request_code == REQUEST_LOGIN)
      {
        /* Device Info */
        
        // Device Type
        data_packet[data_array_index++] = 0x01;         // OBD
        
        // Device Name
        uint8_t device_name[20] = "SRC-S500";
        memcpy(&data_packet[data_array_index], device_name, 20);
        data_array_index += 20;
        
        // Device Version
        Big_endian_data((uint8_t*)&system_temp_params.device_version, &data_packet[data_array_index], 4);
        data_array_index += 4;
        
        // Remained Internet Charge
        data_array_index += 2;          // 0 Rial
        
        // Remained Battery Charge Percent
        tmp_var_16 = (uint16_t)((ADC_Values.Average_Internal_BAT / 4200) * 100);
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
        
        // Device IP
        tmp_var_32 = GSM_local_IP;
        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
        data_array_index += 4;
      }
      else if(request_code == REQUEST_GET_FOTA_CHUNK)
      {
        // Start Address
        tmp_var_32 = firmware_base_address;
        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
        data_array_index += 4;
        
        // Data Length
        tmp_var_16 = firmware_chunk_size;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
      }
      else if(request_code == REQUEST_GET_GEO)
      {
        // Geofence ID
        tmp_var_16 = 1;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
      }
      else if(request_code == REQUEST_SEND_SMARTCARD)
      {
        // TODO: Add informations
      }
      
      break;
    }
    // 80 bytes
  case SERVER_EVENT:
    {
      uint16_t tmp_var_16 = 0;
      uint32_t tmp_var_32 = 0;
      
      // Event Version
      data_packet[data_array_index++] = 0x01;
      
      // Check Digit
      data_array_index += 2;            // Skip temporary
      
      // Number of Packets
      data_packet[data_array_index++] = server_packet_count;
      
      // Length of Packets
      data_array_index += 2;            // Skip temporary
      
      // Packet List
      for(uint8_t iterator = 0; iterator < server_packet_count; iterator++)
      {
        // Event Code
        data_packet[data_array_index++] = server_records[iterator].event_code;
        
        // Date Time
        tmp_var_32 = server_records[iterator].gps_elements.unixtime;
        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
        data_array_index += 4;
        
        // GPS Speed
        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_speed;
        
        // Sensor Speed
        data_packet[data_array_index++] = server_records[iterator].sensor_speed;
        
        // GPS Max Speed
        data_packet[data_array_index++] = server_records[iterator].GPS_max_speed;
        
        // Sensor Max Speed
        data_packet[data_array_index++] = server_records[iterator].sensor_max_speed;
        
        // GPS Total Traveled Distance
        float tmp_var_fl = 1.2 * server_records[iterator].GPS_travel_dist;
        tmp_var_32 = (uint32_t)tmp_var_fl;
        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
        data_array_index += 4;
        
        // Sensor Total Traveled Distance
        tmp_var_32 = server_records[iterator].sensor_travel_dist;
        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
        data_array_index += 4;
        
        // IO Status
        data_packet[data_array_index++] = server_records[iterator].io_status;
        
        // GPS Status
        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_fixMode;
        
        // GPS Latitude
        tmp_var_32 = server_records[iterator].gps_elements.GPS_latitude;
        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
        data_array_index += 4;
        
        // GPS Longitude
        tmp_var_32 = server_records[iterator].gps_elements.GPS_longitude;
        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
        data_array_index += 4;
        
        // GPS Altitude
        tmp_var_16 = server_records[iterator].gps_elements.GPS_altitude;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
        
        // GPS Beraing
        tmp_var_16 = server_records[iterator].gps_elements.GPS_bearing;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
        
        // GPS NSat
        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_nsat;
        
        // GPS PDOP
        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_pdop;
        
        // Sensor High Resolution Fuel Consumption
        data_array_index += 2;            // Not available
        
        // Sensor Fuel Level
        data_array_index += 2;            // Not available
        
        // G Sensor Value
        tmp_var_16 = server_records[iterator].g_sensor_value;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
        
        // Supply Voltage
        tmp_var_16 = server_records[iterator].supply_voltage;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
        
        // Sensor - Engine RPM
        data_array_index += 2;            // Not available
        
        // Sensor - Engine total hour of operation
        data_array_index += 4;            // Not available
        
        // Sensor - High Resolution Vehicle Distance
        data_array_index += 4;            // Not available
        
        // Sensor - Tachograph Speed
        data_array_index += 2;            // Not available
        
        // Sensor - Tachograph Status
        data_array_index += 4;            // Not available
        
        // Sensor - Engine Temprature
        data_array_index += 1;            // Not available
        
        // Sensor - Air Supply Pressure
        data_array_index += 2;            // Not available
        
        // Sensor - Telltale State
        data_array_index += 8;            // Not available
        
        // Sensor - Cruise Control
        data_array_index += 3;            // Not available
        
        // Sensor - Vehicle Weight
        data_array_index += 4;            // Not available
        
        // Analoge Inpute 1
        tmp_var_16 = server_records[iterator].analoge_in_1;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
        
        // Analoge Inpute 2
        tmp_var_16 = server_records[iterator].analoge_in_2;
        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
        data_array_index += 2;
        
        // Extra Data Length
        data_packet[data_array_index++] = 0;
        
        // Extra Data
      }
      
      // Length of Packets
      tmp_var_16 = data_array_index - 16;
      Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[14], 2);
      
      // Check Digit
      tmp_var_16 = SIP_Check_Digit(tmp_var_16);
      Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[11], 2);
      
      break;
    }
//  case SERVER_HEARTBEAT:
//    {
//      uint16_t tmp_var_16 = 0;
//      uint32_t tmp_var_32 = 0;
//      server_packet_count = 1;
//      
//      // Sign
//      data_packet[0] = SERVER_EVENT;
//      
//      // Event Version
//      data_packet[data_array_index++] = 0x01;
//      
//      // Check Digit
//      data_array_index += 2;            // Skip temporary
//      
//      // Number of Packets
//      data_packet[data_array_index++] = server_packet_count;
//      
//      // Length of Packets
//      data_array_index += 2;            // Skip temporary
//      
//      // Packet List
//      for(uint8_t iterator = 0; iterator < server_packet_count; iterator++)
//      {
//        // Event Code
//        data_packet[data_array_index++] = NORMAL_POINT;
//        
//        // Date Time
//        tmp_var_32 = system_Unixtime;
//        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
//        data_array_index += 4;
//        
//        // GPS Speed
//        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_speed;
//        
//        // Sensor Speed
//        data_packet[data_array_index++] = server_records[iterator].sensor_speed;
//        
//        // GPS Max Speed
//        data_packet[data_array_index++] = server_records[iterator].GPS_max_speed;
//        
//        // Sensor Max Speed
//        data_packet[data_array_index++] = server_records[iterator].sensor_max_speed;
//        
//        // GPS Total Traveled Distance
//        tmp_var_32 = server_records[iterator].GPS_travel_dist;
//        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
//        data_array_index += 4;
//        
//        // Sensor Total Traveled Distance
//        tmp_var_32 = server_records[iterator].sensor_travel_dist;
//        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
//        data_array_index += 4;
//        
//        // IO Status
//        data_packet[data_array_index++] = server_records[iterator].io_status;
//        
//        // GPS Status
//        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_fixMode;
//        
//        // GPS Latitude
//        tmp_var_32 = server_records[iterator].gps_elements.GPS_latitude;
//        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
//        data_array_index += 4;
//        
//        // GPS Longitude
//        tmp_var_32 = server_records[iterator].gps_elements.GPS_longitude;
//        Little_endian_data((uint8_t*)&tmp_var_32, &data_packet[data_array_index], 4);
//        data_array_index += 4;
//        
//        // GPS Altitude
//        tmp_var_16 = server_records[iterator].gps_elements.GPS_altitude;
//        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
//        data_array_index += 2;
//        
//        // GPS Beraing
//        tmp_var_16 = server_records[iterator].gps_elements.GPS_bearing;
//        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
//        data_array_index += 2;
//        
//        // GPS NSat
//        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_nsat;
//        
//        // GPS PDOP
//        data_packet[data_array_index++] = server_records[iterator].gps_elements.GPS_pdop;
//        
//        // Sensor High Resolution Fuel Consumption
//        data_array_index += 2;            // Not available
//        
//        // Sensor Fuel Level
//        data_array_index += 2;            // Not available
//        
//        // G Sensor Value
//        tmp_var_16 = server_records[iterator].g_sensor_value;
//        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
//        data_array_index += 2;
//        
//        // Supply Voltage
//        tmp_var_16 = server_records[iterator].supply_voltage;
//        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
//        data_array_index += 2;
//        
//        // Sensor - Engine RPM
//        data_array_index += 2;            // Not available
//        
//        // Sensor - Engine total hour of operation
//        data_array_index += 4;            // Not available
//        
//        // Sensor - High Resolution Vehicle Distance
//        data_array_index += 4;            // Not available
//        
//        // Sensor - Tachograph Speed
//        data_array_index += 2;            // Not available
//        
//        // Sensor - Tachograph Status
//        data_array_index += 4;            // Not available
//        
//        // Sensor - Engine Temprature
//        data_array_index += 1;            // Not available
//        
//        // Sensor - Air Supply Pressure
//        data_array_index += 2;            // Not available
//        
//        // Sensor - Telltale State
//        data_array_index += 8;            // Not available
//        
//        // Sensor - Cruise Control
//        data_array_index += 3;            // Not available
//        
//        // Sensor - Vehicle Weight
//        data_array_index += 4;            // Not available
//        
//        // Analoge Inpute 1
//        tmp_var_16 = server_records[iterator].analoge_in_1;
//        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
//        data_array_index += 2;
//        
//        // Analoge Inpute 2
//        tmp_var_16 = server_records[iterator].analoge_in_2;
//        Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[data_array_index], 2);
//        data_array_index += 2;
//        
//        // Extra Data Length
//        data_packet[data_array_index++] = 0;
//        
//        // Extra Data
//      }
//      
//      // Length of Packets
//      tmp_var_16 = data_array_index - 16;
//      Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[14], 2);
//      
//      // Check Digit
//      tmp_var_16 = SIP_Check_Digit(tmp_var_16);
//      Little_endian_data((uint8_t*)&tmp_var_16, &data_packet[11], 2);
//      
//      break;
//    }
  }
  
  data_packet_length = data_array_index;
}


/*
Sipaad Firmware Upgrade Header:
[Header Info CRC-16][Pack Version][Num of FWs in pack][FW1 Ver][FW1 Start Index][FW1 Length][FW1 MD5 ][...][FWn Ver][FWn Start Index][FWn Length][FWn MD5 ]
[2 Bytes           ][4 Bytes	 ][1 Byte 	     ][4 Bytes][4 Bytes	       ][4 Bytes   ][16 Bytes][...][4 Bytes][4 Bytes	    ][4 Bytes   ][16 Bytes]
*/

// *** Important
// Pack Version is given in big enddian format
// FWx Version is given in big enddian format
// MD5 is given in big enddian format
uint8_t SIP_parse_server_response(uint8_t* data, uint16_t data_length)
{
  uint16_t array_idx = 0;
  
  if(data[array_idx] == SERVER_REQUEST)
  {
    uint8_t answer_code = data[4];
    
    switch(answer_code)
    {
    case SUCCESS_LOGIN:
      {
        uint8_t decrypted_msg[128] = {0};
        uint8_t decrypted_msg_size = 128;
        uint16_t decrypted_length = 0;
        
        last_sipaad_logined_unixtime = system_Unixtime;
        sipaad_server_is_ok = SET;
//        uint16_t extra_data_length = data[5] + (data[6] << 8);
        
        // Decrypting process
        RsaPrivateKey   privateKey;
        uint8_t client_key_data[1300] = {0};
        uint16_t client_key_data_size = SPIF_read_certificate_file_size(CLIENT_KEY_FILE);
        SPIF_read_certificate_file(CLIENT_KEY_FILE, client_key_data, client_key_data_size);
        
        Delay(500);
        
        Initiate_RSA_Key(&privateKey, client_key_data, client_key_data_size);
        
        Delay(500);
        
        decrypted_length = decrypt_server_response(&privateKey, &data[7], 128, decrypted_msg, &decrypted_msg_size);
        if(decrypted_length > 0)
        {
          if(PT_test_server_sipaad_login == RESET)
          {
            logined_to_the_server = SET;
            server_requests.login = RESET;
            memcpy(server_security_code, &decrypted_msg[21], 16);
            
            // Setting Request
            send_request_to_server = SET;
            server_requests.get_settings = SET;
            
            server_errors.wrong_certs = RESET;
            system_error.server_error = RESET;
            
            // Change LED Pattern
            if(system_current_state == SYSTEM_RUNNING)
              LED_change_mode(GSM_LED, LED_ON_STEADY);
          }
          else
          {
            if(system_current_state == SYSTEM_RUNNING)
              LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
          }
        }
        else
        {
          // Error in Private key
        }
        
        rsaFreePrivateKey(&privateKey);
        
        break;
      }
    case SUCCESS_LOGIN_NEW_FW:
      {
        uint8_t decrypted_msg[128] = {0};
        uint8_t decrypted_msg_size = 128;
        uint16_t decrypted_length = 0;
        
        last_sipaad_logined_unixtime = system_Unixtime;
        sipaad_server_is_ok = SET;
//        uint16_t extra_data_length = data[5] + (data[6] << 8);
        
        // Decrypting process
        RsaPrivateKey   privateKey;
        uint8_t client_key_data[1300] = {0};
        uint16_t client_key_data_size = SPIF_read_certificate_file_size(CLIENT_KEY_FILE);
        SPIF_read_certificate_file(CLIENT_KEY_FILE, client_key_data, client_key_data_size);
        
        Delay(500);
        
        Initiate_RSA_Key(&privateKey, client_key_data, client_key_data_size);
        
        Delay(500);
        
        decrypted_length = decrypt_server_response(&privateKey, &data[7], 128, decrypted_msg, &decrypted_msg_size);
        if(decrypted_length > 0)
        {
          if(PT_test_server_sipaad_login == RESET)
          {
            logined_to_the_server = SET;
            server_requests.login = RESET;
            memcpy(server_security_code, &decrypted_msg[21], 16);
            
            // Setting Request
            send_request_to_server = SET;
            server_requests.get_settings = SET;
            
            server_errors.wrong_certs = RESET;
            system_error.server_error = RESET;
            
            // Read software version
            new_firmware_available = SET;
            file_header_is_read = RESET;
            Big_endian_data(&data[7 + 128], (uint8_t*)&fota_file_version, 4);
            
            // Read file length
            
            // Change LED Pattern
            if(system_current_state == SYSTEM_RUNNING)
              LED_change_mode(GSM_LED, LED_ON_STEADY);
          }
          else
          {
            if(system_current_state == SYSTEM_RUNNING)
              LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
          }
        }
        else
        {
          // Error in Private key
        }
        
        rsaFreePrivateKey(&privateKey);
        
        break;
      }
    case LOGIN_ERROR_INVALID_DEVICE:    // Wrong IMEI
      {
        system_error.server_error = SET;
        system_error.server_error_count++;
        server_errors.wrong_IMEI = SET;
        
        save_system_IMEI = SET;
        system_IMEI = 0;
        
        // Change LED Pattern
        if(system_current_state == SYSTEM_RUNNING)
          LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_1SEC);
        
        if(++server_fault_count > 5)
        {
          reset_server_unreachable_counter = 3600;
          server_is_unreachable = SET;
          
          SSL_Close_Socket();
          Delay(3000);
          
          // Reset GSM Internet Process
          SSL_CIPSHUT();
          GSM_HALT_Timer = 10;
          GSM_Parameters.nw_provider = 0;
          GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
          GSM_Parameters.internet_connection = RESET;
          GSM_Parameters.stage_action = SIM_SEND_REQ;
          
          if(system_current_state == SYSTEM_RUNNING)
            LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
        }
        
        logined_to_the_server = RESET;
        
        return SIP_DATA_ERROR;
      }
    case LOGIN_ERROR_RELOGIN:
      {
        logined_to_the_server = RESET;
        
        // Change LED Pattern
        if(system_current_state == SYSTEM_RUNNING)
          LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_1SEC);
        
        return SIP_DATA_ERROR;
      }
    case REQUEST_IS_DONE:
      {
        break;
      }
    case CRC_ERROR_RESEND_DATA:
      {
        logined_to_the_server = RESET;
        return SIP_DATA_ERROR;
      }
    case RELOGIN:
      {
        logined_to_the_server = RESET;
        
        // Change LED Pattern
        if(system_current_state == SYSTEM_RUNNING)
          LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_1SEC);
        break;
      }
      // Settings
    case SETTING_INFORMATIONS:
      {
        server_requests.get_settings = RESET;
        
        uint8_t tmp_var_8 = 0;
        uint16_t tmp_var_16 = 0;
        array_idx = 7;
        
        // Send Data Period
        tmp_var_16 = data[array_idx++];
        tmp_var_16 += (data[array_idx++] << 8);
        if(tmp_var_16 != 0 && setting.data_send_rate_time != tmp_var_16)
        {
          setting.data_send_rate_time = tmp_var_16;
          save_changed_setting = SET;
        }
        
        // Data Time Rate
        tmp_var_16 = data[array_idx++];
        tmp_var_16 += (data[array_idx++] << 8);
        if(tmp_var_16 != 0 && setting.data_sample_rate_time != tmp_var_16)
        {
          if(tmp_var_16 > 10)
          {
            setting.data_sample_rate_time = tmp_var_16;
            save_changed_setting = SET;
          }
        }
        
        // Data Distance Rate
        tmp_var_16 = data[array_idx++];
        tmp_var_16 += (data[array_idx++] << 8);
        if(tmp_var_16 != 0 && setting.data_sample_rate_meter != tmp_var_16)
        {
          if(tmp_var_16 > 200)
          {
            setting.data_sample_rate_meter = tmp_var_16;
            save_changed_setting = SET;
          }
        }
        
        // Min Turn Degree
        tmp_var_8 = data[array_idx++];
        if(tmp_var_8 != 0 && setting.angle_threshold != tmp_var_8)
        {
          if(tmp_var_8 > 15)
          {
            setting.angle_threshold = tmp_var_8;
            save_changed_setting = SET;
          }
        }
        
        // Min Illegal Speed
        tmp_var_8 = data[array_idx++];
        if(tmp_var_8 != 0 && setting.illegal_speed != tmp_var_8)
        {
          setting.illegal_speed = tmp_var_8;
          save_changed_setting = SET;
        }
        
        // AES
        array_idx++;
        
        // Min Accelerate Value
        tmp_var_16 = data[array_idx++];
        tmp_var_16 += (data[array_idx++] << 8);
//        if(tmp_var_16 != 0 && setting.movement_acceleration != tmp_var_16)
//        {
//          if(tmp_var_16 > 50)
//          {
//            setting.movement_acceleration = tmp_var_16;
//            save_changed_setting = SET;
//          }
//        }
        
        // Odometer K Value
        tmp_var_16 = data[array_idx++];
        tmp_var_16 += (data[array_idx++] << 8);
        if(tmp_var_16 != 0 && setting.odometer_k != tmp_var_16)
        {
          setting.odometer_k = tmp_var_16;
          save_changed_setting = SET;
        }
        
        // Digital Output 
        tmp_var_8 = data[array_idx++];
        if(tmp_var_8 != 0 && setting.disable_relay != tmp_var_8)
        {
          setting.disable_relay = tmp_var_8;
          save_changed_setting = SET;
        }
        
        // Setting Request
        send_request_to_server = SET;
        server_requests.get_bt_list = SET;
        
        break;
      }
    case GEOFENCE_LIST:
      {
        server_requests.get_geo_list = RESET;
        break;
      }
    case GEOFENCE_INFO:
      {
        server_requests.get_geofence = RESET;
        break;
      }
      // Firmware File
    case FIRMWARE_CHUNK:
      {
        uint16_t extra_data_length = data[5] + (data[6] << 8);
        
        if(extra_data_length == firmware_chunk_size)
        {
          uint16_t array_idx = 7;
          
          // Header File Chunk
          if(file_header_is_read == RESET)
          {
            // Extracting firmware metadata
            
            // Header CRC-16
            uint16_t header_CRC = 0;
            Little_endian_data(&data[array_idx], (uint8_t*)&header_CRC, 2);
            array_idx += 2;
            
            // Pack Version
            uint32_t header_Version = 0;
            Big_endian_data(&data[array_idx], (uint8_t*)&header_Version, 4);
            array_idx += 4;
            
            // FW information count available in the header packet
            uint8_t fw_count = data[array_idx++];
            
            uint16_t calculated_crc = FOTA_Header_calc_CRC16(&data[7+2], (5 + fw_count*28));
            if(calculated_crc == header_CRC)
            {
              array_idx += (SIPAAD_2G_FW_IDX - 1) * 28;
              
              // FW Version
              uint32_t fw_release_date = 0;
              Big_endian_data(&data[array_idx], (uint8_t*)&fw_release_date, 4);
              array_idx += 4;
              
              if(fw_release_date > APP_RELEASE_DATE)
              {
                // FW Start Index
                Little_endian_data(&data[array_idx], (uint8_t*)&firmware_base_address, 4);
                array_idx += 4;
                
                // FW Length
                Little_endian_data(&data[array_idx], (uint8_t*)&system_temp_params.firmware_size, 4);
                array_idx += 4;
                
                // FW MD5
                Little_endian_data(&data[array_idx], (uint8_t*)&system_temp_params.firmware_md5, 16);
                
                // Limit firmware download size
                if(system_temp_params.firmware_size < MAX_SIZE_OF_FIRMWARE_DOWNLOAD)
                {
                  firmware_chunk_size = SPIF_PAGE_SIZE;
                  firmware_downloaded_size = 0;
                  firmware_chunk_index = 0;
                }
                else
                {
                  system_temp_params.device_version = header_Version;     // fota_file_version
                  SPIF_Write_sys_temp_params();
                  
                  // Disable sending server request
                  new_firmware_available = RESET;
                  send_request_to_server = RESET;
                  firmware_downloading_process = RESET;
                  server_requests.get_firware_chunk = RESET;
                  
                  //Disable_GPS_GSM_LED();
                }
              }
              else
              {
                system_temp_params.device_version = header_Version;     // fota_file_version
                SPIF_Write_sys_temp_params();
                
                // Disable sending server request
                new_firmware_available = RESET;
                send_request_to_server = RESET;
                firmware_downloading_process = RESET;
                server_requests.get_firware_chunk = RESET;
                
                //Disable_GPS_GSM_LED();
              }
              
              file_header_is_read = SET;
            }
            // Error in header CRC
            else
            {
              // Disable sending server request
              new_firmware_available = RESET;
              firmware_downloading_process = RESET;
              send_request_to_server = RESET;
              server_requests.get_firware_chunk = RESET;
              
              //Disable_GPS_GSM_LED();
            }
          }
          // Firmware File Chunk
          else
          {
            // Saving Received chunk
            SPIF_Save_FW_chunk(&data[array_idx], firmware_chunk_size, firmware_chunk_index);
            firmware_chunk_index++;
            firmware_downloaded_size += firmware_chunk_size;
            firmware_base_address += firmware_chunk_size;
            
            // Downloading compeleted
            if(firmware_downloaded_size == system_temp_params.firmware_size)
            {
              // Generating MD5 of the firmware
              uint8_t fw_md5[16] = {0};
              error_t error;
              error = PT_generate_firmware_MD5(fw_md5, APPLICATION_FWR, system_temp_params.firmware_size);
              if(error == NO_ERROR)
              {
                int cmp_result = 0;
                cmp_result = memcmp(fw_md5, system_temp_params.firmware_md5, 16);
                if(cmp_result == 0)
                {
                  // MD5 Match
                  
                  system_temp_params.firmware_CRC_value = PT_compute_Firmware_CRC16(system_temp_params.firmware_size, APPLICATION_FWR, 0);
                  system_temp_params.firmware_available = SET;
                  system_temp_params.firmware_source = SRC_SERVER;
                  
                  system_temp_params.device_version_temp = fota_file_version;
                  SPIF_Write_sys_temp_params();
                  
                  // Reseting system
                  __NVIC_SystemReset();
                }
                else
                {
                  new_firmware_available = RESET;
                  firmware_downloading_process = RESET;
                  send_request_to_server = RESET;
                  server_requests.get_firware_chunk = RESET;
                  logined_to_the_server = RESET;
                  
                  //Disable_GPS_GSM_LED();
                }
              }
              else
              {
                new_firmware_available = RESET;
                firmware_downloading_process = RESET;
                send_request_to_server = RESET;
                server_requests.get_firware_chunk = RESET;
                logined_to_the_server = RESET;
                
                //Disable_GPS_GSM_LED();
              }
            }
            // Error in file downloading
            else if (firmware_downloaded_size > system_temp_params.firmware_size)
            {
              new_firmware_available = RESET;
              firmware_downloading_process = RESET;
              send_request_to_server = RESET;
              server_requests.get_firware_chunk = RESET;
              logined_to_the_server = RESET;
              
              //Disable_GPS_GSM_LED();
            }
            // Continue downloading
            else
            {
              if( (system_temp_params.firmware_size - firmware_downloaded_size) >= SPIF_PAGE_SIZE )
              {
                firmware_chunk_size = SPIF_PAGE_SIZE;
              }
              else
              {
                firmware_chunk_size = system_temp_params.firmware_size - firmware_downloaded_size;
              }
            }
          }
        }
        
        break;
      }
    case INVALID_REQUEST:
      {
        break;
      }
    case INVALID_SIGNATURE:
      {
        logined_to_the_server = RESET;
        
        // Change LED Pattern
        if(system_current_state == SYSTEM_RUNNING)
          LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_1SEC);
        
        return SIP_DATA_ERROR;
      }
      // Bug in server packet data field count
    case UNKNOWN_ERROR_RETRY:
      {
        system_error.server_error = SET;
        system_error.server_error_count++;
        
        if(++server_fault_count > 5)
        {
          reset_server_unreachable_counter = 3600;
          server_is_unreachable = SET;
          
          if(system_current_state == SYSTEM_RUNNING)
            LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
        }
		
        return SIP_DATA_ERROR;
      }
    case INVALID_TIMING:
      {
        return SIP_DATA_ERROR;
      }
    case BLUETOOTH_LIST:
      {
        server_requests.get_bt_list = RESET;
        
        uint16_t extra_data_length = data[5] + (data[6] << 8);
        uint16_t array_idx = 7;
        
        uint8_t bt_count = data[array_idx++];
        
        if( extra_data_length > 0 && (extra_data_length == (bt_count*BT_MAC_ADDR_SIZE + 1)))
        {
          bt_count = MIN(bt_count, MAX_BT_DEVICE_COUNT);
          bluetooth_device_count = bt_count;  
          
          for(uint8_t iterator = 0; iterator < bt_count; iterator++)
          {
            memcpy(&bluetooth_mac_addr[iterator][0], &data[array_idx], BT_MAC_ADDR_SIZE);
            array_idx += BT_MAC_ADDR_SIZE;
          }
        }
        
//        // Test
//        bluetooth_device_count = 1;
//        uint8_t tmp_mac[] = {0xdc,0xb7,0x2e,0x65,0x19,0x6f};
//        memcpy(bluetooth_mac_addr[0], tmp_mac, 6);
//        
        break;
      }
    case BLUETOOTH_TEST:
      {
        server_requests.send_bt_status = RESET;
        break;
      }
    case LOGIN_ERROR_INVALID_CERTIFICATE:
      {
        logined_to_the_server = RESET;
        
        // Change LED Pattern
        if(system_current_state == SYSTEM_RUNNING)
          LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_1SEC);
        
        server_errors.wrong_certs++;
        system_error.server_error = SET;
        system_error.server_error_count++;
        
        if(server_errors.wrong_certs > 5)
        {
          SSL_Close_Socket();
          Delay(3000);
          
          server_errors.wrong_certs = 0;
          system_error.no_certificate = SET;
          
          enable_send_data_to_sipaad = RESET;
          sending_data_is_in_progress = RESET;
          
          PT_test_server_sipaad_login = SET;
          PT_test_login_packet_is_created = RESET;
          
          // Redownload Certificate files
          SPIF_Erase_certificates();
          check_server_certificates_and_fwu = SET;
          
          // Reset GSM Internet Process
          SSL_CIPSHUT();
          GSM_HALT_Timer = 10;
          GSM_Parameters.nw_provider = 0;
          GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
          GSM_Parameters.internet_connection = RESET;
          GSM_Parameters.stage_action = SIM_SEND_REQ;
        }
        
        return SIP_DATA_ERROR;
      }
    }
    
    return SIP_PROTOCOL_OK;
  }
  
  return SIP_PROTOCOL_ERROR;
}


void PT_send_sms_panel_information(void)
{
  memcpy(SMS_phone_number, setting.sms_panel, setting.sms_panel_length);
  SMS_phone_num_length = setting.sms_panel_length;

  GSM_add_char_number_to_buffer(SMS_send_buffer, &sms_send_length, system_IMEI);

  system_has_short_message = SET;
  SMS_stage = SMS_SET_FORMAT;
  SMS_stage_action = 0;
  SMS_stage_number_of_retries = 0;
  GSM_status = GSM_HANDLE_SMS;
}


void Little_endian_data(uint8_t* original_data, uint8_t* little_endian_data, uint8_t data_size)
{
  for(uint8_t i = 0; i < data_size; i++)
    *(little_endian_data + i) = *(original_data + i);
}

void Big_endian_data(uint8_t* original_data, uint8_t* little_endian_data, uint8_t data_size)
{
  for(uint8_t i = 0; i < data_size; i++)
    *(little_endian_data + i) = *(original_data+((data_size-1)-i));
}

// CRC16/CCITT-False
uint16_t CRC16_CCITT(uint8_t* pData, uint16_t length)
{
  int i;
  uint16_t wCrc = 0xffff;
  
  while (length--)
  {
    wCrc ^= *(uint8_t *)pData++ << 8;
    
    for (i=0; i < 8; i++)
      wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ CCITT_CRC_POLYNOMIAL : wCrc << 1;
  }
  return wCrc;
}

uint8_t PT_calculate_checksum_8_XOR(uint8_t* data, uint16_t data_length)
{
  uint8_t XOR_result = 0x00;
  
  for(uint16_t iterator = 0; iterator < data_length; iterator++)
    XOR_result ^= *(data+iterator);
  
  return XOR_result;
}

uint16_t FOTA_Header_calc_CRC16(uint8_t* data, uint16_t size)
{
  uint16_t out = 0;
  int bits_read = 0, bit_flag;
  
  /* Sanity check: */
  if(data == NULL)
    return 0;
  
  while(size > 0)
  {
    bit_flag = out >> 15;
    
    /* Get next bit: */
    out <<= 1;
    out |= (*data >> bits_read) & 1; // item a) work from the least significant bits
    
    /* Increment bit counter: */
    bits_read++;
    if(bits_read > 7)
    {
      bits_read = 0;
      data++;
      size--;
    }
    
    /* Cycle check: */
    if(bit_flag)
      out ^= HEADER_CRC16_POLY;
  }
  
  // item b) "push out" the last 16 bits
  int i;
  for (i = 0; i < 16; ++i)
  {
    bit_flag = out >> 15;
    out <<= 1;
    if(bit_flag)
      out ^= HEADER_CRC16_POLY;
  }
  
  // item c) reverse the bits
  uint16_t crc = 0;
  i = 0x8000;
  int j = 0x0001;
  for (; i != 0; i >>=1, j <<= 1)
    if (i & out) crc |= j;
  
  return crc;
}

error_t PT_generate_firmware_MD5(uint8_t *digest, uint8_t file_type, uint32_t file_size)
{
  uint32_t SPIF_data_address;
  uint8_t data_array[SPIF_PAGE_SIZE] = {0};
  
  if(file_type == BOOTLOADER_FWR)
    SPIF_data_address = SPIF_BOOT_FW_FILE_START_SECTOR * SPIF_SECTOR_SIZE;
  else if(file_type == APPLICATION_FWR)
    SPIF_data_address = SPIF_APP_FW_FILE_START_SECTOR * SPIF_SECTOR_SIZE;
  
  uint16_t page_count = file_size / SPIF_PAGE_SIZE;
  uint16_t last_page_remain_bytes = file_size % SPIF_PAGE_SIZE;
  
  
  //Allocate a memory buffer to hold the MD5 context
  Md5Context *context = cryptoAllocMem(sizeof(Md5Context));
  //Failed to allocate memory?
  if(context == NULL)
    return ERROR_OUT_OF_MEMORY;
  
  //Initialize the MD5 context
  md5Init(context);
  
  // Reading entire sector
  for(uint16_t SPIF_sector_itr = 0; SPIF_sector_itr < page_count; SPIF_sector_itr++)
  {
    SPIF_Read(data_array, SPIF_data_address, SPIF_PAGE_SIZE);
    
    Delay(300);
    
    //Digest the message
    md5Update(context, data_array, SPIF_PAGE_SIZE);
    
    SPIF_data_address += SPIF_PAGE_SIZE;
    IWDG_ReloadCounter();
  }
  
  // Reading the last sector
  if(last_page_remain_bytes > 0)
  {
    SPIF_Read(data_array, SPIF_data_address, last_page_remain_bytes);
    //Digest the message
    md5Update(context, data_array, last_page_remain_bytes);
  }
  
  //Finalize the MD5 message digest
  md5Final(context, digest);
  
  //Free previously allocated memory
  cryptoFreeMem(context);
  
  //Successful processing
  return NO_ERROR;
}

uint32_t PT_compute_Firmware_CRC16(uint32_t chunk_size, uint8_t file_type, uint32_t start_addr)
{
  int i;
  uint16_t wCrc = 0xffff;
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
      wCrc ^= data_array[iterator] << 8;
      
      for (i=0; i < 8; i++)
        wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ CRC_POLYNOMIAL : wCrc << 1;
    }
    
    SPIF_data_address += SPIF_PAGE_SIZE;
    IWDG_ReloadCounter();
  }
  
  // Reading the last sector
  if(last_page_remain_bytes > 0)
  {
    SPIF_Read(data_array, SPIF_data_address, last_page_remain_bytes);
    
    Delay(50);
    
    for(uint16_t iterator = 0; iterator < last_page_remain_bytes; iterator++)
    {
      wCrc ^= data_array[iterator] << 8;
      
      for (i=0; i < 8; i++)
        wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ CRC_POLYNOMIAL : wCrc << 1;
    }
  }
  
  return wCrc;
}


void AVL_Handle_Tasks(void)
{
  // ---------- Recovery I2C ----------
  if(recover_Accel_error == SET)
  {
    recover_Accel_error = RESET;
    disable_accel_timer = SET;
    
    if(Accel_Recovery_I2C() == KX023_OK)
    {
      if(reconfig_accelerometer == SET)
      {
        reconfig_accelerometer = RESET;
        KX023_Config();
      }
      else
      {
        KX023_startup_calibration();
      }
      
      disable_accel_timer = RESET;
    }
    else
    {
      recover_accel_error_counter = 1;
    }
  }
  
  // ---------- Check System Errors ----------
  System_check_errors();
  
  // ---------- Save SMS Setting ----------
  if(save_changed_setting == SET)
  {
    if(Disable_Relay_process == SET)
    {
      if(GPS_data.speed_Kmh < 10)
      {
        SPIF_write_setting();
        save_changed_setting = RESET;
        Disable_Relay_process = RESET;
        IO_ON_OFF_Relay(OFF);
      }
    }
    else
    {
      SPIF_write_setting();
      save_changed_setting = RESET;
    }
  }
  
  // ---------- ACC State Changed ----------
  if(ignition_changed == SET)
  {
    ignition_changed = RESET;
    IO_Ignition_Changed();
  }
  
  // ---------- VCC State Changed ----------
  if(vcc_digital_changed == SET)
  {
    vcc_digital_changed = RESET;
    IO_vcc_digital_Changed();
  }
  
  // Record Production
  if(change_record_production == SET)
  {
    change_record_production = RESET;
    
    if(enable_disable_record_production == SET)
      activate_record_production = ENABLE;
    else
    {
      if(IO_Digital.ignition == OFF)
        activate_record_production = DISABLE;
    }
  }
  
  // Server OK
  if(check_server_ok_after_gps_fix == SET)
  {
    if(gps_is_fixed_for_first_time == SET)
    {
      check_server_ok_after_gps_fix = RESET;
      
      if(sipaad_server_is_ok == SET)
      {
        if(last_sipaad_logined_unixtime < RTC_SECONDS_PER_DAY)
        {
          last_sipaad_logined_unixtime = system_Unixtime;
        }
      }
      else
      {
        uint32_t temp_32 = system_Unixtime - last_sipaad_logined_unixtime;
        
        if(temp_32 < (3*RTC_SECONDS_PER_DAY))
          sipaad_server_is_ok = SET;
      }
    }
  }
  
  // ---------- GPS ----------
  GPS_superloop_call();
  
  // ---------- GSM ----------
  if(turn_off_gsm == SET)
  {
    turn_off_gsm = RESET;
    
    GSM_power_off();
    
    // Change LED Pattern
    if(system_current_state == SYSTEM_RUNNING)
      LED_change_mode(GSM_LED, LED_OFF);
  }
  
  if(turn_on_gsm == SET)
  {
    turn_on_gsm = RESET;
    
    GSM_power_on();
    
    // Change LED Pattern
    if(system_current_state == SYSTEM_RUNNING)
      LED_change_mode(GSM_LED, LED_ON_BLINK_500MS);
  }
  
  if(read_system_IMEI == SET)
  {
    read_system_IMEI = RESET;
    
    SPIF_read_system_IMEI();
  }
  // ---------- GSM Alive ----------
  if(keep_GSM_module_awake == SET)
  {
    if(GSM_Parameters.nw_provider == 0)
    {
      GSM_status = GSM_INITIATING;
      
      if(disable_any_gsm_activity == RESET)
        GSM_SSL_routine_pro(GSM_Parameters.stage);
    }
  }
  
  // ---------- Short Message ----------
  if(system_has_short_message
     && GSM_Parameters.stage_action == SIM_SEND_REQ
       && GSM_Parameters.stage != SIM_STAGE_SEND_DATA
         && GSM_Parameters.stage != SIM_STAGE_RECEIVE_SERVER_RESPONSE
           && GSM_Parameters.stage != SIM_STAGE_READ_SERVER_RESPONSE)
  {
    if(GSM_status == GSM_IDLE)
    {
      GSM_status = GSM_HANDLE_SMS;
      sending_data_is_in_progress = RESET;
      
      SMS_stage = SMS_READ_MESSAGE;
      SMS_stage_action = SIM_SEND_REQ;
      SMS_stage_number_of_retries = 0;
    }
    else if(GSM_status == GSM_HANDLE_SMS)
    {
      GSM_send_SMS_routine_pro(sim_SMS_stage_timeout_timer[SMS_stage], SMS_stage);      
      Delay(50);
    }
  }
  
  // ---------- FOTA ----------
  if(firmware_downloading_process == SET)
  {
    if(file_header_is_read == RESET)
    {
      firmware_base_address = 0;
      firmware_chunk_size = FOTA_HEADER_CHUNK_SIZE;
    }
    
    send_request_to_server = SET;
    server_requests.get_firware_chunk = SET;
  }
  
  // ---------- Check Certificates ----------
  if(check_server_certificates_and_fwu == SET && GSM_status == GSM_IDLE)
  {
    check_server_certificates_and_fwu = RESET;
    
    if(setting.disable_http == RESET)
    {
      // Always check FWU
      HTTP_Params.dl_fwr_info = SET;
      HTTP_Params.send_information = SET;
      
      Sipaad_check_SPIF_Certs();
      
      enable_http_process = SET;
    }
  }
  
  // ---------- Download HTTP Files ----------
  if(enable_http_process == SET)
  {
    if(GSM_Parameters.nw_provider != 0)
    {
      if(halt_http_process == RESET)
      {
        HTTP_preconfig_http_requirments();
        
        if(GSM_status == GSM_HTTP_PROCESS)
        {
          // Protect routine from unexpected events
          if(GSM_Parameters.stage > HTTP_STAGE_SEEK_OK)
          {
            GSM_Parameters.stage_action = SIM_SEND_REQ;
            GSM_Parameters.stage = HTTP_STAGE_TEST_SERIAL;
            GSM_Parameters.prev_stage = 0;
            GSM_Parameters.next_stage = 0;
            GSM_Parameters.number_of_retries_command = 0;
          }
          
          uint16_t resp = GSM_HTTP_routine_pro(GSM_Parameters.stage);
          if(resp == GSM_HTTP_DONE_PROCESS)
          {
            if(http_has_request == RESET)
            {
              GSM_status = GSM_IDLE;
              number_of_http_process_run = 0;
              enable_http_process = RESET;
            }
            
            http_has_request = RESET;
            
            // Change LED Pattern
            if(system_current_state == SYSTEM_RUNNING)
              LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
          }
          else if(resp == GSM_HTTP_CHECK_NEXT_REQUEST)
          {
            GSM_status = GSM_IDLE;
            enable_http_process = RESET; 
            number_of_http_process_run = 0;
            
            FCB_modify_sent_records();
            server_packet_count = 0;
            
            // Change LED Pattern
            if(system_current_state == SYSTEM_RUNNING)
              LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
          }
          
          Delay(50);
        }
      }
    }
  }
  
  // ---------- HTTP FWU Ready ----------
  if(http_firmware_upgrade_ready)
  {
    if(IO_Digital.vcc_digital == SET && IO_Digital.ignition == RESET)
    {
      if(setting.force_FOTA == SET)
      {
        setting.force_FOTA = RESET;
        
        SPIF_write_setting();
      }
      
      // Reseting system
      __NVIC_SystemReset();
    }
  }
  
  // ---------- Server packet Records Generation ----------
  if(server_is_unreachable == RESET)
  {
    SIP_Manage_Requests_Records();
  }
  
  // ---------- Sending Server Records ----------
  SIP_Sending_Packet_to_the_server_process();
  
  // ---------- Send CMD to GSM ----------
  if(send_command_to_GSM == SET)
  {
    GSM_send_cmd_to_GSM();
  }
  
  // ---------- Send SMS Panel ----------
  if(sms_panel_send == SET)
  {
    if(GSM_status == GSM_IDLE)
    {
      sms_panel_send = RESET;
      PT_send_sms_panel_information();
    }
  }
}

