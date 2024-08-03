#ifndef _C_MYADTOOLS_DLL_H_
#define _C_MYADTOOLS_DLL_H_

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

#ifdef _WIN32
    #define MYADTOOLS_DLL_PUBLIC __declspec(dllexport)
#else
    #define MYADTOOLS_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif  /* _WIN32 */

#define MYADTOOLS_DBC_CONFIG "DBCSettings.txt"
#define MYADTOOLS_DBC_CONFIG_CHUNK 256


extern int MYADTOOLS_DLL_PUBLIC recognizeEx(int imgType,
                                            unsigned int imgSize,
                                            void *imgData,
                                            char *buff,
                                            int buffSize,
                                            char *captchaId,
                                            int captchaIdSize);
extern int MYADTOOLS_DLL_PUBLIC rateEx(char *captchaId, char rate);
extern BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* !_C_MYADTOOLS_DLL_H_ */
