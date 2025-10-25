#include "circular_buffer.h"


FCB_header_typedef              FCB_read_header = {0};
FCB_header_typedef              FCB_write_header = {0};
FCB_info_typedef                FCB_profile = {0};


// This function check the not send status of given record from zero to "FCB_MAX_RECORDS_IN_ONE_SECTOR" within the header profile
// record_idx: from  0 to (FCB_MAX_RECORDS_IN_ONE_SECTOR - 1)
uint8_t FCB_check_server_x_record_nsend_status(FCB_header_typedef* header, uint8_t server_x, uint8_t record_idx)
{
  uint8_t nsend_status = RESET;
  
  if(record_idx < FCB_MAX_RECORDS_IN_ONE_SECTOR)
  {
    uint8_t nsend_array_idx = record_idx / 8;        // 8 = sizeof(uint8_t)
    uint8_t nsend_array_bit_idx = record_idx % 8;
    
    if(server_x == FCB_SERVER_1)
    {
      if((header->S1_nsend_records[nsend_array_idx] & (1<<nsend_array_bit_idx)) != 0)
        nsend_status = SET;
    }
    else if(server_x == FCB_SERVER_2)
    {
      if((header->S2_nsend_records[nsend_array_idx] & (1<<nsend_array_bit_idx)) != 0)
        nsend_status = SET;
    }
  }
  
  return nsend_status;
}


void FCB_modify_server_x_record_nsend_status(FCB_header_typedef* header, uint8_t server_x, uint8_t record_idx, uint8_t status)
{
  if(record_idx < FCB_MAX_RECORDS_IN_ONE_SECTOR)
  {
    uint8_t nsend_array_idx = record_idx / 8;        // 8 = sizeof(uint8_t)
    uint8_t nsend_array_bit_idx = record_idx % 8;
    
    if(server_x == FCB_SERVER_1)
    {
      if(status == SET)
        header->S1_nsend_records[nsend_array_idx] |= (1<<nsend_array_bit_idx);
      else if(status == RESET)
        header->S1_nsend_records[nsend_array_idx] &= ~(1<<nsend_array_bit_idx);
    }
    else if(server_x == FCB_SERVER_2)
    {
      if(status == SET)
        header->S2_nsend_records[nsend_array_idx] |= (1<<nsend_array_bit_idx);
      else if(status == RESET)
        header->S2_nsend_records[nsend_array_idx] &= ~(1<<nsend_array_bit_idx);
    }
  }
}


uint8_t FCB_calculate_header_server_x_nsend_count(FCB_header_typedef* header, uint8_t server_x)
{
  uint8_t temp_8 = 0;
  uint8_t record_count = 0;
  
  for(uint8_t i = 0; i < FCB_SECTOR_NOT_SEND_RECORD_SIZE; i++)
  {
    if(server_x == FCB_SERVER_1)
      temp_8 = header->S1_nsend_records[i];
    else if(server_x == FCB_SERVER_2)
      temp_8 = header->S2_nsend_records[i];
    
    for(uint8_t j = 0; j < 8; j++)
    {
      if((temp_8 & (1<<j)) != 0)
        record_count++;
    }
  }
  
  return record_count;
}


uint8_t FCB_check_sector_validity(uint16_t sector_index)
{
  uint8_t state = SET;
  
  if(sector_index >= 500 && sector_index < 532)
    state = RESET;
  else if(sector_index >= 260 && sector_index <= 264)
    state = RESET;
  
  return state;
}


void FCB_erase_sector(uint16_t sector_idx)
{
  uint32_t sector_address = sector_idx * SPIF_SECTOR_SIZE;
  SPIF_Erase_Sector(sector_address);
}


void FCB_increment_sector_index(uint16_t* sector_index)
{
  if(*sector_index >= FCB_LAST_SECTOR_INDEX)
    *sector_index = FCB_FIRST_SECTOR_INDEX;
  else
    *sector_index += 1;
}


