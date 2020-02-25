//#include"encode.h"
#include"inifile.h"

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>       //isspace
#include <time.h>       //time
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdarg.h>
#include<stdexcept>

#include"commondef.h"
#include"cameraModule/CameraResult.h"

#define LINUX32 1


bool Tool_MyMakeDir( const char* chImgPath)
{    
    char DirName[256];
    strcpy(DirName,chImgPath);
    if((access(DirName,F_OK))!=-1)
    {
        return true;
    }

    int i,len = strlen(DirName);
    if(DirName[len-1]!='/')
    { strcat( DirName, "/" ); }
    len = strlen(DirName);
    for(i=1;i<len;i++)
    {
        if(DirName[i]=='/')
        {
            DirName[i] = 0;
            if(access(DirName,NULL) != 0)
            {
#ifdef USE_LINUX
                if(mkdir(DirName,0777) == -1)
                //if(mkdir(DirName) == -1)
                {
                    //WriteLog("mkdir error");
                    return false;
                }
#else
                if(mkdir(DirName) == -1)
                {
                    //WriteLog("mkdir error");
                    return false;
                }
#endif
            }
            DirName[i] = '/';
        }
    }
    return true;
}

/*   删除左边的空格   */
bool Tool_trim(char * szOutput, const char *szInput)
{
    if(szInput == NULL)
    return false;
    if(szOutput == NULL)
    return false;
    if(szOutput == szInput)
    return false;
    for( ; *szInput != '\0' && isspace(*szInput); ++szInput)
    {
        ;
    }    
    strcpy(szOutput,szInput );
    return true;
}

long Tool_GetFile_size(const char* filename)
{
    FILE *fp=fopen(filename,"r");
    if( !fp ) { return -1; }
    fseek(fp,0L,SEEK_END);
    long isize=ftell(fp);
    fclose(fp);

    return isize;
}

bool Tool_ReadCommand(char* fileName, char* chCommand)
{
    char chBuff[1024] = {0};

    long iFileSize = Tool_GetFile_size(fileName);
    if(iFileSize <= 0)
    { return false; }

    FILE* pFile = fopen(fileName, "rb");
    size_t iReadSize = 0;
    if(pFile)
    {
        iReadSize = fread(chBuff, 1, iFileSize, pFile);
        fclose(pFile);
        pFile = NULL;
    }
    if(iReadSize != iFileSize)
    { return false; }
    memcpy(chCommand, chBuff, iReadSize);
    return true;
}

bool Tool_libWriteLog( const char* chlog )
{
    if(NULL == chlog)
    { return false; }

    char chFileName[260];
#ifdef LINUX32
    //sprintf( chFileName, "./" );
    char chBuffer1[256] = {0};
    getcwd(chBuffer1,sizeof(chBuffer1));
    sprintf(chFileName, "%s", chBuffer1);
    //sprintf(szFileName, "./");
#else
    sprintf( chFileName, "/ATSLANE/CONFIG" );
#endif
    char iniFileName[260] = {0};
    strcat( iniFileName, chFileName );
    strcat( iniFileName, INI_FILE_NAME );
    int iLog =  read_profile_int( "Log", "Enable", 1, iniFileName );
    if( iLog != 1 )
    { return false; }

    time_t now;         //实例化time_t结构
    struct tm* pTM;         //实例化tm结构指针
    time(&now);
    //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
    pTM = localtime(&now);

    const int MAXPATH = 260;

    char szFileName[ MAXPATH] = {0};
    //GetModuleFileName(NULL, szFileName, MAXPATH);	//取得包括程序名的全路径
    //PathRemoveFileSpec(szFileName);				//去掉程序名

#ifdef LINUX32
    char chBuffer[MAXPATH] = {0};
    getcwd(chBuffer,sizeof(chBuffer));
    sprintf(szFileName, "%s", chBuffer);

    //sprintf(szFileName, "./");
#else
    sprintf(szFileName, "/ATSLANE/LOG");
#endif

    char chLogPath[512] = {0};
    sprintf( chLogPath, "%s/XLWLog/%04d-%02d-%02d/",  szFileName,
             pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday );
    Tool_MyMakeDir(chLogPath);


    char chLogFileName[512] = {0};
    sprintf( chLogFileName, "%s/libVpr_-%d-%02d_%02d.log", chLogPath,
             pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday );


    FILE *file = NULL;
    file = fopen(chLogFileName, "a+");
    if (file)
    {
        //            fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d:%03d : %s\n",  pTM->tm_year + 1900, pTM->tm_mon+1, pTM->tm_mday,
        //                pTM->tm_hour, pTM->tm_min, pTM->tm_sec, dwMS, chlog);
        fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d: %s\n",  pTM->tm_year + 1900, pTM->tm_mon+1, pTM->tm_mday,
                pTM->tm_hour, pTM->tm_min, pTM->tm_sec, chlog);
        fclose(file);
        file = NULL;
    }

    return true;
}

