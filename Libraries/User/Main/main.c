#include "main.h"

#pragma default_variable_attributes = @ "BOOT_Data" __no_init

uint32_t                Bootloader_Password;
uint32_t                Bootloader_release_date;
uint32_t                system_Unixtime;
uint32_t                reset_count;
#pragma default_variable_attributes =



uint8_t system_is_initiating = SET;
uint8_t timers_does_not_work = RESET;


//0x021F0005
int main()
{
  System_Config();
  
  while(1)
  {
    if(system_is_initiating == RESET)
      IWDG_ReloadCounter();
    
    Record_Production();
    AVL_Handle_Tasks();
    System_check_state();
    
  }     /* End of while(1) */
}       /* End of main() */