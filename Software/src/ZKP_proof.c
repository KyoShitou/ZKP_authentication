#include "ZKP_proof.h"

mpz_t p, g;
gmp_randstate_t state;


static void Argument_init(){
    static int initialized = 0;
    if (initialized == 0){
        mpz_init(p);
        mpz_init(g);
        initialized = 1;
        gmp_randinit_default(state);

        // TODO: Need to update
        unsigned long seed = (unsigned long)time(NULL);
        gmp_randseed_ui(state, seed);
        mpz_set_str(p, "23", 10);
        mpz_set_str(g, "5", 10);
    }
}

void ZKP_proof(mpz_t x,
    void (*commit_to_verifier)(mpz_t C),
    bool* (*verifier_challenge)(void), // User pointer to indicate end of request (if NULL)
    void (*respond)(mpz_t Response)
){
    Argument_init();
    mpz_t r, C, response;
    mpz_inits(r, C,response, NULL);

    for ( ; ; ) {
    ZKP_commit(C, r, x);
    commit_to_verifier(C);

    bool *challenge = verifier_challenge();

    if (challenge == NULL) break;
    
    ZKP_response(response, r, x, *challenge);
    respond(response);
    }

    mpz_clears(r, C, response, NULL);
}

void ZKP_commit(mpz_t commit, mpz_t r, mpz_t x) 
// Sets r = rand(), commit = g^r mod p
{
    // Need to increase r length
    mpz_urandomb(r, state, 16);
    mpz_powm(commit, g, r, p);
    return;
}

void ZKP_response(mpz_t response, mpz_t r, mpz_t x, bool request){
    if (request == 0){ // Reveal r
        mpz_set(response, r);
    } 
    else { // send (x + r) mod (p - 1)
        mpz_t p_dec, x_pls_r, one;
        mpz_inits(p_dec, x_pls_r, one, NULL);
        mpz_set_ui(one, 1);

        mpz_sub(p_dec, p, one);
        mpz_add(x_pls_r, x, r);
        mpz_mod(response, x_pls_r, p_dec);

        mpz_clears(p_dec, x_pls_r, one, NULL);
    }
    return;
}