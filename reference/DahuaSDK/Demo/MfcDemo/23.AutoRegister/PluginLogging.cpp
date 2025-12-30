#include "stdafx.h"
#include "PluginLogging.h" // windows.h must not included before afx.h


#include <string.h>
#include <crtdbg.h>
#include <time.h>
#include <string>
#include <Windows.h>
#include <io.h>
#include <direct.h>

#if _MSC_VER >= 1400
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1 // change strcpy to strcpy_s, etc.
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1 // change strncpy to strncpy_s, etc.
#endif

CPluginLog* CPluginLog::s_pInstance = NULL;
DHMutex s_pInstanceLock;

//获得当前目录
void GetCurrentDir(char * pPathBuf, int nBufLen)
{
    GetModuleFileName(NULL, pPathBuf, nBufLen);
    char * pIndex = strrchr (pPathBuf, PSPEC_C);
    if (pIndex != NULL)
    {
        *pIndex = '\0';
    }
}

CPluginLog::CPluginLog()
{
	m_nFileMaxSize = PLUGIN_LOG_FILE_SIZE_MAX;
	m_nFileMaxNum = PLUGIN_LOG_FILE_NUM_MAX;
	
    // 获取当前程序目录下的路径
    memset(m_szFilePath, 0, sizeof(m_szFilePath));
    GetCurrentDir(m_szFilePath, sizeof(m_szFilePath)-1);
    strncat(m_szFilePath, "/", sizeof(m_szFilePath)-strlen(m_szFilePath)-1);
	strncat(m_szFilePath, PLUGIN_LOG_PATH, sizeof(m_szFilePath)-strlen(m_szFilePath)-1);

    memset(m_szFileName, 0, sizeof(m_szFileName));

	m_nFileCurrentNum = GetCurrentFileNum();

	m_fp = NULL;

	CreateLogDir();	
}
/*
CPluginLog::CPluginLog(int nFileSize, int nFileNum, char *szFilePath)
:m_nFileMaxSize(nFileSize), m_nFileMaxNum(nFileNum)
{
	m_nFileCurrentNum = 0;

	strncpy(m_szFilePath, szFilePath, sizeof(m_szFilePath)-1);
	strncpy(m_szFileName, "", sizeof(m_szFileName)-1);
	m_fp = NULL;

	InitializeCriticalSection(&m_csFile);

	CreateLogDir();	
	GetCurrentFileNum();
	CreateLogFile();
}
*/
CPluginLog::~CPluginLog()
{
	m_csFile.Lock();
	if (NULL != m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
	m_csFile.UnLock();
}

CPluginLog * CPluginLog::Instance()
{
	if (!s_pInstance)   // 加快获取实例
	{
        s_pInstanceLock.Lock(); // 防止多线程问题
        if (!s_pInstance) // 查看是否已经有实例
        {
		    s_pInstance = new CPluginLog();
        }
        s_pInstanceLock.UnLock();
	}
	return s_pInstance;
}

/*
void CPluginLog::StartUpLogger()
{
	_ASSERT(g_pPluginLogger == NULL);
	g_pPluginLogger = new CPluginLog();
}

void CPluginLog::DestoryLogger()
{
	if (g_pPluginLogger)
	{
		delete g_pPluginLogger;
		g_pPluginLogger = NULL;
	}
}
*/

unsigned CPluginLog::GetCurrentFileNum()
{
	WIN32_FIND_DATA stFindFileData;
	
	std::string strWildcardLog(m_szFilePath);
	strWildcardLog += "\\"PLUGIN_LOG_NAME"*.log";
	
	HANDLE hFile = FindFirstFile(strWildcardLog.c_str(), &stFindFileData);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		FindClose(hFile);
		return 0;
	}
	
	unsigned nCount = 1;
	while (FindNextFile(hFile, &stFindFileData))
	{
		nCount++;
	}
	
	FindClose(hFile);
	
	return nCount;
}

