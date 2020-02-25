//#include "stdafx.h"
#include "ResultListManager.h"
#include "utilityTool/tool_function.h"
//#include "utilityTool/log4z.h"
#include <new>
#include <exception>
#include<cstdlib>


ResultListManager::ResultListManager()
{
    //InitializeCriticalSection(&m_hcs);
}

ResultListManager::~ResultListManager()
{
    //在vs2015之前的版本,在dll中 使用std::mutex 会有bug
    ClearALLResult();
    //DeleteCriticalSection(&m_hcs);
}

size_t ResultListManager::size()
{
    //std::unique_lock<std::mutex> locker(m_mtx);
    MYLocker locker(&m_hcs);
    return m_list.size();   
}

bool ResultListManager::empty()
{
    //std::unique_lock<std::mutex> locker(m_mtx);
    MYLocker locker(&m_hcs);

    return m_list.empty();
}

void ResultListManager::front(Result_Type& result)
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);
        Result_Type pValue;
		pValue = NULL;
        if (!m_list.empty())
        {
            pValue = m_list.front();
            //m_list.pop_front();
        }
		result = pValue;
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::front, bad_exception, error msg = %s", e.what());
        //return std::make_shared<CameraResult>();
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::front, bad_alloc, error msg = %s", e.what());
        //return std::make_shared<CameraResult>();
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::front, exception, error msg = %s.", e.what());
        //return std::make_shared<CameraResult>();
    }
    catch (void*)
    {
        printf("ResultListManager::front,  void* exception");
        //return std::make_shared<CameraResult>();
    }
    catch (...)
    {
        printf("ResultListManager::front,  unknown exception");
       // return std::make_shared<CameraResult>();
    }
}

ResultListManager::Result_Type ResultListManager::GetOneByIndex(int iPosition)
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);
        Result_Type tmpValue;
		tmpValue = NULL;
        if (m_list.empty())
        {
            return tmpValue;
        }

        int iPos = 0;
        for (list_Type::iterator it = m_list.begin(); it != m_list.end(); it++)
        {
            if (iPos == iPosition)
            {
                tmpValue = *it;
                break;
            }
            iPos++;
        }
        return tmpValue;
    }
    catch (std::bad_exception& e)
    {
        printf("GetOneByIndex, bad_exception, error msg = %s", e.what());
        //return std::make_shared<CameraResult>();
		return NULL;
    }
    catch (std::bad_alloc& e)
    {
        printf("GetOneByIndex, bad_alloc, error msg = %s", e.what());
        //return std::make_shared<CameraResult>();
		return NULL;
    }
    catch (std::exception& e)
    {
        printf("GetOneByIndex, exception, error msg = %s.", e.what());
        //return std::make_shared<CameraResult>();
		return NULL;
    }
    catch (void*)
    {
        printf("ResultListManager::GetOneByIndex,index = %d,   void* exception", iPosition);
        //return std::make_shared<CameraResult>();
		return NULL;
    }
    catch (...)
    {
        printf("ResultListManager::GetOneByIndex,index = %d,   unknown exception", iPosition);
        //return std::make_shared<CameraResult>();
		return NULL;
    }
}

void ResultListManager::pop_front()
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);

        if (!m_list.empty())
        {
            m_list.pop_front();
        }
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::pop_front, bad_exception, error msg = %s", e.what());
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::pop_front, bad_alloc, error msg = %s", e.what());
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::pop_front, exception, error msg = %s.", e.what());
    }
    catch (void*)
    {
        printf("ResultListManager::pop_front,  void* exception");
    }
    catch (...)
    {
        printf("ResultListManager::pop_front,  unknown exception");
    }
}

void ResultListManager::pop_back()
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);

        if (!m_list.empty())
        {
            m_list.pop_back();
        }
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::pop_back, bad_exception, error msg = %s", e.what());
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::pop_back, bad_alloc, error msg = %s", e.what());
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::pop_back, exception, error msg = %s.", e.what());
    }
    catch (void*)
    {
        printf("ResultListManager::pop_back,  void* exception");
    }
    catch (...)
    {
        printf("ResultListManager::pop_back,  unknown exception");
    }
}

void ResultListManager::push_back(Result_Type result)
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        //MYLocker locker(&m_hcs);
        if (result != NULL)
        {
            m_list.push_back(result);
        }
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::push_back, bad_exception, error msg = %s", e.what());
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::push_back, bad_alloc, error msg = %s", e.what());
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::push_back, exception, error msg = %s.", e.what());
    }
    catch (void*)
    {
        printf("ResultListManager::push_back,  void* exception");
    }
    catch (...)
    {
        printf("ResultListManager::push_back,  unknown exception");
    }
}

int ResultListManager::GetPositionByPlateNo(const char* plateNo)
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);
        int iRet = -1;
        if (m_list.empty() || NULL == plateNo)
        {
            return iRet;
        }
        int iPos = 0;
        std::string strSrcPlate(plateNo);
        for (list_Type::iterator it = m_list.begin(); it != m_list.end(); it++)
        {
            Result_Type value = *it;
            std::string strDestPlate(value->chPlateNO);
            if (std::string::npos != strSrcPlate.find(strDestPlate))
            {
                iRet = iPos;
                break;
            }
            else if (Tool_DimCompare(strSrcPlate.c_str(), strDestPlate.c_str()))
            {
                iRet = iPos;
                break;
            }
            else
            {
                iPos++;
            }
        }
        return iRet;
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::GetPositionByPlateNo, bad_exception, error msg = %s", e.what());
        return -1;
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::GetPositionByPlateNo, bad_alloc, error msg = %s", e.what());
        return -1;
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::GetPositionByPlateNo, exception, error msg = %s.", e.what());
        return -1;
    }
    catch (void*)
    {
        printf("ResultListManager::GetPositionByPlateNo,  void* exception");
        return -1;
    }
    catch (...)
    {
        printf("ResultListManager::GetPositionByPlateNo,  unknown exception");
        return -1;
    }
}

