
#include "System_Timers.h"

uint8_t ten_sec_timer = 0;
uint32_t system_uptime = 0;

uint32_t counter_x = 0;
uint32_t counter_y = 0;
uint8_t disable_accel_timer = SET;

uint16_t last5_seconds_speed[5] = {0};
uint32_t average_speed = 0;

// Odo meter Variables
#ifdef ENABLE_ODOMETER
uint8_t  ODO_20ms_overflow;
uint8_t  ODO_200_ms_overflow;
uint16_t pervious_TCNT1;
uint8_t  count_of_ignoring_tcnt;
uint16_t totall_TCNT1;
uint16_t current_tcnt;
uint8_t  ODO_meter_need_update ;
float Current_ODO_Speed = 0;
#endif



void Odo_Timer1_Init(void)
{
  GPIOA->MODER = (GPIOA->MODER & (~GPIO_MODER_MODE12) | GPIO_MODER_MODE12_1);
  GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED12 ;                                     // set speed on very fast
  GPIOA->PUPDR = (GPIOA->PUPDR & (~GPIO_PUPDR_PUPD12)) | GPIO_PUPDR_PUPD12_0;   // Pulled UP         
  GPIOA->AFR[1] = (GPIOA->AFR[1] & 0xFFF0FFFF) | 0x00020000;                    // set ETR as alternative function
  
  // Reset the Timer 1
  RCC->APBRSTR2 |= RCC_APBRSTR2_TIM1RST;
  RCC->APBRSTR2 &= ~RCC_APBRSTR2_TIM1RST;
  
  // Enable Timers clock
  RCC->APBENR2 |= RCC_APBENR2_TIM1EN ;
  
  TIM1->SMCR |= TIM_SMCR_ETP; // Down edge
  TIM1->SMCR |= TIM_SMCR_ECE; // Enable external clock mode 2
  
  TIM1->CR1 |= TIM_CR1_CEN; // // Start timer to count
  
  if( (TIM1->SR & TIM_SR_UIF) == TIM_SR_UIF )
    TIM1->SR = 0;
}


void OneSec_Timer_Init(void)
{
  // Reset the Timer 6
  RCC->APBRSTR1 |= RCC_APBRSTR1_TIM6RST;
  RCC->APBRSTR1 &= ~RCC_APBRSTR1_TIM6RST;
  
  TIM6->PSC = 3199;                     // Set Prescaler to 3200 --> Timer CLK = 20KHz (64MHz / 3200)
  TIM6->ARR = 19999;                    // Tick Every 1 Second
  TIM6->DIER |= TIM_DIER_UIE;           // Enable the Interrupt on update
  
  NVIC_EnableIRQ(TIM6_IRQn);
  NVIC_ClearPendingIRQ(TIM6_IRQn);
  NVIC_SetPriority(TIM6_IRQn, 3);
  
  TIM6->CR1 |= TIM_CR1_CEN;             // Start timer to count
  
  if( (TIM6->SR & TIM_SR_UIF) == TIM_SR_UIF )
    TIM6->SR = 0;
}


// 20 milisecond
void Fifty_Hz_Timer_Init(void)
{
  // Reset the Timer 7
  RCC->APBRSTR1 |= RCC_APBRSTR1_TIM7RST;
  RCC->APBRSTR1 &= ~RCC_APBRSTR1_TIM7RST;
  
  TIM7->PSC = 3199;                     // Set Prescaler to 3200 --> Timer CLK = 20KHz (64MHz / 3200)
  TIM7->ARR = 399;                      // Tick Every 20 Mili Seconds (50 HZ)
  TIM7->DIER |= TIM_DIER_UIE;           // Enable the Interrupt on update
  
  NVIC_EnableIRQ(TIM7_IRQn);
  NVIC_ClearPendingIRQ(TIM7_IRQn);
  NVIC_SetPriority(TIM7_IRQn, 2);
  
  TIM7->CR1 |= TIM_CR1_CEN;             // Start timer to count
  
  if( (TIM7->SR & TIM_SR_UIF) == TIM_SR_UIF )
    TIM7->SR = 0;
}


