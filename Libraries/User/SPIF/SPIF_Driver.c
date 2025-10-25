#include "SPIF_Driver.h"

// Read the chip ID W25Q32FV. ID: 0xEF15
uint16_t SPIF_ReadID(void)
{
  uint16_t Tmp = 0;
  
  SPIF_CS_LOW;
  SPIx_ReadWriteByte(SPIF_ManufactDeviceID);	    
  SPIx_ReadWriteByte(SPIF_DUMMY_BYTE); 	    
  SPIx_ReadWriteByte(SPIF_DUMMY_BYTE);
  SPIx_ReadWriteByte(SPIF_DUMMY_BYTE);
  Tmp |= SPIx_ReadWriteByte(SPIF_DUMMY_BYTE)<<8;
  Tmp |= SPIx_ReadWriteByte(SPIF_DUMMY_BYTE);
  SPIF_CS_HIGH;
  
  return Tmp;
}

// SPIF write enable
// Set WEL  
void SPIF_Write_Enable(void)   
{
  SPIF_CS_LOW;                                  //Select chip   
  SPIx_ReadWriteByte(SPIF_WriteEnable);         //Send write enable  
  SPIF_CS_HIGH;                                 //Deselect chip    	      
}

// SPIF write is forbidden
// Clear WEL 
void SPIF_Write_Disable(void)   
{
  SPIF_CS_LOW;                                  //Select chip   
  SPIx_ReadWriteByte(SPIF_WriteDisable);        //Send write inhibit command    
  SPIF_CS_HIGH;                                 //Deselect chip    	      
}

void SPIF_SR_Volatile_Write_Enable(void)
{
  SPIF_CS_LOW;                                  //Select chip   
  SPIx_ReadWriteByte(SPIF_Volatile_SR_WriteEnable);     //Send Write Status Register command    
  SPIF_CS_HIGH; 
}

// Write SPIF status register
// Only SPR, TB, BP2, BP1, BP0 (bit 7, 5, 4, 3, 2) can be written!!!
void SPIF_Write_SR_Volatile(uint8_t data, uint8_t status_register)   
{
  SPIF_SR_Volatile_Write_Enable();
  SPIF_CS_LOW;                                  //Select chip
  SPIx_ReadWriteByte(status_register);          //Send Write Status Register command    
  SPIx_ReadWriteByte(data);               	//Write one byte  
  SPIF_CS_HIGH;                                 //Deselect chip    	      
}

uint8_t SPIF_Read_SR(uint8_t status_register)   
{
  uint8_t byte =0;
  
  SPIF_CS_LOW;                                  //Select chip  
  SPIx_ReadWriteByte(status_register);          //Send Read Status Register command   
  byte = SPIx_ReadWriteByte(SPIF_DUMMY_BYTE);              //Read a byte  
  SPIF_CS_HIGH;                                 //Deselect chip
  
  return byte;   
}

void SPIF_Write_SR(uint8_t sr_reg, uint8_t data)
{
  SPIF_Write_Enable();
  
  SPIF_CS_LOW;
  SPIx_ReadWriteByte(sr_reg);
  SPIx_ReadWriteByte(data);
  SPIF_CS_HIGH;
}

void SPIF_enable_RST_Pin(void)
{
  uint8_t tmp = 0;
  
  tmp = SPIF_Read_SR(SPIF_Read_Status_Register_3);
  tmp = Set_bit(tmp, 7);
  SPIF_Write_SR_Volatile(tmp, SPIF_Write_Status_Register_3);
}

void SPIF_enable_full_read_strength(void)
{
  uint8_t tmp = 0;
  
  tmp = SPIF_Read_SR(SPIF_Read_Status_Register_3);
  tmp = Reset_bit(tmp, 5);
  tmp = Reset_bit(tmp, 6);
  SPIF_Write_SR_Volatile(tmp, SPIF_Write_Status_Register_3);
}

void SPIF_enable_WP_pin(void)
{
  uint8_t tmp = 0;
  
  tmp = SPIF_Read_SR(SPIF_Read_Status_Register_1);
  tmp = Set_bit(tmp, 7);
  SPIF_Write_SR_Volatile(tmp, SPIF_Write_Status_Register_1);
  
  tmp = SPIF_Read_SR(SPIF_Read_Status_Register_2);
  tmp = Reset_bit(tmp, 0);
  SPIF_Write_SR_Volatile(tmp, SPIF_Write_Status_Register_2);
}

