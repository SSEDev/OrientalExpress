/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpDriver.h
 *
 * UDP行情驱动器定义头文件
 *
 * @version $Id
 * @since   2014/02/14
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
14-FEB-2014 ZHENGWU         创建
================================================================================
</pre>
*/

#ifndef EPS_UDP_DRIVER_H
#define EPS_UDP_DRIVER_H

/**
 * 包含头文件
 */

#include "recMutex.h"
#include "mktDatabase.h"
#include "udpChannel.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 类型定义
 */

/*
 * UDP驱动器结构
 */
typedef struct EpsUdpDriverTag
{
    uint32          hid;                    /* 句柄ID */
    
    EpsUdpChannelT  channel;                /* 网络通道 */
    EpsMktDatabaseT database;               /* 行情数据库 */
    EpsClientSpiT   spi;                    /* 用户回调接口 */
    EpsRecMutexT    lock;                   /* 驱动器锁 */

    char   username[EPS_USERNAME_MAX_LEN+1]; /* 用户账号 */
    char   password[EPS_PASSWORD_MAX_LEN+1]; /* 用户密码 */
    uint16 heartbeatIntl;                    /* 心跳周期 */
    uint16 recvIdleTimes;                    /* 接收空闲计数 */  
} EpsUdpDriverT;


/**
 * 函数申明
 */

/*
 *  初始化UDP驱动器
 */
ResCodeT InitUdpDriver(EpsUdpDriverT* pDriver);

/*
 *  反初始化UDP驱动器
 */
ResCodeT UninitUdpDriver(EpsUdpDriverT* pDriver);

/*
 *  注册UDP驱动器回调函数集
 */
ResCodeT RegisterUdpDriverSpi(EpsUdpDriverT* pDriver, const EpsClientSpiT* pSpi);

/*
 *  建立UDP服务器连接
 */
ResCodeT ConnectUdpDriver(EpsUdpDriverT* pDriver, const char* address);

/*
 *  断开UDP服务器连接
 */
ResCodeT DisconnectUdpDriver(EpsUdpDriverT* pDriver);

/*
 *  登陆UDP驱动器 
 */
ResCodeT LoginUdpDriver(EpsUdpDriverT* pDriver, 
        const char* username, const char* password, uint16 heartbeatIntl);

/*
 *  登出UDP驱动器 
 */
ResCodeT LogoutUdpDriver(EpsUdpDriverT* pDriver, const char* reason);

/*
 *  订阅UDP驱动器 
 */
ResCodeT SubscribeUdpDriver(EpsUdpDriverT* pDriver, EpsMktTypeT mktType);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UDP_DRIVER_H */
