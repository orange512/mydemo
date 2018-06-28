// 校验微信插件个性化数据服务.
// Written by gavinmlwang at 2016/04/26.

#ifndef __WXPLUG_HELPER__
#define __WXPLUG_HELPER__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#pragma pack(push,1)

const uint32_t CONST_NEWS_NUM = 4;							// 每个消息组包含的新闻个数
const char CONST_MAGIC = 0xf3;								// 验证字节
#define VALID_OPERID(cOperId)	(cOperId > EOper_Start && cOperId < EOper_End)

enum EQueryOperId
{
	EOper_Start 	  				= 0,
	EOper_QueryDefautlMsgGoup 		= 1,
	EOper_QueryFeatureMsgGroup 		= 2,
	EOper_QueryArticleInfo 			= 3,
	EOper_QueryMetaInfo 			= 4,
	EOper_End 	  					= 5,
    EOper_QueryOpenIdInfo           = 31,
};

struct MQueryHeader
{
	char 		cOperId;				// EQueryOperId
	char 		cMagic;					// 验证字符
};

// 请求默认新闻组
// cOperId|0xf3|uiPushId
struct MCQueryDefaultMsgGroup : MQueryHeader
{
	uint32_t	uiPushId;
};

// 返回默认新闻组
// cOperId|0xf3|uiPushId|uiNewsIdx1|uiNewsIdx2|uiNewsIdx3|uiNewsIdx4
struct MSQueryDefaultMsgGroup : MQueryHeader
{
	uint32_t	uiPushId;
	uint32_t	uiNewsIdx[CONST_NEWS_NUM];
};

// 请求个性化新闻组
// cOperId|0xf3|uiPushId|cUserIdLen|acUserId
struct MCQueryFeatureMsgGroup : MQueryHeader
{
	uint32_t	uiPushId;
	char		cUserIdLen;				//用户id长度
	char 		acUserId[0];			//用户id
};

// 返回个性化新闻组
// cOperId|0xf3|uiPushId|uiNewsIdx1|uiNewsIdx2|uiNewsIdx3|uiNewsIdx4|cUserIdLen|acUserId
struct MSQueryFeatureMsgGroup : MQueryHeader
{
	uint32_t	uiPushId;
	uint32_t	uiNewsIdx[CONST_NEWS_NUM];
	char		cUserIdLen;				//用户id长度
	char 		acUserId[0];			//用户id
};

// 请求文章信息
// cOperId|0xf3|uiNewsIdx
struct MCQueryArticleInfo : MQueryHeader
{
	uint32_t	uiNewsIdx;
};

// 返回文章信息
// cOperId|0xf3|uiNewsIdx|nArticleLen|acArticleInfo
struct MSQueryArticleInfo : MQueryHeader
{
	uint32_t	uiNewsIdx;
	int 		nArticleLen;			// 新闻长度
	char 		acArticleInfo[0];		// 新闻信息
};

// 请求元数据
// cOperId|0xf3|uiPushId
struct MCQueryMetaInfo : MQueryHeader
{
	uint32_t	uiPushId;
};

// 返回元数据
// cOperId|0xf3|uiPushId|iCreateTime|cWork
struct MSQueryMetaInfo : MQueryHeader
{
	uint32_t	uiPushId;
	int 		iCreateTime;
	char 		cWork;	
};

#pragma pack(pop)

#endif // __WXPLUG_HELPER__
