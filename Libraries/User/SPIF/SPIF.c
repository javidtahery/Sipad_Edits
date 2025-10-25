#include "SPIF.h"
#include "general_functions.h"


SYS_temp_params_typedef                 system_temp_params;

uint16_t SPIF_device_ID = 0;

extern uint16_t firmware_chunk_index;


void SPIF_cfg_chip(void)
{
  SPIF_device_ID = SPIF_ReadID();
  
  SPIF_enable_RST_Pin();
  SPIF_enable_full_read_strength();
  //  SPIF_enable_WP_pin();
}


void SPIF_reset_chip(void)
{
  SPIF_SPIx->CR1 &= ~SPI_CR1_SPE;  
  SPIF_RST_GPIO->BRR = SPIF_RST_PIN;
  Delay(2);
  SPIF_RST_GPIO->BSRR = SPIF_RST_PIN;
  SPIF_SPIx->CR1 |= SPI_CR1_SPE;
  
  SPIF_cfg_chip();
}


void SPIF_write_setting(void)
{
  SPIF_Write_Sector((uint8_t*)&setting, SPIF_SETTING_SECTOR_INDEX, sizeof(setting_typeDef));
}


void SPIF_read_setting(void)
{
  SPIF_Read((uint8_t*)&setting, SPIF_SETTING_SECTOR_INDEX*SPIF_SECTOR_SIZE, sizeof(setting_typeDef));
  
  if(setting.setting_metadata != SETTING_META_DATA)
  {
    AVL_Initiate_Setting();
    SPIF_write_setting();     
  }
  else
  {
    AVL_check_setting_values();
  }
}


void SPIF_read_system_IMEI(void)
{
  uint64_t memory_imei = 0;
  
  system_error.IMEI_error = RESET;
  
  SPIF_Read_Security_Register(SPIF_SECURITY_REGISTE_1_ADDR, (uint8_t*)&memory_imei, 8);
  if(memory_imei == 0)
  {
    // Error in SPI Flash read process
  }
  else if(memory_imei == UINT64_MAX)
  {
    // Memory is clear
    save_system_IMEI = SET;
  }
  else
  {
    if(GF_calculate_number_digit_count(memory_imei) == 15)
    {
      system_IMEI = memory_imei;
    }
    else
    {
      // there is bug in IMEI
      system_error.IMEI_error = SET;
      save_system_IMEI = SET;
    }
  }
}


void SPIF_read_system_serial(void)
{
  uint64_t memory_serial = 0;
  
  system_error.IMEI_error = RESET;
  
  SPIF_Read_Security_Register(SPIF_SECURITY_REGISTE_3_ADDR, (uint8_t*)&memory_serial, 8);
  if(memory_serial == 0)
  {
    system_Serial = 0;
  }
  else if(memory_serial == UINT64_MAX)
  {
    system_Serial = 0;
  }
  else
  {
    system_Serial = memory_serial;
  }
}


void SPIF_Write_sys_temp_params(void)
{
  SPIF_Write_Sector((uint8_t*)&system_temp_params, SPIF_SYS_TEMP_PARAMS_SECTOR_INDEX, sizeof(SYS_temp_params_typedef));
}


void SPIF_Read_sys_temp_params(void)
{
  uint32_t addr = SPIF_SYS_TEMP_PARAMS_SECTOR_INDEX * SPIF_SECTOR_SIZE;
  
  SPIF_Read((uint8_t*)&system_temp_params, addr, sizeof(SYS_temp_params_typedef));
  
  if(system_temp_params.device_version == 0 || system_temp_params.device_version == 0xFFFFFFFF)
  {
    system_temp_params.device_version = INITIAL_DEVICE_VERSION;
    SPIF_Write_sys_temp_params();
  }
}


uint16_t SPIF_read_certificate_file_size(uint8_t cert_file)
{
  uint16_t tmp_16 = 0;
  uint32_t cert_addr = (SPIF_CERT_FILE_BASE_SECTOR_INDEX + cert_file) * SPIF_SECTOR_SIZE;
  
  SPIF_Read((uint8_t*)&tmp_16, cert_addr, 2);
  
  return tmp_16;
}


void SPIF_read_certificate_file(uint8_t cert_file, uint8_t* file_data, uint16_t file_size)
{
  uint32_t cert_addr = (SPIF_CERT_FILE_BASE_SECTOR_INDEX + cert_file) * SPIF_SECTOR_SIZE + 2;         // The first 2 bytes are file size
  
  SPIF_Read(file_data, cert_addr, file_size);
}


void SPIF_write_certificate_file(uint8_t cert_file, uint8_t* file_data, uint16_t file_size)
{
  SPIF_Write_Sector(file_data, (SPIF_CERT_FILE_BASE_SECTOR_INDEX + cert_file), file_size);
}


