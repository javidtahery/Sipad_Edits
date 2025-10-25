#include "KX023.h"


KX023_acceleration_typeDef   KX023;
uint8_t KX023_EXTI1_Halt_Timer = KX023_EXTI_HALT_DEFAULT_TIME;


void KX023_EXTI_Config(void)
{
//  GPIO_InitTypeDef GPIO_InitStruct;
//  EXTI_InitTypeDef EXTI_InitStruct;
//  NVIC_InitTypeDef NVIC_InitStruct;
//  
//  GPIO_InitStruct.GPIO_Pin = KX023_INT1_Pin;
//  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
//  GPIO_Init(KX023_INT1_PORT, &GPIO_InitStruct);
//  
//  // ----------------- INT1 EXTI -----------------
//  GPIO_EXTILineConfig(KX023_INT1_EXTI_PortSource, KX023_INT1_EXTI_PinSource);
//  
//  EXTI_InitStruct.EXTI_Line = KX023_INT1_EXTI_Line;
//  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStruct.EXTI_Trigger = KX023_INT1_EXTI_TRIG_MODE;
//  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStruct);
//  
//  NVIC_InitStruct.NVIC_IRQChannel = KX023_INT1_EXTI_IRQn;
//  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
//  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
//  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStruct);
// /*
//  // ----------------- INT2 EXTI -----------------
//  GPIO_EXTILineConfig(ADXL_INT2_EXTI_PortSource, ADXL_INT2_EXTI_PinSource);
//  
//  EXTI_InitStruct.EXTI_Line = ADXL_INT2_EXTI_Line;
//  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStruct.EXTI_Trigger = ADXL_INT2_EXTI_TRIG_MODE;
//  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStruct);
//  
//  NVIC_InitStruct.NVIC_IRQChannel = ADXL_INT2_EXTI_IRQn;
//  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
//  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
//  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStruct);
//*/
}


uint8_t KX023_Who_AM_I(uint8_t* who)
{
  if(Accel_read_byte(who, KX023_REG_WHO_AM_I))
    return KX023_TIMEOUT;
  return KX023_OK;
}


uint8_t KX023_Standby(void)  // PC1 disable
{
  uint8_t tmp_buff = 0;
  if(Accel_read_byte(&tmp_buff, KX023_REG_CNTL1))
    return KX023_TIMEOUT;
  if(Accel_write_byte(Reset_bit(tmp_buff, 7), KX023_REG_CNTL1))
    return KX023_TIMEOUT;
  return KX023_OK;
}


// Setting 1 will enable respective interrupt
uint8_t KX023_INT_Disable(void)
{
  uint8_t tmp_buff = 0;
  if(Accel_read_byte(&tmp_buff, KX023_REG_CNTL1))
    return KX023_TIMEOUT;
  if(Accel_write_byte(Reset_bit(tmp_buff, 5), KX023_REG_CNTL1)) //DRDYE 
    return KX023_TIMEOUT;
  if(Accel_write_byte(Reset_bit(tmp_buff, 5), KX023_REG_INC1)) 
    return KX023_TIMEOUT; 
  if(Accel_write_byte(0, KX023_REG_INC4)) //reset all inc4 
    return KX023_TIMEOUT;
  if(Accel_write_byte(Reset_bit(tmp_buff, 5), KX023_REG_INC5)) //reset
    return KX023_TIMEOUT;
  if(Accel_write_byte(tmp_buff, KX023_REG_INC6)) //reset all inc6 
    return KX023_TIMEOUT;
  return KX023_OK;
}


uint8_t KX023_Set_BW_RATE(uint8_t rate)
{
  if(rate > 15)
    return KX023_FAULT;
  if(Accel_write_byte(rate, KX023_REG_CNTL3))
    return KX023_TIMEOUT;
  return KX023_OK;
}


uint8_t KX023_Set_Output_rate(void)
{
  if(Accel_write_byte(0x02, KX023_REG_ODCNTL))
    return KX023_TIMEOUT;
  return KX023_OK;
}


