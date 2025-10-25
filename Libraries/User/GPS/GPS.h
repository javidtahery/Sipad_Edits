#ifndef __GPS_H
#define __GPS_H

/* --------------------- Include --------------------- */
#include "main.h"

/* --------------------- Define --------------------- */
#define GPS_NMEA_BUFFERSIZE                             128

#define GPS_PROTOCOL_OK                                 RESET
#define GPS_PROTOCOL_TIMEOUT                            SET
#define GPS_PROTOCOL_FAULT                              2

#define PRECISION_POLYNOMIAL                            10000000

#define GPS_FIND_USART_BUAD_TIMEOUT                     5000
#define GPS_FIND_USART_GPTXT_TIMEOUT                    2000
#define GPS_RMC_NMEA_INACTIVITY_TIMEOUT                 300
#define GPS_LOST_FIX_TIMEOUT                            240

#define RTC_LEAP_YEAR(year)                             ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define RTC_DAYS_IN_YEAR(x)                             RTC_LEAP_YEAR(x) ? 366 : 365    /**< number of days in year */
#define RTC_OFFSET_YEAR                                 1970                            /**< time begins since 1970 for unix timstamp */
#define RTC_SECONDS_PER_DAY                             86400                           /**< seconds in a day */
#define RTC_SECONDS_PER_HOUR                            3600                            /**< seconds in one hour */
#define RTC_SECONDS_PER_MINUTE                          60                              /**< seconds in one minute */

#define MOVING_THRESHOLD_SPEED                          7       // Kmh
#define GOOD_PDOP_THRESHOLD                             60
#define GPS_EARTH_RADIUS								6371
#define DISTANCE_FILTER                                 60
#define GPS_JUMP_MIN_DURATION                           60      // Seconds

#define GPS_DEGREES2RADIANS(x)	                        ((x) * (float)0.01745329251994)
#define GPS_RADIANS2DEGREES(x)	                        ((x) * (float)57.29577951308232)

#define GPS_OK                                          0
#define GPS_TIMEOUT                                     1
#define GPS_ERROR                                       2


// ----------------- Enum ----------------- //
enum
{
  GPS_NMEA_GGA = 0,
  GPS_NMEA_GLL,
  GPS_NMEA_GSA,
  GPS_NMEA_GSV,
  GPS_NMEA_RMC,
  GPS_NMEA_VTG,
  GPS_NMEA_ZDA,
  GPS_NMEA_GBS,
  GPS_NAV_STATUS,
  GPS_JAMMING_DETECTION,
};

enum
{
  GPS_NOT_FIX           = 1,
  GPS_FIX_2D,
  GPS_FIX_3D,
};

// ----------------- Structure ----------------- //
typedef struct{
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint32_t unix;
}GPS_Date_typedef;

typedef struct{
  uint32_t latitude;            /**< Gps latitude. */
  uint32_t latitude_min;
  uint32_t longitude;           /**< Gps longitude. */
  uint32_t longitude_min;
  int16_t altitude;            /**< Gps altitude. */
  uint16_t bearing;             /**< Gps bearing. */
  uint16_t pdop;                /**< Gps pdop represents gps accuracy. */
  uint16_t hdop;                /**< Gps pdop represents gps accuracy. */
  uint16_t speed_Kmh;           /**< Gps speed. */
  uint8_t sats_in_view;         /**< number of sats in view. */
  uint8_t fix_mode;             /**< GPS fix modes:1 2 3. */
  uint8_t S_N;                  // South = 0, North = 1
  uint8_t E_W;                  // East = 0, West = 1
  uint8_t jamming;              // 1: Not Jamming, 2-3 Jamming:
  GPS_Date_typedef      GPS_UTC;
}GPS_DATA_typedef;

typedef struct{
  uint8_t               chan_1;
  uint8_t               chan_2;
  uint8_t               chan_3;
  uint8_t               chan_4;
  uint8_t               chan_5;
  uint8_t               chan_6;
  uint8_t               chan_7;
  uint8_t               chan_8;
  uint8_t               chan_9;
  uint8_t               chan_10;
  uint8_t               chan_11;
  uint8_t               chan_12;
}gsa_sat_channels;

typedef struct{
  GPS_Date_typedef      time_date;
  char                  status;
  uint32_t              latitude;
  char                  S_N;
  uint32_t              longitude;
  char                  E_W;
  uint16_t              speed;
  uint16_t              cog;
  uint8_t               process_error;
}RMC_NMEA_message;

typedef struct{
  uint32_t              latitude;
  char                  S_N;
  uint32_t              longitude;
  char                  E_W;
  uint8_t               fix_indicator;
  uint8_t               satellites;
  uint16_t              HDOP;
  uint16_t              altitude;
  uint8_t               process_error;
}GGA_NMEA_message;

typedef struct{
  char                  mode_1;
  uint8_t               mode_2;
  uint16_t              PDOP;
  uint16_t              HDOP;
  uint16_t              VDOP;
  gsa_sat_channels      SV_channels;
  uint8_t               process_error;
}GSA_NMEA_message;




// ----------------- Extern ----------------- //
extern GPS_DATA_typedef         GPS_data;


extern uint8_t gps_is_fixed_for_first_time;
extern uint8_t check_location_jump;

extern uint64_t startup_latitude_average;
extern uint64_t startup_longitude_average;
extern uint8_t startup_fixed_counter;
extern uint32_t last_valid_latitude;
extern uint32_t last_valid_longitude;
extern uint16_t last_valid_altitude;
extern uint16_t last_valid_COG;
extern uint32_t last_record_latitude;
extern uint32_t last_record_longitude;
extern uint16_t last_record_COG;
extern uint32_t last_PPS_time_unix;
extern uint8_t device_has_speed;
extern uint8_t moving_status;
extern uint8_t check_updating_timeunix;

extern uint8_t gps_enter_sleep;
extern uint8_t gps_exit_sleep;
extern uint8_t gps_activity;
extern int16_t set_gps_sleep_counter;
extern uint8_t reconfigure_GPS;
extern uint8_t reconfig_gps_protection;
extern uint16_t reconfig_gps_protection_counter;

// ----------------- Prototype ----------------- //
uint32_t GPS_Update_UnixTimeStamp(GPS_Date_typedef* date);
void GPS_clear_rx_buffer(void);
void GPS_exit_Sleep(void);
void GPS_Config_Module(void);
void GPS_superloop_call(void);
uint32_t GPS_get_distance(uint32_t lat1, uint32_t long1, uint32_t lat2, uint32_t long2);
void OnePPS_EXTI_Callback(void);
void GPS_1PPS_SW_EXTI_Callback(void);

#endif  /* __GPS_H */