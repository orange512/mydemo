/*
 * 非常简单的类ini配置文件解析.
 * 现在只采用最简单的线性搜索.
 * key and value最长各为256.
 * TODO: 红黑树方式搜索.
 * Written by nonysun at 2015/03/31.
 */
 
#ifndef __MAYKIT_INIPARSER_HEADER__
#define __MAYKIT_INIPARSER_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

namespace MayKit
{

#define MAYKIT_INI_ITEM_KEY_MAX_LEN 512
#define MAYKIT_INI_ITEM_VAL_MAX_LEN 512
#define MAYKIT_INI_ITEM_MAX_COUNT 2048
#define MAYKIT_INI_FILE_PATH_MAX 256

// ConfigItem的类型
#define MAYKIT_CCONFIG_ITEM_TYPE_UNKNOWN 0
// 有符号整型
#define MAYKIT_CCONFIG_ITEM_TYPE_INT 1
// 无符号整型
#define MAYKIT_CCONFIG_ITEM_TYPE_UINT 2
// 字符串
#define MAYKIT_CCONFIG_ITEM_TYPE_STRING 3
// 以竖线分隔的列表类型
#define MAYKIT_CCONFIG_ITEM_TYPE_LIST 4

    typedef struct _ConfigItem
    {
        const char *m_pcConfigItemName;
        void *m_pvValSpace;
        int m_iType;
    } ConfigItem;
    
    // 解析过程用到的条目结构
    typedef struct _IniParserItem
    {
        char m_acKey[MAYKIT_INI_ITEM_KEY_MAX_LEN + 1];
        char m_acVal[MAYKIT_INI_ITEM_VAL_MAX_LEN + 1];
    } IniParserItem;

    class IniParser
    {
    protected:
        IniParserItem m_stIniItemTable[MAYKIT_INI_ITEM_MAX_COUNT + 1];
        char m_acIniFilePath[MAYKIT_INI_FILE_PATH_MAX + 1];
        int m_iItemTabPos;
        
    public:
        IniParser(const char acIniFilePath[])
        {
            strncpy(m_acIniFilePath, acIniFilePath, MAYKIT_INI_FILE_PATH_MAX);
            for (int i = 0; i < MAYKIT_INI_ITEM_MAX_COUNT; i++) {
                memset(m_stIniItemTable[i].m_acKey, 0, MAYKIT_INI_ITEM_KEY_MAX_LEN + 1);
                memset(m_stIniItemTable[i].m_acVal, 0, MAYKIT_INI_ITEM_VAL_MAX_LEN + 1);
            }
            
            m_iItemTabPos = 0;
        }
        
        virtual ~IniParser()
        {
        }
        
        virtual int Parse();
        virtual int GetIniItem(const char *pcKey, char acVal[], int iValLen);
        // 格式化配置文件
        // astConfigItem是以NULL作为数组结束条件
        virtual int FormatConfigItems(ConfigItem astConfigItem[], string &strErrMsg);
        
    protected:
        virtual int ParseLine(char acKey[], int iKeyLen, char acVal[], int iValLen, const char *pcLine);
        virtual int InsertIniItem(IniParserItem &stItem);
        virtual int IsCommentLine(const char *pcLine);
        
    private:
        vector<string> Split(string& str, const char *c);
    };
}

#endif // __MAYKIT_INIPARSER_HEADER__
