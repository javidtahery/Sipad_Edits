#include "dev_attachment.h"


uint8_t accel_idle_x_detection_counter = 0;
uint8_t accel_idle_y_detection_counter = 0;
uint8_t accel_idle_z_detection_counter = 0;
uint16_t accel_move_detection_counter = 0;
uint16_t accel_idle_detection_counter = 0;
uint8_t accident_detection_counter = 0;

// Accelerometer Timing Window
uint8_t accel_500ms_window_counter = 0;
int16_t window_start_x_accel = 0;
int16_t window_start_y_accel = 0;
int16_t window_start_z_accel = 0;
int16_t window_end_x_accel = 0;
int16_t window_end_y_accel = 0;
int16_t window_end_z_accel = 0;
uint16_t window_dx_accel = 0;
uint16_t window_dy_accel = 0;
uint16_t window_dz_accel = 0;

uint8_t Device_detached_G_Occur_counter;
uint8_t Device_detached_Flag = RESET;
uint8_t PRE_Device_detached_Flag = RESET;
uint8_t Time_of_PRE_Device_detached_Flag = 0;
uint8_t Time_Counter_of_new_detached_algoritm = 0;
//uint8_t temp_mode_of_detect[250] ;
//uint8_t temp_mode_of_detect_COUNT ;

int16_t Count_of_pre_detached_occure;
uint8_t timer_of_Count_of_pre_detached_occure;

uint8_t Flag_Check_data_for_half_SEC = 0;
int16_t Base_sample_axis;
int16_t New_x_sample_that_must_analyze;
int16_t New_y_sample_that_must_analyze;
int16_t New_z_sample_that_must_analyze;

uint8_t NewStart_flag = RESET;
uint8_t Device_detached_G_Occur_counter2;
uint8_t Device_detached_G_Occur_counter3;
uint8_t Device_detached_G_Occur_counter4;
uint8_t Device_detached_G_Occur_counter5;

int16_t Previous_X_axis;
int16_t Previous_Y_axis;
int16_t Previous_Z_axis;

uint8_t Time_of_car_has_been_turned_off = 0;
uint8_t Car_is_turn_off_for_more_than_4_sec = RESET;
uint8_t Last_Time_of_Device_detached_Flag_has_been_activated = 0;




