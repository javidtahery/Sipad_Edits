#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#include "main.h"
#include "..\Libraries\User\SIP_Protocol\sip_protocol.h"

// Flash Circular Buffer
#define FCB_SECTOR_NOT_SEND_RECORD_SIZE                 11
#define FCB_MAX_RECORDS_IN_ONE_SECTOR                   84                     // Total number of records saved in one sector
#define FCB_MAX_USED_SECTORS                            1536                    // Total number of used SPIF sector
#define FCB_FIRST_SECTOR_INDEX                          0
#define FCB_LAST_SECTOR_INDEX                           1536                    // FCB_FIRST_SECTOR_INDEX + FCB_MAX_USED_SECTORS - 1
#define FCB_MAX_RECORD_READ_MULTI                       10
#define FCB_MAX_RECORDS_COUNT                           129024                  // 1536* * 84
#define FCB_HEADER_SIZE                                 sizeof(FCB_header_typedef)

#define FCB_SERVER_1                                    0
#define FCB_SERVER_2                                    1

/* ---------------- Structure ---------------- */

// If each record is 48 bytes we can store 84 records in one sector
// Total number of bytes is 48 * 84 = 4032
// We need 11 bytes for FCB_SECTOR_NOT_SEND_RECORD_SIZE: 11 * 8 = 88 Recorde max
// Header of each sector has 25 bytes size.
__packed typedef struct
{
  uint8_t       record_count;
  uint8_t       S1_nsend_records[FCB_SECTOR_NOT_SEND_RECORD_SIZE];// Each bit represent not send record status
  uint8_t       S2_nsend_records[FCB_SECTOR_NOT_SEND_RECORD_SIZE];// Each bit represent not send record status
  uint16_t      header_crc_16;
}FCB_header_typedef;


typedef struct{
  uint32_t s1_nsend_count;
  uint32_t s2_nsend_count;
  uint16_t sector_write_index;
  uint16_t sector_read_index;
  uint8_t  sector_record_wr_idx;
  uint8_t  sector_record_rd_idx;
  uint8_t  read_is_modified;
}FCB_info_typedef;


extern FCB_info_typedef                FCB_profile;

void FCB_startup_record_analyze(void);
void FCB_read_records(uint8_t* record_count);
//void FCB_write_records(record_typedef* record);
void FCB_modify_sent_records(void);
void FCB_erase_all_records();

#endif