/*
 * crypto.h
 *
 *  Created on: 18.01.2025
 *      Author: michael
 */

#ifndef COMPONENTS_CRYPTO_CRYPTO_H_
#define COMPONENTS_CRYPTO_CRYPTO_H_

#include <stdint.h>
#include <stddef.h>

/**
 * Decrypts data using AES-GCM.
 *
 * @param key Pointer to the decryption key.
 * @param key_len Length of the decryption key in bytes.
 * @param iv Pointer to the initialization vector.
 * @param iv_len Length of the initialization vector in bytes.
 * @param cipher_text Pointer to the encrypted data.
 * @param cipher_text_len Length of the encrypted data in bytes.
 * @param plaintext_buffer Pointer to the buffer where decrypted data will be stored.
 * @param plaintext_buffer_len Length of the plaintext buffer in bytes.
 * @param decrypted_data_len Pointer to the length of the decrypted data.
 * @return Size of the decrypted data in bytes.
 */
size_t decrypt_aes_gcm(const uint8_t* key, const uint8_t key_len, const uint8_t * iv, const uint8_t iv_len, const uint8_t * cipher_text, const size_t cipher_text_len, uint8_t* plaintext_buffer, const size_t plaintext_buffer_len, size_t* decrypted_data_len);


#endif /* COMPONENTS_CRYPTO_CRYPTO_H_ */