// This function read the given sector index's header data.
void FCB_SPIF_read_header_x(FCB_header_typedef* header, uint16_t sector_idx)
{
  uint32_t spif_address = sector_idx * SPIF_SECTOR_SIZE;
  SPIF_Read((uint8_t*)header, spif_address, FCB_HEADER_SIZE);
}


// This function update the header information of sector x and writes the whole sector in SPIF
void FCB_update_header_of_sector_x(FCB_header_typedef* header, uint16_t sector_idx)
{
  uint8_t sector_buffer[SPIF_SECTOR_SIZE] = {0};
  uint32_t spif_address = sector_idx * SIZEOF_RECORD;
  spif_address += FCB_HEADER_SIZE;
  
  // Calculate header CRC
  uint16_t header_crc = CRC16_CCITT((uint8_t*)header, FCB_HEADER_SIZE - 2);
  header->header_crc_16 = header_crc;
  
  // Read sector records
  uint16_t records_size = FCB_MAX_RECORDS_IN_ONE_SECTOR * SIZEOF_RECORD;
  SPIF_Read(&sector_buffer[FCB_HEADER_SIZE], spif_address, records_size);
  
  // Update the header
  memcpy(sector_buffer, (uint8_t*)header, FCB_HEADER_SIZE);
  
  // Write the sector
  SPIF_Write_Sector(sector_buffer, sector_idx, SPIF_SECTOR_SIZE);
}


void FCB_merge_write_header_with_read_header(void)
{
  for(uint8_t i = 0; i < FCB_read_header.record_count; i++)
  {
    if( FCB_check_server_x_record_nsend_status(&FCB_read_header, FCB_SERVER_1, i) == RESET )
      FCB_modify_server_x_record_nsend_status(&FCB_write_header, FCB_SERVER_1, i, RESET);
  }
}


uint8_t FCB_find_next_read_sector(void)
{
  uint8_t sector_record_nsend_count = 0;
  uint8_t sector_valid_result = RESET;
  
  uint16_t start_sector_read_index = FCB_profile.sector_read_index;
  
  while(sector_record_nsend_count == 0)
  {
    // Read next sector header
    while(sector_valid_result == RESET)
    {
      // Increment the sector read index
      FCB_increment_sector_index(&FCB_profile.sector_read_index);
      
      // Protect important sectors
      sector_valid_result = FCB_check_sector_validity(FCB_profile.sector_read_index);
    }
    sector_valid_result = RESET;
    
    // To stop the infinite while loop and jump out of it.
    if(FCB_profile.sector_read_index == start_sector_read_index)
      break;
    
    // Read header until there is a nsend record
    if(FCB_profile.sector_read_index == FCB_profile.sector_write_index)
    {
      memcpy(&FCB_read_header, &FCB_write_header, FCB_HEADER_SIZE);
      sector_record_nsend_count = FCB_calculate_header_server_x_nsend_count(&FCB_read_header, FCB_SERVER_1);
    }
    else
    {
      FCB_SPIF_read_header_x(&FCB_read_header, FCB_profile.sector_read_index);
      sector_record_nsend_count = 0;
      
      // validating header information
      if(FCB_read_header.record_count <= FCB_MAX_RECORDS_IN_ONE_SECTOR)
      {
        uint16_t temp_16 = FCB_HEADER_SIZE - 2;
        uint16_t header_crc = CRC16_CCITT((uint8_t*)&FCB_read_header, temp_16);
        
        // header information is valid
        if(header_crc == FCB_read_header.header_crc_16)
          sector_record_nsend_count = FCB_calculate_header_server_x_nsend_count(&FCB_read_header, FCB_SERVER_1);
      }
    }
  }
  
  FCB_profile.sector_record_rd_idx = 0;
  
  return sector_record_nsend_count;
}



