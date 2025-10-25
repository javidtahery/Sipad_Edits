#ifndef __SPIF_DRIVER_H
#define __SPIF_DRIVER_H

/* ---------------- Include ---------------- */
#include "main.h"

/* ---------------- Define ---------------- */
//Instruction list
#define SPIF_DUMMY_BYTE                                 0x00
#define SPIF_WriteEnable		                0x06 
#define SPIF_WriteDisable		                0x04 
#define SPIF_Volatile_SR_WriteEnable	                0x50
#define SPIF_Read_Status_Register_1                     0x05
#define SPIF_Read_Status_Register_2                     0x35
#define SPIF_Read_Status_Register_3                     0x15
#define SPIF_Write_Status_Register_1                    0x01
#define SPIF_Write_Status_Register_2                    0x31
#define SPIF_Write_Status_Register_3                    0x11
#define SPIF_ReadData			                0x03
#define SPIF_FastReadData		                0x0B 
#define SPIF_PageProgram		                0x02 
#define SPIF_BlockErase			                0xD8    // 64KB 
#define SPIF_SectorErase_4KB		                0x20    // 4KB 
#define SPIF_ChipErase			                0xC7 
#define SPIF_PowerDown			                0xB9 
#define SPIF_ReleasePowerDown	                        0xAB 
#define SPIF_DeviceID			                0xAB 
#define SPIF_ManufactDeviceID	                        0x90 
#define SPIF_JedecDeviceID		                0x9F 
#define SPIF_BLOCK_SECTOR_LOCK                          0x36
#define SPIF_BLOCK_SECTOR_UNLOCK                        0x39
#define SPIF_READ_BLOCK_SECTOR_LOCK                     0x3D
#define SPIF_CMD_ERASE_SECURITY_REG                     0x44
#define SPIF_CMD_PROGRAM_SECURITY_RED                   0x42
#define SPIF_CMD_READ_SECURITY_REG                      0x48

#define SPIF_SECURITY_REGISTE_1_ADDR                    0x001000
#define SPIF_SECURITY_REGISTE_2_ADDR                    0x002000
#define SPIF_SECURITY_REGISTE_3_ADDR                    0x003000

#define SPIF_PAGE_SIZE                                  256
#define SPIF_SECTOR_SIZE                                4096
#define SPIF_MAX_NUM_PAGE_PER_SECTOR                    16

/* ---------------- Structure ---------------- */


/* ---------------- Enum ---------------- */


/* ---------------- Extern ---------------- */


/* ---------------- Prototype ---------------- */
uint16_t SPIF_ReadID(void);
void SPIF_enable_RST_Pin(void);
void SPIF_enable_full_read_strength(void);
void SPIF_enable_WP_pin(void);
void SPIF_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void SPIF_Write_Page(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPIF_Write_Sector(uint8_t* pBuffer, uint16_t sector_index, uint16_t NumByteToWrite);
void SPIF_Write_address_without_erase(uint8_t* pBuffer, uint32_t start_address, uint16_t NumByteToWrite);
void SPIF_Write_Page_Continous(uint8_t* data, uint32_t start_address, uint16_t data_length);
void SPIF_Erase_Sector(uint32_t Dst_Addr);
void SPIF_Erase_Security_Register(uint32_t sec_reg);
void SPIF_Program_Security_Register(uint32_t sec_reg, uint8_t* pdata, uint8_t data_length);
void SPIF_Read_Security_Register(uint32_t sec_reg, uint8_t* pdata, uint8_t data_length);
void SPIF_Lock_Security_Reg_Permenant(uint32_t sec_reg);
void SPIF_Lock_Security_Reg_Temporary(uint32_t sec_reg);
void SPIF_Unlock_Security_Reg(uint32_t sec_reg);


#endif  /* __SPIF_DRIVER_H */