/**
 * Death By Captcha command-line client.
 * Feel free to use however you see fit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deathbycaptcha.h"

void _show_version()
{
    printf("%s\n(C) 2010 and onwards, deathbycaptcha.com\n", DBC_API_VERSION);
}

/* Show usage help */
void _show_usage_help(char *bin)
{
    _show_version();
    printf("\n");

    printf("To check your balance, run:\n");
    printf("%s -l USERNAME -p PASSWORD\n", bin);
    printf("-or if using authentication token\n");
    printf("%s -a AUTHTOKEN\n", bin);
    printf("Your balance will be saved in balance.txt file and printed out on the standard output.\n\n");

    printf("To solve a CAPTCHA, run:\n");
    printf("%s -l USERNAME -p PASSWORD -c CAPTCHA_FILE_NAME [-t TIMEOUT] [-d WORK_DIR]\n", bin);
    printf("-or if using authentication token\n");
    printf("%s -a AUTHTOKEN -c CAPTCHA_FILE_NAME [-t TIMEOUT] [-d WORK_DIR]\n", bin);
    printf("If solved, the CAPTCHA ID will be saved in id.txt, the CAPTCHA text will be saved in answer.txt, and both ID and text will be printed out on the standard output separated by a space.\n");
    printf("CAPTCHA_FILE_NAME can be '-', in that case the image itself will be read from standard input.\n");
    printf("TIMEOUT option defines CAPTCHA solving timeout in seconds (default is 60 seconds).\n");
    printf("If WORK_DIR option is defined, the id.txt and answer files will be saved to that directory, otherwise they will be saved to the working directory.\n\n");

    printf("To report an incorrectly solved CAPTCHA, run\n");
    printf("%s -l USERNAME -p PASSWORD -n CAPTCHA_ID\n", bin);
    printf("-or if using authentication token\n");
    printf("%s -a AUTHTOKEN -n CAPTCHA_ID\n", bin);
}

/* Fetch and dump user's balance */
int _dump_balance(const char *work_dir, dbc_client *client)
{
    char *dump_fn = (char *)calloc(strlen(work_dir) + 12, sizeof(char));
    dump_fn = strcat(dump_fn, work_dir);
    dump_fn = strcat(dump_fn, "balance.txt");
    dbc_get_balance(client);
    printf("client:\n");
    printf("client.c: %ld\n",client->user_id);
    if (client->user_id == 0){
        printf("FAILED\n");
        return 1;
    }

    if (0 < client->user_id) {
        FILE *f = fopen(dump_fn, "w");
        if (NULL != f) {
            fprintf(f, "%.4f", client->balance);
            fclose(f);
        }
        printf("balance: %.4f\n", client->balance);
    }

    free(dump_fn);
    return 0;
}

/* Report a CAPTCHA as incorrectly solved */
void _report_captcha(const char *work_dir, dbc_client *client, unsigned int id)
{
    dbc_captcha *captcha = (dbc_captcha *)malloc(sizeof(dbc_captcha));
    if (dbc_init_captcha(captcha)) {
        fprintf(stderr, "Failed initializing a CAPTCHA instance\n");
    } else {
        captcha->id = id;
        printf("%s\n", dbc_report(client, captcha) ? "FAILED" : "DONE");
        dbc_close_captcha(captcha);
    }
    free(captcha);
}

/* Upload and solve a CAPTCHA */
void _decode_captcha_file(const char *work_dir, dbc_client *client, const char *filename, unsigned int timeout)
{
    FILE *f = !strcmp("-", filename) ? stdin : fopen(filename, "rb");
    if (NULL == f) {
        fprintf(stderr, "%s not found or unreadable\n", filename);
    } else {
        dbc_captcha *captcha = (dbc_captcha *)malloc(sizeof(dbc_captcha));
        if (dbc_init_captcha(captcha)) {
            fprintf(stderr, "Failed initializing a CAPTCHA instance\n");
        } else {
            if (dbc_decode_file(client, captcha, f, timeout)) {
                if (0.0 > client->balance) {
                    fprintf(stderr, "Insufficied funds\n");
                } else {
                    fprintf(stderr, "Failed uploading/solving %s\n", filename);
                }
            } else if (!captcha->is_correct) {
                fprintf(stderr, "CAPTCHA was marked as invalid by an operator, check if %s is in fact a CAPTCHA image and not corrupted\n", filename);
            } else {
                FILE *dump_f = NULL;

                char *dump_fn = (char *)calloc(strlen(work_dir) + 7, sizeof(char));
                dump_fn = strcat(dump_fn, work_dir);
                dump_fn = strcat(dump_fn, "id.txt");
                if (NULL != (dump_f = fopen(dump_fn, "w"))) {
                    fprintf(dump_f, "%d", captcha->id);
                    fclose(dump_f);
                }
                free(dump_fn);

                dump_fn = (char *)calloc(strlen(work_dir) + 11, sizeof(char));
                dump_fn = strcat(dump_fn, work_dir);
                dump_fn = strcat(dump_fn, "answer.txt");
                if (NULL != (dump_f = fopen(dump_fn, "w"))) {
                    fprintf(dump_f, "%s", captcha->text);
                    fclose(dump_f);
                }
                free(dump_fn);

                printf("%d %s\n", captcha->id, captcha->text);
            }
            if (!strcmp("-", filename)) {
                f = NULL;
            } else {
                fclose(f);
            }
            dbc_close_captcha(captcha);
        }
        free(captcha);
    }
}


