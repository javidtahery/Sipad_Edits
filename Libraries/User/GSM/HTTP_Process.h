#ifndef __HTTP_PROCESS_H
#define __HTTP_PROCESS_H

/* ---------------- Include ---------------- */
#include "main.h"

/* ---------------- Define ---------------- */

#define FW_UPGRADE_INFO_FILE_SIZE               52
#define MAX_FS_FILE_READ_SIZE                   500
#define CRC32_POLYNOMIAL                        0xEDB88320

/* ---------------- Enum ---------------- */
enum{
  HTTP_STAGE_RESET_MODULE                         = 0,
  HTTP_STAGE_TEST_SERIAL,//----------------------// 1
  HTTP_STAGE_CFG_FOREGROUND_CNT,//---------------// 2
  HTTP_STAGE_CFG_APN,//--------------------------// 3
  HTTP_STAGE_REG_TCP_STACK,//--------------------// 4
  HTTP_STAGE_ACTIVATE_FGCNT,//-------------------// 5
  HTTP_STAGE_QUERY_LOCAL_IP,//-------------------// 6
  HTTP_STAGE_MANAGE_FILE_PROCESS,//--------------// 7
  HTTP_STAGE_ENTER_URL_LENGTH,//-----------------// 8
  HTTP_STAGE_ENTER_URL,//------------------------// 9
  HTTP_STAGE_POST_DATA,//------------------------// 10
  HTTP_STAGE_SEND_POST_DATA,//-------------------// 11
  HTTP_STAGE_READ_POST_DATA,//-------------------// 12
  HTTP_STAGE_DELETE_RAM_FILES,//-----------------// 13
  HTTP_STAGE_GET,//------------------------------// 14
  HTTP_STAGE_DL_FILE,//--------------------------// 15
  HTTP_STAGE_FS_OPEN_FILE,//---------------------// 16
  HTTP_STAGE_FS_SET_POSITION,//------------------// 17
  HTTP_STAGE_FS_READ_FILE,//---------------------// 18
  HTTP_STAGE_FS_CLOSE_FILE,//--------------------// 19
  HTTP_STAGE_DEACT_PDP,//------------------------// 20
  HTTP_STAGE_SEEK_OK,//--------------------------// 21
};
	

/* ---------------- Structure ---------------- */
typedef struct{
  uint8_t       dl_ca;
  uint8_t       dl_cc;
  uint8_t       dl_ck;
  uint8_t       dl_boot;
  uint8_t       dl_app;
  uint8_t       dl_fwr_info;
  uint8_t       send_record;
  uint8_t       send_information;
  uint8_t       get_setting;
  uint8_t       send_interval;
  uint8_t       send_hash;
  uint8_t       pdp_activated;
  uint8_t       file_chunk_idx;
  uint8_t       request_type;
  uint16_t      file_size;
  uint32_t      file_handle;
  uint8_t       checksum_retry;
  uint8_t       checksum_file_dl_retry;
}http_tmp_typedef;


typedef struct{
  uint32_t      boot_size;
  uint8_t       boot_chunk_count;
  uint32_t      boot_release_date;
  uint8_t       boot_md5[16];
  uint32_t      app_size;
  uint8_t       app_chunk_count;
  uint32_t      app_release_date;
  uint8_t       app_md5[16];
  uint32_t      chunk_crc32;
}http_fwu_info_typedef;

/* ---------------- Extern ---------------- */
extern uint8_t enable_http_process;
extern uint8_t http_has_request;

extern uint8_t halt_http_process;
extern uint16_t halt_http_process_counter;
extern uint8_t number_of_http_process_run;
extern uint8_t http_firmware_upgrade_ready;
extern uint16_t api_interval_counter;

extern http_tmp_typedef         HTTP_Params;


/* ---------------- Prototype ---------------- */
void Sipaad_check_SPIF_Certs(void);
void HTTP_preconfig_http_requirments(void);
uint16_t GSM_HTTP_routine_pro(uint8_t function_index);

#endif