/*************************************************************************
	> File Name: helper_cli.cpp
	> Author:    orange
	> Mail:      huiyi950512@gmail.com
	> Created Time: 2018年05月29日 星期二 11时07分02秒
 ************************************************************************/

#include <iostream>
using namespace std;

#include<stdio.h>
#include<string.h>
#include<sys/types.h>  /* See NOTES */
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"WxPlugHelper.hpp"
#include <jsoncpp/json/json.h>

 #define RED    "\e[0;31m"
void menu()
{
    printf("-----1.查询默认组-----\n");
    printf("-----2.查询个性化组---\n");
    printf("-----3.查询文章信息---\n");
    printf("-----4.查询元数据信息-\n");
    printf("-----5.查询openid---\n");
    printf("--- 请输入查询的序号--\n");
}


void QueryFeatureMsgGroup(int sockfd,struct sockaddr *dst)
{
    //组装头部
    char req_buff[1024];
    char ret_buff[65535];
    socklen_t len = sizeof(*dst);
    MQueryHeader *myinfo = (MQueryHeader *)req_buff;
    myinfo->cOperId = 2;
    myinfo->cMagic = 0xf3;
    //组装BODY
    MCQueryFeatureMsgGroup *info = (MCQueryFeatureMsgGroup*)myinfo;

    printf("input Pushid and acUserId\n");
    uint32_t pushid;
    char acUserId[28];
    scanf("%d %s",&pushid,acUserId);
    info->uiPushId = htonl(pushid);
    info->cUserIdLen = 28;
    memcpy(info->acUserId,acUserId,28);

    sendto(sockfd,req_buff,sizeof(MCQueryFeatureMsgGroup)+sizeof(acUserId),0,dst,len);

    printf("\n\n------result---------\n\n ");
    struct sockaddr_in src;
    int ret = recvfrom(sockfd,ret_buff,sizeof(ret_buff),0,(struct sockaddr*)&src,&len);
    if(ret == 0)
    {
        cout << "server closed\n"<<endl;
        return;
    }
    else if(ret < 0)
        perror("recv");
    else
    {
        if(!strcmp(ret_buff,"not found"))
        {
            printf("%s\n",ret_buff);
            return ;
        }
        MSQueryFeatureMsgGroup *info = (MSQueryFeatureMsgGroup *)ret_buff;
        printf("uiNewsIdx[0] =  %d\n",ntohl(info->uiNewsIdx[0]));
    
        printf("uiPushId = %d\n",ntohl(info->uiPushId));
    }
}

