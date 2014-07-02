/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    tcpChannel.h
 *
 * TCP收发通道定义头文件
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

#ifndef EPS_TCP_CHANNEL_H
#define EPS_TCP_CHANNEL_H

/**
 * 包含头文件
 */

#include "uniQueue.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 类型定义
 */

/*
 * TCP通道状态枚举
 */
typedef enum EpsTcpChannelStatusTag
{
    EPS_TCPCHANNEL_STATUS_STOP     = 0,    /* 停止状态 */
    EPS_TCPCHANNEL_STATUS_IDLE     = 1,    /* 空闲状态 */
    EPS_TCPCHANNEL_STATUS_WORK     = 2,    /* 工作状态 */
} EpsTcpChannelStatusT;

/*
 * 异步回调接口
 */
typedef void (*EpsTcpChannelConnectedCallback)(void* pListener);
typedef void (*EpsTcpChannelDisconnectedCallback)(void* pListener, ResCodeT result, const char* reason);
typedef void (*EpsTcpChannelReceivedCallback)(void* pListener, ResCodeT result, const char* data, uint32 dataLen);
typedef void (*EpsTcpChannelSendedCallback)(void* pListener, ResCodeT result, const char* data, uint32 dataLen);

/*
 * UDP通道监听者接口
 */
typedef struct EpsTcpChannelListenerTag
{
    void*                               pListener;          /* 监听者对象 */
    EpsTcpChannelConnectedCallback      connectedNotify;    /* 连接成功通知 */
    EpsTcpChannelDisconnectedCallback   disconnectedNotify; /* 连接断开通知 */
    EpsTcpChannelReceivedCallback       receivedNotify;     /* 数据接收通知 */
    EpsTcpChannelSendedCallback         sendedNotify;       /* 数据发送通知 */
} EpsTcpChannelListenerT;


/*
 * TCP通道结构
 */
typedef struct EpsTcpChannelTag
{
    char        srvAddr[EPS_IP_MAX_LEN+1];  /* 服务器地址 */
    uint16      srvPort;                    /* 服务器端口 */

    SOCKET      socket;						/* 通讯套接字 */

#if defined(__WINDOWS__)
    HANDLE      thread;						/* 线程对象 */
    DWORD		tid;						/* 线程id */
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
    pthread_t   tid;                        /* 线程id */
#endif

    EpsUniQueueT sendQueue;                 /* 发送队列 */
    char        recvBuffer[EPS_SOCKET_RECVBUFFER_LEN];/* 接收缓冲区 */
    BOOL        canStop;                    /* 允许停止线程运行标记 */
    EpsTcpChannelStatusT status;            /* 通道状态 */

    EpsTcpChannelListenerT listener;        /* 监听者接口 */
} EpsTcpChannelT;


/**
 * 函数申明
 */

/*
 * 初始化TCP通道
 */
ResCodeT InitTcpChannel(EpsTcpChannelT* pChannel);

/*
 * 反初始化TCP通道
 */
ResCodeT UninitTcpChannel(EpsTcpChannelT* pChannel);

/*
 * 启动TCP通道
 */
ResCodeT StartupTcpChannel(EpsTcpChannelT* pChannel);

/*
 * 停止TCP通道
 */
ResCodeT ShutdownTcpChannel(EpsTcpChannelT* pChannel);

/*
 * 打开TCP通道
 */
ResCodeT OpenTcpChannel(EpsTcpChannelT* pChannel);

/*
 * 关闭TCP通道
 */
ResCodeT CloseTcpChannel(EpsTcpChannelT* pChannel);

/*
 * 等待TCP通道结束
 */
ResCodeT JoinTcpChannel(EpsTcpChannelT* pChannel);

/*
 * 向TCP通道发送数据
 */
ResCodeT SendTcpChannel(EpsTcpChannelT* pChannel, const char* data, uint32 dataLen);

/*
 * 注册通道监听者接口
 */
ResCodeT RegisterTcpChannelListener(EpsTcpChannelT* pChannel, const EpsTcpChannelListenerT* pListener);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UDP_CHANNEL_H */
