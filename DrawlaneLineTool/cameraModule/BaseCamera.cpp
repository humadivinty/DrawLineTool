#include "BaseCamera.h"
#include "tinyxml/tinyxml.h"
#include<math.h>
//#include<shellapi.h>
//#pragma comment(lib, "./lib/hvdevice/HvDevice.lib")
//注意：这里要把HvDeviceDLL.h 里面的 #include "HvDeviceUtils.h" 注释掉,否则无法编译通过
#include "libHvDevice/HvDeviceBaseType.h"
#include "libHvDevice/HvDeviceCommDef.h"
#include "libHvDevice/HvDeviceEx.h"
//#include "libHvDevice/HvCamera.h"
//#pragma comment(lib, "libHvDevice/HvDevice.lib")
//#define  VEHICLE_LISTEN_PORT 99999
//#include <WinSock2.h>
//#pragma comment(lib,"WS2_32.lib")
//#pragma comment(lib, "H264API/H264.lib")

#include"utilityTool/inifile.h"
#include <stdarg.h>


#define  BUFFERLENTH 256
#define SAFE_MODE_INI "/SafeModeConfig.ini"

BaseCamera::BaseCamera() :
m_hHvHandle(NULL),
m_hWnd(NULL),
m_iMsg(0),
m_iConnectMsg(0x402),
m_iDisConMsg(0x403),
m_iConnectStatus(0),
m_iLoginID(0),
m_iCompressQuality(20),
m_iDirection(0),
m_iIndex(0),
m_bLogEnable(true),
m_bSynTime(true),
m_bThreadCreateSussess(false),
m_strIP("")
{
    //合成图片初始化
    ReadConfig();
}

BaseCamera::~BaseCamera()
{
    InterruptionConnection();
    DisConnectCamera();
    //m_strIP.clear();

    m_hWnd = NULL;
    WriteLog("finish delete Camera");
}

void BaseCamera::ReadHistoryInfo()
{
    //读取可靠性配置文件
    Tool_ReadIntValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "SafeModeEnable", m_SaveModelInfo.iSafeModeEnable);
    Tool_ReadKeyValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "BeginTime",  m_SaveModelInfo.chBeginTime, sizeof(m_SaveModelInfo.chBeginTime));
    Tool_ReadKeyValueFromConfigFile(SAFE_MODE_INI,m_strIP.c_str(), "EndTime", m_SaveModelInfo.chEndTime, sizeof(m_SaveModelInfo.chEndTime));

    Tool_ReadIntValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "Index", m_SaveModelInfo.iIndex);
    Tool_ReadIntValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "DataType", m_SaveModelInfo.iDataType);
}

void BaseCamera::WriteHistoryInfo(SaveModeInfo& SaveInfo)
{
    char iniFileName[MAX_PATH] = { 0 };
    char iniDeviceInfoName[MAX_PATH] = { 0 };

    strcat(iniFileName, Tool_GetCurrentPath());
    strcat(iniFileName, "\\SafeModeConfig.ini");

    //读取配置文件
    char chTemp[256] = { 0 };
    sprintf(chTemp,  "%d", m_SaveModelInfo.iSafeModeEnable);

    //if(m_SaveModelInfo.iSafeModeEnable == 0)
    //{
    //	SYSTEMTIME st;	
    //	GetLocalTime(&st);
    //	sprintf_s(m_SaveModelInfo.chBeginTime, "%d.%d.%d_%d", st.wYear, st.wMonth, st.wDay, st.wHour);
    //}
    Tool_WriteKeyValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "SafeModeEnable", chTemp, sizeof(chTemp));
    Tool_WriteKeyValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "BeginTime", SaveInfo.chBeginTime,sizeof(SaveInfo.chBeginTime) );
    Tool_WriteKeyValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "EndTime", SaveInfo.chEndTime, sizeof(SaveInfo.chEndTime));
    memset(chTemp, 0, sizeof(chTemp));
    //sprintf_s(chTemp, sizeof(chTemp), "%d", SaveInfo.iIndex);
    sprintf(chTemp, "%d", SaveInfo.iIndex);
    Tool_WriteKeyValueFromConfigFile(SAFE_MODE_INI, m_strIP.c_str(), "Index", chTemp, sizeof(chTemp));
}

void BaseCamera::setSaveBufferPath(const char *path, size_t bufLength)
{
    if(path == NULL
            || strlen(path) <= 0
            || strlen(path) > 256
            || bufLength <=0
            || bufLength > 256)
    {
        return;
    }
    m_csFuncCallback.lock();
    sprintf(m_chBufferPath, "%s",path );
    m_csFuncCallback.unlock();
}

