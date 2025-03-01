#ifndef ZKP_PROOF_H
#define ZKP_PROOF_H

#include <stdbool.h>
#include <gmp.h>

#ifndef ZKP_BASE
#define ZKP_BASE 10
#endif

void ZKP_proof(char *secret,
    void (*commit_to_verifier)(char* C),
    void (*verifier_challenge)(int *), // 0 - reveal r, 1 - send (x + r) mod (p - 1), 2 - terminate
    void (*respond)(char* r)
);

void ZKP_commit(mpz_t commit, mpz_t r, mpz_t x);
void ZKP_response(mpz_t response, mpz_t r, mpz_t x, int request);

#endif