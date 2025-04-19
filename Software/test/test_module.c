// test_pam_zkp.c
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>

static struct pam_conv conv = {
    misc_conv,
    NULL};

int main()
{
    pam_handle_t *pamh = NULL;
    int ret;

    ret = pam_start("pamtest", NULL, &conv, &pamh);
    if (ret != PAM_SUCCESS)
    {
        printf("pam_start failed\n");
        return 1;
    }

    ret = pam_authenticate(pamh, 0);
    if (ret == PAM_SUCCESS)
        printf("Authentication success!\n");
    else
        printf("Authentication failed: %s\n", pam_strerror(pamh, ret));

    pam_end(pamh, ret);
    return ret == PAM_SUCCESS ? 0 : 1;
}
