#ifndef ZKP_VERIFY_H
#define ZKP_VERIFY_H

#include <stdbool.h>
#include <gmp.h>

#ifndef ZKP_BASE
#define ZKP_BASE 10
#endif

bool ZKP_verify(mpz_t y, void (*get_prover_commit)(char *), void (*challenge_prover)(bool), void (*get_prover_response)(char *), int rounds);

#endif