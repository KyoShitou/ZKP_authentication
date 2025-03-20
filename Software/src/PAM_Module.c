#define _GNU_SOURCE
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#include <syslog.h>

#include "ZKP_verify.h" // Include your ZKP verification logic

#define ZKP_ROUNDS 3 // Number of challenge-response rounds

// Function prototypes
static int get_user_public_key(const char *username, char *y_str, size_t max_len);
static void get_prover_commit(char *commit);
static void challenge_prover(bool challenge);
static void get_prover_response(char *response);

// PAM authentication function
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    const char *username;
    char y_str[1024];

    // Get the username
    if (pam_get_user(pamh, &username, NULL) != PAM_SUCCESS)
    {
        pam_syslog(pamh, LOG_ERR, "Failed to retrieve username.");
        return PAM_AUTH_ERR;
    }

    // Retrieve the stored public key y for the user
    if (get_user_public_key(username, y_str, sizeof(y_str)) != 0)
    {
        pam_syslog(pamh, LOG_ERR, "Failed to retrieve public key for user %s.", username);
        return PAM_AUTH_ERR;
    }

    // Perform ZKP verification
    mpz_t y;
    mpz_init(y);
    mpz_set_str(y, y_str, 10); // Convert string to GMP big integer

    if (!ZKP_verify(y, get_prover_commit, challenge_prover, get_prover_response, ZKP_ROUNDS))
    {
        pam_syslog(pamh, LOG_ERR, "ZKP verification failed.");
        mpz_clear(y);
        return PAM_AUTH_ERR;
    }

    mpz_clear(y);

    // Authentication successful
    pam_syslog(pamh, LOG_INFO, "User %s authenticated successfully using ZKP.", username);
    return PAM_SUCCESS;
}

// PAM account management (not required but can be expanded)
PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

// Function to retrieve user's public key from a secure file
static int get_user_public_key(const char *username, char *y_str, size_t max_len)
{
    FILE *file = fopen("/etc/security/zkp_users", "r");
    if (!file)
        return -1;

    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        char user[256], key[1024];
        if (sscanf(line, "%255s %1023s", user, key) == 2)
        {
            if (strcmp(user, username) == 0)
            {
                strncpy(y_str, key, max_len - 1);
                y_str[max_len - 1] = '\0';
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -1;
}

// Callbacks for ZKP communication
static void get_prover_commit(char *commit)
{
    printf("Enter commitment: ");
    fflush(stdout);
    fgets(commit, 1024, stdin);
    commit[strcspn(commit, "\n")] = 0; // Remove newline
}

static void challenge_prover(bool challenge)
{
    printf("Challenge: %d\n", challenge);
    fflush(stdout);
}

static void get_prover_response(char *response)
{
    printf("Enter response: ");
    fflush(stdout);
    fgets(response, 1024, stdin);
    response[strcspn(response, "\n")] = 0; // Remove newline
}
