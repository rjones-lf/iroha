//
//  sha3.h
//  SpirtualCoin
//
//  Created by Kaji Satoshi on 2016/09/07.
//  Copyright © 2016年 Kaji Satoshi. All rights reserved.
//

#ifndef sha3_h
#define sha3_h

#include <stddef.h>
#include <stdint.h>

/* 'Words' here refers to uint64_t */
#define SHA3_KECCAK_SPONGE_WORDS \
(((1600)/8/*bits to byte*/)/sizeof(uint64_t))

/* 'Words' here refers to uint64_t */


#ifndef SHA3_ROTL64
#define SHA3_ROTL64(x, y) \
(((x) << (y)) | ((x) >> ((sizeof(uint64_t)*8) - (y))))
#endif

typedef struct sha3_context_ {
    uint64_t saved;             /* the portion of the input message that we
                                 * didn't consume yet */
    union {                     /* Keccak's state */
        uint64_t s[SHA3_KECCAK_SPONGE_WORDS];
        uint8_t sb[SHA3_KECCAK_SPONGE_WORDS * 8];
    };
    unsigned byteIndex;         /* 0..7--the next byte after the set one
                                 * (starts from 0; 0--none are buffered) */
    unsigned wordIndex;         /* 0..24--the next word to integrate input
                                 * (starts from 0) */
    unsigned capacityWords;
    
    unsigned numOutputBytes;/* the double size of the hash output in
                             * words (e.g. 16 for Keccak 512) */
} sha3_context;

void sha3_Init256(void *priv);
void sha3_Init384(void *priv);
void sha3_Init512(void *priv);
void sha3_Update(void *priv, void const *bufIn, size_t len);
void sha3_Finalize(void *priv, unsigned char *out);
void sha3_256(const unsigned char *message, size_t message_len, unsigned char *out);
void sha3_384(const unsigned char *message, size_t message_len, unsigned char *out);
void sha3_512(const unsigned char *message, size_t message_len, unsigned char *out);

#endif
