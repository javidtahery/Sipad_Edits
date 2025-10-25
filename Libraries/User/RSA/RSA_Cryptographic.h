#ifndef __RSA_CRYPTOGRAPHIC_H
#define __RSA_CRYPTOGRAPHIC_H

#include "main.h"

#include "Config/crypto_config.h"
#include "Config/os_port_config.h"


//extern RsaPrivateKey           privateKey;;


void Initiate_RSA_Key(RsaPrivateKey* privateKey, uint8_t* client_key, uint16_t client_key_size);
uint8_t decrypt_server_response(RsaPrivateKey* privateKey, uint8_t* cipherTxt, uint16_t cipherTxt_length, uint8_t* decrypted_buffer, uint8_t* decrypted_buffer_size);


#endif  /* __RSA_CRYPTOGRAPHIC_H */