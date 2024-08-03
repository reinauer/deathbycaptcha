#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
#endif  /* _WIN32 */

#include "deathbycaptcha.h"


int main(int argc, char *argv[])
{
    if (4 > argc) {
        printf("Usage: %s USERNAME PASSWORD CAPTCHA_FILE [...]\n", argv[0]);
        printf("Usage: %s AUTH_TOKEN CAPTCHA_FILE [...]\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

#ifdef _WIN32
    HINSTANCE lib = LoadLibrary(".\\deathbycaptcha.dll");
    if (NULL == lib) {
        fprintf(stderr, "LoadLibrary(): %d\n", (int)GetLastError());
        exit(EXIT_FAILURE);
    }
#else
    void *lib = dlopen("./libdeathbycaptcha.so", RTLD_LAZY);
    if (!lib) {
        fprintf(stderr, "dlopen(): %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
#endif  /* _WIN32 */

    int (*dbc_init)(dbc_client *, char *, char *) = NULL;
    // using token authentication
    // int (*dbc_init_token)(dbc_client *, char *) = NULL;
    void (*dbc_close)(dbc_client *) = NULL;
    double (*dbc_get_balance)(dbc_client *) = NULL;
    int (*dbc_decode_file)(dbc_client *, dbc_captcha *, FILE *, int) = NULL;
    int (*dbc_report)(dbc_client *, dbc_captcha *) = NULL;
    void (*dbc_close_captcha)(dbc_captcha *) = NULL;
#ifdef _WIN32
    dbc_init = (void *)GetProcAddress(lib, "dbc_init");
    dbc_close = (void *)GetProcAddress(lib, "dbc_close");
    dbc_get_balance = (void *)GetProcAddress(lib, "dbc_get_balance");
    dbc_decode_file = (void *)GetProcAddress(lib, "dbc_decode_file");
    dbc_report = (void *)GetProcAddress(lib, "dbc_report");
    dbc_close_captcha = (void *)GetProcAddress(lib, "dbc_close_captcha");
#else
    dbc_init = dlsym(lib, "dbc_init");
    dbc_close = dlsym(lib, "dbc_close");
    dbc_get_balance = dlsym(lib, "dbc_get_balance");
    dbc_decode_file = dlsym(lib, "dbc_decode_file");
    dbc_report = dlsym(lib, "dbc_report");
    dbc_close_captcha = dlsym(lib, "dbc_close_captcha");
#endif  /* _WIN32 */

    dbc_client *client = calloc(1, sizeof(dbc_client));
    // using token authentication
    // if (dbc_init_token(client, argv[1])) {
    if (dbc_init(client, argv[1], argv[2])) {
        fprintf(stderr, "dbc_init(%s, %s)\n", argv[1], argv[2]);
    } else {
        /*client->is_verbose = 1;*/

        double balance = dbc_get_balance(client);
        printf("User ID: %d\nBalance: %.3f US cents\n", client->user_id, client->balance);

        int i;
        for (i = 3; i < argc; i++) {
            FILE *f = fopen(argv[i], "rb");
            if (NULL == f) {
                fprintf(stderr, "fopen(%s): %d\n", argv[i], errno);
            } else {
                dbc_captcha *captcha = calloc(1, sizeof(dbc_captcha));
                if (dbc_decode_file(client, captcha, f, DBC_TIMEOUT)) {
                    fprintf(stderr, "dbc_decode_file(%s)\n", argv[i]);
                } else {
                    printf("%s: %s\n", argv[i], captcha->text);
                    if (dbc_report(client, captcha)) {
                        fprintf(stderr, "dbc_report(%s)\n", argv[i]);
                    }
                }
                dbc_close_captcha(captcha);
                free(captcha);
                fclose(f);
            }
        }
    }
    dbc_close(client);
    free(client);

#ifdef _WIN32
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif  /* _WIN32 */

    exit(EXIT_SUCCESS);
}