void QueryDefaultMsgGroup(int sockfd,struct sockaddr* dst)
{
    char req_buff[1024];
    char ret_buff[65535];
    socklen_t len = sizeof(*dst);
    MQueryHeader *myinfo = (MQueryHeader *)req_buff;
    myinfo->cOperId = 1;
    myinfo->cMagic = 0xf3;

    MCQueryDefaultMsgGroup *info = (MCQueryDefaultMsgGroup*)myinfo;
    info->uiPushId = htonl(2018052301);

    sendto(sockfd,req_buff,sizeof(MCQueryDefaultMsgGroup),0,dst,len);

    printf("\n\n------result---------\n\n ");
    struct sockaddr_in src;
    int ret = recvfrom(sockfd,ret_buff,sizeof(ret_buff),0,(struct sockaddr*)&src,&len);
    if(ret == 0)
    {
        cout << "server closed\n"<<endl;
        return;
    }
    else if(ret < 0)
        perror("recv");
    else
    {
        if(!strcmp(ret_buff,"not found"))
        {
            printf("%s\n",ret_buff);
            return ;
        }
        MSQueryFeatureMsgGroup *info = (MSQueryFeatureMsgGroup *)ret_buff;
        printf("uiNewsIdx[0] =  %d\n",ntohl(info->uiNewsIdx[0]));
    
        printf("uiPushId = %d\n",ntohl(info->uiPushId));
    }

}
void QueryArticleInfo(int sockfd,struct sockaddr *dst)
{
    char req_buff[1024];
    char ret_buff[65535];
    socklen_t len = sizeof(*dst);
    MQueryHeader *myinfo = (MQueryHeader *)req_buff;
    myinfo->cOperId = 3;
    myinfo->cMagic = 0xf3;

    MCQueryArticleInfo *info = (MCQueryArticleInfo*)myinfo;
    info->uiNewsIdx = htonl(1222);

    sendto(sockfd,req_buff,sizeof(MCQueryArticleInfo),0,dst,len);

    printf("\n\n------result---------\n\n ");
    struct sockaddr_in src;
    int ret = recvfrom(sockfd,ret_buff,sizeof(ret_buff),0,(struct sockaddr*)&src,&len);
    if(ret == 0)
    {
        cout << "server closed\n"<<endl;
        return;
    }
    else if(ret < 0)
        perror("recv");
    else
    {
        if(!strcmp(ret_buff,"not found"))
        {
            printf("%s\n",ret_buff);
            return ;
        }
        MSQueryArticleInfo *info = (MSQueryArticleInfo *)ret_buff;
    
        printf("nArticleLen = %d\n",ntohl(info->nArticleLen));
        printf("Article = %s\n",info->acArticleInfo);
    }


}
void QueryMetaInfo(int sockfd,struct sockaddr *dst)
{
    char req_buff[1024];
    char ret_buff[65535];
    socklen_t len = sizeof(*dst);
    MQueryHeader *myinfo = (MQueryHeader *)req_buff;
    myinfo->cOperId = 4;
    myinfo->cMagic = 0xf3;

    MCQueryMetaInfo *info = (MCQueryMetaInfo*)myinfo;
    info->uiPushId = htonl(2018053001);

    sendto(sockfd,req_buff,sizeof(MCQueryMetaInfo),0,dst,len);

    printf("\n\n------result---------\n\n ");
    struct sockaddr_in src;
    int ret = recvfrom(sockfd,ret_buff,sizeof(ret_buff),0,(struct sockaddr*)&src,&len);
    if(ret == 0)
    {
        cout << "server closed\n"<<endl;
        return;
    }
    else if(ret < 0)
        perror("recv");
    else
    {
        if(!strcmp(ret_buff,"not found"))
        {
            printf("%s\n",ret_buff);
            return ;
        }
        MSQueryMetaInfo *info = (MSQueryMetaInfo *)ret_buff;
    
        printf("uiPushId = %d\n",ntohl(info->uiPushId));
    }

}
void QueryOpenid(int sockfd,struct sockaddr *dst)
{
    Json::Value root;

    char req_buff[1024];
    char ret_buff[65535];
    memset(ret_buff,0,sizeof(ret_buff));
    socklen_t len = sizeof(*dst);
    root["op"] = "31";
    root["openid"] = "o04IBAA-1En0kdQpw0z0awgOadlI";

    Json::FastWriter fa;
    std::string json = fa.write(root);
    unsigned int n = json.size() + 1;
    memcpy(req_buff,json.c_str(),n);
    printf("rep_buff = %s\n",req_buff);
    sendto(sockfd,req_buff,n,0,dst,len);

    printf("\n\n------result---------\n\n ");
    struct sockaddr_in src;
    int ret = recvfrom(sockfd,ret_buff,sizeof(ret_buff),0,(struct sockaddr*)&src,&len);
    if(ret == 0)
    {
        cout << "server closed\n"<<endl;
        return;
    }
    else if(ret < 0)
        perror("recv");
    else
    {
        printf("ret_buff = %s\n",ret_buff);
        Json::Value obj;
        Json::Reader reader;
        reader.parse(ret_buff,obj);
        string pa = obj["user_file"].asString();
        printf("user_file = %s\n",pa.c_str());

    }
}
void start(int sockfd,struct sockaddr *dst)
{
    menu();
    int choose;
    cin>>choose;
    switch(choose)
    {
    case 1:
        QueryDefaultMsgGroup(sockfd,dst);break;
    case 2:
        QueryFeatureMsgGroup(sockfd,dst);break;
    case 3:
        QueryArticleInfo(sockfd,dst);break;
    case 4:
        QueryMetaInfo(sockfd,dst);break;
    case 5:
        QueryOpenid(sockfd,dst);break;
    case 0:
        break;
    }
}
int main()
{
    
    int sock;
    if((sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
        perror("socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(7341);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
//    if(connect(sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr))< 0)
//        perror("connect");

    start(sock,(struct sockaddr*)&serveraddr);
    return 0;
}
