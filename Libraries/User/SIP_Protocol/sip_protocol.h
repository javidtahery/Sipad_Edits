#ifndef __SIP_PROTOCOL_H
#define __SIP_PROTOCOL_H

/* ---------------- Include ---------------- */
#include "main.h"
#include "time.h"
#include "error.h"

/* ---------------- Define ---------------- */
#define GPS_1PPS_TIMEOUT                                3
#define GPS_SLEEP_TIMEOUT                               2400    // 40 * 60
#define GSM_SLEEP_TIMEOUT_AFTER_IGN_OFF                 3600    // 60 * 60
#define GSM_PRIODIC_ON                                  3600    // 60 * 60
#define GSM_PRIODIC_OFF                                 300     // 5 * 60
#define HALT_EVENT_PRODUCTION_TIMER_DEFAULT             200
#define HALT_STARTUP_EVENT_PRODUCTION                   5

#define URL_MAX_LENGTH_SIZE                             64
#define SMS_PANEL_MAX_LENGTH_SIZE                      64
#define APN_MAX_LENGTH_SIZE                             30
#define MAX_NUM_OF_SERVER_RECORD_PACKET                 10
#define MAX_SIZE_OF_DATA_PACKET                         850
#define FIRMWARE_CHUNK_MAX_SIZE                         512
#define MAX_BT_DEVICE_COUNT                             10
#define BT_MAC_ADDR_SIZE                                6
#define MAX_SIZE_OF_FIRMWARE_DOWNLOAD                   114688          // 112*1024 Kb

#define SIZEOF_RECORD                                   sizeof(record_typedef)
#define FOTA_HEADER_CHUNK_SIZE                          256
#define CCITT_CRC_POLYNOMIAL                            0x1021
#define HEADER_CRC16_POLY                               0x8005
#define SIPAAD_2G_FW_IDX                                1

// ------- Setting Defaults -------
#define SETTING_META_DATA                               0x4B18AFD5
#define DEFAULT_NORMAL_POINT_DISTANCE_STARTUP           400     // meters
#define DEFAULT_NORMAL_POINT_TIME_INTERVAL_STARTUP      30      // seconds
#define DEFAULT_SEND_DATA_TIME_INTERVAL_STARTUP         900     // seconds
#define DEFAULT_IGNITION_VOLTAGE_LVL                    13000
#define DEFAULT_MOVEMENT_ACCELERATION                   100      // Mili G
#define DEFAULT_STATIC_ACCELERATION                     100      // Mili G
#define DEFAULT_ANGLE                                   30
#define DEFAULT_ILLEGAL_SPEED                           90
#define DEFAULT_ODOMETER_K_FACTOR                       25000
#define DEFAULT_HEARTBEAT_RATE_TIME                     15

// -------------------------------------------------- //
#define LOW_BATTERY_ALARM_THRESHOLD                     3400    // mV
#define ODO_GNSS_SPEED_DIFFERENCE                       15      // Km
#define VEHICLE_LOCATION_JUMP_THRESHOLD                 5000    // Meter
#define VEHICLE_IDLE_TIME_THRESHOLD                     600     // Second
#define REPETITION_LOW_BAT_EVENT_THRESHOLD              7200    // Second
#define REPETITION_GNSS_ODO_SPEED_THRESHOLD             60      // Second
#define REPETITION_ACCIDENT_EVENT_THRESHOLD             300     // Second


// -------------------------------------------------- //
#define SERVER_REQUEST                                  0x35
#define SERVER_EVENT                                    0x36
#define SERVER_HEARTBEAT                                0x37

// Server Requests
#define NO_REQUEST                                      0x00
#define REQUEST_LOGIN                                   0x01
#define REQUEST_GET_SEETINGS                            0x08
#define REQUEST_GET_FOTA_CHUNK                          0x09
#define REQUEST_GET_GEO_LIST                            0x0A
#define REQUEST_GET_GEO                                 0x0B
#define REQUEST_GET_BT_LIST                             0x0C
#define REQUEST_SEND_SMARTCARD                          0x0D

