#include "IniParser.hpp"

int MayKit::IniParser::Parse()
{
    FILE *fp = fopen(m_acIniFilePath, "r");
    if (!fp)
        return -1;
    
    // 不要问为啥多个100，实话告诉你，100是拍脑袋想出来的:)
    char acLine[MAYKIT_INI_ITEM_KEY_MAX_LEN + MAYKIT_INI_ITEM_VAL_MAX_LEN + 100];	
    while (!feof(fp)) {
        char *pcLine = fgets(acLine, MAYKIT_INI_ITEM_KEY_MAX_LEN + MAYKIT_INI_ITEM_VAL_MAX_LEN + 100, fp);
        if (!pcLine && !feof(fp)) {
            fclose(fp);
            return -1;
        }
        
        IniParserItem stItem;
        
        if (ParseLine(stItem.m_acKey, MAYKIT_INI_ITEM_KEY_MAX_LEN, stItem.m_acVal, MAYKIT_INI_ITEM_VAL_MAX_LEN, acLine)) {
            fclose(fp);
            return -1;
        }
        
        if (strlen(stItem.m_acKey) > 0 && InsertIniItem(stItem)) {
            fclose(fp);
            return -1;
        }
    }
        
    fclose(fp);
    
    return 0;
}

int MayKit::IniParser::ParseLine(char acKey[], int iKeyLen, char acVal[], int iValLen, const char *pcLine)
{
    // 如果是注释的话，直接忽略掉
    if (IsCommentLine(pcLine)) {
        acKey[0] = '\0';
        acVal[0] = '\0';
        return 0;
    }
    
    const char *pcTmpLine = pcLine;
    int iTmpPos;
    char c;
    
    // 解析key
    iTmpPos = 0;
    while ('=' != (c = *pcTmpLine++)) {
        if (' ' == c || '	' == c || '\n' == c || '\r' == c)
            continue;
        
        if (iTmpPos < iKeyLen - 1)
            acKey[iTmpPos] = c;
            
        iTmpPos++;
    }
    acKey[iTmpPos] = '\0';
    
    // 解析value
    iTmpPos = 0;
    while (0 != (c = *pcTmpLine++)) {
        if (' ' == c || '	' == c || '\n' == c || '\r' == c)
            continue;
        
        if (iTmpPos < iValLen - 1)
            acVal[iTmpPos] = c;
            
        iTmpPos++;
   }
   
    if ('\r' == acVal[iTmpPos - 2])
        acVal[iTmpPos - 2] = '\0';
    else if ('\n' == acVal[iTmpPos - 1])
        acVal[iTmpPos - 1] = '\0';
    else
        acVal[iTmpPos] = '\0';
    
   return 0;
}

int MayKit::IniParser::IsCommentLine(const char *pcLine)
{
    const char *pcTmpLine = pcLine;
    char c;

    while ((c = *pcTmpLine++)) {
        if (' ' == c || '	' == c)
            continue;
        
        if ('\n' == c || '#' == c || ';' == c || '[' == c)
            return 1;
            
        return 0;
    }
    
    return 0;
}

int MayKit::IniParser::InsertIniItem(IniParserItem &stItem)
{
    if (m_iItemTabPos >= MAYKIT_INI_ITEM_MAX_COUNT)
        return -1;
        
    memcpy(m_stIniItemTable[m_iItemTabPos].m_acKey, stItem.m_acKey, strlen(stItem.m_acKey));
    memcpy(m_stIniItemTable[m_iItemTabPos].m_acVal, stItem.m_acVal, strlen(stItem.m_acVal));
    m_iItemTabPos++;
    
    return 0;
}

// 现在简单采用线性搜索.
// TODO: 红黑树
int MayKit::IniParser::GetIniItem(const char *pcKey, char acVal[], int iValLen)
{
    for (int i = 0; i < MAYKIT_INI_ITEM_MAX_COUNT; i++) {
        if (!strcmp(pcKey, m_stIniItemTable[i].m_acKey)) {
            memcpy(acVal, m_stIniItemTable[i].m_acVal, strlen(m_stIniItemTable[i].m_acVal) + 1);
            return 0;
        }
    }
    
    return -1;
}

// astConfigItem是以NULL作为数组结束条件
int MayKit::IniParser::FormatConfigItems(ConfigItem astConfigItem[], string &strErrMsg)
{
    char acErrMsg[256];
    
    if (Parse()) {
        strErrMsg = "Config file parse failed!";
        return -1;
    }
        
    char acValTmp[MAYKIT_INI_ITEM_VAL_MAX_LEN];
    char acFind[] = {',', 0};
    string strValTmp;

    for (int i = 0; i < MAYKIT_INI_ITEM_MAX_COUNT; i++) {
        // 以数组NULL作为结束条件
        if (!astConfigItem[i].m_pcConfigItemName)
            return 0;
        
        if (GetIniItem(astConfigItem[i].m_pcConfigItemName, acValTmp, MAYKIT_INI_ITEM_VAL_MAX_LEN - 1)) {
            snprintf(acErrMsg, 256, "Get config item [%s] failed!", astConfigItem[i].m_pcConfigItemName);
            strErrMsg = acErrMsg;
            return -1;
        }
        
        switch (astConfigItem[i].m_iType) {
            // 字符串
            case MAYKIT_CCONFIG_ITEM_TYPE_STRING:
                strcpy((char *)(astConfigItem[i].m_pvValSpace), acValTmp);
                break;
            
            // 无符号整型            
            case MAYKIT_CCONFIG_ITEM_TYPE_UINT:
                *((unsigned int *)(astConfigItem[i].m_pvValSpace)) = (unsigned int)(strtoul(acValTmp, 0, 10));
                break;

            // 有符号整型
            case MAYKIT_CCONFIG_ITEM_TYPE_INT:
               *((int *)(astConfigItem[i].m_pvValSpace)) = atoi(acValTmp);
               break;
           
            // 有符号整型           
            case MAYKIT_CCONFIG_ITEM_TYPE_LIST:
                strValTmp.clear();
                strValTmp.append(acValTmp, strlen(acValTmp));
                *((vector<string> *)(astConfigItem[i].m_pvValSpace)) = Split(strValTmp, acFind);
                break;
                
            default:
                snprintf(acErrMsg, 256, "Unknown item type [item:%s type:%d]!", astConfigItem[i].m_pcConfigItemName, astConfigItem[i].m_iType);
                strErrMsg = acErrMsg;
                return -1;
        }
    }
    
    return 0;
}

// 小心内存泄露！
// 使用后记得释放
vector<string> MayKit::IniParser::Split(string &str, const char *c)
{
    char *cstr, *p;
    vector<string> vRes;

    cstr = new char[str.size() + 1];
    strcpy(cstr, str.c_str());
    p = strtok(cstr, c);
    while (p) {
        vRes.push_back(p);
        p = strtok(NULL, c);
    }
    delete[] cstr;
    cstr = NULL;
    
    return vRes;
}
