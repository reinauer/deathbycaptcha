#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
    #define WINVER 0x0501
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif  /* _WIN32 */

#include "../deathbycaptcha.h"

#include "DeathByCaptcha.h"


dbc_client *client = NULL;
unsigned int timeout = DBC_TIMEOUT;


int MYADTOOLS_DLL_PUBLIC recognizeEx(int imgType,
                                     unsigned int imgSize,
                                     void *imgData,
                                     char *buff,
                                     int buffSize,
                                     char *captchaId,
                                     int captchaIdSize)
{
    int err = 1;
    if (NULL != client) {
        dbc_captcha *captcha = (dbc_captcha *)calloc(1, sizeof(dbc_captcha));
        memset(buff, 0, buffSize);
        memset(captchaId, 0, captchaIdSize);
        if (!dbc_decode(client, captcha, (char *)imgData, (size_t)imgSize, timeout)) {
            if (captcha->is_correct) {
                snprintf(captchaId, captchaIdSize, "%u", captcha->id);
                strncpy(buff, captcha->text, buffSize - 1);
                err = 0;
            } else {
                err = 2;
            }
            dbc_close_captcha(captcha);
        }
        free(captcha);
    }
    return err;
}

int MYADTOOLS_DLL_PUBLIC rateEx(char *captchaId, char rate)
{
    int err = 1;
    if (0 == rate || '0' == rate) {
        if (NULL != client) {
            dbc_captcha *captcha = (dbc_captcha *)calloc(1, sizeof(dbc_captcha));
            if (!dbc_init_captcha(captcha)) {
                if (sscanf(captchaId, "%u", &(captcha->id)) && 0 < captcha->id) {
                    err = dbc_report(client, captcha);
                }
                dbc_close_captcha(captcha);
            }
            free(captcha);
        }
    }
    return err;
}

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    if (DLL_PROCESS_ATTACH == dwReason) {
        FILE *f = fopen(MYADTOOLS_DBC_CONFIG, "r");
        if (NULL == f) {
            fprintf(stderr, "Failed opening configuration file %s\n",
                    MYADTOOLS_DBC_CONFIG);
        } else {
            int is_verbose = 0;
            char *username = NULL, *password = NULL;
            char buf[MYADTOOLS_DBC_CONFIG_CHUNK] = "";
            memset(buf, 0, MYADTOOLS_DBC_CONFIG_CHUNK * sizeof(char));
            while (NULL != fgets(buf, MYADTOOLS_DBC_CONFIG_CHUNK, f)) {
                char *key = strtok(buf, "=");
                if (NULL != key) {
                    char *value = strtok(NULL, "\r\n\0");
                    if (NULL != value) {
                        int i = strlen(key) - 1;
                        while (0 <= i && (' ' == key[i] || '\t' == key[i] || '\r' == key[i] || '\n' == key[i])) {
                            key[i] = '\0';
                            i--;
                        }
                        while (0 < strlen(value) && (' ' == value[0] || '\t' == value[0] || '\r' == value[0] || '\n' == value[0])) {
                            value = &(value[1]);
                        }
                        i = strlen(value) - 1;
                        while (0 <= i && (' ' == value[i] || '\t' == value[i] || '\r' == value[i] || '\n' == value[i])) {
                            value[i] = '\0';
                            i--;
                        }
                        if (!strcmp(key, "LoginName")) {
                            username = (char *)calloc(strlen(value) + 1, sizeof(char));
                            strcpy(username, value);
                        } else if (!strcmp(key, "Password")) {
                            password = (char *)calloc(strlen(value) + 1, sizeof(char));
                            strcpy(password, value);
                        } else if (!strcmp(key, "Timeout")) {
                            sscanf(value, "%u", &timeout);
                        } else if (!strcmp(key, "LogEvents") && !strcmp(value, "1")) {
                            is_verbose = 1;
                        }
                    }
                }
            }
            fclose(f);

            if (NULL == username || NULL == password ||
                0 == strlen(username) || 0 == strlen(password)) {
                fprintf(stderr, "DBC account credentials are missing, check %s\n",
                        MYADTOOLS_DBC_CONFIG);
            } else {
                client = (dbc_client *)malloc(sizeof(dbc_client));
                if (dbc_init(client, username, password)) {
                    fprintf(stderr, "Failed initializing a DBC client\n");
                    free(client);
                    client = NULL;
                } else {
                    client->is_verbose = is_verbose;
                }
            }
            if (NULL != username) {
                free(username);
                username = NULL;
            }
            if (NULL != password) {
                free(password);
                password = NULL;
            }
        }
        if (NULL == client) {
            return FALSE;
        } else if (client->is_verbose) {
            freopen("DeathByCaptcha.log", "w", stdout);
            freopen("DeathByCaptcha.error.log", "w", stderr);
        }
    } else if (DLL_PROCESS_DETACH == dwReason) {
        if (NULL != client) {
            dbc_close(client);
            free(client);
            client = NULL;
        }
    }
    return TRUE;
}
