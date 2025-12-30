#ifndef _PLUGINLOG_H_
#define _PLUGINLOG_H_

#include <stdarg.h>
#include <stdio.h>
#include "dhmutex.h"

typedef enum
{
	PLUGIN_LOG_LEVEL_NOLOG = 0,
	PLUGIN_LOG_LEVEL_OPERATE = 1,
	PLUGIN_LOG_LEVEL_ERROR = 2,
	PLUGIN_LOG_LEVEL_WARNING = 3,
	PLUGIN_LOG_LEVEL_DEBUG = 4,
	PLUGIN_LOG_LEVEL_MAX
}PLUGIN_LOG_LEVEL_E;

typedef enum
{
	PLUGIN_LOG_TYPE_OPERATE = 1,
	PLUGIN_LOG_TYPE_ERROR = 2,
	PLUGIN_LOG_TYPE_WARNING = 3,
	PLUGIN_LOG_TYPE_DEBUG = 4,
	PLUGIN_LOG_TYPE_MAX
}PLUGIN_LOG_TYPE_E;

#ifndef PLUGIN_LOG_LEVEL
#ifdef _DEBUG
#define PLUGIN_LOG_LEVEL				PLUGIN_LOG_LEVEL_DEBUG
#else
#define PLUGIN_LOG_LEVEL				PLUGIN_LOG_LEVEL_WARNING
#endif
#endif

#ifndef PLUGIN_LOG_FILE_SIZE_MAX
#define PLUGIN_LOG_FILE_SIZE_MAX		(1024*1024)
#endif

#ifndef PLUGIN_LOG_FILE_NUM_MAX
#define PLUGIN_LOG_FILE_NUM_MAX			5
#endif

#ifndef PLUGIN_LOG_PATH
#define PLUGIN_LOG_PATH					"sdk_log"
#endif

#ifndef PLUGIN_LOG_NAME
#define PLUGIN_LOG_NAME					"Info"
#endif
#ifndef PLUGIN_LOG_SIZE
#define PLUGIN_LOG_SIZE					1024
#endif

#if defined(WIN32) || defined(WIN64)
	#define PSPEC_S            "\\" //path 分隔符串
	#define PSPEC_C            '\\' //path 分割字符

	#define REVP_S             "/"  //path分隔符相反的串
	#define REVP_C             '/'  //path分隔符相反字符
#else
	#define PSPEC_S            "/"  //path 分隔符串
	#define PSPEC_C            '/'  //path 分割字符

	#define REVP_S             "\\" //path分隔符相反的串
	#define REVP_C             '\\' //path分隔符相反字符
#endif

void GetCurrentDir(char * pPathBuf, int nBufLen);

class CPluginLog
{
public:

	~CPluginLog();

	static CPluginLog *Instance();

	void WritePluginLog(PLUGIN_LOG_TYPE_E emLogType, const char* szFile, const char* szFunction, const long nLine, 
		const char *szFormat, 
#if _MSC_VER >= 1400
		...);
#else
		va_list args);
#endif
	void WriteLogFile(char* szLogInfo);

	//static void StartUpLogger();

	//static void DestoryLogger();
private:
	unsigned GetCurrentFileNum();

	int CreateLogFile();
	
	int CreateLogDir();

	int FindAndDeleteOldFile();

	unsigned int m_nFileMaxSize;		//max size of every log file
	unsigned int m_nFileMaxNum;			//max number of log file
	char m_szFilePath[256];				//path of log file
	char m_szFileName[512];
	unsigned int m_nFileCurrentNum;		//number of current log file

	FILE *m_fp;							//current file handler
	DHMutex m_csFile;				//file source's lock

	static CPluginLog* s_pInstance;
	// prevent construct
	CPluginLog();
	// CPluginLog(int nFileMaxSize, int nFileMaxNum, char *szFilePath);
	CPluginLog(CPluginLog&);
};

#if _MSC_VER >= 1400
#define __PRINT_(LOG_TYPE, szFormat, ...) CPluginLog::Instance()->WritePluginLog(PLUGIN_LOG_TYPE_##LOG_TYPE, __FILE__, __FUNCTION__, __LINE__, szFormat, __VA_ARGS__)

#define PRINT_OPERATE(szFormat, ...) __PRINT_(OPERATE, szFormat, __VA_ARGS__)
#define PRINT_ERROR(szFormat, ...) __PRINT_(ERROR, szFormat, __VA_ARGS__)
#define PRINT_WARNING(szFormat, ...) __PRINT_(WARNING, szFormat, __VA_ARGS__)
#define PRINT_DEBUG(szFormat, ...) __PRINT_(DEBUG, szFormat, __VA_ARGS__)

#else
// LogWriter - helper class to work around variadic macro
class LogWriter
{
public:
	LogWriter(const PLUGIN_LOG_TYPE_E _emType, const char* _szFile, const char* _szFunction, const int _nLine)
		: emType(_emType), szFile(_szFile), szFunction(_szFunction), nLine(_nLine)
	{
	}
	void operator()(const char* szFormat, ...)
	{
		va_list ap;
		va_start(ap, szFormat);
		CPluginLog::Instance()->WritePluginLog(emType, szFile, szFunction, nLine, szFormat, ap);
		va_end(ap);
	}

private:
	const PLUGIN_LOG_TYPE_E emType;
	const char* szFile;
	const char* szFunction;
	const int nLine;
};
// create a function object for every PRINT_X call
#define __PRINT_(LOG_TYPE) LogWriter(PLUGIN_LOG_TYPE_##LOG_TYPE, __FILE__, "", __LINE__)

#define PRINT_OPERATE __PRINT_(OPERATE)
#define PRINT_ERROR __PRINT_(ERROR)
#define PRINT_WARNING __PRINT_(WARNING)
#define PRINT_DEBUG __PRINT_(DEBUG)
#endif

#endif		//_PLUGINLOG_H_