// 40 mili second
void Twentyfive_Hz_Timer_Init(void)
{
  // Reset the Timer 3
  RCC->APBRSTR1 |= RCC_APBRSTR1_TIM3RST;
  RCC->APBRSTR1 &= ~RCC_APBRSTR1_TIM3RST;
  
  TIM3->PSC = 3199;                     // Set Prescaler to 3200 --> Timer CLK = 20KHz (64MHz / 3200)
  TIM3->ARR = 799;                      // Tick Every 40 Mili Seconds (25 HZ)
  TIM3->DIER |= TIM_DIER_UIE;           // Enable the Interrupt on update
  
  NVIC_EnableIRQ(TIM3_IRQn);
  NVIC_ClearPendingIRQ(TIM3_IRQn);
  NVIC_SetPriority(TIM3_IRQn, 3);
  
  TIM3->CR1 |= TIM_CR1_CEN;             // Start timer to count
  
  if( (TIM3->SR & TIM_SR_UIF) == TIM_SR_UIF )
    TIM3->SR = 0;
}


void OneSec_handler(void)
{
  if( (TIM6->SR & TIM_SR_UIF) == TIM_SR_UIF )
  {
    timers_does_not_work = RESET;
    
    if(system_is_initiating == SET)
      IWDG_ReloadCounter();
    
    system_Unixtime++;
    system_uptime++;
    
    // Reset Every day
    if(system_uptime > RTC_SECONDS_PER_DAY)
    {
      reset_count = 0;
      vehicle_total_movement = 0;
      total_generated_record = 0;
      __NVIC_SystemReset();
    }
    
    /* -------- Device Atachment -------- */
    // After 5 seconds system is allowed to detect attachment
    if(Time_of_car_has_been_turned_off != 0) 
    {
      Time_of_car_has_been_turned_off++;
      if(Time_of_car_has_been_turned_off > 4) 
      {
        Time_of_car_has_been_turned_off = 0;
        Car_is_turn_off_for_more_than_4_sec = SET;
      }
    }
    
    if(Time_of_PRE_Device_detached_Flag != 0)
      Time_of_PRE_Device_detached_Flag++; 
    
    if(Count_of_pre_detached_occure != 0)
      timer_of_Count_of_pre_detached_occure++;
    if(timer_of_Count_of_pre_detached_occure > 10) 
    {
      if(Count_of_pre_detached_occure >= 2)
      {
        if(Device_detached_Flag == RESET )
        {
          Device_detached_Flag = SET;
          Force_Produce_Event();
          Event_Flags.device_deattached = ENABLE;
          Last_Time_of_Device_detached_Flag_has_been_activated = 1;//turn on timer  //after 4 min, Device_detached_Flag turn reset again
        }
      }
      
      timer_of_Count_of_pre_detached_occure=0;
      Count_of_pre_detached_occure = 0;
    }
    
    if(Last_Time_of_Device_detached_Flag_has_been_activated != 0) 
    {
      Last_Time_of_Device_detached_Flag_has_been_activated++; 
      if (Last_Time_of_Device_detached_Flag_has_been_activated > 250) // after 4 min , reset the detached flag
      {
        Last_Time_of_Device_detached_Flag_has_been_activated = 0 ;
        
        if(system_error.accel_error == RESET)
        {
          disable_accel_timer = SET;
          KX023_startup_calibration();
          disable_accel_timer = RESET;
        }
        
        Device_detached_Flag = RESET; 
        
        Previous_X_axis = 0;     
        Previous_Y_axis = 0;
        Previous_Z_axis = 0;
        Flag_Check_data_for_half_SEC = 0;
        NewStart_flag = RESET;    
        
        window_start_x_accel = 0 ;
        window_start_y_accel = 0 ;
        window_start_z_accel = 0 ;
      }
    }
    
    /* -------------- System Counters -------------- */
    if(system_pause_change_state_counter > 0)
      system_pause_change_state_counter--;
    
    // Debouncing Input Power
    if(mainpower_connection_counter > 0)
    {
      mainpower_connection_counter--;
    }
    if(mainpower_disconnection_counter > 0)
    {
      mainpower_disconnection_counter--;
    }
    
    /* -------------- GSM Commands Timer -------------- */
    if(GSM_HALT_Timer < GSM_TIMER_DEFAULT)
    {
      GSM_HALT_Timer--;
      if(GSM_HALT_Timer <= 0)
        GSM_HALT_Timer = GSM_TIMER_DEFAULT;
    }
    
    // GSM Registery
    if(GSM_Creg_two_hour_counter > 0)
    {
      if(++GSM_Creg_two_hour_counter > 7200)
      {
        GSM_Creg_two_hour_counter = 0;
        GSM_network_search_counter = 0;
        disable_any_gsm_activity = RESET;
      }
    }
    
    // HTTP
    if(halt_http_process_counter > 0)
    {
      halt_http_process_counter--;
      if(halt_http_process_counter == 0)
      {
        halt_http_process = RESET;
        number_of_http_process_run = 0;
      }
    }
    
    // Simcard Counter
    if(GSM_check_simcard_counter < 15)
      GSM_check_simcard_counter++;
    else
    {
      GSM_check_simcard_counter = 0;
      if(GSM_status == GSM_IDLE && GSM_Parameters.simcard_available == 1)
      {
        send_command_to_GSM = SET;
        GSM_cmd_execution_code = SIM_STAGE_QUERY_SIMCARD;
      }
    }
    /* -------------- Heartbeat Production Counter -------------- */
//    if(create_Heartbeat_counter != 0)
//    {
//      if(--create_Heartbeat_counter <= 0)
//      {
//        create_Heartbeat_counter = setting.heartbeat_rate_time * 60;
//        create_Heartbeat = ENABLE;
//        
//        if(system_current_state != SYSTEM_RUNNING)
//        {
//          logined_to_the_server = RESET;
//        }
//      }
//    }
    /* -------------- Data send rate Counter -------------- */
    if(data_send_rate_counter != 0)
    {
      if(--data_send_rate_counter <= 0)
      {
        data_send_rate_counter = setting.data_send_rate_time;
        if(FCB_profile.s1_nsend_count > 0)
          send_data_less_than_10_records = SET;
      }
    }
    
    /* -------------- API Interval Counter -------------- */
    if(api_interval_counter != 0)
    {
      if(--api_interval_counter <= 0)
      {
        api_interval_counter = setting.api_interval_time * 60;
        if(setting.api_interval_disable == RESET)
        {
          HTTP_Params.send_interval = SET;
          enable_http_process = SET;
        }
      }
    }
    
    /* -------------- Normal Event Production Counter -------------- */
    if(create_normal_event_counter != 0)
    {
      if(--create_normal_event_counter <= 0)
      {
        create_normal_event_counter = setting.data_sample_rate_time;
        create_normal_event = ENABLE;
      }
    }
    /* -------------- Event Production Enable Timer -------------- */
    if(halt_event_production_counter < HALT_EVENT_PRODUCTION_TIMER_DEFAULT)
    {
      if(halt_event_production_counter == 0)
      {
        halt_event_production_counter = HALT_EVENT_PRODUCTION_TIMER_DEFAULT;
//        create_Heartbeat_counter = setting.heartbeat_rate_time * 60;
        data_send_rate_counter = setting.data_send_rate_time;
        api_interval_counter = setting.api_interval_time * 60;
      }
      else
      {
        halt_event_production_counter--;
      }
    }
    /* -------------- Update System Unix Counter -------------- */
    if( ten_sec_timer++ >= 10 )
    {
      ten_sec_timer = 0;
      check_updating_timeunix = SET;
    }
    /* -------------- Detect GPS not fix -------------- */
    uint32_t time_diff = abs(system_Unixtime - last_PPS_time_unix);
    if( time_diff > GPS_1PPS_TIMEOUT )
    {
      if(system_current_state == SYSTEM_RUNNING)
      {
        if(GPS_data.fix_mode == GPS_FIX_2D || GPS_data.fix_mode == GPS_FIX_3D || GPS_data.speed_Kmh != 0)
        {
          GPS_data.fix_mode = GPS_NOT_FIX;
          
          startup_fixed_counter = 0;
          startup_latitude_average = 0;
          startup_longitude_average = 0;
          
          GPS_data.speed_Kmh = 0;
          GPS_data.sats_in_view = 0;
          device_has_speed = 0;
          check_location_jump = SET;
          
          if(++gps_notfix_event_counter > 30)
          {
            gps_notfix_event_counter = 0;
            
            // Generating Event
            Force_Produce_Event();
            Event_Flags.gnss_lost_signal = SET;
          }
        }
        
        // Change LED Pattern
        LED_change_mode(GPS_LED, LED_ON_BLINK_500MS);
        
        
        if(time_diff > GPS_LOST_FIX_TIMEOUT && reconfig_gps_protection == RESET)
        {
          reconfigure_GPS = SET;
          reconfig_gps_protection = SET;
          reconfig_gps_protection_counter = 300;          // 6 minutes
        }
      }
    }
    
    if(reconfig_gps_protection_counter != 0)
    {
      reconfig_gps_protection_counter--;
      if(reconfig_gps_protection_counter == 0)
        reconfig_gps_protection = RESET;
    }
    
    /* -------------- Set GPS Sleep -------------- */
    if(set_gps_sleep_counter > 0 && gps_is_fixed_for_first_time == SET)
    {
      if( ++set_gps_sleep_counter > GPS_SLEEP_TIMEOUT)
      {
        gps_enter_sleep = SET;
        gps_exit_sleep = RESET;
        set_gps_sleep_counter = 0;
      }
    }
    
    /* -------------- Idle Detection  -------------- */
    if(check_vehicle_idle == SET)
    {
      if(++idle_started_counter > VEHICLE_IDLE_TIME_THRESHOLD)
      {
        check_vehicle_idle = RESET;
        vehicle_idle_state = SET;
        idle_started_counter = 0;
        
        Force_Produce_Event();
        Event_Flags.idle_begin = ENABLE;
      }
    }
    
    /* -------------- Repetition Counters  -------------- */
#ifdef ENABLE_ODOMETER
    if(repetition_odometer_gnss_speed_difference_counter > 0)
      repetition_odometer_gnss_speed_difference_counter--;
#endif
    if(repetition_low_battery_counter > 0)
      repetition_low_battery_counter--;
    if(repetition_accident_detection_counter > 0)
      repetition_accident_detection_counter--;
    
    /* -------------- Error solve counter  -------------- */
    if(recover_accel_error_counter > 0)
    {
      if(++recover_accel_error_counter > ACCEL_EXECUTE_RECOVER_COUNTER_THRESH)
      {
        recover_accel_error_counter = 0;
        recover_Accel_error = SET;
        reconfig_accelerometer = SET;
      }
    }
    
    if(reset_server_unreachable_counter > 0)
    {
      if(--reset_server_unreachable_counter == 0)
      {
        server_is_unreachable = RESET;
        server_fault_count = 0;
      }
    }
    
    /* -------------- Debug  -------------- */
    if(debug_mode_counter > 0)
    {
      if(--debug_mode_counter == 0)
      {
        __NVIC_SystemReset();
      }
    }
    
    TIM6->SR = 0;
    NVIC_ClearPendingIRQ(TIM6_IRQn);
  }
}


