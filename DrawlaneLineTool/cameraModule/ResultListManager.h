#pragma once

#include "CameraModule/CameraResult.h"
#include <memory>
#include <deque>
//#include <mutex>
#include"utilityTool/tool_function.h"

#define MAX_VFR_LENGTH (10)

#ifdef WIN32_
class MYLocker
{
public:
    MYLocker(CRITICAL_SECTION* inHandle) : m_handle(inHandle)
    {
        if (NULL != m_handle)
        {
            EnterCriticalSection(m_handle);
        }
    }

    ~MYLocker()
    {
        if (NULL != m_handle)
        {
            LeaveCriticalSection(m_handle);
            m_handle = NULL;
        }
    }
private:
    CRITICAL_SECTION* m_handle;
};
#else

class MYLocker
{
public:
    MYLocker(ThreadMutex* inHandle) : m_handle(inHandle)
    {
        if (NULL != m_handle)
        {
            m_handle->lock();
        }
    }

    ~MYLocker()
    {
        if (NULL != m_handle)
        {
            m_handle->unlock();

            m_handle = NULL;
        }
    }
private:
    ThreadMutex* m_handle;
};

#endif


class ResultListManager
{
    //typedef std::shared_ptr<CameraResult> Result_Type;
	typedef CameraResult* Result_Type;
    typedef std::deque<Result_Type> list_Type;
public:
    ResultListManager();
    ~ResultListManager();

public:
    size_t size();
    bool empty();

    void front(Result_Type& result);
	void back(Result_Type& result);
    Result_Type GetOneByIndex(int iPosition);
    void pop_front();
    void pop_back();
    void push_back(Result_Type result);

    int GetPositionByPlateNo(const char* plateNo);
    void DeleteToPosition(int position);

    std::string GetAllPlateString();
    std::string GetAllVehTypeString();
    
    void ClearALLResult();

        
private:
    list_Type m_list;
    //std::mutex m_mtx;
//#ifdef WIN32
//    CRITICAL_SECTION m_hcs;
//#else
//    ThreadMutex m_hcs;
//#endif

    ThreadMutex m_hcs;

    friend class MYLocker;
};

