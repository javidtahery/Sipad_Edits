#include "RSA_Cryptographic.h"



void Initiate_RSA_Key(RsaPrivateKey* privateKey, uint8_t* client_key, uint16_t client_key_size)
{
  rsaInitPrivateKey(privateKey);
  
  pemImportRsaPrivateKey((char*)client_key, client_key_size, privateKey);
}

uint8_t decrypt_server_response(RsaPrivateKey* privateKey, uint8_t* cipherTxt, uint16_t cipherTxt_length, uint8_t* decrypted_buffer, uint8_t* decrypted_buffer_size)
{
  error_t error;
  size_t  msg_output_length;
  
  //Decrypt the premaster secret using the server private key
  error = rsaesPkcs1v15Decrypt(privateKey, cipherTxt, cipherTxt_length, decrypted_buffer, *decrypted_buffer_size, &msg_output_length);
  if(error == NO_ERROR )
    return msg_output_length;
  
  return 0;
}