// Server Events
#define NORMAL_POINT                                    0x02
#define MAIN_PWR_DISCONNECT                             0x20
#define MAIN_PWR_CONNECT                                0x21
#define LOW_INTERNAL_BATTERY                            0x22
#define IGNITION_ON                                     0x23
#define IGNITION_OFF                                    0x24
#define GNSS_ANTENNA_DISCONNECT                         0x25
#define GNSS_ANTENNA_SHORT_CIRCUIT                      0x26
#define CAN_INPUT_DATA_ERROR                            0x27
#define GNSS_ODOMETER_SPEED_DIF                         0x28
#define CRASH_HAPPEND                                   0x29
#define UNALLOWED_DRIVING_SPEED                         0x2A
#define STOP_STARTED                                    0x2B
#define STOP_ENDED                                      0x2C
#define DEVICE_DEATTACHED                               0x2D
#define DEVICE_ATTACHED                                 0x2E
#define GPS_JAMMING                                     0x2F
#define GSM_JAMMING                                     0x30
#define FUEL_LEVELED_UP                                 0x31
#define GEOFENCE_ENTERED                                0x32
#define GEOFENCE_EXIT                                   0x33
#define CHANGE_DIRECTION                                0x34
#define GPS_LOCATION_JUMP                               0x35
#define GNSS_SIGNAL_LOST                                0x36
#define GSM_SIGNAL_LOST                                 0x37


/* ---------------- Enum ---------------- */
enum{
  SIP_PROTOCOL_OK = 0,
  SIP_PROTOCOL_ERROR,
  SIP_DATA_ERROR,
};


typedef enum{
  GSM_INITIATING                                = 0,
  GSM_IDLE,
  GSM_LOGIN,
  GSM_SENDING_HB,
  GSM_SENDING_PACKET,
  GSM_HANDLE_SMS,
  GSM_SERVER_TEST,
  GSM_HTTP_PROCESS,
}gsm_status;


// Server Packet Type
enum{
  LOGIN_PACKET                                  = 0xAB,
  LOCATION_PACKET,                              // 0xAC
  HEARTBEAT_PACKET,                             // 0xAD
  REQUEST_PACKET,                               // 0xAE
  SETTING_RESPONSE,                             // 0xAF
};

typedef enum{
  SUCCESS_LOGIN                                 = 0xE0,
  SUCCESS_LOGIN_NEW_FW,                         // 0xE1
  LOGIN_ERROR_INVALID_DEVICE,                   // 0xE2
  LOGIN_ERROR_RELOGIN,                          // 0xE3
  REQUEST_IS_DONE,                              // 0xE4
  CRC_ERROR_RESEND_DATA,                        // 0xE5
  RELOGIN,                                      // 0xE6
  SETTING_INFORMATIONS,                         // 0xE7
  GEOFENCE_LIST,                                // 0xE8
  GEOFENCE_INFO,                                // 0xE9
  FIRMWARE_CHUNK,                               // 0xEA
  INVALID_REQUEST,                              // 0xEB
  INVALID_SIGNATURE,                            // 0xEC
  UNKNOWN_ERROR_RETRY,                          // 0xED
  INVALID_TIMING,                               // 0xEE
  BLUETOOTH_LIST,                               // 0xEF
  BLUETOOTH_TEST                                = 0xF1,
  LOGIN_ERROR_INVALID_CERTIFICATE,              // 0xF2
}sip_server_response;

enum{
  SRC_NONE                      = 0,
  SRC_CONFIGURATOR,
  SRC_SERVER,
};      // Firmware Upgrade Source

/* ---------------- Structure ---------------- */
typedef struct{
  uint64_t one_pps_last_tick_time;
}last_variables_typeDef;

typedef struct{
  uint8_t
    login,
    wrong_certs,
    wrong_IMEI;
}server_errors_typedef;

typedef struct{
  uint8_t
    login,
    get_settings,
    get_firware_chunk,
    get_geo_list,
    get_geofence,
    get_bt_list,
    send_bt_status;
}server_request_typedef;


