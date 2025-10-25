#include "Flash.h"


void Flash_Unlock_Memory(void);
void Flash_Lock_Memory(void);
__ramfunc void FLASH_Program_FastPage(uint32_t flash_addr, uint32_t* pdata);
void FLASH_Program_Page(uint32_t flash_addr, uint32_t* pdata);
void Flash_Erase_Page(uint32_t page_number);



/*
** STM32G070xx has 1 bank Flash memory.
** Each Flash page is 2 Kbytes.
** Main memory block containing 64 pages of 2 Kbytes, each page with eight rows of 256 bytes.
*/
void Flash_Execute_FirmwareUpgrade(uint32_t firmware_size)
{
//  RCC->AHBENR |= RCC_AHBENR_FLASHEN;
  uint32_t SPIF_data_address = SPIF_BOOT_FW_FILE_START_SECTOR * SPIF_SECTOR_SIZE;
  uint32_t flash_program_address = BOOTLOADER_ADDRESS;
  
  uint8_t verify_read_row[FLASH_PAGE_CHUNK_SIZE] = {0};
  
  uint8_t sector_count = firmware_size / SPIF_SECTOR_SIZE;
  uint16_t last_sector_bytes_remain = firmware_size % SPIF_SECTOR_SIZE;
  
  // Starting Flash Page index
  uint32_t flash_page_index = 0;
  
  // Unlock the Flash memory to begin the program operation
  Flash_Unlock_Memory();
  
  uint8_t sector_buffer[SPIF_SECTOR_SIZE];
  
  // Reading all the firmware file from SPI Flash memory and program the MCU Flash memory(except the last SPIF sector).
  for(uint8_t SPIF_sector_itr = 0; SPIF_sector_itr < sector_count; SPIF_sector_itr++)
  {
    Delay(100);
    memset(sector_buffer, 0, SPIF_SECTOR_SIZE);
    
    // Reading one sector
    SPIF_Read(sector_buffer, SPIF_data_address, SPIF_SECTOR_SIZE);
    
    // programming 4096 bytes of the data
    uint8_t chunk_itr = 0;
    while(chunk_itr < 16)
    {
      // Erase the MCU Flash page every 2048 bytes(1 page size)
      if( (chunk_itr % 8) == 0)
        Flash_Erase_Page(flash_page_index++);
      
      // Program one chunk of 256 bytes of data
      FLASH_Program_Page(flash_program_address, (uint32_t*)(sector_buffer + (chunk_itr * FLASH_PAGE_CHUNK_SIZE)) );
      
      // Verify the written chunk
      memcpy(verify_read_row, (uint8_t*)flash_program_address, FLASH_PAGE_CHUNK_SIZE);
      if(memcmp(verify_read_row, (sector_buffer + (chunk_itr * FLASH_PAGE_CHUNK_SIZE)), FLASH_PAGE_CHUNK_SIZE) == 0 )
      {
        flash_program_address += FLASH_PAGE_CHUNK_SIZE;
        chunk_itr++;
      }
      else
      {
        uint8_t tmp_var = chunk_itr % 8;
        chunk_itr -= tmp_var;
        flash_page_index--;
        flash_program_address -= (tmp_var * FLASH_PAGE_CHUNK_SIZE);
        continue;
      }
    }
    
    SPIF_data_address += SPIF_SECTOR_SIZE;
    IWDG_ReloadCounter();
  }
  
  // Program less than 4096 Bytes of data
  Delay(100);
  memset(sector_buffer, 0xFF, SPIF_SECTOR_SIZE);
  SPIF_Read(sector_buffer, SPIF_data_address, last_sector_bytes_remain);
  uint8_t end_chunk = last_sector_bytes_remain / FLASH_PAGE_CHUNK_SIZE;
  
  uint8_t chunk_itr = 0;
  while(chunk_itr < end_chunk + 1)
  {
    if( (chunk_itr % 8) == 0)
      Flash_Erase_Page(flash_page_index++);
    
    FLASH_Program_Page(flash_program_address, (uint32_t*)(sector_buffer + (chunk_itr * FLASH_PAGE_CHUNK_SIZE)) );
    
    // Verify the written chunk
    memcpy(verify_read_row, (uint8_t*)flash_program_address, FLASH_PAGE_CHUNK_SIZE);
    if(memcmp(verify_read_row, (sector_buffer + (chunk_itr * FLASH_PAGE_CHUNK_SIZE)), FLASH_PAGE_CHUNK_SIZE) == 0 )
    {
      flash_program_address += FLASH_PAGE_CHUNK_SIZE;
      chunk_itr++;
    }
    else
    {
      uint8_t tmp_var = chunk_itr % 8;
      chunk_itr -= tmp_var;
      flash_page_index--;
      flash_program_address -= (tmp_var * FLASH_PAGE_CHUNK_SIZE);
      continue;
    }
  }
  
  // Lock the MCU Flash memory to end the firmware upgrade operation.
  Flash_Lock_Memory();
  IWDG_ReloadCounter();
}

void Flash_Unlock_Memory(void)
{
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  if( (FLASH->CR & FLASH_CR_LOCK) == FLASH_CR_LOCK)
  {
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
  }
}

void Flash_Lock_Memory(void)
{
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  while( (FLASH->SR & FLASH_SR_CFGBSY) == FLASH_SR_CFGBSY);
  
  if( (FLASH->CR & FLASH_CR_LOCK) != FLASH_CR_LOCK)
    FLASH->CR |= FLASH_CR_LOCK;
}

void Flash_Unlock_OptionByte(void)
{
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  if( (FLASH->CR & FLASH_CR_OPTLOCK) == FLASH_CR_OPTLOCK)
  {
    FLASH->OPTKEYR = 0x08192A3B;
    FLASH->OPTKEYR = 0x4C5D6E7F;
  }
}

