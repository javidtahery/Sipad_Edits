#include "system.h"


sys_err_typedef         system_error = {0};

uint8_t system_has_error = RESET;
uint8_t recover_Accel_error = RESET;
uint16_t recover_accel_error_counter = 0;
uint8_t system_pause_change_state_counter = 0;
system_states system_current_state = SYSTEM_INITIAL;
system_states system_new_state = SYSTEM_INITIAL;


extern uint8_t firmware_downloading_process;


void System_check_errors(void)
{
  uint8_t error_state = RESET;
  
  // Accelerometer
  if(system_error.accel_error == SET)
    error_state = RESET;
  
  // GPS
  if(system_error.gps_error == SET)
    error_state = RESET;
  
  system_has_error = error_state;
}


void System_change_state(system_states new_state)
{
  if(new_state != system_current_state)
  {
    if(system_pause_change_state_counter == 0)
    {
      system_pause_change_state_counter = 3;
      system_new_state = new_state;
    }
  }
}


void System_execute_change_state(void)
{
  if(system_current_state != system_new_state)
  {
    system_current_state = system_new_state;
    
    switch(system_current_state)
    {
    case SYSTEM_RUNNING:
      {
        if(Time_of_car_has_been_turned_off > 0)
          Time_of_car_has_been_turned_off = 0;
        
        Car_is_turn_off_for_more_than_4_sec = RESET;        //for Deattached algoritm
        
        // Device Attachment
        //Accel_check_device_attachment_by_angle();
        
        // Enable record generation
        create_normal_event_counter = setting.data_sample_rate_time;
        
        // GPS Sleep
        if(gps_activity == RESET)
          gps_exit_sleep = SET;
        gps_enter_sleep = RESET;
        set_gps_sleep_counter = 0;
        
        // Firmware upgrade
        if(firmware_downloading_process == SET)
        {
          GSM_status = GSM_IDLE;
          GSM_Parameters.stage = SIM_STAGE_MODULE_OFF;
          GSM_Parameters.stage_action = SIM_SEND_REQ;
          
          firmware_downloading_process = RESET;
          send_request_to_server = RESET;
          server_requests.get_firware_chunk = RESET;
          sending_data_is_in_progress = RESET;
          logined_to_the_server = RESET;
        }
        
        // Reopen the server socket	
        if(gps_is_fixed_for_first_time == SET)      // For system startup protection	
        {
          if(GSM_status == GSM_HTTP_PROCESS)
          {
            // Change LED Pattern	
            LED_change_mode(GSM_LED, LED_ON_TWICE_EACH_1SEC);
          }
          else if(GSM_status == GSM_SENDING_PACKET ||  GSM_status == GSM_SENDING_HB)
          {
            // Change LED Pattern	
            LED_change_mode(GSM_LED, LED_ON_STEADY);
          }
          else if(GSM_status != GSM_HANDLE_SMS)
          {
            GSM_status = GSM_IDLE;	
            logined_to_the_server = RESET;	
            sending_data_is_in_progress = RESET;	
            
            GSM_Parameters.stage = SIM_STAGE_CLOSE_SOCKET;	
            GSM_Parameters.stage_action = SIM_SEND_REQ;	
            GSM_Parameters.prev_stage = 0;	
            GSM_Parameters.next_stage = 0;	
            GSM_Parameters.number_of_retries_command = 0;
            
            // Change LED Pattern	
            LED_change_mode(GSM_LED, LED_ON_PULSE_EACH_3SEC);
          }
        }
    	
        // Reset low battery detection flag
        if(low_battery_detection == SET && repetition_low_battery_counter == 0)
          low_battery_detection = RESET;
        
        break;
      }
    case SYSTEM_SLEEP:
      {
        if(system_error.accel_error == RESET)
        {
          disable_accel_timer = SET;
          KX023_startup_calibration();
          disable_accel_timer = RESET;
        }
        
        // Device Attachment
        //Accel_save_current_data_in_flash();
        Time_of_car_has_been_turned_off = 1;                // Turn on 1 sec timer for detached algoritm
        
        // Disable normal event generating
        create_normal_event_counter = 0;
        
        // GPS
        set_gps_sleep_counter = 1;
        
        // Save server OK result
        save_server_ok();
        
        break;
      }
    case SYSTEM_ON_BATTERY:
      {
        create_normal_event_counter = setting.data_sample_rate_time;
        Force_Produce_Event();
        Event_Flags.main_pwr_disconnected = SET;
        
        // GPS Sleep
        if(gps_activity == RESET)
          gps_exit_sleep = SET;
        gps_enter_sleep = RESET;
        set_gps_sleep_counter = 0;
        
        // Reopen the server socket
        if(gps_is_fixed_for_first_time == SET)      // For startup process
        {
          GSM_status = GSM_IDLE;
          sending_data_is_in_progress = RESET;
          
          GSM_Parameters.stage = SIM_STAGE_CLOSE_SOCKET;
          GSM_Parameters.stage_action = SIM_SEND_REQ;
          GSM_Parameters.prev_stage = 0;
          GSM_Parameters.next_stage = 0;
          GSM_Parameters.number_of_retries_command = 0;
        }
        
        // Save server OK result
        save_server_ok();
        
        break;
      }
    }
  }
}


void System_check_state(void)
{
  if(system_pause_change_state_counter == 0)
  {
    System_execute_change_state();
  }
  
  if(IO_Digital.vcc_digital == SET)
  {
    if(IO_Digital.ignition == SET)
      System_change_state(SYSTEM_RUNNING);
    else
      System_change_state(SYSTEM_SLEEP);
  }
  else
  {
    System_change_state(SYSTEM_ON_BATTERY);
  }
  
  
}
