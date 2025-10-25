#ifndef __FLASH_H
#define __FLASH_H

#include "main.h"

#define BOOTLOADER_ADDRESS                      (uint32_t)0x08000000
#define APPLICATION_ADDRESS                     (uint32_t)0x08004000
#define FLASH_PAGE_SIZE                         2048
#define FLASH_PAGE_CHUNK_SIZE                   256


void Flash_Disable_RDP(void);
void Flash_Enable_RDP(void);
uint8_t Flash_check_RDP(void);
void Flash_Execute_FirmwareUpgrade(uint32_t firmware_size);

#endif  /*__FLASH_H */