void How_the_vehicle_moves(void)  //each 20ms / 50hz
{
  /* --------- Deattached Algoritm when acc is off---------- */
//  if ((IO_Digital.ignition == 0) && (Car_is_turn_off_for_more_than_4_sec == SET )) // if acc is off and vehile moves
//  {
//    if(KX023.axis_x > 100 || KX023.axis_y > 100 || KX023.axis_z > 100 || KX023.axis_x < -100 || KX023.axis_y < -100 || KX023.axis_z < -100) 
//    {
//      Device_detached_G_Occur_counter++;
//      if(Device_detached_G_Occur_counter > 50) 
//      {
//         if((PRE_Device_detached_Flag == RESET) && (Device_detached_Flag == RESET ))
//      {
//        PRE_Device_detached_Flag = SET ; //if for 1 sec device rotate in each direction more than standard
//        Time_of_PRE_Device_detached_Flag = 1; //turn on timer
//      }
//      Device_detached_G_Occur_counter=0;
//      }
//    }
//    else
//    {
//      Device_detached_G_Occur_counter = 0;
//    }
//  }
  if ((PRE_Device_detached_Flag == SET) &&(Time_of_PRE_Device_detached_Flag > 6))
  {
     if (IO_Digital.ignition == 0) //if after 5sec, acc is still off, then it must be device detached
     {
       PRE_Device_detached_Flag = RESET;
       Time_of_PRE_Device_detached_Flag = 0;            //disable incrasing time in timer 1
       Last_Time_of_Device_detached_Flag_has_been_activated = 1;        //turn on timer  //after 4 min, Device_detached_Flag turn reset again
       
       Device_detached_Flag = SET;
       Force_Produce_Event();
       Event_Flags.device_deattached = ENABLE;
     }
     else  
     {
       PRE_Device_detached_Flag = RESET;
       Time_of_PRE_Device_detached_Flag = 0;    //disable incrasing time in timer 1
       Device_detached_G_Occur_counter = 0;
     }
  }
  
  ////  Deattached Algoritm --when ACC is ON or OFF(independent on ACC state) /////
  //-------------new Device_detached algoritm-----------------//
   if(KX023.vehicle_moving == 0)
   {
     Time_Counter_of_new_detached_algoritm = 0;
     Flag_Check_data_for_half_SEC = 0;
     NewStart_flag = RESET;
     Device_detached_G_Occur_counter2 = 0;
     Device_detached_G_Occur_counter3 = 0;
     Device_detached_G_Occur_counter4 = 0;
     Device_detached_G_Occur_counter5 = 0;

   }
 if(KX023.vehicle_moving != 0)
  {
    
      ////  ----------  method 1  -----------  ////
    Time_Counter_of_new_detached_algoritm++ ;
    if(Time_Counter_of_new_detached_algoritm > 25) //check every 0.5 sec
    {
      Time_Counter_of_new_detached_algoritm = 0;
      if(((abs(Previous_X_axis - (int16_t)KX023.axis_x)) > 400)  && (Flag_Check_data_for_half_SEC == 0))
      {
        Flag_Check_data_for_half_SEC = 1 ; 
        Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
      }
      else if (((abs(Previous_Y_axis - (int16_t)KX023.axis_y)) > 400)  && (Flag_Check_data_for_half_SEC == 0))
      {
        Flag_Check_data_for_half_SEC = 2 ; 
        Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
      }
      else if (((abs(Previous_Z_axis - (int16_t)KX023.axis_z)) > 400) && (Flag_Check_data_for_half_SEC == 0))
      {
        Flag_Check_data_for_half_SEC = 3 ; 
        Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
      }    
      else if (((abs(Previous_X_axis - (int16_t)KX023.axis_x)) > 150) &&((abs(Previous_Y_axis - (int16_t)KX023.axis_y)) > 150) && (Flag_Check_data_for_half_SEC == 0 ))
      {
        Flag_Check_data_for_half_SEC = 4 ;  // X & Y 
        Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
      }    
      else if (((abs(Previous_X_axis - (int16_t)KX023.axis_x)) > 150) &&((abs(Previous_Z_axis - (int16_t)KX023.axis_z)) > 150) && (Flag_Check_data_for_half_SEC == 0 ))
      {
        Flag_Check_data_for_half_SEC = 5 ;  // X & Z
        Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
      }  
      else if (((abs(Previous_Z_axis - (int16_t)KX023.axis_z)) > 150) &&((abs(Previous_Y_axis - (int16_t)KX023.axis_y)) > 150) && (Flag_Check_data_for_half_SEC == 0 ))
      {
        Flag_Check_data_for_half_SEC = 6 ;  // Z & Y 
        Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
      }       
      else if (((abs(Previous_X_axis - (int16_t)KX023.axis_x)) > 100) &&((abs(Previous_Z_axis - (int16_t)KX023.axis_z)) > 100) &&((abs(Previous_Y_axis - (int16_t)KX023.axis_y)) > 100) && (Flag_Check_data_for_half_SEC == 0 ))
      {
        Flag_Check_data_for_half_SEC = 7 ;  // X & Z & Y 
        Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
      }            
      Previous_X_axis = (int16_t)KX023.axis_x;     
      Previous_Y_axis = (int16_t)KX023.axis_y;
      Previous_Z_axis = (int16_t)KX023.axis_z;
    }
    
   if( Flag_Check_data_for_half_SEC != 0)
     Check_data_for_half_SEC(Flag_Check_data_for_half_SEC);
   
/////    -----   method 2 ----------- //
   if((KX023.axis_x > 900) || (KX023.axis_x < -900 )||(KX023.axis_y > 900) || (KX023.axis_y < -900 )||(KX023.axis_z > 900) || (KX023.axis_z < -900 ))
   {
     Device_detached_G_Occur_counter5++;
     if(Device_detached_G_Occur_counter5 > 75) // 1.5  Secs
     {
//       temp_mode_of_detect[temp_mode_of_detect_COUNT]=8;
//       temp_mode_of_detect_COUNT++;
//       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;

       if(Device_detached_Flag == RESET )
       {
         Device_detached_Flag = SET;
         Force_Produce_Event();
         Event_Flags.device_deattached = ENABLE;
         Last_Time_of_Device_detached_Flag_has_been_activated = 1;//turn on timer  //after 4 min, Device_detached_Flag turn reset again
       }
       Device_detached_G_Occur_counter5 = 0; 
     }
   }
   else if(Device_detached_G_Occur_counter5 != 0)
   {
     Device_detached_G_Occur_counter5 = 0;
   }
  }
}