void Tool_WriteFormatLog(const char* szfmt, ...)
{
    static char g_szPbString[10240] = { 0 };
    memset(g_szPbString, 0, sizeof(g_szPbString));

    va_list arg_ptr;
    va_start(arg_ptr, szfmt);
    //vsnprintf_s(g_szPbString, sizeof(g_szPbString), szfmt, arg_ptr);
    _vsnprintf(g_szPbString, sizeof(g_szPbString), szfmt, arg_ptr);

    Tool_libWriteLog(g_szPbString);

    va_end(arg_ptr);
}

unsigned long Tool_GetTickCount()
{
   // tms tm;
    //return times;
    unsigned long currentTime;
    struct timeval current;
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000 + current.tv_usec/1000;

    return currentTime;
}

bool Tool_SaveFile(void* pData, int length,const char* chFileName)
{
    FILE* pFile = NULL;
    //fopen_s(&pFile, chFileName, "wb");
    pFile = fopen(chFileName, "wb+");

    if (pFile)
    {
        fwrite(pData, 1, length, pFile);
        fclose(pFile);
        pFile = NULL;

        return true;
    }
    else
    {
        return false;
    }
}

bool Tool_IsFileExist(const char *FilePath)
{
    if (FilePath == NULL)
    {
        return false;
    }
    FILE* tempFile = NULL;
    bool bRet = false;
    tempFile = fopen(FilePath, "r");
    if (tempFile)
    {
        bRet = true;
        fclose(tempFile);
        tempFile = NULL;
    }
    else
    {
        //printf("Tool_IsFileExist, failed, error code = %d", errCode);
        printf("Tool_IsFileExist, failed");
    }
    return bRet;
}

