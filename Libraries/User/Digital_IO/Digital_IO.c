#include "Digital_IO.h"

io_typedef              IO_Digital;
io_states_typedef       IO_states[NUMBER_OF_DIGITAL_INPUTS] = {0};

GPIO_TypeDef*           GPIO_Port[NUMBER_OF_DIGITAL_INPUTS] = {IO_ACC_GPIO, IO_VCC_DIGITAL_GPIO};
uint32_t                GPIO_Pin[NUMBER_OF_DIGITAL_INPUTS] = {IO_ACC_PIN, IO_VCC_DIGITAL_PIN};

__IO uint16_t systick_check_IO_counter = 0;

uint8_t ignition_changed = RESET;
uint8_t vcc_digital_changed = RESET;

/*
ACC:             ON(High Voltage), OFF(Ground-Open)
VCC:             ON(High Voltage), OFF(Ground-Open)
*/

void IO_cfg_Digital_IOs(void)
{
  /* ------------- System LED ------------- */
  IO_SYSTEM_LED_GPIO->MODER = (IO_SYSTEM_LED_GPIO->MODER & (~GPIO_MODER_MODE0)) | GPIO_MODER_MODE0_0;           // Set as Output
  IO_SYSTEM_LED_GPIO->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED0;                                                         // Lowest Speed. Due to Pins limitations.
  IO_SYSTEM_LED_GPIO->PUPDR = (IO_SYSTEM_LED_GPIO->PUPDR & (~GPIO_PUPDR_PUPD0)) | GPIO_PUPDR_PUPD0_1;           // Pulled Down
  SYS_LED_OFF;
  
  /* ------------- GSM LED ------------- */
  IO_GSM_LED_GPIO->MODER = (IO_GSM_LED_GPIO->MODER & (~GPIO_MODER_MODE15)) | GPIO_MODER_MODE15_0;                // Set as Output
  IO_GSM_LED_GPIO->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED15;                                                            // Lowest Speed. Due to Pins limitations.
  IO_GSM_LED_GPIO->PUPDR = (IO_GSM_LED_GPIO->PUPDR & (~GPIO_PUPDR_PUPD15)) | GPIO_PUPDR_PUPD15_1;                // Pulled Down
  GSM_LED_OFF;
  
  /* ------------- GPS LED ------------- */
  IO_GPS_LED_GPIO->MODER = (IO_GPS_LED_GPIO->MODER & (~GPIO_MODER_MODE1)) | GPIO_MODER_MODE1_0;                 // Set as Output
  IO_GPS_LED_GPIO->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED1;                                                            // Lowest Speed. Due to Pins limitations.
  IO_GPS_LED_GPIO->PUPDR = (IO_GPS_LED_GPIO->PUPDR & (~GPIO_PUPDR_PUPD1)) | GPIO_PUPDR_PUPD1_1;                 // Pulled Down
  GPS_LED_OFF;
  
  /* ------------- Digital Output ------------- */
//  IO_DIGITAL_OUT_GPIO->MODER = (IO_DIGITAL_OUT_GPIO->MODER & (~GPIO_MODER_MODE15)) | GPIO_MODER_MODE15_0;       // Set as Output
//  IO_DIGITAL_OUT_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED15;
//  IO_DIGITAL_OUT_GPIO->PUPDR = (IO_DIGITAL_OUT_GPIO->PUPDR & (~GPIO_PUPDR_PUPD15)) | GPIO_PUPDR_PUPD15_1;       // Pulled Down
//  
//  // Turn On the Relay
//  IO_ON_OFF_Relay(ON);
  
  /* ------------- ACC ------------- */
  IO_ACC_GPIO->MODER = IO_ACC_GPIO->MODER & (~GPIO_MODER_MODE11);                                               // Set as Input
  IO_ACC_GPIO->PUPDR = (IO_ACC_GPIO->PUPDR & (~GPIO_PUPDR_PUPD11)) | GPIO_PUPDR_PUPD11_0;                        // Pulled UP

  IO_Digital.ignition = !( (IO_ACC_GPIO->IDR & IO_ACC_PIN) == IO_ACC_PIN ? OFF : ON);
  
  /* ------------- VCC Digital ------------- */
  IO_VCC_DIGITAL_GPIO->MODER = IO_VCC_DIGITAL_GPIO->MODER & (~GPIO_MODER_MODE7);                                // Set as Input
  IO_VCC_DIGITAL_GPIO->PUPDR = (IO_VCC_DIGITAL_GPIO->PUPDR & (~GPIO_PUPDR_PUPD7)) | GPIO_PUPDR_PUPD7_0;         // Pulled UP
  
  IO_Digital.vcc_digital = !( (IO_VCC_DIGITAL_GPIO->IDR & IO_VCC_DIGITAL_PIN) == IO_VCC_DIGITAL_PIN ? OFF : ON);
}