// This function should be called at system startup.
// This function finds the first not send record and the last record has been written.
// After finding last written record, the next sector will be selected for new record write.
void FCB_startup_record_analyze(void)
{
  FCB_header_typedef    temp_header;
  
  uint32_t min_unixtime = UINT32_MAX;
  uint32_t max_unixtime = 1;            // Something greater than zero
  
  // To find the first sector to be read
  FCB_profile.sector_read_index = 0;
  FCB_profile.sector_write_index = 0;
  FCB_profile.s1_nsend_count = 0;
  FCB_profile.s2_nsend_count = 0;
  FCB_profile.sector_record_rd_idx = 0;
  FCB_profile.sector_record_wr_idx = 0;
  
  for(uint16_t sec_idx = FCB_FIRST_SECTOR_INDEX; sec_idx <= FCB_LAST_SECTOR_INDEX; sec_idx++)
  {
    // Protect important sectors
    if(FCB_check_sector_validity(sec_idx) == SET)
    {
      FCB_SPIF_read_header_x(&temp_header, sec_idx);
      
      // validating header information
      if(temp_header.record_count <= FCB_MAX_RECORDS_IN_ONE_SECTOR)
      {
        uint16_t temp_16 = FCB_HEADER_SIZE - 2;
        uint16_t header_crc = CRC16_CCITT((uint8_t*)&temp_header, temp_16);
        
        // header information is valid
        if(header_crc == temp_header.header_crc_16)
        {
          record_typedef        temp_record = {0};
          uint16_t record_crc;
          uint32_t spif_address = sec_idx * SPIF_SECTOR_SIZE;
          spif_address += FCB_HEADER_SIZE;
          
          // Analyze each record
          for(uint8_t i = 0; i < temp_header.record_count; i++)
          {
            SPIF_Read((uint8_t*)&temp_record, spif_address, SIZEOF_RECORD);
            record_crc = CRC16_CCITT((uint8_t*)&temp_record, (SIZEOF_RECORD - 2));
            if(temp_record.crc_16_h == (record_crc >> 8)
               && temp_record.crc_16_l == (record_crc & 0xFF))
            {
              // Finding the last write index
              // There is one record newer than last checked record
              if(temp_record.gps_elements.unixtime >= max_unixtime)
              {
                max_unixtime = temp_record.gps_elements.unixtime;
                FCB_profile.sector_write_index = sec_idx;
                FCB_profile.sector_record_wr_idx = i;
              }
              if( FCB_check_server_x_record_nsend_status(&temp_header, FCB_SERVER_1, i) == SET )
              {
                // Extracting server not send count
                FCB_profile.s1_nsend_count++;
                
                // Finding read index
                if(temp_record.gps_elements.unixtime < min_unixtime)
                {
                  min_unixtime = temp_record.gps_elements.unixtime;
                  FCB_profile.sector_read_index = sec_idx;
                  FCB_profile.sector_record_rd_idx = i;
                }
              }
            }
            
            spif_address += SIZEOF_RECORD;
          }
        }
      }
    }
  }     // End of for(uint16_t sec_idx = 0;..)
  
  
  if(FCB_profile.s1_nsend_count > 0)
  {
    // Preparing Write sector
    FCB_increment_sector_index(&FCB_profile.sector_write_index);
    FCB_profile.sector_record_wr_idx = 0;
    
    // Protect important sectors
    while(FCB_check_sector_validity(FCB_profile.sector_write_index) == RESET)
    {
      FCB_increment_sector_index(&FCB_profile.sector_write_index);
    }
    
    // Formating the next sector
    FCB_SPIF_read_header_x(&FCB_write_header, FCB_profile.sector_write_index);
    uint8_t sector_record_nsend_count = 0;
    // validating header information
    if(FCB_write_header.record_count <= FCB_MAX_RECORDS_IN_ONE_SECTOR)
    {
      uint16_t temp_16 = FCB_HEADER_SIZE - 2;
      uint16_t header_crc = CRC16_CCITT((uint8_t*)&FCB_write_header, temp_16);
      
      // header information is valid
      if(header_crc == FCB_write_header.header_crc_16)
        sector_record_nsend_count = FCB_calculate_header_server_x_nsend_count(&FCB_write_header, FCB_SERVER_1);
    }
    
    FCB_profile.s1_nsend_count -= sector_record_nsend_count;
    
    if(FCB_profile.sector_write_index == FCB_profile.sector_read_index)
      FCB_find_next_read_sector();
    else
      FCB_SPIF_read_header_x(&FCB_read_header, FCB_profile.sector_read_index);
  }
  else if(FCB_profile.sector_write_index > FCB_FIRST_SECTOR_INDEX)
  {
    FCB_increment_sector_index(&FCB_profile.sector_write_index);
    
    FCB_profile.s1_nsend_count = 0;
    FCB_profile.s2_nsend_count = 0;
    FCB_profile.sector_read_index = FCB_FIRST_SECTOR_INDEX;
    FCB_profile.sector_record_rd_idx = 0;
    FCB_profile.sector_record_wr_idx = 0;
  }
  else
  {
    FCB_profile.s1_nsend_count = 0;
    FCB_profile.s2_nsend_count = 0;
    FCB_profile.sector_read_index = FCB_FIRST_SECTOR_INDEX;
    FCB_profile.sector_write_index = FCB_FIRST_SECTOR_INDEX;
    FCB_profile.sector_record_rd_idx = 0;
    FCB_profile.sector_record_wr_idx = 0;
  }
  
  memset(&FCB_write_header, 0, FCB_HEADER_SIZE);
  FCB_erase_sector(FCB_profile.sector_write_index);
}