bool BaseCamera::SaveImgToDisk(char* chImgPath, unsigned char* pImgData, unsigned long dwImgSize)
{
    WriteLog("begin SaveImgToDisk");
    if (NULL == pImgData || NULL == chImgPath)
    {
        WriteLog("SaveImgToDisk, failed.NULL == pImgData || NULL == chImgPath");
        return false;
    }
    char chLogBuff[MAX_PATH] = { 0 };
    bool bRet = false;

    if (NULL != strstr(chImgPath, "\\") || NULL != strstr(chImgPath, "/"))
    {
        std::string tempFile(chImgPath);
        size_t iPosition = std::string::npos;
        if (NULL != strstr(chImgPath, "\\"))
        {
            iPosition = tempFile.rfind("\\");
        }
        else
        {
            iPosition = tempFile.rfind("/");
        }
        std::string tempDir = tempFile.substr(0, iPosition + 1);
        if (!Tool_MyMakeDir(tempDir.c_str()))
        {
            memset(chLogBuff, '\0', sizeof(chLogBuff));
            sprintf(chLogBuff, "%s save failed, create path failed.", chImgPath);
            //sprintf_s(chLogBuff, sizeof(chLogBuff), "%s save failed, create path failed.", chImgPath);
            WriteLog(chLogBuff);
            return false;
        }
    }

    size_t iWritedSpecialSize = 0;
    FILE* fp = NULL;
    fp = fopen(chImgPath, "wb+");
    //errno_t errCode;
    //_set_errno(0);
    //errCode = fopen_s(&fp, chImgPath, "wb+");
    if (fp)
    {
        //iWritedSpecialSize = fwrite(pImgData, dwImgSize , 1, fp);
        iWritedSpecialSize = fwrite(pImgData, sizeof(unsigned char), dwImgSize, fp);
        fflush(fp);
        fclose(fp);
        fp = NULL;
        bRet = true;
    }
    else
    {
        memset(chLogBuff, '\0', sizeof(chLogBuff));
        sprintf(chLogBuff, "%s save failed", chImgPath);
        //sprintf_s(chLogBuff, sizeof(chLogBuff), "%s open failed, error code = %d", chImgPath, errCode);
        WriteLog(chLogBuff);
    }
    if (iWritedSpecialSize == dwImgSize)
    {
        memset(chLogBuff, '\0', sizeof(chLogBuff));
        sprintf(chLogBuff, "%s save success", chImgPath);
        //sprintf_s(chLogBuff, sizeof(chLogBuff), "%s save success", chImgPath);
        WriteLog(chLogBuff);
    }
    else
    {
        memset(chLogBuff, '\0', sizeof(chLogBuff));
        //sprintf_s(chLogBuff, "%s save success", chImgPath);
        //_get_errno(&errCode);
        sprintf(chLogBuff, /*sizeof(chLogBuff),*/ "%s write no match, size = %lu, write size = %lu",
            chImgPath,
            dwImgSize,
            iWritedSpecialSize
            /*,errCode*/);
        WriteLog(chLogBuff);
    }

    WriteLog("end SaveImgToDisk");
    return bRet;
}

bool BaseCamera::SaveImgToDisk(char* chImgPath, unsigned char* pImgData, unsigned long dwImgSize, int iWidth, int iHeight, int iType /*= 0*/)
{
    //iType 为0时压缩图像，1时不压缩
    if (pImgData == NULL || dwImgSize < 0 || iWidth < 0 || iHeight < 0)
    {
        return false;
    }
    return false;
}

bool BaseCamera::SetCameraInfo(CameraInfo& camInfo)
{
    m_strIP = std::string(camInfo.chIP);
    m_strDeviceID = std::string(camInfo.chDeviceID);
    m_bLogEnable = camInfo.bLogEnable;
    m_bSynTime = camInfo.bSynTimeEnable;
    m_iDirection = camInfo.iDirection;

    sprintf(m_chDeviceID, "%s", camInfo.chDeviceID);
    sprintf(m_chLaneID, "%s", camInfo.chLaneID);
    sprintf(m_chStationID, "%s", camInfo.chStationID);
    return true;
}

int BaseCamera::GetCamStatus()
{
    if (NULL == m_hHvHandle)
        return 1;
    unsigned long dwStatus = 1;

    if (HVAPI_GetConnStatusEx((HVAPI_HANDLE_EX)m_hHvHandle, CONN_TYPE_RECORD, &dwStatus) == S_OK)
    {
        if (dwStatus == CONN_STATUS_NORMAL
            /*|| dwStatus == CONN_STATUS_RECVDONE*/)
        {
            m_iConnectStatus = 0;
        }
        else if (dwStatus == CONN_STATUS_RECONN)
        {
            m_iConnectStatus = 1;
        }
        else
        {
            m_iConnectStatus = 1;
        }
    }
    else
    {
        m_iConnectStatus = 1;
    }
    return m_iConnectStatus;
}

char* BaseCamera::GetStationID()
{
    return m_chStationID;
}

char* BaseCamera::GetDeviceID()
{
    return m_chDeviceID;
}

char* BaseCamera::GetLaneID()
{
    return m_chLaneID;
}

const char* BaseCamera::GetCameraIP()
{
    return m_strIP.c_str();
}