/* DeathByCaptcha command-line client */
int main(int argc, char *argv[])
{
    printf("DeathByCaptcha command-line client \n");
    if (3 > argc) {
        _show_usage_help(argv[0]);
    } else {
        int i;
#ifdef WIN32
        char path_sep = '\\';
#else
        char path_sep = '/';
#endif  /* WIN32 */
        char *username = NULL, *password = NULL, *authtoken = NULL, *filename = NULL, *work_dir = (char *)calloc(1, sizeof(char));
        unsigned int is_verbose = 1, captcha_id = 0, timeout = DBC_TIMEOUT;

        for (i = 1; i < argc; i++) {
            if (!strcmp("-v", argv[i])) {
                is_verbose = 1;
            } else if (!strcmp("-l", argv[i])) {
                username = argv[++i];
            } else if (!strcmp("-p", argv[i])) {
                password = argv[++i];
            } else if (!strcmp("-a", argv[i])) {
                authtoken = argv[++i];
            } else if (!strcmp("-c", argv[i])) {
                filename = argv[++i];
            } else if (!strcmp("-t", argv[i])) {
                if (!sscanf(argv[++i], "%u", &timeout)) {
                    fprintf(stderr, "Invalid timeout %s, using the default one\n", argv[i]);
                } else if (!timeout) {
                    timeout = DBC_TIMEOUT;
                }
            } else if (!strcmp("-n", argv[i])) {
                if (!sscanf(argv[++i], "%u", &captcha_id)) {
                    fprintf(stderr, "Invalid CAPTCHA ID %s\n", argv[i]);
                }
            } else if (!strcmp("-d", argv[i])) {
                i++;
                if (NULL != work_dir) {
                    free(work_dir);
                }
                work_dir = (char *)calloc(strlen(argv[i]) + 1, sizeof(char));
                strcpy(work_dir, argv[i]);
            }
        }

        if (0 < strlen(work_dir) && path_sep != work_dir[i = strlen(work_dir) - 1]) {
            work_dir = (char *)realloc(work_dir, (strlen(work_dir) + 2) * sizeof(char));
            work_dir[i + 1] = path_sep;
            work_dir[i + 2] = '\0';
        }

        if ((NULL == username || NULL == password) && NULL == authtoken) {
            _show_usage_help(argv[0]);
            fprintf(stderr, "Username, password or authtoken is missing\n");
        } else {
            dbc_client *client = (dbc_client *)malloc(sizeof(dbc_client));
            if (authtoken != NULL){
                printf("Using token authentication:\n");
                if (dbc_init_token(client, authtoken)) {
                    fprintf(stderr, "Failed initializing a DBC client\n");
                } else {
                    printf("Initializing DBC client\n");
                    client->is_verbose = is_verbose;
                    if (0 < captcha_id) {
                        printf("Looking captcha ID ...\n");
                        _report_captcha(work_dir, client, captcha_id);
                    } else if (NULL != filename) {
                        printf("Uploading captcha(token) ... %s\n", filename);
                        _dump_balance(work_dir, client);
                        _decode_captcha_file(work_dir, client, filename, timeout);
                    } else {
                        _dump_balance(work_dir, client);
                    }
                }
            } else {
                printf("Using username/password:\n");
                if (dbc_init(client, username, password)) {
                    fprintf(stderr, "Failed initializing a DBC client\n");
                    printf("DBC C Sockets Client FAILED ... \n");
                    return -1;
                } else {
                    printf("dbc_init: OK\n");
                    client->is_verbose = is_verbose;
                    if (0 < captcha_id) {
                        printf("_report_captcha\n");
                        _report_captcha(work_dir, client, captcha_id);
                    } else if (NULL != filename) {
                        printf("_decode_captcha_file\n");
                        printf("Uploading captcha(username/password) ... %s\n", filename);
                        _decode_captcha_file(work_dir, client, filename, timeout);
                    } else {
                        printf("_dump_balance\n");
                        int r_value = _dump_balance(work_dir, client);
                        if (r_value != 0){
                            printf("PROGRAM FAILED\n");
                            return 1;
                        }
                    }
                }
            }
            dbc_close(client);
            free(client);
        }

        free(work_dir);
    }
    return 0;
}
