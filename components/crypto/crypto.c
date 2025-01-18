/*
 * crypto.c
 *
 *  Created on: 18.01.2025
 *      Author: michael
 */

#include "crypto.h"
#include <mbedtls/gcm.h>

size_t decrypt_aes_gcm(const uint8_t* key, const uint8_t key_len, const uint8_t * iv, const uint8_t iv_len, const uint8_t * cipher_text, const size_t cipher_text_len, uint8_t* plaintext_buffer, const size_t plaintext_buffer_len, size_t* decrypted_data_len) {
    uint8_t resultCode = 0;
    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    resultCode = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, key_len*8 );

    if (resultCode != 0) {
        mbedtls_gcm_free(&gcm);
        return resultCode;
    }

    resultCode = mbedtls_gcm_starts(&gcm, MBEDTLS_GCM_DECRYPT, iv, iv_len);

    if (resultCode != 0) {
        mbedtls_gcm_free(&gcm);
        return resultCode;
    }

    resultCode = mbedtls_gcm_update(&gcm, cipher_text, cipher_text_len, plaintext_buffer, plaintext_buffer_len, decrypted_data_len);

    mbedtls_gcm_free(&gcm);
    return resultCode;
}