int BaseCamera::ConnectToCamera()
{
    if (m_strIP.empty())
    {
        WriteLog("ConnectToCamera:: please finish the camera ip address");
        return -1;
    }
    if (NULL != m_hHvHandle)
    {
        InterruptionConnection();
        HVAPI_CloseEx((HVAPI_HANDLE_EX)m_hHvHandle);
        m_hHvHandle = NULL;
    }
    m_hHvHandle = HVAPI_OpenEx(m_strIP.c_str(), NULL);
    //m_hHvHandle = HVAPI_OpenChannel(m_strIP.c_str(), NULL, 0);
    if (NULL == m_hHvHandle)
    {
        WriteLog("ConnectToCamera:: Open CameraHandle failed!");
        return -2;
    }

    ReadHistoryInfo();
    char chCommand[1024] = { 0 };
    //sprintf_s(chCommand, sizeof(chCommand), "DownloadRecord,BeginTime[%s],Index[%d],Enable[%d],EndTime[%s],DataInfo[%d]",
    //    m_SaveModelInfo.chBeginTime,
    //    m_SaveModelInfo.iIndex,
    //    m_SaveModelInfo.iSafeModeEnable,
    //    m_SaveModelInfo.chEndTime,
    //    m_SaveModelInfo.iDataType);
    sprintf(chCommand, "DownloadRecord,BeginTime[%s],Index[%d],Enable[%d],EndTime[%s],DataInfo[%d]",
		m_SaveModelInfo.chBeginTime,
		m_SaveModelInfo.iIndex,
		m_SaveModelInfo.iSafeModeEnable,
		m_SaveModelInfo.chEndTime,
		m_SaveModelInfo.iDataType);

    WriteLog(chCommand);

    if ((HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoBeginCallBack, this, 0, CALLBACK_TYPE_RECORD_INFOBEGIN, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoEndCallBack, this, 0, CALLBACK_TYPE_RECORD_INFOEND, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoPlateCallBack, this, 0, CALLBACK_TYPE_RECORD_PLATE, NULL) != S_OK) /*||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoBigImageCallBack, this, 0, CALLBACK_TYPE_RECORD_BIGIMAGE, chCommand) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoSmallImageCallBack, this, 0, CALLBACK_TYPE_RECORD_SMALLIMAGE, chCommand) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoBinaryImageCallBack, this, 0, CALLBACK_TYPE_RECORD_BINARYIMAGE, chCommand) != S_OK)*//* ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)JPEGStreamCallBack, this, 0, CALLBACK_TYPE_JPEG_FRAME, NULL) != S_OK)*/
        )
    {
        WriteLog("ConnectToCamera:: SetCallBack failed.");
        HVAPI_CloseEx(m_hHvHandle);
        m_hHvHandle = NULL;
        return -3;
    }
    else
    {
        WriteLog("ConnectToCamera:: SetCallBack success.");
    }

    return 0;
}

int BaseCamera::ConnectToCameraWithoutResult()
{
    if (m_strIP.empty())
    {
        WRITE_LOG("please finish the camera ip address");
        return -1;
    }
    if (NULL != m_hHvHandle)
    {
        InterruptionConnection();
        HVAPI_CloseEx((HVAPI_HANDLE_EX)m_hHvHandle);
        m_hHvHandle = NULL;
    }
    m_hHvHandle = HVAPI_OpenEx(m_strIP.c_str(), NULL);
    //m_hHvHandle = HVAPI_OpenChannel(m_strIP.c_str(), NULL, 0);
    if (NULL == m_hHvHandle)
    {
        WRITE_LOG("Open CameraHandle failed!");
        return -2;
    }
    WRITE_LOG("Open CameraHandle success handel = %p!",  m_hHvHandle);
    return 0;
}

void BaseCamera::ReadConfig()
{
    //读取可靠性配置文件
    int iLog = 0;
    Tool_ReadIntValueFromConfigFile(INI_FILE_NAME, "Log", "Enable", iLog);
    m_bLogEnable = (iLog == 1) ? true : false;

//    char chTemp[256] = { 0 };
//    sprintf(chTemp, "%d", iLog);
//    Tool_WriteKeyValueFromConfigFile(INI_FILE_NAME, "Log", "Enable", chTemp, sizeof(chTemp));
}

void BaseCamera::WriteFormatLog(const char* szfmt, ...)
{
    static char g_szPbString[10240] = { 0 };
    memset(g_szPbString, 0, sizeof(g_szPbString));

    va_list arg_ptr;
    va_start(arg_ptr, szfmt);
    //vsnprintf_s(g_szPbString, sizeof(g_szPbString), szfmt, arg_ptr);
	_vsnprintf(g_szPbString,  sizeof(g_szPbString),  szfmt, arg_ptr);

    WriteLog(g_szPbString);

    va_end(arg_ptr);
}

bool BaseCamera::WriteLog(const char* chlog)
{
    ReadConfig();
    if (!m_bLogEnable || NULL == chlog)
        return false;

    //取得当前的精确毫秒的时间
    struct tm stimeNow;
    long iTimeInMilliseconds = 0;
    Tool_GetTime(& stimeNow, &iTimeInMilliseconds);

    char chLogPath[512] = { 0 };
    char chLogRoot[256] = { 0 };
    Tool_ReadKeyValueFromConfigFile(INI_FILE_NAME, "Log", "Path", chLogRoot, sizeof(chLogRoot));
    if (strlen(chLogRoot) > 0)
    {
        sprintf(chLogPath, "%s/%04d-%02d-%02d/%s/",
			chLogRoot,
            stimeNow.tm_year,
            stimeNow.tm_mon,
            stimeNow.tm_mday,
			m_strIP.c_str());
    }
    else
    {
        sprintf(chLogPath,  "%s/XLWLog/%04d-%02d-%02d/%s/",
			Tool_GetCurrentPath(),
             stimeNow.tm_year,
             stimeNow.tm_mon,
             stimeNow.tm_mday,
			m_strIP.c_str());
    }

    Tool_MyMakeDir(chLogPath);

    char chLogFileName[512] = { 0 };
    sprintf(chLogFileName,"%s/CameraLog_%d-%02d_%02d.log",
            chLogPath,
            stimeNow.tm_year,
            stimeNow.tm_mon,
            stimeNow.tm_mday);

    m_csLog.lock();

    FILE *file = NULL;
    file = fopen(chLogFileName, "a+");
    //fopen_s(&file, chLogFileName, "a+");
    if (file)
    {
        fprintf(file, "[%04d-%02d-%02d %02d:%02d:%02d:%03d] : %s\n",
                stimeNow.tm_year,
                stimeNow.tm_mon,
                stimeNow.tm_mday,
                stimeNow.tm_hour,
                stimeNow.tm_min,
                stimeNow.tm_sec,
                iTimeInMilliseconds%1000,
                chlog);
        fclose(file);
        file = NULL;
    }

    m_csLog.unlock();

    return true;
}