uint8_t KX023_Set_Range_Resolution(void)
{
  uint8_t tmp_buff = 0;
  if(Accel_read_byte(&tmp_buff, KX023_REG_CNTL1))
    return KX023_TIMEOUT;
  tmp_buff = Set_bit(tmp_buff, 6);// 1 = high resolution. Bandwidth (Hz) = ODR/2 
  tmp_buff = Set_bit(tmp_buff, 4);// 8G
  tmp_buff = Reset_bit(tmp_buff, 3);//8G
  if(Accel_write_byte(tmp_buff , KX023_REG_CNTL1)) 
    return KX023_TIMEOUT;
  
  return KX023_OK;
}


uint8_t KX023_Set_FIFO_ctl(void)
{
  uint8_t tmp_buff = 0;
  if(Accel_write_byte(32, KX023_REG_BUF_CNTL1)) //32 samples that will trigger a watermark interrupt
    return KX023_TIMEOUT; 
  if(Accel_read_byte(&tmp_buff, KX023_REG_BUF_CNTL2))
    return KX023_TIMEOUT;
  tmp_buff = Reset_bit(tmp_buff, 7);//controls activation of the sample buffer
  tmp_buff = Reset_bit(tmp_buff, 6);//8-bit samples are accumulated in the buffer
  tmp_buff = Reset_bit(tmp_buff, 5);//buffer full interrupt enable bit
  tmp_buff = Set_bit(tmp_buff, 0);//FIFo stream
  tmp_buff = Reset_bit(tmp_buff, 1);//FIFO stream

  if(Accel_write_byte(tmp_buff, KX023_REG_BUF_CNTL2)) 
    return KX023_TIMEOUT;
  
 return KX023_OK;
}


uint8_t KX023_Set_Thresh_Tap(uint8_t thresh1 ,uint8_t thresh2)
{
  if(Accel_write_byte(thresh1, KX023_REG_TTH))
    return KX023_TIMEOUT;
  if(Accel_write_byte(thresh2, KX023_REG_TTL))
    return KX023_TIMEOUT;
  return KX023_OK;
}


uint8_t KX023_Set_FTD(uint8_t duration)
{
  if(duration == 0)
    return KX023_FAULT;
  if(Accel_write_byte(duration, KX023_REG_FTD))
    return KX023_TIMEOUT;
  return KX023_OK;
}


// The scale factor is 1/400hz LSB
uint8_t KX023_Set_TLT(uint8_t duration)
{
  if(duration == 0)
    return KX023_FAULT;
  if(Accel_write_byte(duration, KX023_REG_TLT))
    return KX023_TIMEOUT;
  return KX023_OK;
}


uint8_t KX023_Set_TWS(uint8_t duration)
{
  if(duration == 0)
    return KX023_FAULT;
  if(Accel_write_byte(duration, KX023_REG_TWS))
    return KX023_TIMEOUT;
  return KX023_OK;
}


uint8_t KX023_Enable_SingleTap(void)
{
//  uint8_t tmp_buff = 0;
//  if(Accel_read_byte(&tmp_buff, KX023_REG_CNTL1))
//    return KX023_TIMEOUT;
//  if(Accel_write_byte(Set_bit(tmp_buff, 2), KX023_REG_CNTL1))// enables the Directional TapTM function 
//    return KX023_TIMEOUT;

  if(Accel_write_byte(0x01, KX023_REG_TDTRC))// enables the single tab & disable double tap
    return KX023_TIMEOUT;
  return KX023_OK;

}


uint8_t KX023_new_acceleration_data_INT_Enable(void)
{
  uint8_t tmp_buff = 0;
  if(Accel_read_byte(&tmp_buff, KX023_REG_CNTL1))
    return KX023_TIMEOUT;
  if(Accel_write_byte(Set_bit(tmp_buff, 5), KX023_REG_CNTL1))// enables the reporting of the availability of new acceleration data as an interrupt
    return KX023_TIMEOUT; 
  return KX023_OK;
}


