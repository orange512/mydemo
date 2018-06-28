// 校验微信插件个性化数据服务.
// Written by gavinmlwang at 2016/04/26.

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/errno.h>
#include <map>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <bossapi.h>
#include <time.h>
#include "Log.hpp"
#include "IniParser.hpp"
#include "PushStorage.h"
#include "WxPlugHelper.hpp"
#include "consistent_hash.cc"
#include <jsoncpp/json/json.h>
using namespace std;

#define _LOGGER() g_pstLogger && g_pstLogger
#define LOGGER() _LOGGER()

static void Daemon();
static bool InitServer();
static void DestroyServer();
static void StartServer();
static int 	ProcessMsg(char *pcPackage, int iPackLen, char *pBuffRet, int iBuffLen);

static int QueryDefaultMsgGroup(MCQueryDefaultMsgGroup *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen);
static int QueryFeatureMsgGroup(MCQueryFeatureMsgGroup *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen);
static int QueryArticleInfo(MCQueryArticleInfo *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen);
static int QueryMetaInfo(MCQueryMetaInfo *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen);
static int QueryOpenIdInfo(Json::Value value,char *pacRetBuff,int iBuffLen);

// log实例
static MayKit::Logger *g_pstLogger = NULL;

// 监听的socket
static int g_iSvrScoket = -1;

// 全局配置结构
struct _SvrConfig
{
	// 监听ip
    char m_acSvrIp[256];
    // 监听端口
    char m_acSvrPort[256];
    // Log目录名
    char m_acLogBaseName[256];
    // 是否打开文件log
    int m_iIsOpenFileLog;
    // 是否以后台守护程序方式启动
    int m_iIsDaemon;
    // 1、默认消息组相关配置
	// 默认消息组共享内存id
	int m_iDefaultMsgGroupShmId;
	// 默认消息组共享内存哈希表行数
	uint32_t m_uiDefaultMsgGroupRows;
	// 默认消息组共享内存哈希表列数
	uint32_t m_uiDefaultMsgGroupCols;
    // 2、个性化消息组相关配置
    // 个性化消息组其实共享内存id
	int m_iFeatureMsgGroupShmIdStart;
    // 共享内存总个数
    uint32_t m_iFeatureMsgGroupShmTotalNum;
	// 个性化消息组共享内存哈希表行数
	uint32_t m_uiFeatureMsgGroupRows;
	// 个性化消息组共享内存哈希表列数
	uint32_t m_uiFeatureMsgGroupCols;
    // 3、文章信息相关配置
	// 文章信息共享内存id
	int m_iArticleInfoShmId;
	// 文章信息共享内存哈希表行数
	uint32_t m_uiArticleInfoRows;
	// 文章信息共享内存哈希表列数
	uint32_t m_uiArticleInfoCols;
    // 4、元数据相关配置
	// 元数据共享内存id
	int m_iMetaInfoShmId;
	// 元数据共享内存哈希表行数
	uint32_t m_uiMetaInfoRows;
	// 元数据共享内存哈希表列数
	uint32_t m_uiMetaInfoCols;
} g_stSvrConfig;

