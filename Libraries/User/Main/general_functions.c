#include "general_functions.h"

unsigned char GF_find_cell_in_array(unsigned char* array, unsigned short int array_length, unsigned char data_to_find, unsigned short int* cell_index)
{
  unsigned char fn_result = 0;
  
  for(unsigned short int i = 0; i < array_length; i++)
  {
    if(*(array+i) == data_to_find)
    {
      *cell_index = i;
      fn_result = 1;
      break;
    }
  }
  
  return fn_result;
}

unsigned char GF_extract_SubArray_from_array(unsigned char* array, unsigned char* SubArray, unsigned char start_index, char terminator, unsigned char length_limit)
{
  unsigned char iterator = 0;
  
  for(iterator = 0; iterator < length_limit && array[start_index+iterator] != terminator; iterator++)
    *(SubArray+iterator) = array[start_index+iterator];
  
  return iterator;
}

unsigned char GF_convert_string_hex_number_to_integer(unsigned char character)
{  
  if(character >= '0' && character <= '9')
    return (character - 0x30);
  if(character == 'a' || character == 'A')
    return 0x0A;
  if(character == 'b' || character == 'B')
    return 0x0B;
  if(character == 'c' || character == 'C')
    return 0x0C;
  if(character == 'd' || character == 'D')
    return 0x0D;
  if(character == 'e' || character == 'E')
    return 0x0E;
  if(character == 'f' || character == 'F')
    return 0x0F;
  
  return 0;
}

unsigned char compareArray(unsigned char a[],unsigned char b[],unsigned int size)
{
  unsigned int i;
  for(i=0; i < size; i++)
  {
    if(a[i] != b[i])
      return 1;
  }
  
  return 0;
}

unsigned char GF_calculate_checksum_8_XOR(unsigned char* data, unsigned short int data_length)
{
  unsigned char XOR_result = 0x00;
  
  for(unsigned short int iterator = 0; iterator < data_length; iterator++)
    XOR_result ^= *(data+iterator);
  
  return XOR_result;
}

unsigned char GF_calculate_number_digit_count(unsigned long long number)
{
  unsigned char digit_count = 0;
  unsigned long long tmp_number = number;
  
  if(number > 0)
  {
    while(tmp_number > 0)
    {
      digit_count++;
      tmp_number /= 10;
    }
  }
  else
  {
    digit_count = 0;
  }
  
  return digit_count;
}