// This function reads multi record and update the count of read records.
void FCB_read_records(uint8_t* record_count)
{
  uint8_t iterator = 0;
  *record_count = iterator;
  
  // End of reading sector. Header should be updated and the next sector should be read
  if(FCB_profile.sector_record_rd_idx == FCB_read_header.record_count)
  {
    uint8_t find_next_read_sector = SET;
    
    // Update header information
    // If the read sector is write sector just update the write header information
    if(FCB_profile.sector_read_index == FCB_profile.sector_write_index)
    {
      FCB_merge_write_header_with_read_header();
      
      if(FCB_profile.sector_record_wr_idx > FCB_profile.sector_record_rd_idx)
      {
        memcpy(&FCB_read_header, &FCB_write_header, FCB_HEADER_SIZE);
        FCB_profile.sector_record_rd_idx = 0;
        
        find_next_read_sector = RESET;
      }
    }
    else
      FCB_update_header_of_sector_x(&FCB_read_header, FCB_profile.sector_read_index);
    
    if(find_next_read_sector == SET)
    {
      uint8_t nsend = FCB_find_next_read_sector();
      if(nsend == 0)
        asm("nop");
    }
  }
  
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
  
  // Multi records Reading
  uint8_t max_record_read_count = FCB_read_header.record_count - FCB_profile.sector_record_rd_idx;
  if(max_record_read_count > FCB_MAX_RECORD_READ_MULTI)
    max_record_read_count = FCB_MAX_RECORD_READ_MULTI;
  
  record_typedef        temp_record = {0};
  uint16_t record_crc;
  uint32_t spif_address = FCB_profile.sector_read_index * SPIF_SECTOR_SIZE;
  spif_address += FCB_HEADER_SIZE;
  spif_address += FCB_profile.sector_record_rd_idx * SIZEOF_RECORD;
  
  for(iterator = 0; iterator < max_record_read_count; iterator++)
  {
    if( FCB_check_server_x_record_nsend_status(&FCB_read_header, FCB_SERVER_1, (FCB_profile.sector_record_rd_idx + iterator)) == SET )
    {
      SPIF_Read((uint8_t*)&temp_record, spif_address, SIZEOF_RECORD);
      record_crc = CRC16_CCITT((uint8_t*)&temp_record, (SIZEOF_RECORD - 2));
      if(temp_record.crc_16_h == (record_crc >> 8)
         && temp_record.crc_16_l == (record_crc & 0xFF))
      {
        memcpy((uint8_t*)&server_records[iterator], (uint8_t*)&temp_record, SIZEOF_RECORD);
      }
      else
      {
        // Bug in record data. Clear the not send bit
        FCB_modify_server_x_record_nsend_status(&FCB_read_header, FCB_SERVER_1, (FCB_profile.sector_record_rd_idx + iterator), RESET);
        if(FCB_profile.s1_nsend_count > 0)
          FCB_profile.s1_nsend_count--;
        
        break;
      }
    }
    else
    {
      break;
    }
    
    spif_address += SIZEOF_RECORD;
  }
  
  *record_count = iterator;
  if(iterator == 0)
    FCB_profile.sector_record_rd_idx++;
}