typedef struct{
  uint16_t data_send_rate_time;                 // Second
  uint16_t data_sample_rate_time;               // Second
  uint16_t data_sample_rate_meter;              // Meter
  uint16_t server_port;
  uint16_t ignition_volt_lvl;                   // Mili volts
  uint16_t movement_acceleration;               // Mili G
  uint16_t static_acceleration;                 // Mili G
  uint16_t odometer_k;                          // Pulse/Km
  uint8_t angle_threshold;                      // unit: Degree (0 - 255)
  uint8_t illegal_speed;                        // Km/h
  uint8_t heartbeat_rate_time;                  // Minutes
  uint8_t disable_relay;                        // Set -> Disable, Reset -> Enable
  uint8_t disable_http;
  uint8_t server_url[URL_MAX_LENGTH_SIZE];
  uint8_t server_url_length;
  uint8_t APN[APN_MAX_LENGTH_SIZE];
  uint8_t APN_length;
  uint8_t custom_APN;
  uint8_t http_server[URL_MAX_LENGTH_SIZE];
  uint8_t http_server_length;
  uint8_t sms_panel[SMS_PANEL_MAX_LENGTH_SIZE];
  uint8_t sms_panel_length;
  uint8_t force_FOTA;
  uint8_t api_interval_disable;
  uint16_t api_interval_time;                   // Minutes
  uint16_t debug_mode_time;                     // Minutes
  uint32_t api_setting_code;
  uint32_t hash_nsend_count;
  uint32_t setting_metadata;
}setting_typeDef;

// Typical Packet Length: 22 Bytes
__packed typedef struct{
  uint32_t              unixtime;
  uint32_t              GPS_latitude;
  uint32_t              GPS_longitude;
  uint16_t              GPS_altitude;
  uint16_t              GPS_bearing;
  uint8_t               GPS_fixMode;                            // 3D: 0x03, 2D:0x02, not fix: 0x01
  uint8_t               GPS_speed;                              // Km/h
  uint8_t               GPS_nsat;
  uint8_t               GPS_pdop;
  uint8_t               S_N;                                    // South / North
  uint8_t               E_W;                                    // East / West
}GPS_Packet_typedef;

// 48 Bytes
__packed typedef struct{
  GPS_Packet_typedef    gps_elements;
  uint32_t              GPS_travel_dist;
  uint32_t              sensor_travel_dist;
  uint16_t              g_sensor_value;
  uint16_t              supply_voltage;
  uint16_t              analoge_in_1;
  uint16_t              analoge_in_2;
  uint8_t               reserve;
  uint8_t               io_status;
  uint8_t               event_code;
  uint8_t               event_data;
  uint8_t               GSM_lvl;
  uint8_t               sensor_speed;
  uint8_t               GPS_max_speed;
  uint8_t               sensor_max_speed;
  uint8_t               crc_16_h;
  uint8_t               crc_16_l;
}record_typedef;


typedef struct{
  uint8_t
    main_pwr_disconnected,
    main_pwr_connected,
    ignition_high,
    ignition_low,
    g_sensor_activated,
    change_direction,
    low_internal_battery,
    gnss_odometer_speed_dif,
    unallowed_speed,
    idle_begin,
    idle_end,
    device_attached,
    device_deattached,
    gnss_loc_jump,
    gnss_lost_signal,
    gsm_lost_signal,
    gps_jamming;
}Event_production_flags_typedef;


/* ---------------- Extern ---------------- */
extern last_variables_typeDef           last_vars;
extern setting_typeDef                  setting;
extern Event_production_flags_typedef   Event_Flags;
extern record_typedef                   server_records[MAX_NUM_OF_SERVER_RECORD_PACKET];
extern server_request_typedef           server_requests;


// --------------- General variables of the protocol ---------------
extern int create_normal_event_counter;
//extern int create_Heartbeat_counter;
extern int data_send_rate_counter;
extern uint8_t send_data_less_than_10_records;
extern uint8_t halt_event_production_counter;
extern uint8_t create_normal_event;
extern uint8_t force_create_event;
//extern uint8_t create_Heartbeat;
extern uint8_t activate_record_production;
extern uint8_t change_record_production;
extern uint8_t enable_disable_record_production;
extern uint8_t enable_send_data_to_sipaad;
extern uint8_t PT_test_server_sipaad_login;
extern uint8_t PT_test_login_packet_is_created;
extern uint32_t vehicle_total_movement;
extern uint16_t total_generated_record;