// Accelerometer - 20mS
void Fifty_Hz_handler(void)
{
  if( (TIM7->SR & TIM_SR_UIF) == TIM_SR_UIF )
  {
    /* ----- ODO Meter ----- */
#ifdef ENABLE_ODOMETER
    ODO_20ms_overflow++;
    if(ODO_20ms_overflow >= 10) // 200ms
    {
      if ((TIM1->CNT  < (18 + pervious_TCNT1))) //25->17.5 km/s //20 --> hard abs will make fault 
      {
        totall_TCNT1 += TIM1->CNT;
	pervious_TCNT1 = TIM1->CNT;
	TIM1->CNT = 0;
	count_of_ignoring_tcnt = 0;
      }
      else  // if ABS activated
      {
	if (Current_ODO_Speed <= 10) //if speed is too low , it must be takeoff of wheel
        {
          totall_TCNT1 += TIM1->CNT;
          pervious_TCNT1 = TIM1->CNT;
          TIM1->CNT = 0;
          count_of_ignoring_tcnt = 0;
        }
        else
        {
          if((count_of_ignoring_tcnt >= 5) && (count_of_ignoring_tcnt < 10))
          {
            totall_TCNT1 = totall_TCNT1 + (pervious_TCNT1/3); //(pervious_TCNT1/2) -->> estimate the speed(speed will be slow)
            TIM1->CNT = 0 ;
            count_of_ignoring_tcnt++;  
          }
          else if (count_of_ignoring_tcnt >= 10)
          {
            totall_TCNT1 = totall_TCNT1 + (pervious_TCNT1/2); //(pervious_TCNT1/2) -->> estimate the speed(speed will be slow)
            TIM1->CNT = 0 ;
            count_of_ignoring_tcnt++;                
          }
          else  
          {
            totall_TCNT1 = totall_TCNT1 + pervious_TCNT1; //(pervious_TCNT1/2) -->> estimate the speed(speed will be slow)
            TIM1->CNT = 0 ;
            count_of_ignoring_tcnt++;                 
          }
        }
      }
      ODO_20ms_overflow = 0;
      ODO_200_ms_overflow++;
      
      if(ODO_200_ms_overflow == 5) //1000 ms
      {
        current_tcnt = totall_TCNT1;
        totall_TCNT1 = 0;
        ODO_meter_need_update = 1; 
        ODO_200_ms_overflow = 0;
      }
    }
#endif
    /* ----- End of ODO Meter ----- */
    static volatile int counter_=0;
    if(disable_accel_timer == RESET)
    {
      uint8_t counter = 0;
      int16_t data_x = 0;
      int16_t data_y = 0;
      int16_t data_z = 0;
      int32_t accel_x_sum = 0;
      int32_t accel_y_sum = 0;
      int32_t accel_z_sum = 0;
      while(counter < 41)
      {
        KX023_read_raw_data(&data_x, &data_y, &data_z);
        
        accel_x_sum += data_x;
        accel_y_sum += data_y;
        accel_z_sum += data_z;
        
        counter++;
      }
      KX023.axis_x = (accel_x_sum / 41) * 0.244140625;
      KX023.axis_y = (accel_y_sum / 41) * 0.244140625;
      KX023.axis_z = (accel_z_sum / 41) * 0.244140625;
      
      counter_++;
      if(counter_ > 25) //500ms
      {
        counter_=0;
        push_To_KX023_buffer(&KX023);
      }
      Calculate_MovingAverage_Offset(&KX023.x_offset, &KX023.y_offset, &KX023.z_offset);
      KX023.axis_x -= KX023.x_offset;
      KX023.axis_y -= KX023.y_offset;
      KX023.axis_z -= KX023.z_offset;
      
      if(KX023.axis_x <= ACCEL_CORRECTION_WINDOW && KX023.axis_x >= -ACCEL_CORRECTION_WINDOW)
        KX023.axis_x = 0;
      if(KX023.axis_y <= ACCEL_CORRECTION_WINDOW && KX023.axis_y >= -ACCEL_CORRECTION_WINDOW)
        KX023.axis_y = 0;
      if(KX023.axis_z <= ACCEL_CORRECTION_WINDOW && KX023.axis_z >= -ACCEL_CORRECTION_WINDOW)
        KX023.axis_z = 0;
      
      Accel_movement_check();
      How_the_vehicle_moves();
    }
    
    TIM7->SR = 0;
    NVIC_ClearPendingIRQ(TIM7_IRQn);
  }
}