// 
void FCB_write_records(record_typedef* record)
{
  uint16_t record_crc = 0;
  uint32_t spif_address = 0;
  
  // Calculate and update record crc-16
  record_crc = CRC16_CCITT((uint8_t*)record, (SIZEOF_RECORD - 2));
  record->crc_16_l = record_crc & 0xFF;
  record->crc_16_h = record_crc >> 8;
  
  if(FCB_write_header.record_count == FCB_MAX_RECORDS_IN_ONE_SECTOR)
  {
    // Merge Read header and Write header
    if(FCB_profile.sector_write_index == FCB_profile.sector_read_index)
    {
      FCB_merge_write_header_with_read_header();
    }
    
    // Calculate header CRC
    uint16_t header_crc = CRC16_CCITT((uint8_t*)&FCB_write_header, FCB_HEADER_SIZE - 2);
    FCB_write_header.header_crc_16 = header_crc;
    
    // Copy edited header to read header
    if(FCB_profile.sector_write_index == FCB_profile.sector_read_index)
    {
      memcpy(&FCB_read_header, &FCB_write_header, FCB_HEADER_SIZE);
    }
    
    spif_address = FCB_profile.sector_write_index * SPIF_SECTOR_SIZE;
    SPIF_Write_Page_Continous((uint8_t*)&FCB_write_header, spif_address, FCB_HEADER_SIZE);
    
    uint8_t sector_record_nsend_count = 0;
    uint8_t sector_valid_result = RESET;
    
    // Find next valid sector
    while(sector_valid_result == RESET)
    {
      // Increment the sector write index
      FCB_increment_sector_index(&FCB_profile.sector_write_index);
      
      // Protect important sectors
      sector_valid_result = FCB_check_sector_validity(FCB_profile.sector_write_index);
    }
    
    /* Erasing the next write sector */
    if(FCB_profile.sector_write_index == FCB_profile.sector_read_index)
    {
      memcpy(&FCB_write_header, &FCB_read_header, FCB_HEADER_SIZE);
      FCB_profile.sector_record_rd_idx = 0;
      sector_valid_result = RESET;
      
      // Reset gsm sending process
      if(sending_data_is_in_progress == SET)
      {
        sending_data_is_in_progress = RESET;
        if( GSM_status == GSM_LOGIN || GSM_status == GSM_SENDING_PACKET )
          GSM_status = GSM_IDLE;
      }
      
      // Find next valid read sector
      FCB_find_next_read_sector();
      
      sector_record_nsend_count = 0;
      sector_record_nsend_count = FCB_calculate_header_server_x_nsend_count(&FCB_write_header, FCB_SERVER_1);
    }
    else
    {
      FCB_SPIF_read_header_x(&FCB_write_header, FCB_profile.sector_write_index);
      
      // validating header information
      if(FCB_write_header.record_count <= FCB_MAX_RECORDS_IN_ONE_SECTOR)
      {
        uint16_t temp_16 = FCB_HEADER_SIZE - 2;
        uint16_t header_crc = CRC16_CCITT((uint8_t*)&FCB_write_header, temp_16);
        
        // header information is valid
        if(header_crc == FCB_write_header.header_crc_16)
        {
          sector_record_nsend_count = FCB_calculate_header_server_x_nsend_count(&FCB_write_header, FCB_SERVER_1);
        }
      }
    }
    
    if(sector_record_nsend_count > 0)
    {
      if(FCB_profile.s1_nsend_count <= sector_record_nsend_count)
        FCB_profile.s1_nsend_count = 0;
      else
        FCB_profile.s1_nsend_count -= sector_record_nsend_count;
    }
    
    memset(&FCB_write_header, 0, FCB_HEADER_SIZE);
    FCB_erase_sector(FCB_profile.sector_write_index);
    FCB_profile.sector_record_wr_idx = 0;
  }     // if(FCB_write_header.record_count == FCB_MAX_RECORDS_IN_ONE_SECTOR)
  
  
  if(FCB_write_header.record_count < FCB_MAX_RECORDS_IN_ONE_SECTOR)
  {
    FCB_modify_server_x_record_nsend_status(&FCB_write_header, FCB_SERVER_1, FCB_profile.sector_record_wr_idx, SET);
    
    spif_address = FCB_profile.sector_write_index * SPIF_SECTOR_SIZE;
    spif_address += FCB_HEADER_SIZE;
    spif_address += FCB_profile.sector_record_wr_idx * SIZEOF_RECORD;
    
    SPIF_Write_Page_Continous((uint8_t*)record, spif_address, SIZEOF_RECORD);
    
    FCB_write_header.record_count++;
    FCB_profile.sector_record_wr_idx++;
    
    if(FCB_profile.s1_nsend_count == 0)
    {
      // To protect
      if(FCB_profile.read_is_modified == SET)
      {
        FCB_profile.read_is_modified = RESET;
        
        // Update header information
        // If the read sector is write sector just update the write header information
        if(FCB_profile.sector_read_index == FCB_profile.sector_write_index)
          FCB_merge_write_header_with_read_header();
        else
          FCB_update_header_of_sector_x(&FCB_read_header, FCB_profile.sector_read_index);
      }
      
      FCB_profile.sector_read_index = FCB_profile.sector_write_index;
      memcpy(&FCB_read_header, &FCB_write_header, FCB_HEADER_SIZE);
      FCB_profile.sector_record_rd_idx = 0;
    }
    if(FCB_profile.s1_nsend_count < FCB_MAX_RECORDS_COUNT)
      FCB_profile.s1_nsend_count++;
  }
}


