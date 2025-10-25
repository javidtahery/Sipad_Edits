#ifndef __GENERAL_FUNCTIONS_H
#define __GENERAL_FUNCTIONS_H

unsigned char GF_find_cell_in_array(unsigned char* array, unsigned short int array_length, unsigned char data_to_find, unsigned short int* cell_index);
unsigned char GF_extract_SubArray_from_array(unsigned char* array, unsigned char* SubArray, unsigned char start_index, char terminator, unsigned char length_limit);
unsigned char GF_convert_string_hex_number_to_integer(unsigned char character);
unsigned char compareArray(unsigned char a[],unsigned char b[],unsigned int size);
unsigned char GF_calculate_checksum_8_XOR(unsigned char* data, unsigned short int data_length);
unsigned char GF_calculate_number_digit_count(unsigned long long number);

#endif /* __GENERAL_FUNCTIONS_H */