bool BaseCamera::TakeCapture()
{
    if (NULL == m_hHvHandle)
        return false;

    bool bRet = true;
    char chRetBuf[1024 * 10] = { 0 };
    int nRetLen = 0;
    char chCaptureLog3[MAX_PATH] = { 0 };

    if (HVAPI_ExecCmdEx(m_hHvHandle, "SoftTriggerCapture", chRetBuf, sizeof(chRetBuf), &nRetLen) != S_OK)
    {
        bRet = false;

        //sprintf_s(chCaptureLog3, sizeof(chCaptureLog3), "Camera: %s SoftTriggerCapture failed", m_strIP.c_str());
		sprintf(chCaptureLog3,  "Camera: %s SoftTriggerCapture failed", m_strIP.c_str());
        WriteLog(chCaptureLog3);
    }
    else
    {
        //sprintf_s(chCaptureLog3, sizeof(chCaptureLog3), "Camera: %s SoftTriggerCapture success", m_strIP.c_str());
		sprintf(chCaptureLog3,  "Camera: %s SoftTriggerCapture success", m_strIP.c_str());
        WriteLog(chCaptureLog3);
    }
    return bRet;
}

bool BaseCamera::SynTime()
{
    if (NULL == m_hHvHandle)
        return false;
    //if (!m_bSynTime)
    //{
    //	return false;
    //}
    WriteLog("SynTime begin");

     char chTemp[256] = { 0 };

     struct tm st_localTime;
     long iMilSeceond = 0;
     Tool_GetTime(&st_localTime, &iMilSeceond);

	sprintf(chTemp, "SetTime,Date[%d.%02d.%02d],Time[%02d:%02d:%02d]",
        st_localTime.tm_year,
        st_localTime.tm_mon,
        st_localTime.tm_mday,
        st_localTime.tm_hour,
        st_localTime.tm_min,
        st_localTime.tm_sec);

    WriteLog(chTemp);
    char szRetBuf[1024] = { 0 };
    int nRetLen = 0;
    if (m_hHvHandle != NULL)
    {
        char chSynTimeLogBuf[MAX_PATH] = { 0 };
        try
        {
            if (HVAPI_ExecCmdEx(m_hHvHandle, chTemp, szRetBuf, 1024, &nRetLen) != S_OK)
            {
                memset(chSynTimeLogBuf, 0, sizeof(chSynTimeLogBuf));
                //sprintf_s(chSynTimeLogBuf, sizeof(chSynTimeLogBuf), "Camera: %s SynTime failed", m_strIP.c_str());
				sprintf(chSynTimeLogBuf, "Camera: %s SynTime failed", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf);
                return false;
            }
            else
            {
                memset(chSynTimeLogBuf, 0, sizeof(chSynTimeLogBuf));
                //sprintf_s(chSynTimeLogBuf, sizeof(chSynTimeLogBuf), "Camera: %s SynTime success", m_strIP.c_str());
				sprintf(chSynTimeLogBuf,  "Camera: %s SynTime success", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf);
            }
        }
        catch (...)
        {
            memset(chSynTimeLogBuf, 0, sizeof(chSynTimeLogBuf));
            //sprintf_s(chSynTimeLogBuf, sizeof(chSynTimeLogBuf), "Camera: %s SynTime take exception", m_strIP.c_str());
			sprintf(chSynTimeLogBuf,  "Camera: %s SynTime take exception", m_strIP.c_str());
            WriteLog(chSynTimeLogBuf);
        }
    }
    WriteLog("SynTime end");
    return true;
}