// ADC, GPS and LEDs
void Twentyfive_Hz_handler(void)
{
  if( (TIM3->SR & TIM_SR_UIF) == TIM_SR_UIF )
  {
    ADC_calculate_average();
    
    
    /*  ------ ODO Meter ------ */
#ifdef ENABLE_ODOMETER
    if (ODO_meter_need_update == 1)  
    {
      ODO_meter_need_update = 0 ;
      Current_ODO_Speed = ((float)current_tcnt) * (1000/setting.odometer_k) ; // Meter
      Odo_travelled_distance = Odo_travelled_distance + Current_ODO_Speed ; //Meter
      Current_ODO_Speed_KMh = Current_ODO_Speed * 3.6; //KM/Sec
      
      if(Current_ODO_Speed_KMh > Odo_max_speed)
        Odo_max_speed = (uint8_t)Current_ODO_Speed_KMh;
      if( abs((uint16_t)Current_ODO_Speed_KMh - GPS_data.speed_Kmh) > ODO_GNSS_SPEED_DIFFERENCE )
      {
        if(repetition_odometer_gnss_speed_difference_counter == 0)
        {
          Force_Produce_Event();
          Event_Flags.gnss_odometer_speed_dif = ENABLE;
          repetition_odometer_gnss_speed_difference_counter = REPETITION_GNSS_ODO_SPEED_THRESHOLD;
        }
      }
    }
#endif
    
    /*  ------ LEDs ------ */
    LED_Control_LEDs();
    
    
    TIM3->SR = 0;
    NVIC_ClearPendingIRQ(TIM3_IRQn);
  }
}