void SPIF_Wait_Busy(void)   
{
  uint8_t SR_REG = 0;
  do                                            // Waiting for the BUSY bit to clear
  {
    SR_REG = SPIF_Read_SR(SPIF_Read_Status_Register_1);
  }
  while( (SR_REG&0x01) == 0x01 );
}

// Read SPIF
// Start reading the specified length of data at the specified address
// pBuffer: data storage area
// ReadAddr: the address to start reading (24bit)
// NumByteToRead: the number of bytes to read (maximum 65535)
void SPIF_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)   
{
//  uint16_t i;
  SPIF_CS_LOW;                                          //Select chip   
  SPIx_ReadWriteByte(SPIF_ReadData);                    //Send read command   
  SPIx_ReadWriteByte((uint8_t)((ReadAddr)>>16));        //Send a 24-bit address    
  SPIx_ReadWriteByte((uint8_t)((ReadAddr)>>8));   
  SPIx_ReadWriteByte((uint8_t)ReadAddr);
  
  SPIF_read_SPIx_DMA(pBuffer, NumByteToRead);
//  for(i=0; i < NumByteToRead; i++)
//    pBuffer[i] = SPIx_ReadWriteByte(SPIF_DUMMY_BYTE);              //Cyclic reading  
  
  SPIF_CS_HIGH;                                         //Deselect chip    	      
} 

// SPI writes less than 256 bytes of data in one page (0~65535)
// Start writing the maximum 256 bytes of data at the specified address
// pBuffer: data storage area
// WriteAddr: the address to start writing (24bit)
// NumByteToWrite: The number of bytes to write (maximum 256), which should not exceed the remaining bytes of the page!!! 
void SPIF_Write_Page(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
//  uint16_t i;
  SPIF_Write_Enable();                                  //SET WEL 
  SPIF_CS_LOW;                                          //Select chip   
  SPIx_ReadWriteByte(SPIF_PageProgram);                 //Send page write command   
  SPIx_ReadWriteByte((uint8_t)((WriteAddr)>>16));       //Send a 24-bit address    
  SPIx_ReadWriteByte((uint8_t)((WriteAddr)>>8));   
  SPIx_ReadWriteByte((uint8_t)WriteAddr);
  
  SPIF_write_SPIx_DMA(pBuffer, NumByteToWrite);
//  for(i = 0; i < NumByteToWrite; i++)
//    SPIx_ReadWriteByte(pBuffer[i]);                     //Cyclic write number  
  SPIF_CS_HIGH;                                         //Deselect chip
  SPIF_Wait_Busy();					//Waiting for writing to end
}

void SPIF_Write_Sector(uint8_t* pBuffer, uint16_t sector_index, uint16_t NumByteToWrite)
{
  uint32_t start_address = sector_index * SPIF_SECTOR_SIZE;
  uint8_t iterator = 0;
  
  SPIF_Erase_Sector(start_address);
  
  if(NumByteToWrite == SPIF_SECTOR_SIZE)
  {
    for(iterator = 0; iterator < SPIF_MAX_NUM_PAGE_PER_SECTOR; iterator++)
      SPIF_Write_Page( (pBuffer + (iterator * SPIF_PAGE_SIZE)), (start_address + (iterator * SPIF_PAGE_SIZE)), SPIF_PAGE_SIZE);
  }
  else
  {
    uint8_t max_page_count = NumByteToWrite / SPIF_PAGE_SIZE;
    for(iterator = 0; iterator < max_page_count; iterator++)
      SPIF_Write_Page( (pBuffer + (iterator * SPIF_PAGE_SIZE)), (start_address + (iterator * SPIF_PAGE_SIZE)), SPIF_PAGE_SIZE);
    
    uint16_t remain_data_length = NumByteToWrite % SPIF_PAGE_SIZE;
    if(remain_data_length > 0)
      SPIF_Write_Page( (pBuffer + (iterator * SPIF_PAGE_SIZE)), (start_address + (max_page_count * SPIF_PAGE_SIZE)), remain_data_length);
  }
}

