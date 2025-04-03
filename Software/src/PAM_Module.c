#define _GNU_SOURCE
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <libusb-1.0/libusb.h>

#include <syslog.h>

#include "ZKP_verify.h" // Include your ZKP verification logic

#define ZKP_ROUNDS 3 // Number of challenge-response rounds
#define ENDPOINT_ADDRESS_OUT 0x01
#define ENDPOINT_ADDRESS_IN 0x81

// Function prototypes
static int get_user_public_key(const char *username, char *y_str, size_t max_len);
static void get_prover_commit(char *commit);
static void challenge_prover(bool challenge);
static void get_prover_response(char *response);

static int init_USB_key(libusb_context *ctx, libusb_device_handle *dev_handle);
static int release_USB_key(libusb_context *ctx, libusb_device_handle *dev_handle);

// Global variable for connection to the USB Key
libusb_context *ctx;
libusb_device_handle *dev_handle;

// PAM authentication function
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    const char *username;
    char y_str[1024];

    if (init_USB_key(ctx, dev_handle) == -1) {
        pam_syslog(pamh, LOG_ERR, "Failed to initialize connection with USB key.");
    }

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
    release_USB_key(ctx, dev_handle);
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
    // printf("Enter commitment: ");
    // fflush(stdout);
    // fgets(commit, 1024, stdin);
    // commit[strcspn(commit, "\n")] = 0; // Remove newline

    int r;
    int actual_length;

    r = libusb_bulk_transfer(dev_handle,
        ENDPOINT_ADDRESS_IN,
        commit,
        1024,
        &actual_length,
        1000);
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
    // printf("Enter response: ");
    // fflush(stdout);
    // fgets(response, 1024, stdin);
    // response[strcspn(response, "\n")] = 0; // Remove newline

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
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
    return 0;
}   