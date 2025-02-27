#ifndef ZKP_PROOF_H
#define ZKP_PROOF_H

#include <gmp.h>
#include <stdbool.h>

void ZKP_proof(mpz_t secret,
    void (*commit_to_verifier)(mpz_t C),
    bool* (*verifier_challenge)(void), // User pointer to indicate end of request (if NULL)
    void (*respond)(mpz_t r)
);

void ZKP_commit(mpz_t commit, mpz_t r, mpz_t x);
void ZKP_response(mpz_t response, mpz_t r, mpz_t x, bool request);

#endif