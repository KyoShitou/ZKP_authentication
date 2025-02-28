#ifndef ZKP_VERIFY_H
#define ZKP_VERIFY_H

#include <stdbool.h>
#include <gmp.h>

bool ZKP_verify(mpz_t y, void (*prover_commit)(mpz_t), void (*challenge_prover)(bool), void (*prover_response)(mpz_t), int rounds);

#endif