extern uint8_t reconfig_accelerometer;

extern uint8_t Disable_Relay_process;

extern uint8_t save_changed_setting;
extern uint8_t low_battery_detection;

extern uint16_t repetition_odometer_gnss_speed_difference_counter;
extern uint16_t repetition_low_battery_counter;
extern uint16_t repetition_accident_detection_counter;
extern uint64_t system_IMEI;
extern uint64_t system_Serial;
extern uint8_t save_system_IMEI;

extern uint8_t GPS_max_speed;
extern uint8_t Odo_max_speed;
extern float Odo_travelled_distance;
extern float Current_ODO_Speed_KMh;
extern uint8_t over_speed_happened;
extern uint32_t GPS_traveled_distance;
extern uint32_t Sensor_traveled_distance;
extern uint8_t gps_notfix_event_counter;

extern uint8_t server_url_length;
extern uint8_t server_packet_count;
extern uint8_t logined_to_the_server;
extern uint8_t server_socket;
extern uint8_t sending_data_is_in_progress;
extern gsm_status GSM_status;
extern uint16_t data_packet_length;
extern uint8_t data_packet[MAX_SIZE_OF_DATA_PACKET];
extern uint8_t server_packet_type;
extern uint32_t GSM_local_IP;
extern uint16_t idle_started_counter;
extern uint8_t check_vehicle_idle;
extern uint8_t vehicle_idle_state;
extern uint8_t check_server_certificates_and_fwu;
extern uint32_t last_sipaad_logined_unixtime;
extern uint8_t sipaad_server_is_ok;
extern uint8_t check_server_ok_after_gps_fix;

extern uint8_t server_is_unreachable;
extern uint8_t server_fault_count;
extern uint16_t reset_server_unreachable_counter;

extern uint8_t keep_GSM_module_awake;
extern uint8_t gsm_power_status;
extern uint8_t turn_off_gsm;
extern uint8_t turn_on_gsm;
extern uint8_t GSM_cmd_execution_code;
extern uint8_t send_command_to_GSM;
extern uint8_t GSM_check_simcard_counter;
extern uint8_t sms_panel_send;

extern uint8_t send_request_to_server;
extern uint8_t bluetooth_device_count;
extern uint8_t bluetooth_mac_addr[MAX_BT_DEVICE_COUNT][BT_MAC_ADDR_SIZE];

extern uint8_t debug_mode;
extern uint16_t debug_mode_counter;

extern uint16_t gps_jump_count;
extern uint16_t mainpower_disconnection_counter;
extern uint16_t mainpower_connection_counter;

/* ---------------- Prototype ---------------- */
void FCB_write_records(record_typedef* record);

void System_Startup_Initiate(void);
void AVL_Initiate_Setting(void);
void AVL_check_setting_values(void);
void check_startup_server_ok(void);
void save_server_ok(void);
void Force_Produce_Event(void);
void Record_Production(void);
void SIP_Manage_Requests_Records(void);
void SIP_Sending_Packet_to_the_server_process(void);
uint8_t SIP_parse_server_response(uint8_t* data, uint16_t data_length);
uint8_t PT_calculate_checksum_8_XOR(uint8_t* data, uint16_t data_length);
uint32_t PT_compute_Firmware_CRC16(uint32_t chunk_size, uint8_t file_type, uint32_t start_addr);
uint16_t CRC16_CCITT(uint8_t* pData, uint16_t length);
uint16_t FOTA_Header_calc_CRC16(uint8_t* data, uint16_t size);
error_t PT_generate_firmware_MD5(uint8_t *digest, uint8_t file_type, uint32_t file_size);
void Little_endian_data(uint8_t* original_data, uint8_t* little_endian_data, uint8_t data_size);
void Big_endian_data(uint8_t* original_data, uint8_t* little_endian_data, uint8_t data_size);
void AVL_Handle_Tasks(void);

#endif /* __SIP_PROTOCOL_GT_H */