uint8_t KX023_SingleTap_INT_Enable(void)
{
  if(Accel_write_byte(0x30, KX023_REG_INC1))// enables the physical interrupt PIN1
    return KX023_TIMEOUT;
  if(Accel_write_byte(0x3C, KX023_REG_INC3))// enables the Directional Tap in X & y 
    return KX023_TIMEOUT;   
  
  if(Accel_write_byte(0x04, KX023_REG_INC4))// enables single/double tab interrupt on PIN1
    return KX023_TIMEOUT;
  return KX023_OK;
  
}


uint8_t KX023_Measure_On(void)
{
  uint8_t tmp_buff = 0;
  if(Accel_read_byte(&tmp_buff, KX023_REG_CNTL1))
    return KX023_TIMEOUT;
  if(Accel_write_byte(Set_bit(tmp_buff, 7), KX023_REG_CNTL1))
    return KX023_TIMEOUT;
  return KX023_OK; 
  
}


// To clear the interrupts, read the interrupts.
uint8_t KX023_Read_INT_Source(uint8_t* data)
{
  if(Accel_read_byte(data, KX023_REG_INS1))
    return KX023_TIMEOUT;
  if(Accel_read_byte(data, KX023_REG_INS2))
    return KX023_TIMEOUT;
  return KX023_OK;
}


void KX023_EXTI1_Callback(void)
{
//  if(KX023_EXTI1_Halt_Timer == KX023_EXTI_HALT_DEFAULT_TIME)
//      KX023_EXTI1_Halt_Timer = KX023_EXTI_ON_HALT_TIME;
//  
//  EXTI_ClearITPendingBit(KX023_INT1_EXTI_Line);
}


void KX023_INT1_Handle(void)
{
//  uint8_t tmp = 0;
//  uint8_t KX023_response = 0;
//  uint8_t single_tap_it = RESET;
//  
//  do
//  {
//    KX023_response = KX023_Read_INT_Source(&tmp);
//    /* The I2C bus is busy by main while and we are in iterrupt routine which its priority is higher than main while.
//       We should return to the main while and complete the I2C related task.*/
//    if(KX023_response != KX023_OK)
//    {
//      KX023_EXTI1_Halt_Timer = KX023_EXTI_ON_HALT_TIME;
//      break;
//    }
//    
//    if(tmp > 0)
//    {
//      single_tap_it = Check_bit(tmp, 2);
//      
//      if(single_tap_it == SET)
//      {
//        Event_Flags.g_sensor_activated = SET;
//        force_create_event = SET;
//      }
//    }
//  }
//  while(tmp == 0);
//  
//  uint8_t exit_while = RESET;
//  
//  while(exit_while == RESET)
//  {
//    KX023_response = KX023_Read_INT_Source(&tmp);
//    if(KX023_response == KX023_OK)
//    {
//      if(tmp == 0)
//        continue;
//      else
//      {
//        single_tap_it = Check_bit(tmp, 2);
//        
//        if(Event_Flags.g_sensor_activated == RESET && single_tap_it == SET)
//        {
//          Event_Flags.g_sensor_activated = SET;
//          force_create_event = SET;
//        }
//        
//        if(single_tap_it == RESET)
//          exit_while = SET;
//      }
//    }
//    else
//    {
//      KX023_EXTI1_Halt_Timer = KX023_EXTI_ON_HALT_TIME;
//      break;
//    }
//  }
//  
//  EXTI_ClearITPendingBit(KX023_INT1_EXTI_Line); ///?????????????????????????????????
}


