//#include "stdafx.h"
#include "Camera6467_plate.h"
#include "libHvDevice/HvDeviceBaseType.h"
#include "libHvDevice/HvDeviceCommDef.h"
#include "libHvDevice/HvDeviceEx.h"
//#include "libHvDevice/HvDeviceNew.h"
//#include "libHvDevice/HvCamera.h"
#include "utilityTool/tool_function.h"
#include"utilityTool/inifile.h"
//#include "utilityTool/log4z.h"
#include <process.h>
#include<time.h>


#define CHECK_ARG(arg)\
if (arg == NULL) \
{\
    WriteFormatLog("%s is NULL", #arg); \
    return 0; \
}

Camera6467_plate::Camera6467_plate() :
BaseCamera(),
m_iTimeInvl(3),
m_iCompressBigImgSize(COMPRESS_BIG_IMG_SIZE),
m_iCompressSamllImgSize(COMPRESS_PLATE_IMG_SIZE),
m_pTempBin(NULL),
m_pTempBig1(NULL),
m_pCaptureImg(NULL),
m_pTempBig(NULL),
m_CameraResult(NULL),
m_BufferResult(NULL),
m_pJpegCallbackFunc(NULL),
m_pUserData(NULL),
m_bResultComplete(false),
m_bJpegComplete(false),
m_bSaveToBuffer(false),
m_bOverlay(false),
m_bCompress(false),
m_bStatusCheckThreadExit(false)
{
	m_bProvideBigImg = true;
	m_bUseHwnd = false;
    memset(m_chResultPath, '\0', sizeof(m_chResultPath));
    ReadConfig();

    int iRet_thrd = pthread_create(&m_threadCheckStatus, NULL, &Camera_StatusCheckThread, (void *) this);
    m_bThreadCreateSussess = (iRet_thrd == 0) ? true :false;
    WriteFormatLog("pthread_create = %d", iRet_thrd);
}

Camera6467_plate::~Camera6467_plate()
{
    SetCheckThreadExit(true);
    InterruptionConnection();

    SAFE_DELETE_OBJ(m_CameraResult);
    SAFE_DELETE_OBJ(m_BufferResult);

    SAFE_DELETE_ARRAY(m_pTempBin);
    SAFE_DELETE_ARRAY(m_pTempBig1);
    SAFE_DELETE_ARRAY(m_pCaptureImg);
    SAFE_DELETE_ARRAY(m_pTempBig);

    if(m_bThreadCreateSussess)
    {
        pthread_join(m_threadCheckStatus, NULL);
    }
}

int Camera6467_plate::AnalysisVelchType(const char* vehType)
{
    if (vehType == NULL)
    {
        return UNKOWN_TYPE;
    }
    if (strstr(vehType, "客1"))
    {
        return BUS_TYPE_1;
    }
    else if (strstr(vehType, "客2"))
    {
        return BUS_TYPE_2;
        //printf("the Vehicle type code is 2.\n");
    }
    else if (strstr(vehType, "客3"))
    {
        return BUS_TYPE_3;
    }
    else if (strstr(vehType, "客4"))
    {
        return BUS_TYPE_4;
    }
    else if (strstr(vehType, "客5"))
    {
        return BUS_TYPE_5;
    }
    else if (strstr(vehType, "货1"))
    {
        return TRUCK_TYPE_1;
    }
    else if (strstr(vehType, "货2"))
    {
        return TRUCK_TYPE_2;
    }
    else if (strstr(vehType, "货3"))
    {
        return TRUCK_TYPE_3;
    }
    else if (strstr(vehType, "货4"))
    {
        return TRUCK_TYPE_4;
    }
    else if (strstr(vehType, "货5"))
    {
        return TRUCK_TYPE_5;
    }
    else
    {
        return UNKOWN_TYPE;
    }
}

void Camera6467_plate::ReadConfig()
{
    char szPath[MAX_PATH] = {0};
    //sprintf_s(szPath, sizeof(szPath), "%s\\Result\\", Tool_GetCurrentPath());
    sprintf(szPath, "%s/Result/", Tool_GetCurrentPath());
    Tool_ReadKeyValueFromConfigFile(INI_FILE_NAME, "Result", "Path", szPath, MAX_PATH);
    //strcpy_s(m_chResultPath, sizeof(m_chResultPath), szPath);
	strcpy(m_chResultPath, szPath);

	
	int iValue = 0;
	Tool_ReadIntValueFromConfigFile(INI_FILE_NAME, "COMPRESS_SET", "CompressEnable",  iValue);
	m_bCompressEnable = (bool)iValue;

	m_iCompressBigImgSize = COMPRESS_BIG_IMG_SIZE;
	Tool_ReadIntValueFromConfigFile(INI_FILE_NAME, "COMPRESS_SET", "CompressSize",  m_iCompressBigImgSize);

	m_iCompressBigImgWidth = 1600;
	Tool_ReadIntValueFromConfigFile(INI_FILE_NAME, "COMPRESS_SET", "BigImgWidth", m_iCompressBigImgWidth);

	m_iCompressBigImgHeight = 1200;
	Tool_ReadIntValueFromConfigFile(INI_FILE_NAME, "COMPRESS_SET", "BigImgHeight", m_iCompressBigImgWidth);

    BaseCamera::ReadConfig();
}

void Camera6467_plate::SetCheckThreadExit(bool bExit)
{
    m_csFuncCallback.lock();
    m_bStatusCheckThreadExit = bExit;
    m_csFuncCallback.unlock();
}

bool Camera6467_plate::GetCheckThreadExit()
{
    bool bExit = false;
    m_csFuncCallback.lock();
    bExit = m_bStatusCheckThreadExit;
    m_csFuncCallback.unlock();
    return bExit;
}

int Camera6467_plate::RecordInfoBegin(DWORD dwCarID)
{
    WriteFormatLog("RecordInfoBegin -begin- dwCarID = %lu", dwCarID);

    if (dwCarID == m_dwLastCarID)
    {
        WriteLog("相同carID,丢弃该结果");
        return 0;
    }
    SetResultComplete(false);
    SAFE_DELETE_OBJ(m_CameraResult);

    m_CameraResult = new CameraResult();
    if (m_CameraResult)
    {
        //sprintf_s(m_CameraResult->chServerIP, sizeof(m_CameraResult->chServerIP), "%s", m_strIP.c_str());
		sprintf(m_CameraResult->chServerIP,"%s", m_strIP.c_str());
        m_CameraResult->dwCarID = dwCarID;
    }

    WriteFormatLog("RecordInfoBegin -end- dwCarID = %lu", dwCarID);
    return 0;
}

int Camera6467_plate::RecordInfoEnd(DWORD dwCarID)
{
    WriteFormatLog("RecordInfoEnd begin, dwCarID = %d", dwCarID);

    if (dwCarID != m_dwLastCarID)
    {
        m_dwLastCarID = dwCarID;
    }
    else
    {
        WriteLog("相同CarID, 丢弃该结果");
        return 0;
    }

    if (NULL == m_CameraResult)
    {
        return 0;
    }

    //std::shared_ptr<CameraResult> pResult = std::shared_ptr<CameraResult>(m_CameraResult);
    //if (pResult)
    //{
    //    if (m_lsResultList.size() < 200)
    //    {
    //        m_lsResultList.push_back(pResult);
    //    }
    //    else
    //    {
    //        WriteFormatLog("RecordInfoEnd end, the result list is larger than 200, save the result direct.");
    //        //SerializationResult(pResult);
    //        SerializationResultEx(pResult);
    //    }        
    //    m_CameraResult = NULL;
    //}
    //SAFE_DELETE_OBJ(m_CameraResult);

    m_csResult.lock();
	if (m_BufferResult != NULL)
	{
		delete m_BufferResult;
		m_BufferResult = NULL;
	}
	m_BufferResult = new CameraResult(*m_CameraResult);
    m_csResult.unlock();

	SetResultComplete(true);

    //WriteFormatLog("RecordInfoEnd end, dwCarID = %lu", dwCarID, m_lsResultList.size());
	WriteFormatLog("RecordInfoEnd end, dwCarID = %lu", dwCarID);
    return 0;
}

int Camera6467_plate::RecordInfoPlate(DWORD dwCarID,
    LPCSTR pcPlateNo,
    LPCSTR pcAppendInfo,
    DWORD dwRecordType,
    DWORD64 dw64TimeMS)
{
    SetResultComplete(false);
    CHECK_ARG(m_CameraResult);

    WriteFormatLog("RecordInfoPlate -begin- dwCarID = %lu", dwCarID);

    if (dwCarID == m_dwLastCarID)
    {
        BaseCamera::WriteLog("相同carID,丢弃该结果");
        return 0;
    }
//#ifdef DEBUG
//    char chName[256] = { 0 };
//    sprintf_s(chName, "%lu.xml", GetTickCount());
//    Tool_SaveFileToPath(chName, (void*)pcAppendInfo, strlen(pcAppendInfo));
//#endif // DEBUG

    if (m_CameraResult->dwCarID == dwCarID)
    {
        m_CameraResult->dw64TimeMS = dw64TimeMS;
        //sprintf_s(m_CameraResult->chPlateNO, sizeof(m_CameraResult->chPlateNO), "%s", pcPlateNo);
		sprintf(m_CameraResult->chPlateNO, "%s", pcPlateNo);
        //m_CameraResult->strAppendInfo = std::string(pcAppendInfo);
        if (strlen(pcAppendInfo) < sizeof(m_CameraResult->pcAppendInfo) )
        {
            memset(m_CameraResult->pcAppendInfo, '\0', sizeof(m_CameraResult->pcAppendInfo));
            memcpy(m_CameraResult->pcAppendInfo, pcAppendInfo, strlen(pcAppendInfo));
        }
        else
        {
            WriteFormatLog("RecordInfoPlate -begin- strlen(pcAppendInfo) %u < %u sizeof(m_CameraResult->pcAppendInfo)",
                strlen(pcAppendInfo),
                sizeof(m_CameraResult->pcAppendInfo));
        }

        BaseCamera::WriteLog(m_CameraResult->chPlateNO);  
        BaseCamera::WriteLog(pcAppendInfo);
    }
	AnalysisAppendInfo(m_CameraResult);
    WriteFormatLog("RecordInfoPlate -end- dwCarID = %lu", dwCarID);
    return 0;
}

int Camera6467_plate::RecordInfoBigImage(DWORD dwCarID,
    WORD wImgType,
    WORD wWidth,
    WORD wHeight,
    PBYTE pbPicData,
    DWORD dwImgDataLen,
    DWORD dwRecordType,
    DWORD64 dw64TimeMS)
{
    SetResultComplete(false);

    CHECK_ARG(m_CameraResult);

    WriteFormatLog("RecordInfoBigImage -begin- dwCarID = %ld, dwRecordType = %#x， ImgType=%d, size = %ld",
        dwCarID, 
        dwRecordType, 
        wImgType,
        dwImgDataLen);

    if (dwCarID == m_dwLastCarID)
    {
        WriteLog("相同carID,丢弃该结果");
        return 0;
    }
    if (m_CameraResult->dwCarID == dwCarID)
    {
        if (wImgType == RECORD_BIGIMG_BEST_SNAPSHOT)
        {
            WriteLog("RecordInfoBigImage BEST_SNAPSHO  ");

            CopyDataToIMG(m_CameraResult->CIMG_BestSnapshot, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_LAST_SNAPSHOT)
        {
            WriteLog("RecordInfoBigImage LAST_SNAPSHOT  ");

            CopyDataToIMG(m_CameraResult->CIMG_LastSnapshot, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_BEGIN_CAPTURE)
        {
            WriteLog("RecordInfoBigImage BEGIN_CAPTURE  ");

            CopyDataToIMG(m_CameraResult->CIMG_BeginCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_BEST_CAPTURE)
        {
            WriteLog("RecordInfoBigImage BEST_CAPTURE  ");

            CopyDataToIMG(m_CameraResult->CIMG_BestCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_LAST_CAPTURE)
        {
            WriteLog("RecordInfoBigImage LAST_CAPTURE  ");

            CopyDataToIMG(m_CameraResult->CIMG_LastCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else
        {
            WriteLog("RecordInfoBigImage other Image, put it to  LAST_CAPTURE .");
            CopyDataToIMG(m_CameraResult->CIMG_LastCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
    }

    WriteFormatLog("RecordInfoBigImage -end- dwCarID = %lu", dwCarID);

    return 0;
}

int Camera6467_plate::RecordInfoSmallImage(DWORD dwCarID,
    WORD wWidth,
    WORD wHeight,
    PBYTE pbPicData,
    DWORD dwImgDataLen,
    DWORD dwRecordType,
    DWORD64 dw64TimeMS)
{
    SetResultComplete(false);
    if (NULL == m_CameraResult)
    {
        return -1;
    }
    WriteFormatLog( "RecordInfoSmallImage  -begin- dwCarID = %lu", dwCarID);

    if (dwCarID == m_dwLastCarID)
    {
        WriteLog("相同carID,丢弃该结果");
        return 0;
    }

    if (m_CameraResult->dwCarID == dwCarID)
    {
        CopyDataToIMG(m_CameraResult->CIMG_PlateImage, pbPicData, wWidth, wHeight, dwImgDataLen, 0x006);
    }
    else
    {
        WriteFormatLog("current car ID  %lu is not same wit result carID %lu.", dwCarID, m_CameraResult->dwCarID);
    }

    WriteFormatLog("RecordInfoSmallImage  -end- dwCarID = %lu", dwCarID);
    return 0;
}

int Camera6467_plate::RecordInfoBinaryImage(DWORD dwCarID,
    WORD wWidth,
    WORD wHeight,
    PBYTE pbPicData,
    DWORD dwImgDataLen,
    DWORD dwRecordType,
    DWORD64 dw64TimeMS)
{
    SetResultComplete(false);

    if (NULL == m_CameraResult)
    {
        return -1;
    }
    WriteFormatLog("RecordInfoBinaryImage -begin- dwCarID = %lu", dwCarID);


    if (dwCarID == m_dwLastCarID)
    {
        WriteLog("相同carID,丢弃该结果");
        return 0;
    }
    //int iBufferlength = 1024 * 1024;
    //if (m_pTempBin == NULL)
    //{
    //    m_pTempBin = new BYTE[1024 * 1024];
    //    memset(m_pTempBin, 0x00, iBufferlength);
    //}
    //if (m_pTempBin)
    //{
    //    memset(m_pTempBin, 0x00, iBufferlength);

    //    HRESULT hRet = HVAPIUTILS_BinImageToBitmapEx(pbPicData, m_pTempBin, &iBufferlength);
    //    if (hRet == S_OK)
    //    {
    //        if (m_Bin_IMG_Temp.pbImgData == NULL)
    //        {
    //            m_Bin_IMG_Temp.pbImgData = new BYTE[MAX_IMG_SIZE];
    //            memset(m_Bin_IMG_Temp.pbImgData, 0x00, MAX_IMG_SIZE);
    //        }
    //        if (m_Bin_IMG_Temp.pbImgData)
    //        {
    //            DWORD iDestLenth = MAX_IMG_SIZE;
    //            memset(m_Bin_IMG_Temp.pbImgData, 0x00, MAX_IMG_SIZE);
    //            WriteLog("bin, convert bmp to jpeg , begin .");
    //            bool bScale = Tool_Img_ScaleJpg(m_pTempBin,
    //                iBufferlength,
    //                m_Bin_IMG_Temp.pbImgData,
    //                &iDestLenth,
    //                wWidth,
    //                wHeight,
    //                90);
    //            if (bScale)
    //            {
    //                WriteLog("bin, convert bmp to jpeg success, begin copy.");
    //                CopyDataToIMG(m_CameraResult->CIMG_BinImage, m_Bin_IMG_Temp.pbImgData, wWidth, wHeight, iDestLenth, 0);
    //                WriteLog("bin, convert bmp to jpeg success, finish copy.");
    //            }
    //            else
    //            {
    //                WriteLog("bin, convert bmp to jpeg failed, use default.");
    //            }
    //        }
    //        else
    //        {
    //            WriteLog("m_Bin_IMG_Temp  is null.");
    //        }
    //    }
    //    else
    //    {
    //        WriteLog("HVAPIUTILS_BinImageToBitmapEx, failed, use default.");
    //        CopyDataToIMG(m_CameraResult->CIMG_BinImage, pbPicData, wWidth, wHeight, dwImgDataLen, 0);
    //    }
    //}
    //else
    {
        //WriteLog("m_pTempBin is NULL ,  use default.");
        CopyDataToIMG(m_CameraResult->CIMG_BinImage, pbPicData, wWidth, wHeight, dwImgDataLen, 0);
    }
    WriteFormatLog("RecordInfoBinaryImage -end- dwCarID = %lu", dwCarID);
    return 0;
}

int Camera6467_plate::DeviceJPEGStream(PBYTE pbImageData,
    DWORD dwImageDataLen,
    DWORD dwImageType,
    LPCSTR szImageExtInfo)
{
    static int iCout = 0;
    if (iCout++ > 100)
    {
        WriteLog("receive one jpeg frame.");
        iCout = 0;
    }

    m_csResult.lock();
    m_bJpegComplete = false;

    m_CIMG_StreamJPEG.dwImgSize = dwImageDataLen;
    m_CIMG_StreamJPEG.wImgWidth = 1920;
    m_CIMG_StreamJPEG.wImgHeight = 1080;
    if (NULL == m_CIMG_StreamJPEG.pbImgData)
    {
        m_CIMG_StreamJPEG.pbImgData = new unsigned char[MAX_IMG_SIZE];
        memset(m_CIMG_StreamJPEG.pbImgData, 0, MAX_IMG_SIZE);
    }
    if (m_CIMG_StreamJPEG.pbImgData)
    {
        memset(m_CIMG_StreamJPEG.pbImgData, 0, MAX_IMG_SIZE);
        memcpy(m_CIMG_StreamJPEG.pbImgData, pbImageData, dwImageDataLen);
        m_bJpegComplete = true;
    }
    m_csResult.unlock();

    SentJpegCallback(iCout);

    return 0;
}

bool Camera6467_plate::GetOneJpegImg(CameraIMG &destImg)
{
    WriteLog("GetOneJpegImg::begin.");
    bool bRet = false;

    if (!destImg.pbImgData)
    {
        WriteLog("GetOneJpegImg:: allocate memory.");
        destImg.pbImgData = new unsigned char[MAX_IMG_SIZE];
        memset(destImg.pbImgData, 0, MAX_IMG_SIZE);
        WriteLog("GetOneJpegImg:: allocate memory success.");
    }

    m_csResult.lock();
    if (m_bJpegComplete)
    {
        if (destImg.pbImgData)
        {
            memset(destImg.pbImgData, 0, MAX_IMG_SIZE);
            memcpy(destImg.pbImgData, m_CIMG_StreamJPEG.pbImgData, m_CIMG_StreamJPEG.dwImgSize);

            destImg.dwImgSize = m_CIMG_StreamJPEG.dwImgSize;
            destImg.wImgHeight = m_CIMG_StreamJPEG.wImgHeight;
            destImg.wImgWidth = m_CIMG_StreamJPEG.wImgWidth;
            bRet = true;
            WriteLog("GetOneJpegImg success.");
            m_bJpegComplete = false;
        }
        else
        {
            WriteLog("GetOneJpegImg:: allocate memory failed.");
        }
    }
    else
    {
        WriteLog("GetOneJpegImg the image is not ready.");
    }
    m_csResult.unlock();
    WriteLog("GetOneJpegImg:: end.");

    return bRet;
}

bool Camera6467_plate::GetResultComplete()
{
    bool bFinish = false;
    m_csResult.lock();
    bFinish = m_bResultComplete;
    m_csResult.unlock();
    return bFinish;
}

CameraResult* Camera6467_plate::GetOneResult()
{
    CameraResult* tempResult = NULL;
    //EnterCriticalSection(&m_csResult);	
    //if (m_ResultList.size() > 0)
    //{
    //	tempResult = m_ResultList.front();
    //	m_ResultList.pop_front();
    //}
    //LeaveCriticalSection(&m_csResult);

    if (GetResultComplete())
    {
        m_csResult.lock();
        tempResult = new CameraResult(*m_BufferResult);
        m_csResult.unlock();
    }
    return tempResult;
}

bool Camera6467_plate::SaveCIMG(CameraIMG& imgStruct,
                                struct tm& systime,
                                DWORD tickCount,
                                int iType,
                                std::string& outputPath)
{
#define BIN_IMG  0
#define SMALL_IMG 1
#define  BIG_IMG 2
#define  SP_IMG 3
    bool bRet = false;
    if (imgStruct.dwImgSize > 0)
    {
        char szImgPath[MAX_PATH] = { 0 };
        //sprintf_s(szImgPath, sizeof(szImgPath), "%s\\%04d-%02d-%02d-%u_s",
        //    m_chResultPath,
        //    systime.wYear,
        //    systime.wMonth,
        //    systime.wDay,
        //    tickCount);
		sprintf(szImgPath,  "%s\\%04d-%02d-%02d-%u_s",
			m_chResultPath,
            systime.tm_year,
            systime.tm_mon,
            systime.tm_yday,
			tickCount);

        switch (iType)
        {
        case BIN_IMG:
            //strcat_s(szImgPath, sizeof(szImgPath), ".bin");
			strcat(szImgPath, ".bin");
            break;
        case SMALL_IMG:
            //strcat_s(szImgPath, sizeof(szImgPath), "_small.jpg");
			strcat(szImgPath, "_small.jpg");
            break;
        case BIG_IMG:
            //strcat_s(szImgPath, sizeof(szImgPath), "_big1.jpg");
			strcat(szImgPath,  "_big1.jpg");
            break;
        case SP_IMG:
            //strcat_s(szImgPath, sizeof(szImgPath), "_big2.jpg");
			strcat(szImgPath, "_big2.jpg");
            break;
        default:
            //strcat_s(szImgPath, sizeof(szImgPath), "_big1.jpg");
			strcat(szImgPath,  "_big1.jpg");
            break;
        }
        outputPath = szImgPath;
        bRet = SaveImgToDisk(szImgPath, imgStruct.pbImgData, imgStruct.dwImgSize);
    }
    return bRet;
}

void Camera6467_plate::SaveResult()
{
    //std::shared_ptr<CameraResult> pTempResult;
    //while (!GetCheckThreadExit())
    //{
    //    if (m_lsResultList.empty())
    //    {
    //        Sleep(50);
    //        continue;
    //    }
    //    m_lsResultList.front(pTempResult);
    //    if (pTempResult)
    //    {            
    //        SerializationResultEx(pTempResult);
    //    }
    //    Sleep(10);
    //}
}

void Camera6467_plate::SetResultComplete(bool bfinish)
{
    m_csResult.lock();
    m_bResultComplete = bfinish;
    m_csResult.unlock();
}

int Camera6467_plate::GetTimeInterval()
{
    int iTimeInterval = 1;
    m_csFuncCallback.lock();
    iTimeInterval = m_iTimeInvl;
    m_csFuncCallback.unlock();
    return iTimeInterval;
}

void Camera6467_plate::CheckStatus()
{
    int iLastStatus = -1;
    long iLastTick = 0, iCurrentTick = 0;
    int iFirstConnctSuccess = -1;

    while (!GetCheckThreadExit())
    {
        usleep(50*1000);
        iCurrentTick = Tool_GetTickCount();
        int iTimeInterval = GetTimeInterval();
        if ((iCurrentTick - iLastTick) >= (iTimeInterval * 1000))
        {
            //UpLoadCameraStatus();
            int iStatus = GetCamStatus();
            if (iStatus != iLastStatus)
            {
                if (iStatus == 0)
                {
                    //if (iStatus != iLastStatus)
                    //{
                    //	pThis->SendConnetStateMsg(true);
                    //}
                    //SendConnetStateMsg(true);
                    WriteLog("设备连接正常.");
                    iFirstConnctSuccess = 0;
                }
                else
                {
                    //SendConnetStateMsg(false);
                    WriteLog("设备连接失败, 尝试重连");

                    if (iFirstConnctSuccess == -1)
                    {
                        //pThis->ConnectToCamera();
                    }
                }
            }
            iLastStatus = iStatus;

            iLastTick = iCurrentTick;
        }
    }
}

void Camera6467_plate::SetDeviceId( unsigned int uID )
{
    m_csFuncCallback.lock();
	m_iDeviceID = uID;
    m_csFuncCallback.unlock();
}

unsigned int Camera6467_plate::GetDeviceID()
{
	unsigned int iDevice = -1;
    m_csFuncCallback.lock();
	iDevice = m_iDeviceID;
    m_csFuncCallback.unlock();

	return iDevice;
}

void Camera6467_plate::SetMsgThreadId( DWORD id )
{
    m_csFuncCallback.lock();
	m_dwMsgThreadID = id;
    m_csFuncCallback.unlock();
}

DWORD Camera6467_plate::GetMsgThreadId()
{
	DWORD iValue = 0;
    m_csFuncCallback.lock();
	iValue = m_dwMsgThreadID;
    m_csFuncCallback.unlock();

	return iValue;
}

void Camera6467_plate::AnalysisAppendInfo( CameraResult* pResult )
{
	if (pResult == NULL)
	{
		return;
	}

    if (0 != pResult->dw64TimeMS)
    {
        //CTime tm(CamResult->dw64TimeMS / 1000);
       struct tm *timeNow = NULL;
       time_t          timeSecond = pResult->dw64TimeMS / 1000;  // Seconds
       timeNow = localtime(&timeSecond);

       sprintf(pResult->chPlateTime, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
           timeNow->tm_year,
           timeNow->tm_mon,
           timeNow->tm_yday,
           timeNow->tm_hour,
           timeNow->tm_min,
           timeNow->tm_sec,
           pResult->dw64TimeMS%1000);
    }
    else
    {
        struct tm timeNow;
        long iTimeInMilliseconds = 0;
        Tool_GetTime(&timeNow, &iTimeInMilliseconds);

        sprintf(pResult->chPlateTime, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
            timeNow.tm_year,
            timeNow.tm_mon,
            timeNow.tm_yday,
            timeNow.tm_hour,
            timeNow.tm_min,
            timeNow.tm_sec,
            iTimeInMilliseconds%1000);
    }

	pResult->iPlateColor = Tool_AnalysisPlateColorNo(pResult->chPlateNO);

	int iVehicleType = 0;
	int iVehicleClass = 0;

	if (strstr(pResult->chPlateNO, "黄")  )
	{//蓝色车牌的时候，给客车，黄色的时候，给货车
		if (strstr(pResult->chPlateNO, "学") )
		{
			iVehicleType = 1;
			iVehicleClass = 1;
		}
		else
		{
			iVehicleType = 2;
			iVehicleClass = 5;
		}
	}
	else
	{
		if (strstr(pResult->chPlateNO, "绿")  )
		{
			int len = 0;
			char szorigePlateNo[64] = {0};
			strcpy(szorigePlateNo, pResult->chPlateNO);
			len = strlen(szorigePlateNo);
			if(szorigePlateNo[len - 1] == 'D' || szorigePlateNo[len - 1] == 'F')
			{
				//iColor = (int)color_yellowGreen;
				//sprintf(pCtrl->m_Result.pchPlate, "%s%s", "黄绿双拼", tmpPlate);
				iVehicleType = 2;
				iVehicleClass = 5;
			}
			else
			{
				//iColor = (int)color_gradientGreen;
				//sprintf(pCtrl->m_Result.pchPlate, "%s%s", "渐变绿", tmpPlate);
				iVehicleType = 1;
				iVehicleClass = 1;
			}
		}
		else
		{
			iVehicleType = 1;
			iVehicleClass = 1;
		}
	}
	pResult->iVehTypeNo = iVehicleType;
	pResult->iVehSizeType =iVehicleClass;
	
	char chBuffer[256] = {0};
	int iBufLenght = sizeof(chBuffer);
    if (Tool_GetDataFromAppenedInfo( pResult->pcAppendInfo, "IsTruck", chBuffer, &iBufLenght ))
	{
		if (strstr(chBuffer, "客"))
		{
			pResult->iVehTypeNo = 1;
		}
		else if (strstr(chBuffer, "货"))
		{
			pResult->iVehTypeNo = 2;
		}
		else
		{
			pResult->iVehTypeNo = 0;
		}
	}
	
	memset(chBuffer, '\0', sizeof(chBuffer));
	iBufLenght = sizeof(chBuffer);
    if (Tool_GetDataFromAppenedInfo( pResult->pcAppendInfo, "VehicleType", chBuffer, &iBufLenght ))
	{
		int iVehType =  AnalysisVelchType(chBuffer);
		pResult->iVehTypeNo = iVehType  >= 10 ? 2 : 1;
		pResult->iVehSizeType = iVehType %10;
    }
}

bool Camera6467_plate::SetJpegCallbackOnly()
{
    if (NULL == m_hHvHandle)
    {
        WRITE_LOG("NULL == m_hHvHandle ");
        return false;
    }

    if (   (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)BaseCamera::JPEGStreamCallBack, this, 0, CALLBACK_TYPE_JPEG_FRAME, NULL) != S_OK)  )
    {
        WRITE_LOG("set JPEGCALLBACK failed ");
        return false;
    }
    else
    {
        WRITE_LOG(" Set JPEGCALLBACK success.");
        return true;
    }
}

void Camera6467_plate::SetJpegReceiveMsgCallback(void* pFunc,void* pUser)
{
    m_csResult.lock();

    m_pJpegCallbackFunc = pFunc;
    m_pUserData = pUser;

    m_csResult.unlock();
}

void Camera6467_plate::SentJpegCallback(int iValue)
{
    m_csResult.lock();

    if(m_pJpegCallbackFunc)
    {
        ((JPEG_Arrive)m_pJpegCallbackFunc)(m_pUserData, iValue);
    }
    m_csResult.unlock();
}