bool Tool_LoadFile(const char* fileName, void* pBuffer, size_t& inputOutputFileSize)
{
    if (NULL == fileName || NULL == pBuffer)
    {
        printf("Tool_LoadFile, NULL == fileName || NULL == pBuffer.\n");
        return false;
    }

    try
    {
        if (!Tool_IsFileExist(fileName))
        {
            printf("Tool_LoadFile, File not Exist.\n");
            return false;
        }

        size_t iFileSize = Tool_GetFile_size(fileName);

        if (iFileSize <= 0 || iFileSize > inputOutputFileSize)
        {
            printf("Tool_LoadFile, iFileSize= %d,  iFileSize <= 0 || iFileSize > inputFileSize \n", iFileSize);
            return false;
        }
        inputOutputFileSize = iFileSize;

        FILE* pFile = NULL;
        //errno_t errCode;
        //_set_errno(0);
        //errCode = fopen_s(&pFile, fileName, "rb");
        pFile = fopen(fileName,  "rb");
        if (NULL != pFile)
        {
            fread(pBuffer, 1, iFileSize, pFile);
            fclose(pFile);
            pFile = NULL;

            return true;
        }
        else
        {
            //printf("Tool_LoadFile, open file failed, error code = %d.\n", errCode);
            printf("Tool_LoadFile, open file failed.\n");
            return false;
        }
    }
    catch (std::bad_exception& e)
    {
        printf("Tool_LoadFile, bad_exception, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::domain_error& e)
    {
        printf("Tool_LoadFile, domain_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::length_error& e)
    {
        printf("Tool_LoadFile, length_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::range_error& e)
    {
        printf("Tool_LoadFile, range_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::out_of_range& e)
    {
        printf("Tool_LoadFile, out_of_range, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::overflow_error& e)
    {
        printf("Tool_LoadFile, overflow_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::logic_error& e)
    {
        printf("Tool_LoadFile, logic_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::runtime_error& e)
    {
        printf("Tool_LoadFile, runtime_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::bad_alloc& e)
    {
        printf("Tool_LoadFile, bad_alloc, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::exception& e)
    {
        printf("Tool_LoadFile, exception, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (void*)
    {
        printf("Tool_LoadFile,  void* exception, error code = %lu.", GET_LAST_ERROR);
        return false;
    }
    catch (...)
    {
        printf("Tool_LoadFile,  unknown exception, error code = %lu.", GET_LAST_ERROR);
        return false;
    }
}

const char *Tool_GetCurrentPath()
{
    static char g_path[256] = {0};
    memset(g_path, '\0', sizeof(g_path));

    return getcwd(g_path, sizeof(g_path));
}

void Tool_GetTime(struct tm* timeNow, long* piTimeInMilliseconds)
{
    long            ms; // Milliseconds
    time_t          timeSecond;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    timeSecond = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999)
    {
            timeSecond++;
            ms = 0;
    }

    struct tm *nowtm;
    nowtm = localtime(&timeSecond);
    memcpy(timeNow, nowtm, sizeof(struct tm));
    *piTimeInMilliseconds = spec.tv_sec*1000 + ms;
}

void Tool_WriteKeyValueFromConfigFile(const char *INIFileName, const char *nodeName, const char *keyName, char *keyValue, int bufferSize)
{
    if (strlen(keyValue) > bufferSize)
    {
        return;
    }
    char chFileName[MAX_PATH] = {0};
    sprintf(chFileName,"%s",  Tool_GetCurrentPath());

    char chIniFileName[MAX_PATH] = { 0 };
    strcat(chIniFileName, chFileName);
    strcat(chIniFileName, INIFileName);

    write_profile_string(nodeName, keyName, keyValue, chIniFileName);
}

void Tool_ReadKeyValueFromConfigFile(const char *IniFileName, const char *nodeName, const char *keyName, char *keyValue, int bufferSize)
{
    if (strlen(keyValue) > bufferSize)
    {
        return;
    }
    char chFileName[MAX_PATH] = {0};
    sprintf(chFileName,"%s",  Tool_GetCurrentPath());

    char chIniFileName[MAX_PATH] = { 0 };
    sprintf(chIniFileName,  "%s/%s", chFileName, IniFileName);

    char chTemp[256] = { 0 };
    read_profile_string(nodeName, keyName, chTemp, sizeof(chTemp), "0", chIniFileName);
    if (strcmp(chTemp, "0") == 0)
    {
        Tool_WriteKeyValueFromConfigFile(IniFileName,nodeName,  keyName,chTemp, sizeof(chTemp) );
    }
    else
    {
        //strcpy_s(keyValue, bufferSize, chTemp);
        strcpy(keyValue, chTemp);
    }
}

void Tool_ReadIntValueFromConfigFile(const char *IniFileName, const char *nodeName, const char *keyName, int &keyValue)
{
    char chFileName[MAX_PATH] = {0};
    sprintf(chFileName,"%s",  Tool_GetCurrentPath());

    char chIniFileName[MAX_PATH] = { 0 };
    sprintf(chIniFileName,  "%s/%s", chFileName, IniFileName);

    int iValue = read_profile_int(nodeName, keyName, keyValue, chIniFileName);
    keyValue = iValue;

    char chTemp[128] = { 0 };
    sprintf(chTemp,  "%d", iValue);
    Tool_WriteKeyValueFromConfigFile(IniFileName,nodeName,  keyName,chTemp, sizeof(chTemp) );
}

bool Tool_GetDataFromAppenedInfo(char *pszAppendInfo, std::string strItemName, char *pszRstBuf, int *piRstBufLen)
{
    if (pszAppendInfo == NULL || piRstBufLen == NULL || *piRstBufLen <= 0)
    {
        return false;
    }

    // <RoadNumber value="0" chnname="车道" />
    // <StreetName value="" chnname="路口名称" />
    try
    {
        std::string strAppendInfo = pszAppendInfo;
        size_t siStart = strAppendInfo.find(strItemName);
        if (siStart == std::string::npos)
        {
            return false;
        }
        siStart = strAppendInfo.find("\"", siStart + 1);
        if (siStart == std::string::npos)
        {
            return false;
        }
        size_t siEnd = strAppendInfo.find("\"", siStart + 1);
        if (siEnd == std::string::npos)
        {
            return false;
        }

        std::string strRst = strAppendInfo.substr(siStart + 1, siEnd - siStart - 1);
        if (*piRstBufLen < (int)strRst.length())
        {
            *piRstBufLen = (int)strRst.length();
            return false;
        }

        //strncpy_s(pszRstBuf, *piRstBufLen, strRst.c_str(), (int)strRst.length());
        strncpy(pszRstBuf, strRst.c_str(), (int)strRst.length());
        *piRstBufLen = (int)strRst.length();
        return true;
    }
    catch (std::bad_exception& e)
    {
        printf("Tool_GetDataFromAppenedInfo, bad_exception, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::overflow_error& e)
    {
        printf("Tool_GetDataFromAppenedInfo, overflow_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::domain_error& e)
    {
        printf("Tool_GetDataFromAppenedInfo, domain_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::length_error& e)
    {
        printf("Tool_GetDataFromAppenedInfo, length_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::out_of_range& e)
    {
        printf("Tool_GetDataFromAppenedInfo, out_of_range, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::range_error& e)
    {
        printf("Tool_GetDataFromAppenedInfo, range_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::runtime_error& e)
    {
        printf("Tool_GetDataFromAppenedInfo, runtime_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::logic_error& e)
    {
        printf("Tool_GetDataFromAppenedInfo, logic_error, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::bad_alloc& e)
    {
        printf("Tool_GetDataFromAppenedInfo, bad_alloc, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (std::exception& e)
    {
        printf("Tool_GetDataFromAppenedInfo, exception, error msg = %s, error code = %lu.", e.what(), GET_LAST_ERROR);
        return false;
    }
    catch (void*)
    {
        printf("Tool_GetDataFromAppenedInfo,  void* exception, error code = %lu.", GET_LAST_ERROR);
        return false;
    }
    catch (...)
    {
        printf("Tool_GetDataFromAppenedInfo,  unknown exception, error code = %lu.", GET_LAST_ERROR);
        return false;
    }
}

bool Tool_DimCompare(const char *szSrcPlateNo, const char *szDesPlateNo)
{

    if (!szSrcPlateNo || !szDesPlateNo)
        return false;

    if (strstr(szSrcPlateNo, "无") || strstr(szDesPlateNo, "无"))
    {
        printf("Info: NoPlate not Compare!!!!!!!!!!");
        return false;
    }
    char chLog[MAX_PATH] = { 0 };
    //sprintf_s(chLog, sizeof(chLog), "DimCompare(%s, %s)", szSrcPlateNo, szDesPlateNo);
    sprintf(chLog, "DimCompare(%s, %s)", szSrcPlateNo, szDesPlateNo);
    printf(chLog);
    //获取6字节标准车牌
    char strStandardCarChar[10] = { 0 };
    int  nPlateNo = strlen(szSrcPlateNo);
    if (nPlateNo > 10)
    {
        printf("Error: szPlateNo!!!!!!!!!!");
        return false;
    }
    else if (nPlateNo > 6)
    {
        //strcpy_s(strStandardCarChar, szSrcPlateNo + 2);
         strcpy(strStandardCarChar, szSrcPlateNo + 2);
        //WriteLog("strcpy iCurPlateNo > 6 ");
    }
    else
    {
        strcpy(strStandardCarChar, szSrcPlateNo);
        //WriteLog("strcpy iCurPlateNo <= 6 ");
    }

    int iMaxMatchCnt = 0;
    int iMaxMatchRate = 0;
    int iStandardPlateLen = strlen(strStandardCarChar);//比对车牌长度
    int i = 0;
    int j = 0;
    int nFlagCompare = -1;
    char strComparePlateChar[10] = { 0 };
    int  iComparePlateLen = 0;
    int  iCurPlateNo = 0;

    iCurPlateNo = strlen(szDesPlateNo);
    if (iCurPlateNo > 10)
    {
        printf("Error: szPlateNo!!!!!!!!!!");
        return false;
    }
    else if (iCurPlateNo > 6)
    {
        strcpy(strComparePlateChar, szDesPlateNo + 2);
        //WriteLog("strcpy iCurPlateNo > 6 ");
    }
    else
    {
        strcpy(strComparePlateChar, szDesPlateNo);
        //WriteLog("strcpy iCurPlateNo <= 6 ");
    }

    //取出list中需要作对比的牌号字符数组
    iComparePlateLen = strlen(strComparePlateChar);//list选中车牌长度

    //取少位数的来遍历,同位匹配
    int iLoopTimes = iComparePlateLen < iStandardPlateLen ? iComparePlateLen : iStandardPlateLen;
    int iEqualCount = 0;
    for (j = 0; j < iLoopTimes; j++)
    {
        if (strComparePlateChar[j] == strStandardCarChar[j])
        {
            ++iEqualCount; //匹配数
        }
    }

    if (iEqualCount >= 5 && iEqualCount > iMaxMatchCnt) //车牌匹配5个或以上，算是匹配了，但仍然循环完，已查找最佳匹配率的记录
    {
        iMaxMatchCnt = iEqualCount;

        int iDenominator = iComparePlateLen > iStandardPlateLen ? iComparePlateLen : iStandardPlateLen;
        int iMatchRate = (int)(iMaxMatchCnt * 100 / iDenominator);

        if (iMatchRate > iMaxMatchRate)
        {
            iMaxMatchRate = iMatchRate;
            nFlagCompare = i;
        }
    }

    //同位匹配5个以上直接认为是同个车牌
    if (nFlagCompare != -1)
    {
        printf("nFlagCompare != -1（同位匹配5个以上直接认为是同个车牌） return true.");
        return true;
    }

    //同位匹配不上,只能继续错位匹配
    iMaxMatchCnt = 0;
    iMaxMatchRate = 0;
    nFlagCompare = -1;
    iLoopTimes = 4; //错位比较中间连续的4位
    if (iStandardPlateLen < 6 || iComparePlateLen < 6)
    {
        printf("iStandardPlateLen < 6 or iComparePlateLen < 6");
        return false;
    }

    //A12345
    //假如车辆有三辆，分别为粤G15678、川B23456、云C12456
    int iEqualCount1 = 0, iEqualCount2 = 0;
    bool bCompare = false;
    for (j = 0; j < iLoopTimes; j++)
    {
        // A1234与15678、23456、12456比较,后5位与主的前5位比较,但中间连续4位必须全匹配
        if (strComparePlateChar[j + 2] == strStandardCarChar[j + 1])
        {
            ++iEqualCount1; //匹配数
            if (iEqualCount1 == iLoopTimes) bCompare = true;
        }

        // 12345与G15678、B23456、C12456比较,前5位与主的后5位比较,但中间连续4位必须全匹配
        if (strComparePlateChar[j] == strStandardCarChar[j + 1])
        {
            ++iEqualCount2; //匹配数
            if (iEqualCount2 == iLoopTimes) bCompare = true;
        }
    }
    if (strComparePlateChar[1] == strStandardCarChar[0])
        ++iEqualCount1;
    if (strComparePlateChar[4] == strStandardCarChar[5])
        ++iEqualCount2;


    //车牌匹配4个或以上，算是匹配了，但仍然循环完，已查找最佳匹配率的记录
    if ((iEqualCount1 >= 4 && iEqualCount1 > iMaxMatchCnt && bCompare)
        || (iEqualCount2 >= 4 && iEqualCount2 > iMaxMatchCnt && bCompare))
    {
        iMaxMatchCnt = iEqualCount1 > iEqualCount2 ? iEqualCount1 : iEqualCount2;

        int iDenominator = iComparePlateLen > iStandardPlateLen ? iComparePlateLen : iStandardPlateLen;
        int iMatchRate = (int)(iMaxMatchCnt * 100 / iDenominator);

        if (iMatchRate > iMaxMatchRate)
        {
            iMaxMatchRate = iMatchRate;
            nFlagCompare = i;
        }
    }
    if (nFlagCompare == -1)
    {
        printf("DimCompare failed.\n");
        return false;
    }
    else
    {
        printf("DimCompare success.\n");
        return true;
    }
}

int Tool_AnalysisPlateColorNo(const char *szPlateNo)
{
    if (szPlateNo == NULL
        || strlen(szPlateNo) <= 1
        )
    {
        return COLOR_UNKNOW;
    }
    else
    {
        if (NULL != strstr(szPlateNo, "蓝")
            || NULL != strstr(szPlateNo, "白")
            )
        {
            if (NULL != strstr(szPlateNo, "蓝")
                && NULL != strstr(szPlateNo, "白")
                )
            {
                return COLOR_BLUE_WHITE;
            }
            if (NULL != strstr(szPlateNo, "蓝"))
            {
                return COLOR_BLUE;
            }
            if (NULL != strstr(szPlateNo, "白"))
            {
                return COLOR_WHITE;
            }
            return COLOR_BLUE;
        }
        else if(NULL != strstr(szPlateNo, "黄"))
        {
            if (NULL != strstr(szPlateNo, "绿"))
            {
                return COLOR_YELLOW_GREEN;
            }
            else
            {
                return COLOR_YELLOW;
            }
        }
        else if (NULL != strstr(szPlateNo, "黑"))
        {
            return COLOR_BLACK;
        }
        else if (NULL != strstr(szPlateNo, "绿") )
        {
            char chLast = szPlateNo[strlen(szPlateNo) - 1];
            int iChar = toupper((int)chLast);
            if (iChar == 'D'
                || iChar == 'F'
                )
            {
                return COLOR_YELLOW_GREEN;
            }
            else
            {
                return COLOR_GRADIENT_CREEN;
            }
        }
        else
        {
            return COLOR_UNKNOW;
        }
    }
}