bool BaseCamera::SynTime(int Year, int Month, int Day, int Hour, int Minute, int Second, int MilientSecond)
{
    if (NULL == m_hHvHandle)
    {
        WriteLog("SynTime  failed: ConnectStatus != 0.");
        return false;
    }
    //if (!m_bSynTime)
    //{
    //	WriteLog("SynTime  failed: SynTime is not open.");
    //	return false;
    //}
    if (abs(Month) > 12 || abs(Day) > 31 || abs(Hour) > 24 || abs(Minute) > 60 || abs(Second) > 60)
    {
        WriteLog("SynTime  failed: time value is invalid.");
        return false;
    }
    WriteLog("SynTime begin");

    char chTemp[256] = { 0 };

	sprintf(chTemp, "SetTime,Date[%d.%02d.%02d],Time[%02d:%02d:%02d]",
		abs(Year), 
		abs(Month),
		abs(Day),
		abs(Hour), 
		abs(Minute),
		abs(Second));
    WriteLog(chTemp);
    char szRetBuf[1024] = { 0 };
    int nRetLen = 0;
    if (m_hHvHandle != NULL)
    {
        char chSynTimeLogBuf1[MAX_PATH] = { 0 };
        try
        {
            if (HVAPI_ExecCmdEx(m_hHvHandle, chTemp, szRetBuf, 1024, &nRetLen) != S_OK)
            //if (HVAPI_SetTime(m_hHvHandle, Year, Month, Day, Hour, Minute, Second, 0) != S_OK)
            {

                memset(chSynTimeLogBuf1, 0, sizeof(chSynTimeLogBuf1));
                //sprintf_s(chSynTimeLogBuf1, sizeof(chSynTimeLogBuf1), "Camera: %s SynTime failed", m_strIP.c_str());
				sprintf(chSynTimeLogBuf1, "Camera: %s SynTime failed", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf1);
                return false;
            }
            else
            {
                memset(chSynTimeLogBuf1, 0, sizeof(chSynTimeLogBuf1));
                //sprintf_s(chSynTimeLogBuf1, sizeof(chSynTimeLogBuf1), "Camera: %s SynTime success.", m_strIP.c_str());
				sprintf(chSynTimeLogBuf1,  "Camera: %s SynTime success.", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf1);
            }
        }
        catch (...)
        {
            memset(chSynTimeLogBuf1, 0, sizeof(chSynTimeLogBuf1));
            //sprintf_s(chSynTimeLogBuf1, sizeof(chSynTimeLogBuf1), "Camera: %s SynTime exception.", m_strIP.c_str());
			 sprintf(chSynTimeLogBuf1, "Camera: %s SynTime exception.", m_strIP.c_str());
            WriteLog(chSynTimeLogBuf1);
        }
    }
    WriteLog("SynTime end");
    return true;
}
bool BaseCamera::GetDeviceTime(DeviceTime& deviceTime)
{
    if (NULL == m_hHvHandle)
        return false;

    char chRetBuf[1024] = { 0 };
    int nRetLen = 0;

    if (HVAPI_ExecCmdEx(m_hHvHandle, "DateTime", chRetBuf, sizeof(chRetBuf), &nRetLen) != S_OK)
    {
        WriteLog("GetDeviceTime:: failed.");
        return false;
    }
    WriteLog(chRetBuf);
    bool bRet = false;
    const char* chFileName = "./DateTime.xml";
    remove(chFileName);

    //FILE* file_L = fopen(chFileName, "wb");
    FILE* file_L = NULL;
	file_L = fopen(chFileName, "wb");
    //fopen_s(&file_L, chFileName, "wb");
    if (file_L)
    {
        size_t size_Read = fwrite(chRetBuf, 1, nRetLen, file_L);
        fclose(file_L);
        file_L = NULL;
        char chFileLog[260] = { 0 };
        //sprintf_s(chFileLog, sizeof(chFileLog), "GetDeviceTime:: DateTime.xml create success, size =%d ", size_Read);
		sprintf(chFileLog, "GetDeviceTime:: DateTime.xml create success, size =%d ", size_Read);
        WriteLog(chFileLog);
        bRet = true;
    }
    if (!bRet)
    {
        WriteLog("GetDeviceTime:: DateTime.xml create failed.");
        return false;
    }

    const char* pDate = NULL;
    const char* pTime = NULL;
    TiXmlDocument cXmlDoc;
    //    if(cXmlDoc.Parse(chRetBuf))
    if (cXmlDoc.LoadFile(chFileName))
    {
        TiXmlElement* pSectionElement = cXmlDoc.RootElement();
        if (pSectionElement)
        {
            TiXmlElement* pChileElement = pSectionElement->FirstChildElement();
            pDate = pChileElement->Attribute("Date");
            pTime = pChileElement->Attribute("Time");
        }
        else
        {
            WriteLog("find Root element failed.");
        }
    }
    else
    {
        WriteLog("parse failed");
    }
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMiliSecond = 0;
    sscanf(pDate, "%04d.%02d.%02d", &iYear, &iMonth, &iDay);
    sscanf(pTime, "%02d:%02d:%02d %03d", &iHour, &iMinute, &iSecond, &iMiliSecond);

    deviceTime.iYear = iYear;
    deviceTime.iMonth = iMonth;
    deviceTime.iDay = iDay;
    deviceTime.iHour = iHour;
    deviceTime.iMinutes = iMinute;
    deviceTime.iSecond = iSecond;
    deviceTime.iMilisecond = iMiliSecond;

    return true;
}