void SPIF_Write_address_without_erase(uint8_t* pBuffer, uint32_t start_address, uint16_t NumByteToWrite)
{
  uint8_t iterator = 0;
  uint8_t max_page_count = NumByteToWrite / SPIF_PAGE_SIZE;
  
  for(iterator = 0; iterator < max_page_count; iterator++)
    SPIF_Write_Page( (pBuffer + (iterator * SPIF_PAGE_SIZE)), (start_address + (iterator * SPIF_PAGE_SIZE)), SPIF_PAGE_SIZE);
  
  uint16_t remain_data_length = NumByteToWrite % SPIF_PAGE_SIZE;
  if(remain_data_length > 0)
    SPIF_Write_Page( (pBuffer + (iterator * SPIF_PAGE_SIZE)), (start_address + (max_page_count * SPIF_PAGE_SIZE)), remain_data_length);
}


// This function writes the data from given address. It is no matter the address is from begining of the page or the middle of the page. This function covers the page wrapping.
void SPIF_Write_Page_Continous(uint8_t* data, uint32_t start_address, uint16_t data_length)
{
  uint32_t page_remained_bytes = 0;
  uint32_t sector_remained_bytes = 0;
  uint32_t wr_address = start_address;
  
  uint16_t chunk_size = data_length;
  uint16_t data_idx = 0;
  uint16_t num_bytes = 0;
  
  while(chunk_size != 0)
  {
    sector_remained_bytes =  wr_address % SPIF_SECTOR_SIZE;
    sector_remained_bytes = SPIF_SECTOR_SIZE - sector_remained_bytes;
    
    page_remained_bytes = wr_address % SPIF_PAGE_SIZE;
    page_remained_bytes = SPIF_PAGE_SIZE - page_remained_bytes;
    
    // Data is always less than 'SPIF_PAGE_SIZE'
    if(chunk_size >= page_remained_bytes)
      num_bytes = page_remained_bytes;
    else
      num_bytes = chunk_size;
    
    if(sector_remained_bytes < num_bytes)
      num_bytes = sector_remained_bytes;
    
    SPIF_Write_Page( (data+data_idx), wr_address, num_bytes);
    wr_address += num_bytes;
    data_idx += num_bytes;
    chunk_size -= num_bytes;
  }
}


void SPIF_Erase_Sector(uint32_t Dst_Addr)   
{
  SPIF_Write_Enable();                          //SET WEL 	 
  SPIF_Wait_Busy();   
  SPIF_CS_LOW;                                  //Select chip   
  SPIx_ReadWriteByte(SPIF_SectorErase_4KB);         //Send sector erase command 
  SPIx_ReadWriteByte((uint8_t)((Dst_Addr)>>16));//Send a 24-bit address    
  SPIx_ReadWriteByte((uint8_t)((Dst_Addr)>>8));   
  SPIx_ReadWriteByte((uint8_t)Dst_Addr);  
  SPIF_CS_HIGH;                                 //Deselect chip    	      
  SPIF_Wait_Busy();   				//Waiting for erase to complete
} 

void SPIF_Erase_Block(uint32_t Dst_Addr)
{
  SPIF_Write_Enable();                          //SET WEL 	 
  SPIF_Wait_Busy();   
  SPIF_CS_LOW;                                  //Select chip   
  SPIx_ReadWriteByte(SPIF_BlockErase);          //Send sector erase command 
  SPIx_ReadWriteByte((uint8_t)((Dst_Addr)>>16));//Send a 24-bit address    
  SPIx_ReadWriteByte((uint8_t)((Dst_Addr)>>8));   
  SPIx_ReadWriteByte((uint8_t)Dst_Addr);  
  SPIF_CS_HIGH;                                 //Deselect chip    	      
  SPIF_Wait_Busy();   				//Waiting for erase to complete
}

void SPIF_Erase_Security_Register(uint32_t sec_reg)
{
  SPIF_Write_Enable();
  SPIF_CS_LOW;
  SPIx_ReadWriteByte(SPIF_CMD_ERASE_SECURITY_REG);
  SPIx_ReadWriteByte( (sec_reg>>16)&0xFF);
  SPIx_ReadWriteByte( (sec_reg>>8)&0xFF);
  SPIx_ReadWriteByte( sec_reg&0xFF);
  SPIF_CS_HIGH;
  SPIF_Wait_Busy();
}

