#ifndef CAMERA6467_VFR_H
#define CAMERA6467_VFR_H
//#include "G:\GitHub\VCR_YN\VCR\VCR\Camera\BaseCamera.h"
#include "CameraModule/BaseCamera.h"
#include"cameraModule/Camera6467_plate.h"
#include"cameraModule/CameraResult.h"
#include "ResultListManager.h"

class Camera6467_VFR :
    public BaseCamera
{
public:
    Camera6467_VFR();
    virtual ~Camera6467_VFR();

private:
    void ReadConfig();
    void AnalysisAppendXML(CameraResult* CamResult);
    int AnalysisVelchType(const char* vehType);
	int GetAlexType(int ivalue);
	int GetAlexGroupCount(const char* AxleType);
	int AnalysisVelchAxleType(const char* AxleType, int  iAxleGroupCount);
    int AnalysisBusVehAxleType(int WheelCount, const char* plateNumber);
    
    //************************************
    // Method:        CheckIfSuperLength
    // Describe:       查看当前车是否为超长车，如果是，则后续的逻辑需清理车型队列
    // FullName:      Camera6467_VFR::CheckIfSuperLength
    // Access:          private 
    // Returns:        bool
    // Returns Describe:
    // Parameter:    CameraResult * CamResult
    //************************************
    bool CheckIfSuperLength(CameraResult* CamResult);

    //************************************
    // Method:        CheckIfBackUpVehicle
    // Describe:        检查该结果是否为倒车结果
    // FullName:      Camera6467_VFR::CheckIfBackUpVehicle
    // Access:          private 
    // Returns:        bool
    // Returns Describe:
    // Parameter:    CameraResult * CamResult
    //************************************
    bool CheckIfBackUpVehicle(CameraResult* CamResult);

public:
    //获取发送状态回调的时间间隔
    int GetTimeInterval();
    void SetDisConnectCallback(void* funcDisc, void* pUser);
    void SetReConnectCallback(void* funcReco, void* pUser);
    void SetConnectStatus_Callback(void* func, void* pUser, int TimeInterval);
    void SendConnetStateMsg(bool isConnect);

    void SetMsgHandleAngMsg(void* handle, int msg);

    void SetCheckThreadExit(bool  bExit);
    bool GetCheckThreadExit();

    bool OpenPlateCamera(const char* ipAddress);
    CameraResult* GetFrontResult();
    CameraResult*  GetFrontResultByPlateNo(const char* plateNo);
    CameraResult*  GetFrontResultByPosition(int position);
    std::string GetAllVehTypeList();
    void DeleteFrontResult(const char* plateNo);
    void ClearALLResult();
    size_t GetResultListSize();

    void SetUdpServerPort(int port);

public:

    virtual int RecordInfoBegin(unsigned long dwCarID);

    virtual int RecordInfoEnd(unsigned long dwCarID);

    virtual int RecordInfoPlate(unsigned long dwCarID,
        const char* pcPlateNo,
        const char* pcAppendInfo,
        unsigned long dwRecordType,
        unsigned long long dw64TimeMS);

    virtual int RecordInfoBigImage(unsigned long dwCarID,
        unsigned short  wImgType,
        unsigned short  wWidth,
        unsigned short  wHeight,
        unsigned char* pbPicData,
        unsigned long dwImgDataLen,
        unsigned long dwRecordType,
        unsigned long long dw64TimeMS);

    virtual int RecordInfoSmallImage(unsigned long dwCarID,
        unsigned short wWidth,
        unsigned short wHeight,
        unsigned char* pbPicData,
        unsigned long dwImgDataLen,
        unsigned long dwRecordType,
        unsigned long long dw64TimeMS);

    virtual int RecordInfoBinaryImage(unsigned long dwCarID,
        unsigned short wWidth,
        unsigned short wHeight,
        unsigned char* pbPicData,
        unsigned long dwImgDataLen,
        unsigned long dwRecordType,
        unsigned long long dw64TimeMS);

    virtual int DeviceJPEGStream(
        unsigned char* pbImageData,
        unsigned long dwImageDataLen,
        unsigned long dwImageType,
        const char* szImageExtInfo);

    virtual void CheckStatus();

private:
    unsigned long m_dwLastCarID;
    int m_iTimeInvl;
    int m_iSuperLenth;
    int m_iResultTimeOut;
    int m_iResultMsg;
    
    bool m_bStatusCheckThreadExit;
    bool m_bJpegComplete;

    //HANDLE m_hStatusCheckThread;			//状态检测线程

    CameraIMG m_Small_IMG_Temp;
    CameraIMG m_CIMG_StreamJPEG;

    ThreadMutex m_csResult;

    //std::shared_ptr<Camera6467_plate> m_Camera_Plate;
	Camera6467_plate* m_Camera_Plate;

    CameraResult* m_pResult;
    ResultListManager m_resultList;

    C_UdpClient* m_pUdpClient;
};

#endif