void BaseCamera::SaveResultToBufferPath(CameraResult* pResult)
{
    if (NULL == pResult)
    {
        return;
    }
    //将图片缓存到缓存目录
    unsigned long long dwPlateTime = 0;
    //char chBigImgFileName[260] = {0};
    //char chBinImgFileName[260] = {0};
    char chPlateColor[32] = { 0 };

    dwPlateTime = pResult->dw64TimeMS / 1000;
    if (strstr(pResult->chPlateNO, "无"))
    {
        //sprintf_s(chPlateColor, sizeof(chPlateColor), "无");
		sprintf(chPlateColor,  "无");
    }
    else
    {
        //sprintf_s(chPlateColor, sizeof(chPlateColor), "%s", pResult->chPlateColor);
		sprintf(chPlateColor, "%s", pResult->chPlateColor);
    }

    std::string strTime(pResult->chPlateTime);

    char chResultPath[256] = {0};
    sprintf(chResultPath, "%s/%4d/%s/",
            m_chBufferPath,
            strTime.substr(0, 4).c_str(),
            strTime.substr(0, 14).c_str());

    Tool_MyMakeDir(chResultPath);

//    if (pResult->CIMG_BinImage.pbImgData)
//    {
//        bool bSave = SaveImgToDisk(pResult->CIMG_BinImage.chSavePath,
//            pResult->CIMG_BinImage.pbImgData,
//            pResult->CIMG_BinImage.dwImgSize);
//    }

//    if (pResult->CIMG_PlateImage.pbImgData)
//    {
//        bool bSave = SaveImgToDisk(pResult->CIMG_PlateImage.chSavePath,
//            pResult->CIMG_PlateImage.pbImgData,
//            pResult->CIMG_PlateImage.dwImgSize);
//    }

    char chFileName[256] = {0};
    if (pResult->CIMG_LastSnapshot.pbImgData)
    {
        //che tou
        strcpy(pResult->CIMG_LastSnapshot.chSavePath, chResultPath);
        sprintf(chFileName, "%s/%s_0.jpg", chResultPath, strTime.substr(0, 14).c_str());

        bool bSave = SaveImgToDisk(chFileName,
            pResult->CIMG_BestSnapshot.pbImgData,
            pResult->CIMG_LastSnapshot.dwImgSize);
    }
    if (pResult->CIMG_BeginCapture.pbImgData)
    {
        //che shen
        strcpy(pResult->CIMG_BeginCapture.chSavePath, chResultPath);
        sprintf(chFileName, "%s/%s_1.jpg", chResultPath, strTime.substr(0, 14).c_str());

        bool bSave = SaveImgToDisk(chFileName,
            pResult->CIMG_BeginCapture.pbImgData,
            pResult->CIMG_BeginCapture.dwImgSize);
    }

    if (pResult->CIMG_BestCapture.pbImgData)
    {
        //che wei
        strcpy(pResult->CIMG_BestCapture.chSavePath, chResultPath);
        sprintf(chFileName, "%s/%s_2.jpg", chResultPath, strTime.substr(0, 14).c_str());

        bool bSave = SaveImgToDisk(pResult->CIMG_BestCapture.chSavePath,
            pResult->CIMG_BestCapture.pbImgData,
            pResult->CIMG_BestCapture.dwImgSize);
    }

//    if (pResult->CIMG_LastCapture.pbImgData)
//    {
//        bool bSave = SaveImgToDisk(pResult->CIMG_LastCapture.chSavePath,
//            pResult->CIMG_LastCapture.pbImgData,
//            pResult->CIMG_LastCapture.dwImgSize);
//    }

//    if (pResult->CIMG_BestSnapshot.pbImgData)
//    {
//        bool bSave = SaveImgToDisk(pResult->CIMG_BestSnapshot.chSavePath,
//            pResult->CIMG_BestSnapshot.pbImgData,
//            pResult->CIMG_BestSnapshot.dwImgSize);
//    }

}

void BaseCamera::InterruptionConnection()
{
    if (NULL == m_hHvHandle)
    {
        return;
    }

    if ((HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_INFOBEGIN, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_PLATE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_BIGIMAGE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_SMALLIMAGE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_BINARYIMAGE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_INFOEND, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_JPEG_FRAME, NULL) != S_OK)
        )
    {
        WriteLog("DisConnectToCamera:: SetCallBack NULL failed.");
        //return false;
    }
    else
    {
        WriteLog("DisConnectToCamera:: SetCallBack NULL success.");
    }
}

bool BaseCamera::DisConnectCamera()
{
    bool bRet = true;
    if (NULL != m_hHvHandle)
    {
        WriteFormatLog("HVAPI_CloseEx, begin.");
        HRESULT hRet = HVAPI_CloseEx((HVAPI_HANDLE_EX)m_hHvHandle);
        if (hRet == S_OK)
        {
            m_hHvHandle = NULL;
            WriteFormatLog("HVAPI_CloseEx, success.");
        }
        else
        {
            bRet = false;
            WriteFormatLog("HVAPI_CloseEx, failed.");
        }
    }
    return bRet;
}