void KX023_startup_calibration(void)
{
  int16_t data_x = 0;
  int16_t data_y = 0;
  int16_t data_z = 0;
  int32_t accel_x_sum = 0;
  int32_t accel_y_sum = 0;
  int32_t accel_z_sum = 0;
  
  for(uint16_t i = 0; i < 1024; i++)
  {
    KX023_read_raw_data(&data_x, &data_y, &data_z);
    accel_x_sum += data_x;
    accel_y_sum += data_y;
    accel_z_sum += data_z;
  }
  accel_x_sum = accel_x_sum >> 10;
  accel_y_sum = accel_y_sum >> 10;
  accel_z_sum = accel_z_sum >> 10;
  
  KX023.axis_x = accel_x_sum * 0.244140625;
  KX023.axis_y = accel_y_sum * 0.244140625;
  KX023.axis_z = accel_z_sum * 0.244140625;
  
  KX023.x_offset = KX023.axis_x;
  KX023.y_offset = KX023.axis_y;
  KX023.z_offset = KX023.axis_z;
}


void KX023_read_raw_data(int16_t* data_x, int16_t* data_y, int16_t* data_z)
{
  uint8_t raw_data[6] = {0};
  
  if(Accel_read_nbytes(raw_data , 6, KX023_REG_DATAX0) == I2C_BUS_OK)
  {
    *data_x = raw_data[0] + (raw_data[1]<<8);
    *data_y = raw_data[2] + (raw_data[3]<<8);
    *data_z = raw_data[4] + (raw_data[5]<<8);
  }
  else
  {
    *data_x = 0;
    *data_y = 0;
    *data_z = 0;
    
    recover_Accel_error = SET;
  }
}


void KX023_Config(void)
{
  KX023_EXTI_Config();
  
  uint8_t config_result = KX023_OK;
  uint8_t who_am_I = 0;
  
  config_result = KX023_Who_AM_I(&who_am_I);
  
  if(config_result == KX023_OK && who_am_I == KX023_WHO_AM_I_RESP)
  {
    config_result = KX023_Standby();
    config_result = KX023_INT_Disable();
    config_result = KX023_Set_BW_RATE(0x9E);     // Output rate: 12.5Hz= for tilt & 50 hz=general motion detection --400 hz= Directional Tap
    config_result = KX023_Set_Output_rate();     // Output rate = 50hz
    config_result = KX023_Set_Range_Resolution();    // +- 8g for raw data &  high resolution
    config_result = KX023_Set_FIFO_ctl();       // FIFO mode is Stream; All data are overwritten
    
    /* .......... */
    config_result = KX023_Measure_On();
    Delay(50);
    KX023_startup_calibration();
    /* .......... */
    
    //  config_result = KX023_Standby();
    //  config_result = KX023_Set_Thresh_Tap(203 , 26);            // 5g(5*16)
    //  config_result = KX023_Set_FTD(0xFD);                   // ???? 5 * (1/400hz) = 12.5 ms for MIN and 77.5 ms for MAX
    //  config_result = KX023_Set_TLT(0x05);                   // ???? 5 * (1/400hz) = 12.5 ms 
    //  config_result = KX023_Set_TWS(0x05);                   // ???? 5 * (1/400hz) = 12.5 ms 
    //  config_result = KX023_Enable_SingleTap();
    //  config_result = KX023_SingleTap_INT_Enable();
    //  config_result = KX023_Measure_On();
    /* .......... */
    
    disable_accel_timer = RESET;
  }
  else
  {
    reconfig_accelerometer = SET;
    recover_Accel_error = SET;
  }
  
  KX023_INT1_Handle();
  
  if(Event_Flags.g_sensor_activated == SET)
  {
    Event_Flags.g_sensor_activated = RESET;
    force_create_event = RESET;
  }
}


uint8_t Accel_Recovery_I2C(void)
{
  uint8_t function_return = KX023_FAULT;
  uint8_t config_result = KX023_OK;
  uint8_t who_am_I = 0;
  
  I2C_generate_stop_condition_manually();
  
  Delay(1000);
  
  config_result = KX023_Who_AM_I(&who_am_I);
  
  if(config_result == KX023_OK && who_am_I == KX023_WHO_AM_I_RESP)
  {
    function_return = KX023_OK;
    system_error.accel_error = RESET;
  }
  else
  {
    system_error.accel_error = SET;
  }
  
  return function_return;
}