void SPIF_Program_Security_Register(uint32_t sec_reg, uint8_t* pdata, uint8_t data_length)
{
  SPIF_Erase_Security_Register(sec_reg);
  
  SPIF_Write_Enable();
  SPIF_CS_LOW;
  SPIx_ReadWriteByte(SPIF_CMD_PROGRAM_SECURITY_RED);
  SPIx_ReadWriteByte( (sec_reg>>16)&0xFF);
  SPIx_ReadWriteByte( (sec_reg>>8)&0xFF);
  SPIx_ReadWriteByte( sec_reg&0xFF);
  for(uint8_t iterator = 0; iterator < data_length; iterator++)
    SPIx_ReadWriteByte(*(uint8_t*)(pdata++));
  SPIF_CS_HIGH;
  SPIF_Wait_Busy();
}

void SPIF_Read_Security_Register(uint32_t sec_reg, uint8_t* pdata, uint8_t data_length)
{
  SPIF_CS_LOW;
  SPIx_ReadWriteByte(SPIF_CMD_READ_SECURITY_REG);
  SPIx_ReadWriteByte( (sec_reg>>16)&0xFF);
  SPIx_ReadWriteByte( (sec_reg>>8)&0xFF);
  SPIx_ReadWriteByte( sec_reg&0xFF);
  SPIx_ReadWriteByte(SPIF_DUMMY_BYTE);
  for(uint8_t iterator = 0; iterator < data_length; iterator++)
    *(uint8_t*)(pdata++) = SPIx_ReadWriteByte(SPIF_DUMMY_BYTE);
  SPIF_CS_HIGH;
  SPIF_Wait_Busy();
}

void SPIF_Lock_Security_Reg_Permenant(uint32_t sec_reg)
{
  uint8_t tmp_data = 0;
  tmp_data = SPIF_Read_SR(SPIF_Read_Status_Register_2);
  if(sec_reg == SPIF_SECURITY_REGISTE_1_ADDR)
    tmp_data |= 0x08;
  else if(sec_reg == SPIF_SECURITY_REGISTE_2_ADDR)
    tmp_data |= 0x10;
  else if(sec_reg == SPIF_SECURITY_REGISTE_3_ADDR)
    tmp_data |= 0x20;
  SPIF_Write_SR(SPIF_Write_Status_Register_2, tmp_data);
}

void SPIF_Lock_Security_Reg_Temporary(uint32_t sec_reg)
{
  uint8_t tmp_data = 0;
  tmp_data = SPIF_Read_SR(SPIF_Read_Status_Register_2);
  if(sec_reg == SPIF_SECURITY_REGISTE_1_ADDR)
    tmp_data |= 0x08;
  else if(sec_reg == SPIF_SECURITY_REGISTE_2_ADDR)
    tmp_data |= 0x10;
  else if(sec_reg == SPIF_SECURITY_REGISTE_3_ADDR)
    tmp_data |= 0x20;
  SPIF_Write_SR_Volatile(SPIF_Write_Status_Register_2, tmp_data);
}

void SPIF_Unlock_Security_Reg(uint32_t sec_reg)
{
  uint8_t tmp_data = 0;
  tmp_data = SPIF_Read_SR(SPIF_Read_Status_Register_2);
  if(sec_reg == SPIF_SECURITY_REGISTE_1_ADDR)
    tmp_data &= ~(0x08);
  else if(sec_reg == SPIF_SECURITY_REGISTE_2_ADDR)
    tmp_data &= ~(0x10);
  else if(sec_reg == SPIF_SECURITY_REGISTE_3_ADDR)
    tmp_data &= ~(0x20);
  SPIF_Write_SR_Volatile(SPIF_Write_Status_Register_2, tmp_data);
  tmp_data = SPIF_Read_SR(SPIF_Read_Status_Register_2);
  tmp_data++;
}

//SPIF_Read_Security_Register(SPIF_SECURITY_REGISTE_1_ADDR, (uint8_t*)&DataLogger_SN, sizeof(DataLogger_SN));