bool BaseCamera::SenMessageToCamera(int iMessageType, int& iReturnValue, int& iErrorCode, int iArg)
{
    if (NULL == m_hHvHandle)
    {
        iErrorCode = -1;
        return false;
    }

    char chRetBuf[1024] = { 0 };
    char chSendBuf[256] = { 0 };
    char chLog[256] = { 0 };
    int nRetLen = 0;
    const char* pAttribute1 = NULL;
    const char* pAttribute2 = NULL;
    const char* pAttribute3 = NULL;
    const char* pAttribute4 = NULL;

    if (iMessageType == CMD_DEL_VEH_HEAD)
    {
        //sprintf_s(chSendBuf, sizeof(chSendBuf), "DeleteHead_ZHWL");
		sprintf(chSendBuf, "DeleteHead_ZHWL");
    }
    else if (iMessageType == CMD_GET_VEH_LENGTH)
    {
        //sprintf_s(chSendBuf, sizeof(chSendBuf), "GetQueueSize_ZHWL");
		sprintf(chSendBuf, "GetQueueSize_ZHWL");
    }
    else if (iMessageType == CMD_DEL_ALL_VEH)
    {
        //sprintf_s(chSendBuf, sizeof(chSendBuf), "DeleteAll_ZHWL");
		sprintf(chSendBuf, "GetQueueSize_ZHWL");
    }
    else if (iMessageType == CMD_GET_VHE_HEAD)
    {
        int iValue = 0;
        if (iArg < 0)
        {
            iValue = 0;
        }
        else
        {
            iValue = iArg;
        }
        //sprintf_s(chSendBuf, sizeof(chSendBuf), "GetResult_ZHWL, Value[%d]", iValue);
		sprintf(chSendBuf, "GetResult_ZHWL, Value[%d]", iValue);
    }

    if (strlen(chSendBuf) <= 0)
    {
        WriteLog("SenMessageToCamera, please input the right command");
        return false;
    }

    if (HVAPI_ExecCmdEx(m_hHvHandle, chSendBuf, chRetBuf, 1024, &nRetLen) != S_OK)
    {
        memset(chLog, 0, sizeof(chLog));
        //sprintf_s(chLog, sizeof(chLog), "%s  send failed.", chSendBuf);
		sprintf(chLog,  "%s  send failed.", chSendBuf);
        WriteLog(chLog);

        iErrorCode = -2;
        return false;
    }
    else
    {
        memset(chLog, 0, sizeof(chLog));
        //sprintf_s(chLog, sizeof(chLog), "%s  send success.", chSendBuf);
		sprintf(chLog,  "%s  send success.", chSendBuf);
        WriteLog(chLog);
    }

    if (iMessageType == CMD_GET_VEH_LENGTH)
    {
        int iLength = 0;
        sscanf(chRetBuf, "%d", &iLength);
        iReturnValue = iLength;
    }

    iErrorCode = 0;
    return true;
}

int BaseCamera::GetLoginID()
{
    int iLoginID = 0;
    m_csFuncCallback.lock();
    iLoginID = m_iLoginID;
    m_csFuncCallback.unlock();
    return iLoginID;
}

void BaseCamera::SetLoginID(int iID)
{
    m_csFuncCallback.lock();
    m_iLoginID = iID;
    m_csFuncCallback.unlock();
}

void BaseCamera::SetCameraIP(const char* ipAddress)
{
    m_csFuncCallback.lock();
    m_strIP = std::string(ipAddress);
    m_csFuncCallback.unlock();
}

void BaseCamera::SetCameraIndex(int iIndex)
{
    m_csFuncCallback.lock();
    m_iIndex = iIndex;
    m_csFuncCallback.unlock();
}

