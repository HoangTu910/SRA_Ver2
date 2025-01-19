#ifndef ENCRYPT_H_
#include "asconCore.hpp"
#include "asconConstants.hpp"
#include <stdlib.h>
#include <time.h>

namespace Ascon {
int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k);
void generate_nonce(unsigned char nonce[ASCON_128_KEYBYTES]);
}
#endif