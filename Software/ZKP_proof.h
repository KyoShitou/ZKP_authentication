#include <stdbool.h>
#include <gmp.h>

bool ZKP_verify(mpz_t y, void (*prover_commit)(mpz_t), void (*prover_response)(mpz_t, bool), int rounds);
mpz_t* ZKP_prove(mpz_t x, bool b);