void IO_ON_OFF_Relay(uint8_t state)
{
  if(state == ON)
  {
    // Turn On DIGITAL_OUT
    DIGITAL_OUT_ON;
    IO_Digital.digital_out = SET;
  }
  else if(state == OFF)
  {
    // Turn Off DIGITAL_OUT
    DIGITAL_OUT_OFF;
    IO_Digital.digital_out = RESET;
  }
}


void IO_Check_IOs(void)
{
  /* Protect from reading in the middle of startup initialization and GPS Config */
  if(system_is_initiating == RESET)
  {
    for(uint8_t i = 0; i < NUMBER_OF_DIGITAL_INPUTS; i++)
    {
      /* Pin is SET */
      if((GPIO_Port[i]->IDR & GPIO_Pin[i]) == GPIO_Pin[i])
      {
        if(IO_states[i].high_state_count < MAXIMUM_NUM_IO_SAMPLING)
        {
          IO_states[i].high_state_count++;
        }
        else if(IO_states[i].high_state_count == MAXIMUM_NUM_IO_SAMPLING)
        {
          IO_states[i].high_state_count++;
          IO_states[i].low_state_count = 0;
          
          switch(i)
          {
          case 0:
            if(IO_Digital.ignition == ON)
            {
              ignition_changed = SET;
              IO_Digital.ignition = OFF;
            }
            break;
          case 1:
            if(IO_Digital.vcc_digital == ON)
            {
              vcc_digital_changed = SET;
              IO_Digital.vcc_digital = OFF;
            }
            break;
          }
        }
      }
      /* Pin is RESET */
      else
      {
        if(IO_states[i].low_state_count < MAXIMUM_NUM_IO_SAMPLING)
        {
          IO_states[i].low_state_count++;
        }
        else if(IO_states[i].low_state_count == MAXIMUM_NUM_IO_SAMPLING)
        {
          IO_states[i].low_state_count++;
          IO_states[i].high_state_count = 0;
          
          switch(i)
          {
          case 0:
            if(IO_Digital.ignition == OFF)
            {
              ignition_changed = SET;
              IO_Digital.ignition = ON;
            }
            break;
          case 1:
            if(IO_Digital.vcc_digital == OFF)
            {
              vcc_digital_changed = SET;
              IO_Digital.vcc_digital = ON;
            }
            break;
          }
        }
      }
    }
  }
}

void IO_Ignition_Changed(void)
{
  if(gps_is_fixed_for_first_time == SET)
  {
    // ACC OFF
    if(IO_Digital.ignition == OFF)
    {
      // Event record generation
      Force_Produce_Event();
      Event_Flags.ignition_low = ENABLE;
    }
    
    // ACC ON
    else if(IO_Digital.ignition == ON)
    {
      Force_Produce_Event();
      Event_Flags.ignition_high = ENABLE;
    }
  }
}

void IO_vcc_digital_Changed(void)
{
  if(gps_is_fixed_for_first_time == SET)
  {
    if(IO_Digital.vcc_digital == RESET)
    {
      // VCC OFF
      // Record production in System_execute_change_state-on battery
    }
    else if(IO_Digital.vcc_digital == SET)
    {
      // VCC ON
      Force_Produce_Event();
      Event_Flags.main_pwr_connected = SET;
    }
  }
}