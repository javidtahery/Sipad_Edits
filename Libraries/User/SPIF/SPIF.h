#ifndef __SPIF_H
#define __SPIF_H

/* ---------------- Include ---------------- */
#include "main.h"
#include "..\Libraries\User\SIP_Protocol\sip_protocol.h"

/* ---------------- Define ---------------- */
#define SPIF_MANF_ID_32                                 0xEF15
#define SPIF_MANF_ID_64                                 0xEF16
#define SPIF_SECTOR_SIZE                                4096                    // 4 * 1024
#define SPIF_READ_SR_REG_TIMEOUT                        3
#define SPIF_MAX_NUM_RECORDS_IN_ONE_SECTOR              85                      // Total number of records saved in one sector
#define SPIF_MAX_NUM_USED_SECTORS                       1024                    // Total number of used SPIF sector
#define SPIF_MAX_NUM_RECORDS                            SPIF_MAX_NUM_RECORDS_IN_ONE_SECTOR*SPIF_MAX_NUM_USED_SECTORS

#define SPIF_APP_FW_FILE_START_SECTOR                   500
#define SPIF_BOOT_FW_FILE_START_SECTOR                  528
#define CRC_POLYNOMIAL                                  0x1021
#define SPIF_SETTING_SECTOR_INDEX                       260
#define SPIF_SYS_TEMP_PARAMS_SECTOR_INDEX               261

#define SPIF_CERT_FILE_BASE_SECTOR_INDEX                262
#define SPIF_CA_FILE_SECTOR_INDEX                       262
#define SPIF_CC_FILE_SECTOR_INDEX                       263
#define SPIF_CK_FILE_SECTOR_INDEX                       264

#define SPIF_OK                                         SUCCESS
#define SPIF_ERR                                        !SUCCESS


/* ---------------- Structure ---------------- */

typedef struct{
  uint8_t                       firmware_available;
  uint8_t                       upgrade_done;
  uint8_t                       upgrade_Error;
  uint8_t                       firmware_source;
  uint8_t                       firmware_md5[16];
  uint16_t                      firmware_CRC_value;
  uint32_t                      firmware_size;
  uint32_t                      device_version;
  uint32_t                      device_version_temp;
  uint32_t                      Boot_release_date;
}SYS_temp_params_typedef;


/* ---------------- Enum ---------------- */
enum{
  SERVER_CERT_FILE              = 0,
  CLIENT_CERT_FILE,             // 1
  CLIENT_KEY_FILE,              // 2
  BOOTLOADER_FWR,               // 3
  APPLICATION_FWR,              // 4
  FW_UPGRADE_INFO,              // 5
  SEND_RECORD,                  // 6
  SEND_INFORMATION,             // 7
  GET_SETTING,                  // 8
  SEND_INTERVAL,                // 9
  SEND_HASH,                    // 10
  NO_REQ,                       // 11
};

/* ---------------- Extern ---------------- */
extern SYS_temp_params_typedef          system_temp_params;
extern uint16_t SPIF_device_ID;


/* ---------------- Prototype ---------------- */
void SPIF_cfg_chip(void);
void SPIF_reset_chip(void);

void SPIF_write_setting(void);
void SPIF_read_setting(void);
void SPIF_read_system_IMEI(void);
void SPIF_read_system_serial(void);
void SPIF_Write_sys_temp_params(void);
void SPIF_Read_sys_temp_params(void);

uint16_t SPIF_read_certificate_file_size(uint8_t cert_file);
void SPIF_read_certificate_file(uint8_t cert_file, uint8_t* file_data, uint16_t file_size);
void SPIF_write_certificate_file(uint8_t cert_file, uint8_t* file_data, uint16_t file_size);

void SPIF_Save_FW_chunk(uint8_t* pdata, uint16_t data_length, uint16_t chunk_index);
void SPIF_save_HTTP_file_chunk(uint8_t* data, uint16_t data_length, uint8_t file_type, uint32_t* spif_file_wr_idx);
void SPIF_Save_Cert_size(uint16_t cert_size, uint8_t cert_type);
uint8_t Flash_check_FW_CRC(void);
void SPIF_Erase_certificates(void);
uint8_t SPIF_check_sector_validity(uint16_t sector_index);


#endif  /* __SPIF_H */