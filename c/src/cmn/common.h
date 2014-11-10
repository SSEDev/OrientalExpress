/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    common.h
 *
 * 通用定义头文件
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Wu Zheng
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 17-02-2014  ZHENGWU         创建
 ================================================================================
  </pre>
*/

#ifndef EPS_COMMON_H
#define EPS_COMMON_H

/** 
 * 平台相关的预编译宏设置
 */

/* 
 * 平台类型检测
 */
#if ! defined (__LINUX__) && (defined (__linux__) || defined (__KERNEL__) \
        || defined(_LINUX) || defined(LINUX))
#   define  __LINUX__               (1)
#elif ! defined (__HPUX__) && (defined (__hpux) || defined (__HPUX) \
        || defined(__hpux__) || defined(hpux) || defined(HPUX))
#   define  __HPUX__                (1)
#elif ! defined (__AIX__) && defined (_AIX)
#   define  __AIX__                 (1)
#elif ! defined (__SOLARIS__) && (defined (__solaris__) || defined (__sun__) \
        || defined(sun))
#   define  __SOLARIS__             (1)
#elif ! defined (__BSD__) && (defined (__FreeBSD__) || defined (__OpenBSD__) \
        || defined (__NetBSD__))
#   define  __BSD__                 (1)
#elif ! defined (__APPLE__) && defined (__MacOSX__)
#   define  __APPLE__               (1)
#elif ! defined (__WINDOWS__) && (defined (_WIN32) || defined (WIN32) \
        || defined(__WIN32__) || defined(__Win32__))
#   define  __WINDOWS__             (1)
#elif ! defined (__CYGWIN__) && (defined (__CYGWIN32__) || defined (CYGWIN))
#   define  __CYGWIN__              (1)
#endif


/**
 * 包含头文件
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#if defined(__LINUX__) || defined(__HPUX__) 
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#if defined(__WINDOWS__)
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NULL 定义
 */
#if ! defined(NULL)
#   ifdef __cplusplus
#       define NULL                 (0L)
#   else
#       define NULL                 ((void*) 0)
#   endif
#endif

/*
 * BOOL 类型定义
 */
#if defined(__LINUX__) || defined(__HPUX__)
#undef  BOOL
#define BOOL                        int
#endif

#undef  TRUE
#define TRUE                        (1)

#undef  FALSE
#define FALSE                       (0)

/** 
 *  宏定义
 */


#if defined(__LINUX__) || defined(__HPUX__) 
#define NET_ERRNO   errno
#define SYS_ERRNO   errno

#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR	(-1)

typedef int         SOCKET;

#endif


#if defined(__WINDOWS__)
#define NET_ERRNO   WSAGetLastError()
#define SYS_ERRNO   GetLastError()

#define SHUT_RDWR   2
typedef int 		socklen_t;
#endif


#define EPS_IP_MAX_LEN                      (32)        /* IP字符串字段最大长度 */

#define EPS_SOCKET_RECVBUFFER_LEN           (4096*1024) /* 套接字接收缓冲区大小，单位: 字节 */
#define EPS_SOCKET_RECV_TIMEOUT             (1*1000)    /* 套接字接收超时，单位: 毫秒 */

#define EPS_CHANNEL_RECONNECT_INTL          (1*1000)    /* 连接通道重连时间间隔，单位: 毫秒 */
#define EPS_CHANNEL_IDLE_INTL               (500)       /* 连接通道空闲时间间隔，单位: 毫秒 */

#define EPS_DRIVER_KEEPALIVE_TIME           (35*1000)   /* 行情驱动器检测活跃时间阀值，单位: 毫秒 */


/**
 *  接口函数申明
 */

/*
 * 获取操作系统错误描述
 */
const char* EpsGetSystemError(int errCode);

#ifdef __cplusplus
}
#endif

#endif /* EPS_COMMON_H */