void SPIF_Save_FW_chunk(uint8_t* pdata, uint16_t data_length, uint16_t chunk_index)
{
  uint32_t sector = chunk_index / 16;
  uint32_t middle_of_sector = chunk_index % 16;
  
  uint32_t addr = (SPIF_APP_FW_FILE_START_SECTOR + sector) * SPIF_SECTOR_SIZE;
  uint8_t page_count = data_length / SPIF_PAGE_SIZE;
  uint8_t page_remain = data_length % SPIF_PAGE_SIZE;
  
  if(middle_of_sector == 0)
    SPIF_Erase_Sector(addr);
  else
    addr += (middle_of_sector * SPIF_PAGE_SIZE);
  
  for(uint8_t iterator = 0; iterator < page_count; iterator++)
  {
    SPIF_Write_Page( pdata, addr, SPIF_PAGE_SIZE);
    
    pdata += SPIF_PAGE_SIZE;
    addr += SPIF_PAGE_SIZE;
  }
  if(page_remain > 0)
  {
    SPIF_Write_Page( pdata, addr, page_remain);
  }
}


void SPIF_save_HTTP_file_chunk(uint8_t* data, uint16_t data_length, uint8_t file_type, uint32_t* spif_file_wr_idx)
{
  uint16_t sector_index = 0;
  
  if(file_type < BOOTLOADER_FWR)
  {
    sector_index = SPIF_CERT_FILE_BASE_SECTOR_INDEX + file_type;
  }
  else
  {
    if(file_type == BOOTLOADER_FWR)
      sector_index = SPIF_BOOT_FW_FILE_START_SECTOR;
    else if(file_type == APPLICATION_FWR)
      sector_index = SPIF_APP_FW_FILE_START_SECTOR;
  }
  
  uint32_t start_address = sector_index * SPIF_SECTOR_SIZE;
  uint32_t page_remained_bytes = 0;
  uint32_t wr_idx = *spif_file_wr_idx;
  
  uint16_t chunk_size = data_length;
  uint16_t data_idx = 0;
  uint16_t num_bytes = 0;
  
  while(chunk_size != 0)
  {
    if(file_type < BOOTLOADER_FWR)
    {
      if(wr_idx == 2)
        SPIF_Erase_Sector(start_address);
    }
    else
    {
      if(wr_idx % SPIF_SECTOR_SIZE == 0)
        SPIF_Erase_Sector(start_address + wr_idx);
    }
    
    uint32_t sector_remained_bytes =  wr_idx % SPIF_SECTOR_SIZE;
    sector_remained_bytes = SPIF_SECTOR_SIZE - sector_remained_bytes;
    
    page_remained_bytes = wr_idx % SPIF_PAGE_SIZE;
    page_remained_bytes = SPIF_PAGE_SIZE - page_remained_bytes;
    
    if(chunk_size >= page_remained_bytes)
      num_bytes = page_remained_bytes;
    else
      num_bytes = chunk_size;
    
    if(sector_remained_bytes < num_bytes)
      num_bytes = sector_remained_bytes;
    
    SPIF_Write_address_without_erase((data+data_idx), (start_address + wr_idx), num_bytes);
    wr_idx += num_bytes;
    data_idx += num_bytes;
    chunk_size -= num_bytes;
  }
  
  *spif_file_wr_idx = wr_idx;
}


void SPIF_Save_Cert_size(uint16_t cert_size, uint8_t cert_type)
{
  // Save certificate file size
  uint16_t sector_index = SPIF_CERT_FILE_BASE_SECTOR_INDEX + cert_type;
  uint32_t start_address = sector_index * SPIF_SECTOR_SIZE;
  
  SPIF_Write_address_without_erase((uint8_t*)&cert_size, start_address, 2);
}


void SPIF_Erase_certificates(void)
{
  // Root Cert
  uint16_t sector_index = SPIF_CA_FILE_SECTOR_INDEX;
  uint32_t start_address = sector_index * SPIF_SECTOR_SIZE;
  SPIF_Erase_Sector(start_address);
  
  // Client Cert
  sector_index = SPIF_CC_FILE_SECTOR_INDEX;
  start_address = sector_index * SPIF_SECTOR_SIZE;
  SPIF_Erase_Sector(start_address);
  
  // Private Key
  sector_index = SPIF_CK_FILE_SECTOR_INDEX;
  start_address = sector_index * SPIF_SECTOR_SIZE;
  SPIF_Erase_Sector(start_address);
}


uint8_t SPIF_check_sector_validity(uint16_t sector_index)
{
  uint8_t state = SET;
  
  if(sector_index >= 500 && sector_index < 532)
  {
    state = RESET;
  }
  else if(sector_index >= 260 && sector_index <= 264)
  {
    state = RESET;
  }
  
  return state;
}