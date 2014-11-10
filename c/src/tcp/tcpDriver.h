/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    tcpDriver.h
 *
 * TCP行情驱动器定义头文件
 *
 * @version $Id
 * @since   2014/03/11
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
11-MAR-2014 ZHENGWU         创建
================================================================================
</pre>
*/

#ifndef EPS_TCP_DRIVER_H
#define EPS_TCP_DRIVER_H

/**
 * 包含头文件
 */

#include "recMutex.h"
#include "mktDatabase.h"
#include "epsData.h"
#include "tcpChannel.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * 类型定义
 */

/*
 * TCP驱动器状态枚举
 */
typedef enum EpsTcpStatusTag
{
    EPS_TCP_STATUS_DISCONNECTED     = 0,    /* 连接断开 */
    EPS_TCP_STATUS_CONNECTED        = 1,    /* 连接成功 */
    EPS_TCP_STATUS_LOGGING          = 2,    /* 登陆中 */
    EPS_TCP_STATUS_LOGINED          = 3,    /* 登陆成功 */
    EPS_TCP_STATUS_PUBLISHING       = 4,    /* 行情发布中 */
    EPS_TCP_STATUS_LOGOUTING        = 5,    /* 登出中 */
    EPS_TCP_STATUS_LOGOUT           = 6,    /* 已登出 */
} EpsTcpStatusT;

/*
 * TCP驱动器结构
 */
typedef struct EpsTcpDriverTag
{
    uint32          hid;                    /* 句柄ID */
    
    EpsTcpChannelT  channel;                /* 网络通道 */
    EpsMktDatabaseT database;               /* 行情数据库 */
    EpsClientSpiT   spi;                    /* 用户回调接口 */
    
    EpsTcpStatusT   status;                 /* 驱动器状态 */
    uint64          msgSeqNum;              /* 消息序号 */
    char            recvBuffer[EPS_SOCKET_RECVBUFFER_LEN*2];/* 接收缓冲区 */
    uint32          recvBufferLen;          /* 接收缓冲区长度 */
    EpsRecMutexT    lock;                   /* 驱动器锁 */
    
    char   username[EPS_USERNAME_MAX_LEN+1]; /* 用户账号 */
    char   password[EPS_PASSWORD_MAX_LEN+1]; /* 用户密码 */
    uint16 heartbeatIntl;                    /* 心跳周期 */
    uint16 recvIdleTimes;                    /* 接收空闲计数 */  
    uint16 commIdleTimes;                    /* 通讯空闲计数 */
} EpsTcpDriverT;


/**
 * 函数申明
 */

/*
 *  初始化TCP驱动器
 */
ResCodeT InitTcpDriver(EpsTcpDriverT* pDriver);

/*
 *  反初始化TCP驱动器
 */
ResCodeT UninitTcpDriver(EpsTcpDriverT* pDriver);

/*
 *  注册TCP驱动器回调函数集
 */
ResCodeT RegisterTcpDriverSpi(EpsTcpDriverT* pDriver, const EpsClientSpiT* pSpi);

/*
 *  建立TCP服务器连接
 */
ResCodeT ConnectTcpDriver(EpsTcpDriverT* pDriver, const char* address);

/*
 *  断开TCP服务器连接
 */
ResCodeT DisconnectTcpDriver(EpsTcpDriverT* pDriver);

/*
 *  登陆TCP驱动器 
 */
ResCodeT LoginTcpDriver(EpsTcpDriverT* pDriver, 
        const char* username, const char* password, uint16 heartbeatIntl);

/*
 *  登出TCP驱动器 
 */
ResCodeT LogoutTcpDriver(EpsTcpDriverT* pDriver, const char* reason);

/*
 *  订阅TCP驱动器 
 */
ResCodeT SubscribeTcpDriver(EpsTcpDriverT* pDriver, EpsMktTypeT mktType);


#ifdef __cplusplus
}
#endif

#endif /* EPS_TCP_DRIVER_H */