void FCB_modify_sent_records(void)
{
  for(uint8_t iterator = 0; iterator < server_packet_count; iterator++)
  {
    FCB_modify_server_x_record_nsend_status(&FCB_read_header, FCB_SERVER_1, (FCB_profile.sector_record_rd_idx + iterator), RESET);
    if(FCB_profile.s1_nsend_count > 0)
      FCB_profile.s1_nsend_count--;
  }
  
  FCB_profile.read_is_modified = SET;
  
  if(FCB_profile.sector_read_index == FCB_profile.sector_write_index)
  {
    FCB_merge_write_header_with_read_header();
    FCB_profile.read_is_modified = RESET;
  }
    
  FCB_profile.sector_record_rd_idx += server_packet_count;
}


void FCB_erase_all_records(void)
{
  uint16_t sector_idx;
  
  for(sector_idx = FCB_FIRST_SECTOR_INDEX; sector_idx <= FCB_LAST_SECTOR_INDEX; sector_idx++)
  {
    if(FCB_check_sector_validity(sector_idx) == SET)
    {
      SPIF_Erase_Sector(sector_idx * SPIF_SECTOR_SIZE);
      IWDG_ReloadCounter();
    }
  }
  
  FCB_profile.s1_nsend_count = 0;
  FCB_profile.s2_nsend_count = 0;
  FCB_profile.sector_read_index = FCB_FIRST_SECTOR_INDEX;
  FCB_profile.sector_write_index = FCB_FIRST_SECTOR_INDEX;
  FCB_profile.sector_record_rd_idx = 0;
  FCB_profile.sector_record_wr_idx = 0;
}