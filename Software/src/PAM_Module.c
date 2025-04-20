#define _GNU_SOURCE
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <unistd.h>

#include <syslog.h>

#include <libusb-1.0/libusb.h>


#include "ZKP_verify.h" // Include your ZKP verification logic

#define ZKP_ROUNDS 3 // Number of challenge-response rounds

#define ENDPOINT_ADDRESS_OUT 0x01
#define ENDPOINT_ADDRESS_IN 0x81

/*
USB Protocol:

0x01 - Request Commit
0x02 - Challenge 0
0x03 - Challenge 1
0x04 - End Of protocol: SUCC
0x05 - End Of protocol: FAIL
*/

#define USB_REQ_COMMIT "1"
#define USB_CHA_0 "2"
#define USB_CHA_1 "3"
#define USB_END "4"


#define USERFILE "/etc/security/users"

// Function prototypes
static int get_user_public_key(const char *username, char *y_str, size_t max_len);
static void get_prover_commit(char *commit);
static void challenge_prover(bool challenge);
static void get_prover_response(char *response);

static int init_USB_key(libusb_context *ctx, libusb_device_handle *dev_handle);
static int release_USB_key(libusb_context *ctx, libusb_device_handle *dev_handle);

libusb_context *ctx;
libusb_device_handle *dev_handle;

/*
Called when no user using ZKP proof was found. Takes in a username and receives the public key,
store them in "/etc/security/zkp_users"
*/
// void create_user(const char *username)
// {
//     FILE *file = fopen("/etc/security/zkp_users", "a");
//     if (!file)
//         return;

//     char key[1024];
//     printf("Enter public key: ");
//     fgets(key, 1024, stdin); // further type checking needed
//     fprintf(file, "%s %s\n", username, key);
//     printf("User <%s> created!\n", username);
// }

// PAM authentication function
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    // FILE *f = fopen("/tmp/asdf.txt", "w");
    // fprintf(f, "reached\n");
    // fclose(f);
    const char *username;
    char y_str[1024];

    // Get the username
    if (pam_get_user(pamh, &username, "gimme username: ") != PAM_SUCCESS)
    {
        fprintf(stderr, "Can't get username");
        return PAM_AUTH_ERR;
    }

    // Retrieve the stored public key y for the user
    if (get_user_public_key(username, y_str, sizeof(y_str)) != 0)
    {
        printf("Failed to retrieve key for user %s\n", username);
        return PAM_AUTH_ERR;
    }

    printf("Retrieved public key: %s for user %s\n", y_str, username);

    for (int i = 0; i != 10; i++) {
        if (init_USB_key(ctx, dev_handle) == 0){
            break;
        }
        printf("Unable to connect to USB key, trail %d/%d\n", i + 1, 10);
        if (i == 9) return PAM_AUTH_ERR;
        sleep(3);
    }

    // Perform ZKP verification
    // mpz_t y;
    // mpz_init(y);
    // mpz_set_str(y, y_str, 10); // Convert string to GMP big integer

    if (!ZKP_verify(y_str, get_prover_commit, challenge_prover, get_prover_response, ZKP_ROUNDS))
    {
        pam_syslog(pamh, LOG_ERR, "ZKP verification failed.");
        // mpz_clear(y);
        return PAM_AUTH_ERR;
    }

    // mpz_clear(y);

    // Authentication successful
    printf("Welecome, user.\n");

    return PAM_SUCCESS;
}

// PAM account management (not required but can be expanded)
PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

// Function to retrieve user's public key from a secure file
static int get_user_public_key(const char *username, char *y_str, size_t max_len)
{
    FILE *file = fopen(USERFILE, "r");
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
    int r;
    int actual_length;

    r = libusb_bulk_transfer(dev_handle,
        ENDPOINT_ADDRESS_OUT,
        USB_REQ_COMMIT,
        sizeof(USB_REQ_COMMIT),
        &actual_length,
        1000
    );

    r = libusb_bulk_transfer(dev_handle,
        ENDPOINT_ADDRESS_IN,
        commit,
        1024,
        &actual_length,
        1000
    );
}

static void challenge_prover(bool challenge)
{
    printf("Challenge: %d\n", challenge);
    unsigned char data_out[2];
    int actual_length = 2;

    if (challenge == true) {
        data_out[0] = '1';
        data_out[1] = '\0';
    }
    else {
        data_out[0] = '0';
        data_out[1] = '\0';
    }

    int r;
    // Send data to the device
    r = libusb_bulk_transfer(dev_handle,
                            ENDPOINT_ADDRESS_OUT,
                            data_out,
                            sizeof(data_out),
                            &actual_length,
                            1000); // timeout in ms
    fflush(stdout);
}

static void get_prover_response(char *response)
{
    int r;
    int actual_length;

    r = libusb_bulk_transfer(dev_handle,
        ENDPOINT_ADDRESS_IN,
        response,
        1024,
        &actual_length,
        1000);
}


static int init_USB_key(libusb_context *ctx, libusb_device_handle *dev_handle){
    int r;

    // Initialize the libusb library
    r = libusb_init(&ctx);
    // Optional: Set debug level
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);

    // Open the device using Vendor ID (VID) and Product ID (PID)
    dev_handle = libusb_open_device_with_vid_pid(ctx, 0x1234, 0x5678);
    if (!dev_handle) {
        fprintf(stderr, "Could not open USB device\n");
        return -1;
    }

    r = libusb_claim_interface(dev_handle, 0);

    if (r < 0) {
        fprintf(stderr, "Failed to claim interface\n");
        return -1;
    }

    return 0;
}

static int release_USB_key(libusb_context *ctx, libusb_device_handle *dev_handle) {
    int r;
    int actual_length;
    // Send data to the device
    r = libusb_bulk_transfer(dev_handle,
                            ENDPOINT_ADDRESS_OUT,
                            USB_END,
                            sizeof(USB_END),
                            &actual_length,
                            1000);
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
    return 0;
}   