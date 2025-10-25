#include "LEDs.h"


uint8_t gsm_led_pulse_mode = LED_OFF;
uint8_t gps_led_pulse_mode = LED_OFF;
uint8_t sys_led_pulse_mode = LED_OFF;
uint8_t gsm_led_counter = 0;
uint8_t gps_led_counter = 0;
uint8_t sys_led_counter = 0;
uint8_t gsm_led_pulse_count = LED_OFF;
uint8_t gps_led_pulse_count = LED_OFF;
uint8_t sys_led_pulse_count = LED_OFF;
uint8_t gsm_led_state = 0;
uint8_t gps_led_state = 0;
uint8_t sys_led_state = 0;
uint8_t gsm_led_twice_counter = 0;
uint8_t gps_led_twice_counter = 0;
uint8_t sys_led_twice_counter = 0;


void LED_change_mode(uint8_t led, uint8_t led_mode)
{
  if(led == GPS_LED)
  {
    if(gps_led_pulse_mode != led_mode)
    {
      gps_led_pulse_mode = led_mode;
      gps_led_counter = 0;
      switch(led_mode)
      {
      case LED_OFF:
        {
          gps_led_pulse_count = LED_FULL_OFF;
          gps_led_state = RESET;
          GPS_LED_OFF;
          break;
        }
      case LED_ON_STEADY:
        {
          gps_led_pulse_count = LED_STEADY;
          gps_led_state = SET;
          GPS_LED_ON;
          break;
        }
      case LED_ON_BLINK_500MS:
        {
          gps_led_pulse_count = LED_ON_500MS;
          gps_led_state = SET;
          GPS_LED_ON;
          break;
        }
      case LED_ON_PULSE_EACH_250MS:
        {
          gps_led_pulse_count = LED_OFF_250MS;
          gps_led_state = RESET;
          GPS_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_1SEC:
        {
          gps_led_pulse_count = LED_OFF_1000MS;
          gps_led_state = RESET;
          GPS_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_3SEC:
        {
          gps_led_pulse_count = LED_OFF_3000MS;
          gps_led_state = RESET;
          GPS_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_5SEC:
        {
          gps_led_pulse_count = LED_OFF_5000MS;
          gps_led_state = RESET;
          GPS_LED_OFF;
          break;
        }
      case LED_ON_TWICE_EACH_1SEC:
        {
          gps_led_pulse_count = LED_OFF_80MS;
          gps_led_state = RESET;
          GPS_LED_OFF;
          break;
        }
      case LED_ON_TIKTOK_250MS:
        {
          gps_led_pulse_count = LED_OFF_250MS;
          gps_led_state = RESET;
          GPS_LED_OFF;
          
          sys_led_pulse_mode = led_mode;
          sys_led_pulse_count = LED_ON_250MS;
          sys_led_state = SET;
          SYS_LED_ON;
          
          gsm_led_pulse_mode = led_mode;
          gsm_led_pulse_count = LED_OFF_250MS;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_80MS:
        {
          gps_led_pulse_count = LED_OFF_80MS;
          gps_led_state = RESET;
          GPS_LED_OFF;
          break;
        }
      case LED_OFF_PULSE_EACH_5SEC:
        {
          gps_led_pulse_count = LED_ON_5000MS;
          gps_led_state = SET;
          GPS_LED_ON;
          break;
        }
      }
    }
  }
  else if(led == SYS_LED)
  {
    if(sys_led_pulse_mode != led_mode)
    {
      sys_led_pulse_mode = led_mode;
      sys_led_counter = 0;
      switch(led_mode)
      {
      case LED_OFF:
        {
          sys_led_pulse_count = LED_FULL_OFF;
          sys_led_state = RESET;
          SYS_LED_OFF;
          break;
        }
      case LED_ON_STEADY:
        {
          sys_led_pulse_count = LED_STEADY;
          sys_led_state = SET;
          SYS_LED_ON;
          break;
        }
      case LED_ON_BLINK_500MS:
        {
          sys_led_pulse_count = LED_ON_500MS;
          sys_led_state = SET;
          SYS_LED_ON;
          break;
        }
      case LED_ON_PULSE_EACH_250MS:
        {
          sys_led_pulse_count = LED_OFF_250MS;
          sys_led_state = RESET;
          SYS_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_1SEC:
        {
          sys_led_pulse_count = LED_OFF_1000MS;
          sys_led_state = RESET;
          SYS_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_3SEC:
        {
          sys_led_pulse_count = LED_OFF_3000MS;
          sys_led_state = RESET;
          SYS_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_5SEC:
        {
          sys_led_pulse_count = LED_OFF_5000MS;
          sys_led_state = RESET;
          SYS_LED_OFF;
          break;
        }
      case LED_ON_TWICE_EACH_1SEC:
        {
          sys_led_pulse_count = LED_OFF_80MS;
          sys_led_state = RESET;
          SYS_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_80MS:
        {
          sys_led_pulse_count = LED_OFF_80MS;
          sys_led_state = RESET;
          SYS_LED_OFF;
          break;
        }
      case LED_OFF_PULSE_EACH_5SEC:
        {
          sys_led_pulse_count = LED_ON_5000MS;
          sys_led_state = SET;
          SYS_LED_ON;
          break;
        }
      }
    }
  }
  else if(led == GSM_LED)
  {
    if(gsm_led_pulse_mode != led_mode)
    {
      if(system_current_state == SYSTEM_RUNNING)
      {
        if(sipaad_server_is_ok == RESET)
        {
          gsm_led_pulse_mode = led_mode;
          gsm_led_counter = 0;
        }
        else
        {
          gsm_led_pulse_mode = LED_STEADY;
          led_mode = LED_ON_STEADY;
          gsm_led_counter = 0;
        }
      }
      else
      {
        gsm_led_pulse_mode = led_mode;
        gsm_led_counter = 0;
      }
      
      switch(led_mode)
      {
      case LED_OFF:
        {
          gsm_led_pulse_count = LED_FULL_OFF;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_ON_STEADY:
        {
          gsm_led_pulse_count = LED_STEADY;
          gsm_led_state = SET;
          GSM_LED_ON;
          break;
        }
      case LED_ON_BLINK_500MS:
        {
          gsm_led_pulse_count = LED_ON_500MS;
          gsm_led_state = SET;
          GSM_LED_ON;
          break;
        }
      case LED_ON_PULSE_EACH_250MS:
        {
          gsm_led_pulse_count = LED_OFF_250MS;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_1SEC:
        {
          gsm_led_pulse_count = LED_OFF_1000MS;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_3SEC:
        {
          gsm_led_pulse_count = LED_OFF_3000MS;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_5SEC:
        {
          gsm_led_pulse_count = LED_OFF_5000MS;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_ON_TWICE_EACH_1SEC:
        {
          gsm_led_pulse_count = LED_OFF_80MS;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_ON_PULSE_EACH_80MS:
        {
          gsm_led_pulse_count = LED_OFF_80MS;
          gsm_led_state = RESET;
          GSM_LED_OFF;
          break;
        }
      case LED_OFF_PULSE_EACH_5SEC:
        {
          gsm_led_pulse_count = LED_ON_5000MS;
          gsm_led_state = SET;
          GSM_LED_ON;
          break;
        }
      }
    }
  }
}


void LED_Control_LEDs(void)
{
  if(system_current_state == SYSTEM_RUNNING)
  {
    // System LED
    if(system_has_error == RESET)
      LED_change_mode(SYS_LED, LED_OFF);
  }
  else if(system_current_state == SYSTEM_SLEEP)
  {
    // GPS LED
    LED_change_mode(GPS_LED, LED_OFF);
    
    // GSM LED
    LED_change_mode(GSM_LED, LED_OFF);
    
    // System LED
    if(system_has_error == RESET)
      LED_change_mode(SYS_LED, LED_ON_PULSE_EACH_3SEC);
  }
  else if(system_current_state == SYSTEM_ON_BATTERY)
  {
    // GPS LED
    LED_change_mode(GPS_LED, LED_OFF);
    
    // GSM LED
    LED_change_mode(GSM_LED, LED_OFF);
    
    // System LED
    if(system_has_error == RESET)
      LED_change_mode(SYS_LED, LED_ON_STEADY);
  }
  
  if(system_has_error == SET)
  {
    LED_change_mode(SYS_LED, LED_ON_PULSE_EACH_80MS);
  }
  
  // GPS LED
  switch(gps_led_pulse_mode)
  {
  case LED_OFF:
    {
      if(gps_led_state == SET)
      {
        gps_led_state = RESET;
        GPS_LED_OFF;                        // LED Off
      }
      break;
    }
  case LED_ON_STEADY:
    {
      if(gps_led_state == RESET)
      {
        gps_led_state = SET;
        GPS_LED_ON;                        // LED Off
      }
      break;
    }
  case LED_ON_BLINK_500MS:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
          GPS_LED_ON;                       // LED On
        else
          GPS_LED_OFF;                      // LED Off
      }
      break;
    }
  case LED_ON_PULSE_EACH_250MS:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_ON;                       // LED On
        }
        else
        {
          gps_led_pulse_count = LED_OFF_250MS;
          GPS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_1SEC:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_ON;                       // LED On
        }
        else
        {
          gps_led_pulse_count = LED_OFF_1000MS;
          GPS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_3SEC:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_ON;                       // LED On
        }
        else
        {
          gps_led_pulse_count = LED_OFF_3000MS;
          GPS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_5SEC:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_ON;                       // LED On
        }
        else
        {
          gps_led_pulse_count = LED_OFF_5000MS;
          GPS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_TWICE_EACH_1SEC:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_ON;                       // LED On
          gps_led_twice_counter++;
        }
        else
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_OFF;                      // LED Off
          
          if(gps_led_twice_counter == 2)
          {
            gps_led_twice_counter = 0;
            gps_led_pulse_count = LED_OFF_1000MS;
          }
        }
      }
      break;
    }
  case LED_ON_TIKTOK_250MS:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_250MS;
          GPS_LED_ON;                       // LED On
        }
        else
        {
          gps_led_pulse_count = LED_OFF_250MS;
          GPS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_80MS:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_ON;                       // LED On
        }
        else
        {
          gps_led_pulse_count = LED_OFF_80MS;
          GPS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_OFF_PULSE_EACH_5SEC:
    {
      if(++gps_led_counter >= gps_led_pulse_count)
      {
        gps_led_counter = 0;
        gps_led_state ^= 1;
        if(gps_led_state == SET)
        {
          gps_led_pulse_count = LED_ON_5000MS;
          GPS_LED_ON;                       // LED On
        }
        else
        {
          gps_led_pulse_count = LED_ON_80MS;
          GPS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  }
  
  // SYS LED
  switch(sys_led_pulse_mode)
  {
  case LED_OFF:
    {
      if(sys_led_state == SET)
      {
        sys_led_state = RESET;
        SYS_LED_OFF;                        // LED Off
      }
      break;
    }
  case LED_ON_STEADY:
    {
      if(sys_led_state == RESET)
      {
        sys_led_state = SET;
        SYS_LED_ON;                        // LED Off
      }
      break;
    }
  case LED_ON_BLINK_500MS:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
          SYS_LED_ON;                       // LED On
        else
          SYS_LED_OFF;                      // LED Off
      }
      break;
    }
  case LED_ON_PULSE_EACH_250MS:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_ON;                       // LED On
        }
        else
        {
          sys_led_pulse_count = LED_OFF_250MS;
          SYS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_1SEC:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_ON;                       // LED On
        }
        else
        {
          sys_led_pulse_count = LED_OFF_1000MS;
          SYS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_3SEC:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_ON;                       // LED On
        }
        else
        {
          sys_led_pulse_count = LED_OFF_3000MS;
          SYS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_5SEC:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_ON;                       // LED On
        }
        else
        {
          sys_led_pulse_count = LED_OFF_5000MS;
          SYS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_TWICE_EACH_1SEC:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_ON;                       // LED On
          sys_led_twice_counter++;
        }
        else
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_OFF;                      // LED Off
          
          if(sys_led_twice_counter == 2)
          {
            sys_led_twice_counter = 0;
            sys_led_pulse_count = LED_OFF_1000MS;
          }
        }
      }
      break;
    }
  case LED_ON_TIKTOK_250MS:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_250MS;
          SYS_LED_ON;                       // LED On
        }
        else
        {
          sys_led_pulse_count = LED_OFF_250MS;
          SYS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_80MS:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_ON;                       // LED On
        }
        else
        {
          sys_led_pulse_count = LED_OFF_80MS;
          SYS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_OFF_PULSE_EACH_5SEC:
    {
      if(++sys_led_counter >= sys_led_pulse_count)
      {
        sys_led_counter = 0;
        sys_led_state ^= 1;
        if(sys_led_state == SET)
        {
          sys_led_pulse_count = LED_ON_5000MS;
          SYS_LED_ON;                       // LED On
        }
        else
        {
          sys_led_pulse_count = LED_ON_80MS;
          SYS_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  }
  
  // GSM LED
  switch(gsm_led_pulse_mode)
  {
  case LED_OFF:
    {
      if(gsm_led_state == SET)
      {
        gsm_led_state = RESET;
        GSM_LED_OFF;                        // LED Off
      }
      break;
    }
  case LED_ON_STEADY:
    {
      if(gsm_led_state == RESET)
      {
        gsm_led_state = SET;
        GSM_LED_ON;                       // LED On
      }
      break;
    }
  case LED_ON_BLINK_500MS:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
          GSM_LED_ON;                       // LED On
        else
          GSM_LED_OFF;                      // LED Off
      }
      break;
    }
  case LED_ON_PULSE_EACH_250MS:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_ON;                       // LED On
        }
        else
        {
          gsm_led_pulse_count = LED_OFF_250MS;
          GSM_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_1SEC:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_ON;                       // LED On
        }
        else
        {
          gsm_led_pulse_count = LED_OFF_1000MS;
          GSM_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_3SEC:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_ON;                       // LED On
        }
        else
        {
          gsm_led_pulse_count = LED_OFF_3000MS;
          GSM_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_5SEC:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_ON;                       // LED On
        }
        else
        {
          gsm_led_pulse_count = LED_OFF_5000MS;
          GSM_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_TWICE_EACH_1SEC:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_ON;                       // LED On
          gsm_led_twice_counter++;
        }
        else
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_OFF;                      // LED Off
          
          if(gsm_led_twice_counter == 2)
          {
            gsm_led_twice_counter = 0;
            gsm_led_pulse_count = LED_OFF_1000MS;
          }
        }
      }
      break;
    }
  case LED_ON_TIKTOK_250MS:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_250MS;
          GSM_LED_ON;                       // LED On
        }
        else
        {
          gsm_led_pulse_count = LED_OFF_250MS;
          GSM_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_ON_PULSE_EACH_80MS:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_ON;                       // LED On
        }
        else
        {
          gsm_led_pulse_count = LED_OFF_80MS;
          GSM_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  case LED_OFF_PULSE_EACH_5SEC:
    {
      if(++gsm_led_counter >= gsm_led_pulse_count)
      {
        gsm_led_counter = 0;
        gsm_led_state ^= 1;
        if(gsm_led_state == SET)
        {
          gsm_led_pulse_count = LED_ON_5000MS;
          GSM_LED_ON;                       // LED On
        }
        else
        {
          gsm_led_pulse_count = LED_ON_80MS;
          GSM_LED_OFF;                      // LED Off
        }
      }
      break;
    }
  }
}