int CPluginLog::CreateLogFile()
{
	int nRet = 0;
	ULARGE_INTEGER uiFreeBytesAvailable = {0};
	ULARGE_INTEGER uiTotalNumberOfBytes = {0};
	ULARGE_INTEGER uiTotalNumberOfFreeBytes = {0};

	char szDisk[4] = {0, ':', '\\', 0};
	char* pszDisk = NULL;
	if(strchr(m_szFilePath, ':') == m_szFilePath + 1)
	{
		szDisk[0] = m_szFilePath[0];
		pszDisk = szDisk;
	}

	//get disk free space
	nRet = GetDiskFreeSpaceEx(pszDisk, &uiFreeBytesAvailable, &uiTotalNumberOfBytes, &uiTotalNumberOfFreeBytes);
	if (0 == nRet)
	{
		return false;
	}

	//no more free disk space
	if (uiTotalNumberOfFreeBytes.HighPart < 1 && uiTotalNumberOfFreeBytes.LowPart < m_nFileMaxSize)
	{
		//if current log file exist, find the oldest one and override it.
		if (m_nFileCurrentNum > 0)
		{
			FindAndDeleteOldFile();
			m_nFileCurrentNum--;
		}

		return false;
	}

	while (m_nFileMaxNum <= m_nFileCurrentNum)
	{
		FindAndDeleteOldFile();
		m_nFileCurrentNum--;
	}

	const time_t t = time(NULL);
	struct tm* ptm = localtime(&t);
	strncpy(m_szFileName, m_szFilePath, sizeof(m_szFileName)-1);
	sprintf(m_szFileName+strlen(m_szFileName), "\\"PLUGIN_LOG_NAME"_%04d-%02d-%02d_%02d-%02d-%02d.log",
                                1900+ptm->tm_year, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	
	if (NULL != m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	m_fp = fopen(m_szFileName, "a+");

	if (NULL == m_fp)
	{
		return false;
	}

	m_nFileCurrentNum++;

	return true;
}

int CPluginLog::CreateLogDir()
{
	if ('0' == m_szFilePath[0])
	{
		return false;
	}

	//if last character of path is '/' or '\',delete it
	char& cLastCharInPath = m_szFilePath[strlen(m_szFilePath)-1];
	if ('\\' == cLastCharInPath || '/' == cLastCharInPath)
	{
		cLastCharInPath = 0;
	}

	using namespace std;
	string strPathTmp(m_szFilePath);

	//find current exist directory
	string::size_type nIndex = string::npos;
	WIN32_FIND_DATA stFindFileData;
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
 	while (INVALID_HANDLE_VALUE == (hFindFile = FindFirstFile(strPathTmp.c_str(), &stFindFileData)))
	{
		nIndex = strPathTmp.rfind('\\');

		if (string::npos != nIndex)
		{
			strPathTmp.erase(nIndex, strPathTmp.length()-nIndex);

			//reach the disk symbol
			if (':' == strPathTmp[nIndex-1])
			{
				break;
			}
		}
		//reach the root directory
		else
		{
			strPathTmp.erase();
			break;
		}
	}

	if (INVALID_HANDLE_VALUE != hFindFile)
	{
		FindClose(hFindFile);
	}

	//create the directory from the exist directory
	const string strPath(m_szFilePath);
	while (strPathTmp.length() != strPath.length())
	{
		if (strPath.find("\\", strPathTmp.length()+1) == string::npos)
		{
			strPathTmp = strPath;
		}
		else
		{
			strPathTmp = strPath.substr(0, nIndex);
		}

		if (!CreateDirectory(strPathTmp.c_str(), NULL))
		{
			if (ERROR_ALREADY_EXISTS == GetLastError())
			{
				return true;
			}
			return false;
		}
	}

	return true;
}

int CPluginLog::FindAndDeleteOldFile()
{
	WIN32_FIND_DATA stFindFileData;
	WIN32_FIND_DATA stMinFindFileData;

	std::string strWildcardLog(m_szFilePath);
	strWildcardLog += "\\"PLUGIN_LOG_NAME"*.log";

	HANDLE hFile = FindFirstFile(strWildcardLog.c_str(), &stMinFindFileData);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return false;
	}
 
	while (FindNextFile(hFile, &stFindFileData))
	{
		if (stFindFileData.ftCreationTime.dwHighDateTime < stMinFindFileData.ftCreationTime.dwHighDateTime 
			|| (stFindFileData.ftCreationTime.dwHighDateTime == stMinFindFileData.ftCreationTime.dwHighDateTime 
				&& stFindFileData.ftCreationTime.dwLowDateTime < stMinFindFileData.ftCreationTime.dwLowDateTime))
		{
			memcpy(&stMinFindFileData, &stFindFileData, sizeof(WIN32_FIND_DATA));
		}
	}
	
	FindClose(hFile);

	std::string strFileName(m_szFilePath);
	strFileName += '\\';
	strFileName += stMinFindFileData.cFileName;
	int nRet = DeleteFile(strFileName.c_str());

	return nRet;
}

void CPluginLog::WritePluginLog(PLUGIN_LOG_TYPE_E emLogType, const char* szFile, const char* szFunction, const long nLine, 
								const char *szFormat, 
#if _MSC_VER >= 1400
								...)
#else
								va_list args)
