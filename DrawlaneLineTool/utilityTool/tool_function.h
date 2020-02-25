#ifndef TOOL_FUNCTION_H
#define TOOL_FUNCTION_H
#include <pthread.h>
#include <unistd.h>
#include<string>

#define SAFE_DELETE_OBJ(obj) \
if (NULL != obj)                                  \
{                                           \
    delete obj;                        \
    obj = NULL;                      \
}

#define SAFE_DELETE_ARRAY(arrayObj) \
if (NULL != arrayObj)                                  \
{                                           \
    delete[] arrayObj;                        \
    arrayObj = NULL;                      \
}

typedef struct ThreadMutex
{
    ThreadMutex()
    {
        pthread_mutex_init(&mtx, NULL);
    }
    ~ThreadMutex()
    {
        pthread_mutex_destroy(&mtx);
    }
    inline void lock()
    {

        pthread_mutex_lock(&mtx);
       //    printf("Enter %d\n", ++i);
    }
    inline void unlock()
    {
        pthread_mutex_unlock(&mtx);
     //     printf("Leave %d\n", ++t);
    }
    pthread_mutex_t mtx;
 //   int i;
 //   int t;
}ThreadMutex;




bool Tool_MyMakeDir( const char* chImgPath);
bool Tool_libWriteLog( const char* chlog );
void Tool_WriteFormatLog(const char* szfmt, ...);

bool Tool_trim(char * szOutput, const char *szInput);
bool Tool_ReadCommand(char* fileName, char* chCommand);
long Tool_GetFile_size(const char* filename);
bool Tool_IsFileExist(const char* FilePath);
unsigned long Tool_GetTickCount();
bool Tool_SaveFile(void* pData, int length, const char* chFileName);
bool Tool_LoadFile(const char* fileName, void* pBuffer, size_t& inputOutputFileSize);
void Tool_GetTime(struct tm* timeNow, long* piTimeInMilliseconds);
const char* Tool_GetCurrentPath();

void Tool_ReadIntValueFromConfigFile(const char* IniFileName, const char* nodeName, const char* keyName, int&keyValue);
void Tool_ReadKeyValueFromConfigFile(const char* IniFileName, const char* nodeName, const char* keyName, char* keyValue, int bufferSize);
void Tool_WriteKeyValueFromConfigFile(const char* INIFileName, const char* nodeName, const char* keyName, char* keyValue, int bufferSize);

bool Tool_GetDataFromAppenedInfo(char *pszAppendInfo, std::string strItemName, char *pszRstBuf, int *piRstBufLen);

bool Tool_DimCompare(const char *szSrcPlateNo, const char *szDesPlateNo);

int Tool_AnalysisPlateColorNo(const char* szPlateNo);


#endif // TOOL_FUNCTION_H