void Flash_Disable_RDP(void)
{
  // Clear OPTLOCK option lock bit
  Flash_Unlock_Memory();
  Flash_Unlock_OptionByte();
  
  // Disable RDP registers (Level 0 Protection)
  FLASH->OPTR = (FLASH->OPTR & 0xFFFFFF00) | 0xAA;
  
  // Check that no Flash memory operation is ongoing
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  // Applying new option to the flash
  FLASH->CR |= FLASH_CR_OPTSTRT;
  
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  // Set OPTLOCK
  FLASH->CR |= FLASH_CR_OPTLOCK;
  
  // Option byte loading
  FLASH->CR |= FLASH_CR_OBL_LAUNCH;
  
  Flash_Lock_Memory();
  
  NVIC_SystemReset();
}

void Flash_Enable_RDP(void)
{
  // Clear OPTLOCK option lock bit
  Flash_Unlock_Memory();
  Flash_Unlock_OptionByte();
  
  // Enable RDP registers (Level 1 Protection)
  FLASH->OPTR = (FLASH->OPTR & 0xFFFFFF00) | 0xBB;
  
  // Check that no Flash memory operation is ongoing
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  // Applying new option to the flash
  FLASH->CR |= FLASH_CR_OPTSTRT;
  
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  // Set OPTLOCK
  FLASH->CR |= FLASH_CR_OPTLOCK;
  
  // Option byte loading
  FLASH->CR |= FLASH_CR_OBL_LAUNCH;
  
  Flash_Lock_Memory();
  
  NVIC_SystemReset();
}

uint8_t Flash_check_RDP(void)
{
  if( (FLASH->OPTR & 0x000000FF) == 0xAA )
    return 0;
  return 1;
}

void Flash_clear_all_Errors(void)
{
  if( (FLASH->SR & FLASH_SR_PROGERR) == FLASH_SR_PROGERR)
    FLASH->SR |= FLASH_SR_PROGERR;
  if( (FLASH->SR & FLASH_SR_WRPERR) == FLASH_SR_WRPERR)
    FLASH->SR |= FLASH_SR_WRPERR;
  if( (FLASH->SR & FLASH_SR_PGAERR) == FLASH_SR_PGAERR)
    FLASH->SR |= FLASH_SR_PGAERR;
  if( (FLASH->SR & FLASH_SR_SIZERR) == FLASH_SR_SIZERR)
    FLASH->SR |= FLASH_SR_SIZERR;
  if( (FLASH->SR & FLASH_SR_PGSERR) == FLASH_SR_PGSERR)
    FLASH->SR |= FLASH_SR_PGSERR;
  if( (FLASH->SR & FLASH_SR_MISERR) == FLASH_SR_MISERR)
    FLASH->SR |= FLASH_SR_MISERR;
  if( (FLASH->SR & FLASH_SR_FASTERR) == FLASH_SR_FASTERR)
    FLASH->SR |= FLASH_SR_FASTERR;
}

//__ramfunc void FLASH_Program_FastPage(uint32_t flash_addr, uint32_t* pdata)
//{
//  uint32_t memory_location = flash_addr;
//  
//  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
//  
//  Flash_clear_all_Errors();
//  
//  /* Enter critical section: row programming should not be longer than 7 ms */
//  __disable_irq();
//  
//  FLASH->CR |= FLASH_CR_FSTPG;
//  
//  for(uint8_t i = 0; i < 32; i++, memory_location += 8U)
//  {
//    *(__IO uint32_t*)(memory_location) = *(uint32_t *)pdata++;
//    __ISB();
//    *(__IO uint32_t*)(memory_location + 4U) = *(uint32_t *)pdata++;
//  }
//  
//  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
//  
//  if( (FLASH->CR & FLASH_CR_FSTPG) == FLASH_CR_FSTPG)
//    FLASH->CR &= ~FLASH_CR_FSTPG;
//  
//  __enable_irq();
//}

void FLASH_Program_Page_DoubleWord(uint32_t flash_addr, uint64_t* pdata)
{
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  Flash_clear_all_Errors();
  
  FLASH->CR |= FLASH_CR_PG;
  
  /* Program first word */
  *(__IO uint32_t*)(flash_addr) = *pdata;
  
  /* Barrier to ensure programming is performed in 2 steps, in right order
    (independently of compiler optimization behavior) */
  __ISB();
  
  /* Program second word */
  *(__IO uint32_t*)(flash_addr + 4) = *pdata >> 32;
  
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  if( (FLASH->CR & FLASH_CR_PG) == FLASH_CR_PG)
    FLASH->CR &= ~FLASH_CR_PG;
}

// Program 256 bytes
void FLASH_Program_Page(uint32_t flash_addr, uint32_t* pdata)
{
  uint32_t memory_location = flash_addr;
  
  for(uint8_t i = 0; i < 32; i++, memory_location += 8U)
  {
    FLASH_Program_Page_DoubleWord(memory_location, (uint64_t*)pdata);
    pdata += 2;
  }
}

/*
** page_number: 0 to 63
*/
void Flash_Erase_Page(uint32_t page_number)
{
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  Flash_clear_all_Errors();
  
  FLASH->CR &= ~FLASH_CR_PNB;
  
  FLASH->CR |= FLASH_CR_PER;
  
  FLASH->CR |= page_number << FLASH_CR_PNB_Pos;
  
  FLASH->CR |= FLASH_CR_STRT;
  
  while( (FLASH->SR & FLASH_SR_BSY1) == FLASH_SR_BSY1);
  
  if( (FLASH->CR & FLASH_CR_PER) == FLASH_CR_PER)
    FLASH->CR &= ~FLASH_CR_PER;
}