#endif
{
	//no need logging
	if (emLogType > PLUGIN_LOG_LEVEL)
	{
		return;
	}

	char szLogType[16] = {0};
	switch (emLogType)
	{
	case PLUGIN_LOG_TYPE_OPERATE:
		{
			strncpy(szLogType, "[Operate]", 15);
			break;
		}
	case PLUGIN_LOG_TYPE_ERROR:
		{
			strncpy(szLogType, "[Error]  ", 15);
			break;
		}
	case PLUGIN_LOG_TYPE_WARNING:
		{
			strncpy(szLogType, "[Warning]", 15);
			break;
		}
	case PLUGIN_LOG_TYPE_DEBUG:
		{
			strncpy(szLogType, "[Debug]  ", 15);
			break;
		}
	default:
		return;
	}

	const time_t t = time(NULL);
	char* szTime = ctime(&t);
	szTime[strlen(szTime)-1] = 0;

	char szLogInfo[PLUGIN_LOG_SIZE] = {0};
	
    // 假如报错文件中有路径，则去掉路径，否则直接使用文件名
    if (strrchr(szFile, '\\') != NULL)
    {
	    _snprintf(szLogInfo, sizeof(szLogInfo) - 1 ,"%s: [%s], [%s], [%s], [%d]\t", szLogType, szTime, strrchr(szFile, '\\')+1, szFunction, nLine);
    }
    else
    {
        _snprintf(szLogInfo, sizeof(szLogInfo) - 1 , "%s: [%s], [%s], [%s], [%d]\t", szLogType, szTime, szFile, szFunction, nLine);
    }

	size_t nLogLen = strlen(szLogInfo);
#if _MSC_VER >= 1400
	va_list args;
	va_start(args, szFormat);
#endif
	_vsnprintf(szLogInfo+nLogLen, PLUGIN_LOG_SIZE-nLogLen, szFormat, args);
	_snprintf(szLogInfo+strlen(szLogInfo), PLUGIN_LOG_SIZE-strlen(szLogInfo), "%s", "\n");
#if _MSC_VER >= 1400
	va_end(args);
#endif
	WriteLogFile(szLogInfo);

}

void CPluginLog::WriteLogFile(char* szLogInfo)
{
	m_csFile.Lock();
	if (!m_fp)
	{
		CreateLogFile();
	}

    if (NULL != m_fp)
    {
	    fseek(m_fp, 0L, SEEK_END);
	    int nfileLen = ftell(m_fp);
	    if (nfileLen + strlen(szLogInfo) > m_nFileMaxSize)
	    {
		    CreateLogFile();
	    }

	    fwrite(szLogInfo, 1, strlen(szLogInfo), m_fp);
	    fflush(m_fp);
    }

	m_csFile.UnLock();
}
