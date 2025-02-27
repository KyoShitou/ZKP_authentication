#include "ZKP_proof.h"

mpz_t* p, g;
gmp_randstate_t state;


static void Argument_init(){
    static int initialized = 0;
    if (initialized == 0){
        mpz_init(p);
        mpz_init(g);
        initialized = 1;
        gmp_randinit_default(state);

        // Need to update
        unsigned long seed = (unsigned long)time(NULL);
        gmp_randseed_ui(state, seed);
        mpz_set_str(p, "23", 10);
        mpz_set_str(g, "5", 10);
    }
}

bool ZKP_verify(mpz_t y, void (*prover_commit)(mpz_t), void (*prover_response)(mpz_t, bool), int rounds){
    Argument_init();
    bool flag = true;

    for (int round = 0; round != rounds; round++){
        mpz_t challenge, commit, response;
        mpz_inits(challenge, commit, response, NULL);
        prover_commit(commit);
        mpz_urandomb(challenge, state, 1);

        if (mpz_cmp_ui(challenge, 0) == 0){
            prover_response(response, 0);
            mpz_t check;
            mpz_init(check);
            mpz_powm(check, g, response, p);
            if (mpz_cmp(check, commit) != 0){
                flag = false;
                break;
            }
        } else {
            prover_response(response, 1);
            mpz_t check;
            mpz_init(check);
            mpz_powm(check, g, response, p);

            mpz_t check2;
            mpz_init(check2);
            mpz_mul(check2, y, commit);
            mpz_mod(check2, check2, p);

            if (mpz_cmp(check, check2) != 0){
                flag = false;
                break;
            }
        }
    }
}