void Check_data_for_half_SEC(uint8_t MODE)
{
   switch (MODE)
    {
    case 1: //X axis  
      if (NewStart_flag==RESET)
      {
        NewStart_flag = SET;
        Base_sample_axis = Previous_X_axis;
        New_x_sample_that_must_analyze = (int16_t)KX023.axis_x;
      }
      if (((New_x_sample_that_must_analyze >= 0)&& ((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) < 50))||((New_x_sample_that_must_analyze < 0)&&((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) > -50)))//if data is positive if axis is also high
       {
        Device_detached_G_Occur_counter2++;
        if(Device_detached_G_Occur_counter2 > 75) //if after stright 1 sec_ axis dose not come down
        {
          Count_of_pre_detached_occure++;
          //       temp_mode_of_detect[temp_mode_of_detect_COUNT]=1;
          //       temp_mode_of_detect_COUNT++;
          //       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;
          
          NewStart_flag = RESET;
          Flag_Check_data_for_half_SEC =0;      
          Device_detached_G_Occur_counter2 = 0;
        }
      }
      else if(Device_detached_G_Occur_counter2 != 0)
      {
        Device_detached_G_Occur_counter2 = 0;
        NewStart_flag = RESET;
        Flag_Check_data_for_half_SEC =0;
      }
      break;
    
    case 2:
      if (NewStart_flag==RESET)
      {
        NewStart_flag = SET;
        Base_sample_axis = Previous_Y_axis;
        New_y_sample_that_must_analyze = (int16_t)KX023.axis_y;
      }
      if (((New_y_sample_that_must_analyze >= 0)&& ((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) < 50))||((New_y_sample_that_must_analyze < 0)&&((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) > -50)))//if data is positive if axis is also high
      {
        Device_detached_G_Occur_counter2++;
        if(Device_detached_G_Occur_counter2 > 75) //if after stright 1 sec_ axis dose not come down
        {
          Count_of_pre_detached_occure++;          
          //       temp_mode_of_detect[temp_mode_of_detect_COUNT]=2;
          //       temp_mode_of_detect_COUNT++;
          //       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;
          
          NewStart_flag = RESET;
          Flag_Check_data_for_half_SEC =0;       
          Device_detached_G_Occur_counter2 = 0;
        }
      }
      else if(Device_detached_G_Occur_counter2 != 0)
      {
        Device_detached_G_Occur_counter2 = 0;
        NewStart_flag = RESET;
        Flag_Check_data_for_half_SEC =0;
      }
      break;

    case 3:
      if (NewStart_flag==RESET)
      {
        NewStart_flag = SET;
        Base_sample_axis = Previous_Z_axis;
        New_z_sample_that_must_analyze = (int16_t)KX023.axis_z;
      }
      if (((New_z_sample_that_must_analyze >= 0)&& ((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) < 50))||((New_z_sample_that_must_analyze < 0)&&((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) > -50)))//if data is positive if axis is also high
       {
        Device_detached_G_Occur_counter2++;
        if(Device_detached_G_Occur_counter2 > 75) //if after stright 1.5 sec_ axis dose not come down
        {
          Count_of_pre_detached_occure++;          
          
//       temp_mode_of_detect[temp_mode_of_detect_COUNT]=3;
//       temp_mode_of_detect_COUNT++;
//       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;

          NewStart_flag = RESET;
          Flag_Check_data_for_half_SEC =0;        
          Device_detached_G_Occur_counter2 = 0;
        }
      }
      else if(Device_detached_G_Occur_counter2 != 0)
      {
        Device_detached_G_Occur_counter2 = 0;
        NewStart_flag = RESET;
        Flag_Check_data_for_half_SEC =0;
      }
      break;
      
    case 4: // X&Y
      if (NewStart_flag==RESET)
      {
        NewStart_flag = SET;
       // Base_sample_axis = Previous_Z_axis;
        New_x_sample_that_must_analyze = (int16_t)KX023.axis_x;
        New_y_sample_that_must_analyze = (int16_t)KX023.axis_y;
      }
      if ((((New_x_sample_that_must_analyze >= 0)&& ((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) < 50))
          ||((New_x_sample_that_must_analyze < 0)&&((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) > -50)))
          && (((New_y_sample_that_must_analyze >= 0)&& ((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) < 50))
          ||((New_y_sample_that_must_analyze < 0)&&((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) > -50))))//if data is positive if axis is also high
       {
        Device_detached_G_Occur_counter3++;
        if(Device_detached_G_Occur_counter3 > 75) //if after stright 1.5 sec_ axis dose not come down
        {
          Count_of_pre_detached_occure++;          
//       temp_mode_of_detect[temp_mode_of_detect_COUNT] = 4;
//       temp_mode_of_detect_COUNT++;
//       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;

          NewStart_flag = RESET;
          Flag_Check_data_for_half_SEC =0;        
          Device_detached_G_Occur_counter3 = 0;
        }
      }
      else if(Device_detached_G_Occur_counter3 != 0)
      {
        Device_detached_G_Occur_counter3 = 0;
        NewStart_flag = RESET;
        Flag_Check_data_for_half_SEC =0;
      }
      break;  
      
    case 5: //X&Z
     if (NewStart_flag==RESET)
      {
        NewStart_flag = SET;
      //  Base_sample_axis = Previous_Z_axis;
        New_x_sample_that_must_analyze = (int16_t)KX023.axis_x;
        New_z_sample_that_must_analyze = (int16_t)KX023.axis_z;
      }
      if ((((New_x_sample_that_must_analyze >= 0)&& ((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) < 50))
          ||((New_x_sample_that_must_analyze < 0)&&((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) > -50)))
          && (((New_z_sample_that_must_analyze >= 0)&& ((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) < 50))
          ||((New_z_sample_that_must_analyze < 0)&&((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) > -50))))//if data is positive if axis is also high
       {
        Device_detached_G_Occur_counter3++;
        if(Device_detached_G_Occur_counter3 > 75) //if after stright 1.5 sec_ axis dose not come down
        {
          Count_of_pre_detached_occure++;          
//       temp_mode_of_detect[temp_mode_of_detect_COUNT]=5;
//       temp_mode_of_detect_COUNT++;
//       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;

          NewStart_flag = RESET;
          Flag_Check_data_for_half_SEC =0;        
          Device_detached_G_Occur_counter3 = 0;
        }
      }
      else if(Device_detached_G_Occur_counter3 != 0)
      {
        Device_detached_G_Occur_counter3 = 0;
        NewStart_flag = RESET;
        Flag_Check_data_for_half_SEC =0;
      }
      break;   
      
    case 6: // Z&Y

     if (NewStart_flag==RESET)
      {
        NewStart_flag = SET;
        Base_sample_axis = Previous_Z_axis;
        New_z_sample_that_must_analyze = (int16_t)KX023.axis_z;
        New_y_sample_that_must_analyze = (int16_t)KX023.axis_y;
      }
      if ((((New_z_sample_that_must_analyze >= 0)&& ((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) < 50))
          ||((New_z_sample_that_must_analyze < 0)&&((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) > -50)))
          && (((New_y_sample_that_must_analyze >= 0)&& ((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) < 50))
          ||((New_y_sample_that_must_analyze < 0)&&((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) > -50))))//if data is positive if axis is also high
       {
        Device_detached_G_Occur_counter3++;
        if(Device_detached_G_Occur_counter3 > 75) //if after stright 1.5 sec_ axis dose not come down
        {
          Count_of_pre_detached_occure++;          
//       temp_mode_of_detect[temp_mode_of_detect_COUNT]=6;
//       temp_mode_of_detect_COUNT++;
//       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;

          NewStart_flag = RESET;
          Flag_Check_data_for_half_SEC =0;        
          Device_detached_G_Occur_counter3 = 0;
        }
      }
      else if(Device_detached_G_Occur_counter3 != 0)
      {
        Device_detached_G_Occur_counter3 = 0;
        NewStart_flag = RESET;
        Flag_Check_data_for_half_SEC =0;
      }
      break;     

    case 7: // X & Z & Y

     if (NewStart_flag==RESET)
      {
        NewStart_flag = SET;
        Base_sample_axis = Previous_Z_axis;
        New_z_sample_that_must_analyze = (int16_t)KX023.axis_z;
        New_y_sample_that_must_analyze = (int16_t)KX023.axis_y;
        New_x_sample_that_must_analyze = (int16_t)KX023.axis_x;
      }
      if ((((New_z_sample_that_must_analyze >= 0)&& ((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) < 40))
          ||((New_z_sample_that_must_analyze < 0)&&((New_z_sample_that_must_analyze - (int16_t)KX023.axis_z) > -40)))
          && (((New_y_sample_that_must_analyze >= 0)&& ((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) < 40))
          ||((New_y_sample_that_must_analyze < 0)&&((New_y_sample_that_must_analyze - (int16_t)KX023.axis_y) > -40)))
          && (((New_x_sample_that_must_analyze >= 0)&& ((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) < 40))
          ||((New_x_sample_that_must_analyze < 0)&&((New_x_sample_that_must_analyze - (int16_t)KX023.axis_x) > -40))))//if data is positive if axis is also high
       {
        Device_detached_G_Occur_counter4++;
        if(Device_detached_G_Occur_counter4 > 50) //if after stright 1.5 sec_ axis dose not come down
        {
          Count_of_pre_detached_occure++;          
//       temp_mode_of_detect[temp_mode_of_detect_COUNT]=7;
//       temp_mode_of_detect_COUNT++;
//       if(temp_mode_of_detect_COUNT > 250) temp_mode_of_detect_COUNT = 0;
          
          NewStart_flag = RESET;
          Flag_Check_data_for_half_SEC =0;        
          Device_detached_G_Occur_counter4 = 0;
        }
      }
      else if(Device_detached_G_Occur_counter4 != 0)
      {
        Device_detached_G_Occur_counter4 = 0;
        NewStart_flag = RESET;
        Flag_Check_data_for_half_SEC =0;
      }
      break;        
    }
}

