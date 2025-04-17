#include "ZKP_verify.h"

#include <time.h>

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
    }
}

bool ZKP_verify(char *y_str, void (*get_prover_commit)(char *), void (*challenge_prover)(bool), void (*get_prover_response)(char *), int rounds) {
    Argument_init();
    bool flag = true;

    mpz_t y;
    mpz_init(y);

    mpz_set_str(y, y_str, ZKP_BASE);

    for (int round = 0; round != rounds; round++){
        mpz_t challenge, commit, response;
        mpz_inits(challenge, commit, response, NULL);

        char prover_commit_str[1024];
        get_prover_commit(prover_commit_str);

        mpz_set_str(commit, prover_commit_str, ZKP_BASE);
        mpz_urandomb(challenge, state, 1);

        if (mpz_cmp_ui(challenge, 0) == 0){
            challenge_prover(0);

            char prover_response_str[1024];
            get_prover_response(prover_response_str);
            mpz_set_str(response, prover_response_str, ZKP_BASE);

            mpz_t check;
            mpz_init(check);
            mpz_powm(check, g, response, p);
            if (mpz_cmp(check, commit) != 0){
                flag = false;
                break;
            }
        } else {
            challenge_prover(1);

            char prover_response_str[1024];
            get_prover_response(prover_response_str);
            mpz_set_str(response, prover_response_str, ZKP_BASE);
            
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

    return flag;
}