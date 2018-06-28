#include "Log.hpp"

int MayKit::Logger::Log(const char *pcFormat, ...)
{
    va_list ap;
    va_start(ap, pcFormat);
    VectorWriteLog(pcFormat, ap);
    va_end(ap);
    return true;
}
    
int MayKit::Logger::VectorWriteLog(const char *pcFormat, va_list ap)
{
    if (!m_iLogActive)
        return 0;
        
    FILE  *pstFile;
    char acLogFileName[MAYKIT_LOG_PATH_MAX_SIZE];

    snprintf(acLogFileName, MAYKIT_LOG_PATH_MAX_SIZE, "%s.log", m_acLogBaseName);
    if ((pstFile = fopen(acLogFileName, "a+")) == NULL)
    {
        fprintf(stderr, "[%s]Fail to open log file %s\n", __FUNCTION__, acLogFileName);
        return -1;
    }

    char acTimeStr[MAYKIT_LOG_MAX_LINE + 1];
    GetCurDateTimeStr(acTimeStr, MAYKIT_LOG_MAX_LINE);
    
    fprintf(pstFile, "[%s] ", acTimeStr);
    vfprintf(pstFile, pcFormat, ap);
    fclose(pstFile);

    return ShiftFiles();
}

int MayKit::Logger::WriteLog(const char *pcFormat, ...)
{
    int iRetCode;
    va_list ap;
    
    va_start(ap, pcFormat);
    iRetCode = VectorWriteLog(pcFormat, ap);
    va_end(ap);

    return iRetCode;
}

void MayKit::Logger::GetCurDateTimeStr(char* pcTimeStr, int iStrLen)
{
    timeval tval;
    gettimeofday(&tval,NULL);
    struct tm curr;
    curr = *localtime(&tval.tv_sec);

    if (curr.tm_year > 50)
    {
        snprintf(pcTimeStr, iStrLen, "%04d-%02d-%02d %02d:%02d:%02d.%06d", 
            curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
            curr.tm_hour, curr.tm_min, curr.tm_sec,(int)tval.tv_usec);
    }
    else
    {
        snprintf(pcTimeStr, iStrLen, "%04d-%02d-%02d %02d:%02d:%02d.%06d",
            curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
            curr.tm_hour, curr.tm_min, curr.tm_sec,(int)tval.tv_usec);
    }
}

int MayKit::Logger::ShiftFiles()
{
    struct stat stStat;
    char acLogFileName[MAYKIT_LOG_PATH_MAX_SIZE];
    char acNewLogFileName[MAYKIT_LOG_PATH_MAX_SIZE];
    int i;

    snprintf(acLogFileName, MAYKIT_LOG_PATH_MAX_SIZE, "%s.log", m_acLogBaseName);
    
    if (stat(acLogFileName, &stStat) < 0)
    {
        return -1;
    }

    if (stStat.st_size < m_lMaxLogSize)
    {
        return 0;
    }

    snprintf(acLogFileName, MAYKIT_LOG_PATH_MAX_SIZE, "%s_%d.log", m_acLogBaseName, m_iMaxLogNum-1);
    if (!access(acLogFileName, F_OK))
    {
        if (remove(acLogFileName) < 0)
        {
            return -1;
        }
    }

    for (i = m_iMaxLogNum-2; i >= 0; i--)
    {
        if (!i)
            snprintf(acLogFileName, MAYKIT_LOG_PATH_MAX_SIZE, "%s.log", m_acLogBaseName);
        else
            snprintf(acLogFileName, MAYKIT_LOG_PATH_MAX_SIZE, "%s_%d.log", m_acLogBaseName, i);
            
        if (!access(acLogFileName, F_OK))
        {
            snprintf(acNewLogFileName, MAYKIT_LOG_PATH_MAX_SIZE, "%s_%d.log", m_acLogBaseName, i+1);
            if (rename(acLogFileName, acNewLogFileName) < 0)
            {
                return -1;
            }
        }
    }
    
    return 0;
}