MayKit::ConfigItem g_stConfigItemArray[MAYKIT_INI_ITEM_MAX_COUNT] = {
	{"SvrIp", &g_stSvrConfig.m_acSvrIp, MAYKIT_CCONFIG_ITEM_TYPE_STRING},
    {"SvrPort", &g_stSvrConfig.m_acSvrPort, MAYKIT_CCONFIG_ITEM_TYPE_STRING},
    {"LogBaseName", &g_stSvrConfig.m_acLogBaseName, MAYKIT_CCONFIG_ITEM_TYPE_STRING},
    {"OpenFileLog", &g_stSvrConfig.m_iIsOpenFileLog, MAYKIT_CCONFIG_ITEM_TYPE_INT},
    {"Daemon", &g_stSvrConfig.m_iIsDaemon, MAYKIT_CCONFIG_ITEM_TYPE_INT},
    {"DefaultMsgGroupShmId", &g_stSvrConfig.m_iDefaultMsgGroupShmId, MAYKIT_CCONFIG_ITEM_TYPE_INT},
	{"DefaultMsgGroupRows", &g_stSvrConfig.m_uiDefaultMsgGroupRows, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
	{"DefaultMsgGroupCols", &g_stSvrConfig.m_uiDefaultMsgGroupCols, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {"FeatureMsgGroupShmIdStart", &g_stSvrConfig.m_iFeatureMsgGroupShmIdStart, MAYKIT_CCONFIG_ITEM_TYPE_INT},
    {"FeatureMsgGroupShmTotalNum", &g_stSvrConfig.m_iFeatureMsgGroupShmTotalNum, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {"FeatureMsgGroupRows", &g_stSvrConfig.m_uiFeatureMsgGroupRows, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {"FeatureMsgGroupCols", &g_stSvrConfig.m_uiFeatureMsgGroupCols, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {"ArticleInfoShmId", &g_stSvrConfig.m_iArticleInfoShmId, MAYKIT_CCONFIG_ITEM_TYPE_INT},
    {"ArticleInfoRows", &g_stSvrConfig.m_uiArticleInfoRows, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {"ArticleInfoCols", &g_stSvrConfig.m_uiArticleInfoCols, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {"MetaInfoShmId", &g_stSvrConfig.m_iMetaInfoShmId, MAYKIT_CCONFIG_ITEM_TYPE_INT},
    {"MetaInfoRows", &g_stSvrConfig.m_uiMetaInfoRows, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {"MetaInfoCols", &g_stSvrConfig.m_uiMetaInfoCols, MAYKIT_CCONFIG_ITEM_TYPE_UINT},
    {NULL, NULL, MAYKIT_CCONFIG_ITEM_TYPE_UNKNOWN},
};

void Daemon()
{
    int iFd;
    
    if (fork())
        exit(1);
        
    setsid();
    
    // 屏蔽掉一些不要的信号
    signal(SIGINT,  SIG_IGN);
    signal(SIGHUP,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    if ((iFd = open("/dev/null", O_RDWR, 0)))
        return;
    
    dup2(iFd, STDIN_FILENO);
    dup2(iFd, STDOUT_FILENO);
    dup2(iFd, STDERR_FILENO);
    if (iFd > STDERR_FILENO)
        close(iFd);
}

// 查询默认消息组
int QueryDefaultMsgGroup(MCQueryDefaultMsgGroup *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen)
{
	if (!pReqMsg || sizeof(MCQueryDefaultMsgGroup) != iPackLen) {
		LOGGER()->Log("invalid req packet MCQueryDefaultMsgGroup packet!\n");
        return -1;
	}

	vector<uint32_t> vNewsIdxList;
    /*
	if (!PushStorageApi_GetUserDefaultMsgGroup(ntohl(pReqMsg->uiPushId), vNewsIdxList)) {
		LOGGER()->Log("QueryDefaultMsgGroup PushStorageApi_GetUserDefaultMsgGroup failed!\n");
		return -1;
	} */
    uint32_t uiPushId = ntohl(pReqMsg->uiPushId);
    vector<uint32_t> vGroupIdList;
    vGroupIdList.clear();
    uint32_t iForceGrpId;
    int iCreateTime;
    char cWork;
    if (!PushStorageApi_GetMetaInfoABtst(uiPushId, vGroupIdList, iForceGrpId, iCreateTime, cWork, NULL)) {
        LOGGER()->Log("PushStorageApi_GetMetaInfoABtst failed\n");
        return -1;
    }

    if (vGroupIdList.size() < 1) {
		LOGGER()->Log("vGroupIdList size invalid\n");
		return -1;
	}

    if (!PushStorageApi_GetUserDefaultMsgGroupABtst(uiPushId, vGroupIdList[vGroupIdList.size()-1], vNewsIdxList, NULL)) {
        LOGGER()->Log("PushStorageApi_GetUserDefaultMsgGroupABtst failed.\n");
        return -1;
    }

	if (vNewsIdxList.size() > CONST_NEWS_NUM) {
		LOGGER()->Log("QueryDefaultMsgGroup  vNewsIdxList size error!\n");
		return -1;
	}

	if((int)sizeof(MSQueryArticleInfo) > iBuffLen) {
		LOGGER()->Log("QueryDefaultMsgGroup too short RetBuff!\n");
		return -1;
	}

	// 填充返回消息
	MSQueryDefaultMsgGroup *pRetMsg = (MSQueryDefaultMsgGroup*)pacRetBuff;
	pRetMsg->cOperId = EOper_QueryDefautlMsgGoup;
	pRetMsg->cMagic = CONST_MAGIC;
	pRetMsg->uiPushId = pReqMsg->uiPushId;
	for (size_t i =0; i < vNewsIdxList.size(); i++) {
		pRetMsg->uiNewsIdx[i] = htonl(vNewsIdxList[i]);
	}

	return sizeof(MSQueryDefaultMsgGroup);
}

// 查询个性化消息组
int QueryFeatureMsgGroup(MCQueryFeatureMsgGroup *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen)
{
	if (!pReqMsg || ((int)sizeof(MCQueryFeatureMsgGroup) + pReqMsg->cUserIdLen != iPackLen)) {
		LOGGER()->Log("invalid req packet MCQueryFeatureMsgGroup failed!\n");
        return -1;
	}

	string strUserId;
	strUserId.append(pReqMsg->acUserId, pReqMsg->cUserIdLen);
	vector<uint32_t> vNewsIdxList;
        uint64_t ullFlag;
	if (!PushStorageApi_GetUserFeatureMsgGroup(strUserId, ntohl(pReqMsg->uiPushId), vNewsIdxList, ullFlag)) {
		LOGGER()->Log("QueryDefaultMsgGroup PushStorageApi_GetUserFeatureMsgGroup failed!\n");
		return -1;
	}

	if (vNewsIdxList.size() > CONST_NEWS_NUM) {
		LOGGER()->Log("QueryFeatureMsgGroup  vNewsIdxList size error!\n");
		return -1;
	}

	if ((int)sizeof(MSQueryFeatureMsgGroup) + pReqMsg->cUserIdLen > iBuffLen) {
		LOGGER()->Log("QueryFeatureMsgGroup too short RetBuff!\n");
		return -1;
	}

	MSQueryFeatureMsgGroup *pRetMsg = (MSQueryFeatureMsgGroup *)pacRetBuff;
	pRetMsg->cOperId = EOper_QueryFeatureMsgGroup;
	pRetMsg->cMagic = CONST_MAGIC;
	pRetMsg->uiPushId = pReqMsg->uiPushId;
	for (size_t i = 0; i < vNewsIdxList.size(); i++) {
		pRetMsg->uiNewsIdx[i] = htonl(vNewsIdxList[i]);
	}

	pRetMsg->cUserIdLen = pReqMsg->cUserIdLen;
	memcpy(pRetMsg->acUserId, pReqMsg->acUserId, pReqMsg->cUserIdLen);

	return (sizeof(MSQueryFeatureMsgGroup) + pRetMsg->cUserIdLen);
}

// 查询文章信息
int QueryArticleInfo(MCQueryArticleInfo *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen)
{
	if (!pReqMsg || sizeof(MCQueryArticleInfo) != iPackLen) {
		LOGGER()->Log("invalid req packet MCQueryArticleInfo failed!\n");
        return -1;
	}

	string strArticleInfo;
	if (!PushStorageApi_GetArticleInfo(ntohl(pReqMsg->uiNewsIdx), strArticleInfo)) {
		LOGGER()->Log("QueryArticleInfo PushStorageApi_GetArticleInfo failed!\n");
		return -1;
	}

	if (strArticleInfo.empty()) {
		LOGGER()->Log("QueryArticleInfo strArticleInfo empty!\n");
		return -1;
	}

	if((int)strArticleInfo.size() + (int)sizeof(MSQueryArticleInfo) > iBuffLen) {
		LOGGER()->Log("QueryArticleInfo too short RetBuff!\n");
		return -1;
	}

	MSQueryArticleInfo *pRetMsg = (MSQueryArticleInfo *)pacRetBuff;
	pRetMsg->cOperId = EOper_QueryArticleInfo;
	pRetMsg->cMagic = CONST_MAGIC;
	pRetMsg->uiNewsIdx = pReqMsg->uiNewsIdx;
	pRetMsg->nArticleLen = htonl(strArticleInfo.size());
	memcpy(pRetMsg->acArticleInfo, strArticleInfo.c_str(), strArticleInfo.size());

	return (sizeof(MSQueryArticleInfo) + strArticleInfo.size());
}

// 查询元数据
int QueryMetaInfo(MCQueryMetaInfo *pReqMsg, int iPackLen, char *pacRetBuff, int iBuffLen)
{
	if (!pReqMsg || sizeof(MCQueryMetaInfo) != iPackLen) {
		LOGGER()->Log("invalid req packet MCQueryMetaInfo failed!\n");
        return -1;
	}

	char cWork = 0;
	int iCreateTime = 0;
    vector<uint32_t> vGroupIdList;
    uint32_t iForceGrpId;
	if (!PushStorageApi_GetMetaInfoABtst(ntohl(pReqMsg->uiPushId), vGroupIdList, iForceGrpId, iCreateTime, cWork)) {
		LOGGER()->Log("QueryMetaInfo PushStorageApi_GetMetaInfo failed!\n");
		return -1;
	}

	if ((int)sizeof(MSQueryMetaInfo) > iBuffLen) {
		LOGGER()->Log("QueryMetaInfo too short RetBuff!\n");
		return -1;
	}

	MSQueryMetaInfo *pRetMsg = (MSQueryMetaInfo *)pacRetBuff;
	pRetMsg->cOperId = EOper_QueryMetaInfo;
	pRetMsg->cMagic = CONST_MAGIC;
	pRetMsg->uiPushId = pReqMsg->uiPushId;
	pRetMsg->iCreateTime = htonl(iCreateTime);
	pRetMsg->cWork = cWork;

	return sizeof(MSQueryMetaInfo);
}
void GetIpList(string Openid,string &iplist1,string &iplist2)
{
    vector<string> v1;
    vector<string> v2;
    ConsistentHash *con = new ConsistentHash;
    con->InitHashRing();
    con->GetNodeSet(Openid,v1,v2);
    
    iplist1 = v1[0] + ":" + v1[1];
    iplist2 = v2[0] + ":" + v2[1];
}
int32_t myexec(const char *cmd,vector<string> &ret_res)
{
    ret_res.clear();
    FILE *fp = popen(cmd,"r");
    if(!fp)
        return -1;
    char tmp[2048];
    while(fgets(tmp,sizeof(tmp),fp) != NULL)
    {
        if(tmp[strlen(tmp) - 1] =='\n')
        {
            tmp[strlen(tmp) - 1] = '\0'; //去除换行符

        }
        ret_res.push_back(tmp);

    }
    pclose(fp);
    return ret_res.size();

}
int QueryOpenIdInfo(Json::Value value,char *pacRetBuff,int iBuffLen)
{
    printf("enter query\n");
    string Openid = value["openid"].asString();
    if(Openid.empty())
    {
		LOGGER()->Log("invalid req packet QueryOpenIdInfo failed!\n");
        return -1;
    }

    string iplist1; 
    string iplist2; 
    GetIpList(Openid,iplist1,iplist2);

    vector<string> res;
    char cmd[1024];
    snprintf(cmd,sizeof(cmd),"grep '%s*' ~/userfile -r",Openid.c_str());
    myexec(cmd,res);
    
    Json::Value root;

    root["op"] = "31";
    root["openid"] = Openid;
    root["ip_list_1"] = iplist1;
    root["ip_list_2"] = iplist2;
    root["ori_user_file"] = "/data";
    root["user_file"] = res[0];

    //将json转成c字符串
    Json::FastWriter fast_write;
    std::string fs = fast_write.write(root);
    iBuffLen = fs.size() + 1;
    memcpy(pacRetBuff,fs.c_str(),iBuffLen);

    return iBuffLen;

}
/* int ProcessMsg(char *pcPackage, int iPackLen, char *pBuffRet, int iBuffLen) */
/* { */
/*     if (!pcPackage || iPackLen <= 0 || iPackLen > 65535) */
/*         return -1; */

/*     // 太小的包是非法包，直接忽略 */
/*     if (iPackLen < (int)sizeof(MQueryHeader)) */
/*         return -1; */

/*     // 正确的包，解析出来看看 */
/*     MQueryHeader *pstReqMsg = (MQueryHeader *)pcPackage; */
/*     char cOperId = pstReqMsg->cOperId; */
/*     char cMagic = pstReqMsg->cMagic; */
/* 	if ( (CONST_MAGIC != cMagic)|| !VALID_OPERID(cOperId) ) { */
/* 		LOGGER()->Log("invalid req packet MQueryHeader failed!\n"); */
/*         return -1; */
/* 	} */

/* 	int nRetLen = -1; */
/* 	switch (cOperId) { */
/* 		case EOper_QueryDefautlMsgGoup: */
/* 			nRetLen = QueryDefaultMsgGroup((MCQueryDefaultMsgGroup*)pstReqMsg, iPackLen, pBuffRet, iBuffLen); */
/* 			break; */

/* 		case EOper_QueryFeatureMsgGroup: */
/* 			nRetLen = QueryFeatureMsgGroup((MCQueryFeatureMsgGroup*)pstReqMsg, iPackLen, pBuffRet, iBuffLen); */
/* 			break; */

/* 		case EOper_QueryArticleInfo: */
/* 			nRetLen = QueryArticleInfo((MCQueryArticleInfo*)pstReqMsg, iPackLen, pBuffRet, iBuffLen); */
/* 			break; */

/* 		case EOper_QueryMetaInfo: */
/* 			nRetLen = QueryMetaInfo((MCQueryMetaInfo*)pstReqMsg, iPackLen, pBuffRet, iBuffLen); */
/* 			break; */

/* 	} */

/* 	return nRetLen; */
/* } */


int ProcessMsg(char *pcPackage, int iPackLen, char *pBuffRet, int iBuffLen)
{
    printf("enter\n");
    if (!pcPackage || iPackLen <= 0 || iPackLen > 65535)
        return -1;

    // 正确的包，解析出来看看
    Json::Reader reader;
    Json::Value value;
    if(reader.parse(pcPackage,value))
    {
        string ch = value["op"].asString();
        int choose = atoi(ch.c_str());
        int nRetLen = -1;
        switch(choose) {
        case EOper_QueryOpenIdInfo:
            nRetLen = QueryOpenIdInfo(value,pBuffRet,iBuffLen);
            break;
        }

        return nRetLen;

    }
    else
        return -1;
}
bool InitServer()
{
    // 1、初始化存储引擎
    vector<int32_t> vUserGroupShmIdList;
    for (uint32_t i = 0; i < g_stSvrConfig.m_iFeatureMsgGroupShmTotalNum; i++)
        vUserGroupShmIdList.push_back(g_stSvrConfig.m_iFeatureMsgGroupShmIdStart + i);

    if (!PushStorage_Global_Init(g_stSvrConfig.m_iMetaInfoShmId, g_stSvrConfig.m_uiMetaInfoRows, g_stSvrConfig.m_uiMetaInfoCols,
                                 g_stSvrConfig.m_iDefaultMsgGroupShmId, g_stSvrConfig.m_uiDefaultMsgGroupRows, g_stSvrConfig.m_uiDefaultMsgGroupCols,
                                 g_stSvrConfig.m_iArticleInfoShmId, g_stSvrConfig.m_uiArticleInfoRows, g_stSvrConfig.m_uiArticleInfoCols,
                                 vUserGroupShmIdList, g_stSvrConfig.m_uiFeatureMsgGroupRows, g_stSvrConfig.m_uiFeatureMsgGroupCols)) {
        _LOGGER()->Log("[File: %s, Function: %s, Line: %d] PushStorage_Global_Init failed!\n",
                       __FILE__,
                       __FUNCTION__,
                       __LINE__);
        return false;
    }

    return true;
}

void DestroyServer()
{
    PushStorage_Global_UnInit();
    return;
}

void StartServer()
{
    if (g_iSvrScoket >= 0)
        close(g_iSvrScoket);

    g_iSvrScoket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_iSvrScoket < 0) {
        _LOGGER()->Log("[File: %s, Function: %s, Line: %d] new server socket failed!\n",
                       __FILE__,
                       __FUNCTION__,
                       __LINE__);
        return;
    }

    // 注意，这个socket是阻塞的!
    struct sockaddr_in stUdpAddr;
    bzero(&stUdpAddr, sizeof(stUdpAddr));
    stUdpAddr.sin_family = AF_INET;
    stUdpAddr.sin_port = htons(atoi(g_stSvrConfig.m_acSvrPort));
    stUdpAddr.sin_addr.s_addr = inet_addr(g_stSvrConfig.m_acSvrIp);

    if (bind(g_iSvrScoket, (struct sockaddr *)&stUdpAddr, sizeof(stUdpAddr))) {
        _LOGGER()->Log("[File: %s, Function: %s, Line: %d] bind server socket failed!\n",
                       __FILE__,
                       __FUNCTION__,
                       __LINE__);
        close(g_iSvrScoket);
        g_iSvrScoket = -1;
        return;
    }

    char acRecvBuf[65535];
    struct sockaddr stClientAddr;
    socklen_t iClientAddrLen = sizeof(struct sockaddr);
    // 开始监听数据包
    while (true) {
        int iRecvLen = recvfrom(g_iSvrScoket, acRecvBuf, sizeof(acRecvBuf), 0, &stClientAddr, &iClientAddrLen);
        if (iRecvLen <= 0)
            continue;

        struct sockaddr_in *pstClientSinAddr = (struct sockaddr_in *)&stClientAddr;

        // 处理消息
		char acRetBuff[10240] = {0};
		int nRetLen = ProcessMsg(acRecvBuf, iRecvLen, acRetBuff, 10240);
        if (nRetLen < 0) {
            _LOGGER()->Log("[File: %s, Function: %s, Line: %d] ProcessMsg! SrcIp=%s, Port=%d\n",
                           __FILE__,
                           __FUNCTION__,
                           __LINE__,
                           inet_ntoa(pstClientSinAddr->sin_addr),
                           ntohs(pstClientSinAddr->sin_port));
            continue;
        }

        printf("acRetBuff = %s\n",acRetBuff);
		int nSendLen = sendto(g_iSvrScoket, acRetBuff, nRetLen, 0, &stClientAddr, iClientAddrLen);
		if (nSendLen != nRetLen) {
			_LOGGER()->Log("[File: %s, Function: %s, Line: %d] sendto failed!\n",
                       __FILE__,
                       __FUNCTION__,
                       __LINE__);
			continue;
		}
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "[File: %s, Function: %s, Line: %d] Too less parameter!\n",
                        __FILE__,
                        __FUNCTION__,
                        __LINE__);
        return 1;
    }

    // 先初始化下配置文件
    MayKit::IniParser stIniParser(argv[1]);
    string strErrMsg;
    if (stIniParser.FormatConfigItems(g_stConfigItemArray, strErrMsg)) {
        fprintf(stderr, "[File: %s, Function: %s, Line: %d] FormatConfigItems config file failed! ErrMsg: %s\n",
                        __FILE__,
                        __FUNCTION__,
                        __LINE__,
                        strErrMsg.c_str());
        return 1;
    }

    // 尽快实例化log
    if (g_stSvrConfig.m_iIsOpenFileLog)
        g_pstLogger = new MayKit::Logger(g_stSvrConfig.m_acLogBaseName, 1024 * 1024 * 256, 10);

	// 是否以守护进程方式运行
    if (g_stSvrConfig.m_iIsDaemon)
        Daemon();

    if (!InitServer()) {
        _LOGGER()->Log("[File: %s, Function: %s, Line: %d] InitServer failed!\n",
                       __FILE__,
                       __FUNCTION__,
                       __LINE__);
        return 0;
    }

	// 启动服务
	StartServer();
    
    // 释放服务器，忽略返回值
    DestroyServer();
    
    // 致命错误
    _LOGGER()->Log("[File: %s, Function: %s, Line: %d] Unknown error cause server exit!\n",
                   __FILE__, 
                   __FUNCTION__, 
                   __LINE__);
    
    return 1;
}
