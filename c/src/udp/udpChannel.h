/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpChannel.h
 *
 * UDP组播通道定义头文件
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

#ifndef EPS_UDP_CHANNEL_H
#define EPS_UDP_CHANNEL_H

/**
 * 包含头文件
 */

#include "common.h"
#include "uniQueue.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 类型定义
 */

/*
 * UDP通道状态枚举
 */
typedef enum EpsUdpChannelStatusTag
{
    EPS_UDPCHANNEL_STATUS_STOP     = 0, /* 停止状态 */
    EPS_UDPCHANNEL_STATUS_IDLE     = 1, /* 空闲状态 */
    EPS_UDPCHANNEL_STATUS_WORK     = 2, /* 工作状态 */
} EpsUdpChannelStatusT;

/*
 * 异步事件
 */
typedef struct EpsUdpChannelEventTag
{
    uint32  eventType;                  /* 事件类型 */
    uint32  eventParam;                 /* 事件参数 */     
} EpsUdpChannelEventT;

/*
 * 异步回调接口
 */
typedef void (*EpsUdpChannelConnectedCallback)(void* pListener);
typedef void (*EpsUdpChannelDisconnectedCallback)(void* pListener, ResCodeT result, const char* reason);
typedef void (*EpsUdpChannelReceivedCallback)(void* pListener, ResCodeT result, const char* data, uint32 dataLen);
typedef void (*EpsUdpChannelEventOccurredCallback)(void* pListener, EpsUdpChannelEventT* pEvent);

/*
 * UDP通道监听者接口
 */
typedef struct EpsUdpChannelListenerTag
{
    void*                               pListener;          /* 监听者对象 */
    EpsUdpChannelConnectedCallback      connectedNotify;    /* 连接成功通知 */
    EpsUdpChannelDisconnectedCallback   disconnectedNotify; /* 连接断开通知 */
    EpsUdpChannelReceivedCallback       receivedNotify;     /* 数据接收通知 */
    EpsUdpChannelEventOccurredCallback  eventOccurredNotify;/* 事件发生通知 */
} EpsUdpChannelListenerT;


/*
 * UDP通道结构
 */
typedef struct EpsUdpChannelTag
{
    char        localAddr[EPS_IP_MAX_LEN+1];/* 本地地址 */
    char        mcAddr[EPS_IP_MAX_LEN+1];   /* 组播地址 */
    uint16      mcPort;                     /* 组播端口 */

    SOCKET      socket;						/* 通讯套接字 */

#if defined(__WINDOWS__)
    HANDLE      thread;						/* 线程对象 */
    DWORD		tid;						/* 线程id */
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
    pthread_t   tid;                        /* 线程id */
#endif

    EpsUniQueueT eventQueue;                /* 事件队列 */
    char        recvBuffer[EPS_SOCKET_RECVBUFFER_LEN];/* 接收缓冲区 */
    BOOL        canStop;                    /* 允许停止线程运行标记 */ 
    EpsUdpChannelStatusT status;            /* 通道状态 */

    EpsUdpChannelListenerT listener;        /* 监听者接口 */
} EpsUdpChannelT;


/**
 * 函数申明
 */

/*
 * 初始化UDP通道
 */
ResCodeT InitUdpChannel(EpsUdpChannelT* pChannel);

/*
 * 反初始化UDP通道
 */
ResCodeT UninitUdpChannel(EpsUdpChannelT* pChannel);

/*
 * 启动UDP通道
 */
ResCodeT StartupUdpChannel(EpsUdpChannelT* pChannel);

/*
 * 停止UDP通道
 */
ResCodeT ShutdownUdpChannel(EpsUdpChannelT* pChannel);

/*
 * 打开UDP通道
 */
ResCodeT OpenUdpChannel(EpsUdpChannelT* pChannel);

/*
 * 关闭UDP通道
 */
ResCodeT CloseUdpChannel(EpsUdpChannelT* pChannel);

/*
 * 等待UDP通道结束
 */
ResCodeT JoinUdpChannel(EpsUdpChannelT* pChannel);

/*
 * 触发异步事件
 */
ResCodeT TriggerUdpChannelEvent(EpsUdpChannelT* pChannel, const EpsUdpChannelEventT event);

/*
 * 注册通道监听者接口
 */
ResCodeT RegisterUdpChannelListener(EpsUdpChannelT* pChannel, const EpsUdpChannelListenerT* pListener);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UDP_CHANNEL_H */
