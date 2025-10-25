#ifndef __DEV_ATTACHMENT_H
#define __DEV_ATTACHMENT_H

/* ---------------- Include ---------------- */
#include "main.h"

/* ---------------- Define ---------------- */
#define ACCIDENT_DETECTION_G_THRESHOLD          1500


/* ---------------- Extern ---------------- */
extern uint8_t Time_of_car_has_been_turned_off;
extern uint8_t Car_is_turn_off_for_more_than_4_sec;
extern uint8_t Last_Time_of_Device_detached_Flag_has_been_activated;
extern uint8_t Time_of_PRE_Device_detached_Flag;
extern int16_t Count_of_pre_detached_occure;
extern uint8_t timer_of_Count_of_pre_detached_occure;
extern uint8_t Device_detached_Flag;
extern int16_t Previous_X_axis;     
extern int16_t Previous_Y_axis;
extern int16_t Previous_Z_axis;
extern uint8_t Flag_Check_data_for_half_SEC;
extern uint8_t NewStart_flag;
extern int16_t window_start_x_accel;
extern int16_t window_start_y_accel;
extern int16_t window_start_z_accel;

//extern uint8_t temp_mode_of_detect[250];
//extern uint8_t temp_mode_of_detect_COUNT;


/* ---------------- Prototype ---------------- */
void How_the_vehicle_moves(void);
void Check_data_for_half_SEC(uint8_t MODE);
void Accel_movement_check(void);

#endif