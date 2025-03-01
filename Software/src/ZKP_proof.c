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
        mpz_set_str(p, "620398778414193301888966326923", 10);
        mpz_set_str(g, "5", 10);
        initialized = 1;
    }
}

void ZKP_proof(char *secret,
    void (*commit_to_verifier)(char* C),
    void (*get_verifier_challenge)(int *), 
    void (*respond_to_verifier)(char* Response)
){
    Argument_init();
    mpz_t x, r, C, response;
    mpz_inits(x, r, C,response, NULL);

    mpz_set_str(x, secret, ZKP_BASE);

    for ( ; ; ) {
    ZKP_commit(C, r, x);
    char commit_str[1024];
    mpz_get_str(commit_str, ZKP_BASE, C);
    commit_to_verifier(commit_str);

    int challenge;

    get_verifier_challenge(&challenge);

    if (challenge == 2) break;
    
    ZKP_response(response, r, x, challenge);

    char response_str[1024];
    mpz_get_str(response_str, ZKP_BASE, response);
    respond_to_verifier(response_str);
    }

    mpz_clears(r, C, response, NULL);
}

void ZKP_commit(mpz_t commit, mpz_t r, mpz_t x) 
// Sets r = rand(), commit = g^r mod p
{
    // Need to increase r length
    mpz_urandomb(r, state, 256);
    mpz_powm(commit, g, r, p);
    return;
}

void ZKP_response(mpz_t response, mpz_t r, mpz_t x, int challenge){
    if (challenge == 0){ // Reveal r
        mpz_set(response, r);
    } 
    else { // challenge == 1, send (x + r) mod (p - 1)
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