bool BaseCamera::SetOverlayVedioFont(int iFontSize, int iColor)
{
    char chLog[260] = { 0 };
    //sprintf_s(chLog, sizeof(chLog), "SetOverlayVedioFont, size = %d, color = %d", iFontSize, iColor);
	sprintf(chLog,  "SetOverlayVedioFont, size = %d, color = %d", iFontSize, iColor);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;
    HRESULT hRet2 = S_FALSE;
    int iR = 255, iG = 255, iB = 255;
    switch (iColor)
    {
    case 0:		//白
        iR = 255, iG = 255, iB = 255;
        break;
    case 1:		//红
        iR = 255, iG = 0, iB = 0;
        break;
    case 2:		//黄
        iR = 255, iG = 255, iB = 0;
        break;
    case 3:		//蓝
        iR = 0, iG = 0, iB = 255;
        break;
    case 4:		//黑
        iR = 0, iG = 0, iB = 0;
        break;
    case 5:		//绿
        iR = 0, iG = 255, iB = 0;
        break;
    case 6:		//紫
        iR = 138, iG = 43, iB = 226;
        break;
    default:
        iR = 255, iG = 255, iB = 255;
        break;
    }

    if (m_hHvHandle)
    {
        //hRet = HVAPI_SetOSDFont((HVAPI_HANDLE_EX)m_hHvHandle, 0, iFontSize, iR, iG, iB);
        //hRet2 = HVAPI_SetOSDFont((HVAPI_HANDLE_EX)m_hHvHandle, 1 , iFontSize, iR, iG, iB);
        //hRet2 = HVAPI_SetOSDFont((HVAPI_HANDLE_EX)m_hHvHandle, 2, iFontSize, iR, iG, iB);
        if (S_OK == hRet)
        {
            WriteLog("set H264 Font  success.");
        }
        if (S_OK == hRet2)
        {
            WriteLog("set JPEG Font  success");
        }
    }
    else
    {
        WriteLog("set time text, but the handle is invalid.");
    }
    if (hRet != S_OK && hRet2 != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayVideoText(int streamId, char* overlayText, int textLength)
{
    char chLog[1024] = { 0 };
    //sprintf_s(chLog, sizeof(chLog), "SetOverlayVedioText, streamID= %d, string length = %d, overlayText = [%s] ", streamId, textLength, overlayText);
	sprintf(chLog,  "SetOverlayVedioText, streamID= %d, string length = %d, overlayText = [%s] ", streamId, textLength, overlayText);
    WriteFormatLog(chLog);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置字符叠加字符串
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	szText			叠加字符串 长度范围：0～255
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        //hRet = HVAPI_SetOSDText(m_hHvHandle, streamId, overlayText);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayVedioText   success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayVedioText, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayVideoTextPos(int streamId, int posX, int posY)
{
    WriteFormatLog("SetOverlayVideoTextPos,streamID= %d posX = %d ,posY=%d", streamId, posX, posY);

    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置字符叠加位置（保持兼容）
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	nPosX			叠加位置X坐标，范围: 0~图像宽
        * @param[in]	nPosY			叠加位置Y坐标，范围：0~图像高
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        //hRet = HVAPI_SetOSDPos(m_hHvHandle, streamId, posX, posY);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayVideoTextPos  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayVideoTextPos, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayVideoTextEnable(int streamId, bool enable)
{
    WriteFormatLog( "SetOverlayVideoTextEnable,streamID= %d enable = %d ", streamId, enable);

    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置字符叠加开关
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	fOSDEnable		0：关闭，1：打开
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        //hRet = HVAPI_SetOSDEnable(m_hHvHandle, streamId, enable);
        if (S_OK == hRet)
        {
            WriteLog("set HVAPI_SetOSDEnable  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayVideoTextEnable, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayTimeEnable(int streamID, bool enable)
{
    WriteFormatLog( "SetOverlayTimeEnable,streamID= %d enable = %d ", streamID, enable);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置时间叠加开关
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	fEnable			字符叠加时间叠加开关，范围：0：关闭，1：打开
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        //hRet = HVAPI_SetOSDTimeEnable(m_hHvHandle, streamID, enable);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayTimeEnable  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayTimeEnable, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayTimeFormat(int streamId, int iformat)
{
    WriteFormatLog("SetOverlayTimeFormat,streamID= %d iformat = %d ", streamId, iformat);

    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        INT nDateSeparator = 0, fShowWeekDay = 0, fTimeNewLine = 0, fShowMicroSec = 0;
        switch (iformat)
        {
        case 0:
            nDateSeparator = 0;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 0;
            break;
        case 1:
            nDateSeparator = 4;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 0;
            break;
        case 2:
            nDateSeparator = 5;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 0;
            break;
        default:
            nDateSeparator = 0;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 1;
            break;
        }
        /**
        * @brief	   设置OSD叠加时间格式
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	nDateSeparator	日期分割符号 0:减号 1:斜杠 2:中文 3:点号
        * @param[in]	fShowWeekDay    显示星期几
        * @param[in]	fTimeNewLine	日期一行，时间另起一行
        * @param[in]	fShowMicroSec	显示微秒
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        //hRet = HVAPI_SetOSDTimeFormat(m_hHvHandle, streamId, nDateSeparator, fShowWeekDay, fTimeNewLine, fShowMicroSec);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayTimeFormat  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayTimeFormat, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayTimePos(int streamId, int posX, int posY)
{
    WriteFormatLog("SetOverlayTimePos,streamID= %d posX = %d ,posY=%d", streamId, posX, posY);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置时间叠加位置
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	nPosX			叠加位置X坐标，范围: 0~图像宽
        * @param[in]	nPosY			叠加位置Y坐标，范围：0~图像高
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        //hRet = HVAPI_SetOSDTimePos(m_hHvHandle, streamId, posX, posY);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayTimePos  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayTimePos, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::GetHardWareInfo(BasicInfo& bsinfo)
{
//    if (NULL == m_hHvHandle)
//    {
//        return false;
//    }
//    CDevBasicInfo tempInfo;
//    HRESULT hr = HVAPI_GetDevBasicInfo((HVAPI_HANDLE_EX)m_hHvHandle, &tempInfo);
//    if (hr == S_OK)
//    {
//        memcpy(&bsinfo, &tempInfo, sizeof(BasicInfo));
//        return true;
//    }
//    else
    {
        return false;
    }
}

std::string BaseCamera::GetHarWareConfig()
{
    if(!m_hHvHandle)
    {
        WRITE_LOG("m_hHvHandle == NULL.");
        return std::string("");
    }
    std::string strXmL;
    int iBufSize = 1024*1024;
    char* chXmlBuf = new char[iBufSize];
    memset(chXmlBuf, '\0', iBufSize);

    if(S_OK == HVAPI_GetParamEx( m_hHvHandle, chXmlBuf, iBufSize, &iBufSize))
    {
        strXmL = std::string(chXmlBuf);
        WRITE_LOG("HVAPI_GetParamEx success.");
    }
    else
    {
        WRITE_LOG("HVAPI_GetParamEx failed.");
    }

    SAFE_DELETE_ARRAY(chXmlBuf);

    return strXmL;
}

bool BaseCamera::UploadHarWareConfig(const char *xmlContents)
{
    if(NULL == m_hHvHandle
            || NULL == xmlContents)
    {
        WRITE_LOG(" NULL == m_hHvHandle  || NULL == xmlContents.");
        return false;
    }

    if(S_OK == HVAPI_SetParamEx( m_hHvHandle, xmlContents))
    {
       WRITE_LOG(" success.");
        return true;
    }
    WRITE_LOG(" failed.");
    return false;
}

void BaseCamera::CheckStatus()
{
	return;
}

void* Camera_StatusCheckThread(void* lpParam)
{
    if (!lpParam)
    {
        return 0;
    }
    BaseCamera* pThis = (BaseCamera*)lpParam;
    pThis->CheckStatus();

    return 0;
}
