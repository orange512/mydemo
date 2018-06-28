/*
 * 循环日志.
 * Written by nonysun at 2015/03/31.
 */
 
#ifndef __MAYKIT_LOG_HEADER__
#define __MAYKIT_LOG_HEADER__

#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

namespace MayKit
{

#define MAYKIT_LOG_PATH_MAX_SIZE 256
#define MAYKIT_LOG_MAX_LINE 512

    class Logger
    {
    protected:
        char m_acLogBaseName[MAYKIT_LOG_PATH_MAX_SIZE + 1];
        long m_lMaxLogSize;
        int m_iMaxLogNum;
        int m_iLogActive;
        
    public:
        Logger(const char *pcLogBaseName, long lMaxLogSize, int iMaxLogNum)
        {
	        memset(m_acLogBaseName, 0, sizeof(m_acLogBaseName));
	        strncpy(m_acLogBaseName, pcLogBaseName, MAYKIT_LOG_PATH_MAX_SIZE);
	        m_lMaxLogSize = lMaxLogSize;
	        m_iMaxLogNum = iMaxLogNum;
            m_iLogActive = 1;
        }
        
        virtual ~Logger()
        {
        }
        
        virtual int Log(const char *pcFormat, ...);
        // 打开Log
        void ActiveLog()
        {
            m_iLogActive = 1;
        }
        // 关闭Log
        void DeActiveLog()
        {
            m_iLogActive = 0;
        }
        
    protected:
        int VectorWriteLog(const char *pcFormat, va_list ap);
        int WriteLog(const char *pcFormat, ...);
        void GetCurDateTimeStr(char* pcTimeStr, int iStrLen);
        int ShiftFiles();
    };
}

#endif // __MAYKIT_LOG_HEADER__