void ResultListManager::DeleteToPosition(int position)
{
    try
    {
        if (position < 0)
        {
            return;
        }
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);
        size_t iPosition = (position >= m_list.size()) ? (m_list.size() - 1) : position;
        if (!m_list.empty())
        {
            for (int i = 0; i <= iPosition; i++)
            {
				Result_Type pValue = m_list.front();	
				if (pValue  != NULL)
				{
					delete pValue;
					pValue = NULL;
				}

                m_list.pop_front();
            }
        }
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::DeleteToPosition, bad_exception, error msg = %s", e.what());
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::DeleteToPosition, bad_alloc, error msg = %s", e.what());
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::DeleteToPosition, exception, error msg = %s.", e.what());
    }
    catch (void*)
    {
        printf("ResultListManager::DeleteToPosition,  void* exception");
    }
    catch (...)
    {
        printf("ResultListManager::DeleteToPosition,  unknown exception");
    }
}

std::string ResultListManager::GetAllPlateString()
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);
        std::string strPlateListString;
        if (m_list.empty())
        {
            return strPlateListString;
        }
        else
        {
            for (list_Type::iterator it = m_list.begin(); it != m_list.end(); it++)
            {
                Result_Type pValue = *it;
                strPlateListString.append(pValue->chPlateNO);
                strPlateListString.append("\n");
            }
            return strPlateListString;
        }
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::GetAllPlateString, bad_exception, error msg = %s", e.what());
        return std::string();
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::GetAllPlateString, bad_alloc, error msg = %s", e.what());
        return std::string();
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::GetAllPlateString, exception, error msg = %s.", e.what());
        return std::string();
    }
    catch (void*)
    {
        printf("ResultListManager::GetAllPlateString,  void* exception");
        return std::string();
    }
    catch (...)
    {
        printf("ResultListManager::GetAllPlateString,  unknown exception");
        return std::string();
    }
}

std::string ResultListManager::GetAllVehTypeString()
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);
        std::string strVehTypeListString;
        char chTemp[64] = {0};
        if (m_list.empty())
        {
            return strVehTypeListString;
        }
        else
        {
            for (list_Type::iterator it = m_list.begin(); it != m_list.end(); it++)
            {
                Result_Type pValue = *it;
                memset(chTemp, '\0', sizeof(chTemp));
                sprintf(chTemp, "%d", pValue->iVehTypeNo);
				strVehTypeListString.append(chTemp);
                strVehTypeListString.append(",");
            }
            return strVehTypeListString;
        }
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::GetAllVehTypeString, bad_exception, error msg = %s", e.what());
        return std::string();
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::GetAllVehTypeString, bad_alloc, error msg = %s", e.what());
        return std::string();
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::GetAllVehTypeString, exception, error msg = %s.", e.what());
        return std::string();
    }
    catch (void*)
    {
        printf("ResultListManager::GetAllVehTypeString,  void* exception");
        return std::string();
    }
    catch (...)
    {
        printf("ResultListManager::GetAllVehTypeString,  unknown exception");
        return std::string();
    }
}

void ResultListManager::ClearALLResult()
{
    try
    {
        //std::unique_lock<std::mutex> locker(m_mtx);
        MYLocker locker(&m_hcs);
		while(!m_list.empty())
		{
			Result_Type pValue = m_list.front();	
			if (pValue  != NULL)
			{
				delete pValue;
				pValue = NULL;
			}

			m_list.pop_front();
		}
        //if (!m_list.empty())
        //{
        //    m_list.clear();
        //}
    }
    catch (std::bad_exception& e)
    {
        printf("ResultListManager::DeleteToPosition, ClearALL, error msg = %s", e.what());
    }
    catch (std::bad_alloc& e)
    {
        printf("ResultListManager::DeleteToPosition, ClearALL, error msg = %s", e.what());
    }
    catch (std::exception& e)
    {
        printf("ResultListManager::DeleteToPosition, ClearALL, error msg = %s.", e.what());
    }
    catch (void*)
    {
        printf("ResultListManager::ClearALL,  void* exception");
    }
    catch (...)
    {
        printf("ResultListManager::ClearALL,  unknown exception");
    }
}

void ResultListManager::back( Result_Type& result )
{
	try
	{
		//std::unique_lock<std::mutex> locker(m_mtx);
		MYLocker locker(&m_hcs);
		Result_Type pValue;
		pValue = NULL;
		if (!m_list.empty())
		{
			pValue = m_list.back();
			//m_list.pop_front();
		}
		result = pValue;
	}
	catch (std::bad_exception& e)
	{
		printf("ResultListManager::back, bad_exception, error msg = %s", e.what());
		//return std::make_shared<CameraResult>();
	}
	catch (std::bad_alloc& e)
	{
		printf("ResultListManager::back, bad_alloc, error msg = %s", e.what());
		//return std::make_shared<CameraResult>();
	}
	catch (std::exception& e)
	{
		printf("ResultListManager::back, exception, error msg = %s.", e.what());
		//return std::make_shared<CameraResult>();
	}
	catch (void*)
	{
		printf("ResultListManager::back,  void* exception");
		//return std::make_shared<CameraResult>();
	}
	catch (...)
	{
		printf("ResultListManager::back,  unknown exception");
		// return std::make_shared<CameraResult>();
	}
}

