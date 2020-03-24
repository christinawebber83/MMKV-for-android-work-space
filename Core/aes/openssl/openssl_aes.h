/*
 * Copyright 2002-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_AES_H
# define HEADER_AES_H
#ifdef  __cplusplus

#include "openssl_opensslconf.h"
#include "openssl_arm_arch.h"
#include <stddef.h>
#include "../../MMKVPredef.h"

namespace openssl {

/*
 * Because array size can't be a const in C, the following two are macros.
 * Both sizes are in bytes.
 */
# define AES_MAXNR 14
# define AES_BLOCK_SIZE 16

/* This should be a hidden type, but EVP requires that the size be known */
struct AES_KEY {
# ifdef AES_LONG
    unsigned long rd_key[4 * (AES_MAXNR + 1)];
# else
    unsigned int rd_key[4 * (AES_MAXNR + 1)];
# endif
    int rounds;
};

void AES_cfb128_encrypt(const unsigned char *in, unsigned char *out,
                        size_t length, const AES_KEY *key,
                        unsigned char *ivec, int *num);

void AES_cfb128_decrypt(const unsigned char *in, unsigned char *out,
                        size_t length, const AES_KEY *key,
                        unsigned char *ivec, int *num);

} // namespace openssl

#if __ARM_MAX_ARCH__ > 0

#ifndef MMKV_ANDROID

extern "C" int openssl_aes_arm_set_encrypt_key(const unsigned char *userKey, const int bits, void *key);
extern "C" void openssl_aes_arm_encrypt(const unsigned char *in, unsigned char *out, const void *key);

#define AES_set_encrypt_key(userKey, bits, key) openssl_aes_arm_set_encrypt_key(userKey, bits, key)

#define AES_encrypt(in, out, key) openssl_aes_arm_encrypt(in, out, key)

#else // MMKV_ANDROID

#if __ARM_MAX_ARCH__ <= 7

extern "C" int openssl_aes_arm_set_encrypt_key(const unsigned char *userKey, const int bits, void *key);
extern "C" void openssl_aes_arm_encrypt(const unsigned char *in, unsigned char *out, const void *key);

#define AES_set_encrypt_key(userKey, bits, key) openssl_aes_arm_set_encrypt_key(userKey, bits, key)
#define AES_encrypt(in, out, key) openssl_aes_arm_encrypt(in, out, key)

#else // __ARM_MAX_ARCH__ > 7

extern "C" int openssl_aes_armv8_set_encrypt_key(const unsigned char *userKey, const int bits, void *key);
extern "C" void openssl_aes_armv8_encrypt(const unsigned char *in, unsigned char *out, const void *key);

typedef int (*aes_set_encrypt_t)(const unsigned char *userKey, const int bits, void *key);
typedef void (*aes_encrypt_t)(const unsigned char *in, unsigned char *out, const void *key);

namespace openssl {

int AES_C_set_encrypt_key(const unsigned char *userKey, const int bits, void *key);

void AES_C_encrypt(const unsigned char *in, unsigned char *out, const void *key);

} // namespace openssl

extern aes_set_encrypt_t AES_set_encrypt_key;
extern aes_encrypt_t AES_encrypt;

#endif // __ARM_MAX_ARCH__ <= 7

#endif // MMKV_ANDROID

#else // __ARM_MAX_ARCH__ <= 0

namespace openssl {

int AES_set_encrypt_key(const unsigned char *userKey, const int bits, AES_KEY *key);

void AES_encrypt(const unsigned char *in, unsigned char *out, const AES_KEY *key);

} // namespace openssl

#endif // __ARM_MAX_ARCH__ > 0

#endif
#endif