void Accel_movement_check(void)   
{
  if(++accel_500ms_window_counter > 12)
  {
    window_end_x_accel = (int16_t)KX023.axis_x;
    window_end_y_accel = (int16_t)KX023.axis_y;
    window_end_z_accel = (int16_t)KX023.axis_z;
    
    window_dx_accel = abs(window_end_x_accel - window_start_x_accel);
    window_dy_accel = abs(window_end_y_accel - window_start_y_accel);
    window_dz_accel = abs(window_end_z_accel - window_start_z_accel);
    
    accel_500ms_window_counter = 0;
    window_start_x_accel = window_end_x_accel;
    window_start_y_accel = window_end_y_accel;
    window_start_z_accel = window_end_z_accel;
    
    // X Axis
    if(window_dx_accel >= setting.movement_acceleration)
    {
      accel_idle_x_detection_counter = 0;
      KX023.movement_x_status = SET;
    }
    else
      accel_idle_x_detection_counter++;
    
    if(accel_idle_x_detection_counter >= 3)
    {
      if(KX023.movement_x_status == SET)
        KX023.movement_x_status = RESET;
    }
    
    // Y Axis
    if(window_dy_accel >= setting.movement_acceleration)
    {
      accel_idle_y_detection_counter = 0;
      KX023.movement_y_status = SET;
    }
    else
      accel_idle_y_detection_counter++;
    
    if(accel_idle_y_detection_counter >= 3)
    {
      if(KX023.movement_y_status == SET)
        KX023.movement_y_status = RESET;
    }
    
    // Z Axis
    if(window_dz_accel >= setting.movement_acceleration)
    {
      accel_idle_z_detection_counter = 0;
      KX023.movement_z_status = SET;
    }
    else
      accel_idle_z_detection_counter++;
    
    if(accel_idle_z_detection_counter >= 3)
    {
      if(KX023.movement_z_status == SET)
        KX023.movement_z_status = RESET;
    }
    
    // Detection
    if(KX023.movement_x_status == SET || KX023.movement_y_status == SET || KX023.movement_z_status == SET)
    {
      accel_idle_detection_counter = 0;
      if( (++accel_move_detection_counter) > 2)
      {
        accel_move_detection_counter = 0;
        if(KX023.vehicle_moving == RESET)
        {
          KX023.vehicle_moving = SET;
        }
      }
    }
    else
    {
      accel_move_detection_counter = 0;
      if( (++accel_idle_detection_counter) > 7)
      {
        accel_idle_detection_counter = 0;
        if(KX023.vehicle_moving == SET)
        {
          KX023.vehicle_moving = RESET;
        }
      }
    }   
  }
  
  // -----------Accident Detection---------------//
  if(abs((int16_t)KX023.axis_x) >= ACCIDENT_DETECTION_G_THRESHOLD
     || abs((int16_t)KX023.axis_y) >= ACCIDENT_DETECTION_G_THRESHOLD
       || abs((int16_t)KX023.axis_z) >= ACCIDENT_DETECTION_G_THRESHOLD) 
  {
    accident_detection_counter++;
    if(accident_detection_counter > 15 && repetition_accident_detection_counter == 0)   // 0.3 Sec
    {
      Force_Produce_Event();
      Event_Flags.g_sensor_activated = ENABLE;
      repetition_accident_detection_counter = REPETITION_ACCIDENT_EVENT_THRESHOLD;
    }
  }
  else
  {
    accident_detection_counter = 0;
  }
}