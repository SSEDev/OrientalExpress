/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpChannel.c
 *
 * UDP组播通道实现文件
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

/**
 * 包含头文件
 */


#include "common.h"
#include "epsTypes.h"
#include "errlib.h"

#include "udpChannel.h"

/**
 * 宏定义
 */

#define EPS_EVENTQUEUE_SIZE                      128


/**
 * 内部函数声明
 */

static void* ChannelTask(void* arg);

static ResCodeT HandleEvent(EpsUdpChannelT* pChannel);
static ResCodeT ReceiveData(EpsUdpChannelT* pChannel);
static ResCodeT ClearEventQueue(EpsUdpChannelT* pChannel);

static BOOL IsChannelInited(EpsUdpChannelT * pChannel);
static BOOL IsChannelStarted(EpsUdpChannelT* pChannel);
static BOOL IsChannelConnected(EpsUdpChannelT * pChannel);

static void OnChannelConnected(void* pListener);
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason);
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen);
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent);


/**
 * 函数实现
 */

/**
 * 初始化UDP通道
 *
 * @param   pChannel            in  - 通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT InitUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "channel");
        }

        pChannel->socket = INVALID_SOCKET;
        pChannel->tid = 0;
        pChannel->canStop = TRUE;
        pChannel->status  = EPS_UDPCHANNEL_STATUS_STOP;
        InitUniQueue(&pChannel->eventQueue, EPS_EVENTQUEUE_SIZE);
 
        EpsUdpChannelListenerT listener =
        {
            NULL,
            OnChannelConnected,
            OnChannelDisconnected,
            OnChannelReceived,
            OnChannelEventOccurred
        };
        pChannel->listener = listener; 
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 反初始化UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UninitUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (! IsChannelInited(pChannel))
        {
            THROW_RESCODE(NO_ERR);
        } 

        CloseUdpChannel(pChannel);
        UninitUniQueue(&pChannel->eventQueue);
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 启动UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT StartupUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (IsChannelStarted(pChannel))
        {
            if (pChannel->status == EPS_UDPCHANNEL_STATUS_IDLE)
            {
                pChannel->status = EPS_UDPCHANNEL_STATUS_WORK;
                THROW_RESCODE(NO_ERR);
            }
            else
            {
                THROW_ERROR(ERCD_EPS_DUPLICATE_CONNECT);
            }
        }
       
        pChannel->canStop = FALSE;
        pChannel->status = EPS_UDPCHANNEL_STATUS_WORK;
        
#if defined(__WINDOWS__)
        DWORD tid = 0;
        HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ChannelTask, 
            (LPVOID)pChannel, 0, (LPDWORD)&tid);
        if (tid == 0)
        {
        	pChannel->status = EPS_UDPCHANNEL_STATUS_STOP;
            int lstErrno = SYS_ERRNO;
        	THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        }
        pChannel->thread = thread;
        pChannel->tid = tid;
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        pthread_t tid;
        int result = pthread_create(&tid, NULL, ChannelTask, (void*)pChannel);
        if (result != 0)
        {
        	pChannel->status = EPS_UDPCHANNEL_STATUS_STOP;
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(SYS_ERRNO));
        }
        pChannel->tid = tid;
#endif
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 停止UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT ShutdownUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (! IsChannelStarted(pChannel))
        {
            THROW_RESCODE(NO_ERR);
        }

#if defined(__WINDOWS__)
        if (pChannel->tid != GetCurrentThreadId())
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        if (pChannel->tid != pthread_self())
#endif
        {
             pChannel->canStop = TRUE;
        }
        else
        {
            if (pChannel->status == EPS_UDPCHANNEL_STATUS_WORK)
            {
                CloseUdpChannel(pChannel);
            }

            pChannel->status = EPS_UDPCHANNEL_STATUS_IDLE;
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 等待UDP通道结束
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
 ResCodeT JoinUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
#if defined(__WINDOWS__)
        if (pChannel->tid != 0 && pChannel->tid != GetCurrentThreadId())
        {
        	int result = WaitForSingleObject(pChannel->thread, INFINITE);
        	if (result != WAIT_OBJECT_0)
        	{
        	    int lstErrno = SYS_ERRNO;
        		THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        	}

        	pChannel->thread = NULL;
        	pChannel->tid = 0;
        }
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        if (pChannel->tid != 0 && pChannel->tid != pthread_self())
        {
            int result = pthread_join(pChannel->tid, NULL);
            if (result != 0)
            {
                THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(SYS_ERRNO));
            }            

            pChannel->tid = 0;
        }
#endif
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * 触发UDP通道异步事件
 *
 * @param   pChannel            in  - UDP通道对象
 * @param   event               in  - 事件对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT TriggerUdpChannelEvent(EpsUdpChannelT* pChannel, const EpsUdpChannelEventT event)
{
    TRY
    {
        if(! IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "channel");
        }

        EpsUdpChannelEventT* pEvent = (EpsUdpChannelEventT*)calloc(1, sizeof(EpsUdpChannelEventT));
        if (pEvent == NULL)
        {
            int lstErrno = SYS_ERRNO;
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        }
        *pEvent = event;

        THROW_ERROR(PushUniQueue(&pChannel->eventQueue, (void*)pEvent));
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}


/**
 * 注册UDP通道监听者接口
 *
 * @param   pChannel            in  - UDP通道对象
 * @param   pListener           in  - 待注册的通道监听者
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT RegisterUdpChannelListener(EpsUdpChannelT* pChannel, const EpsUdpChannelListenerT* pListener)
{
    TRY
    {
        if(! IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "channel");
        }

        pChannel->listener.pListener = pListener->pListener;
        if (pListener->connectedNotify != NULL)
        {
            pChannel->listener.connectedNotify = pListener->connectedNotify;
        }
        if (pListener->disconnectedNotify != NULL)
        {
            pChannel->listener.disconnectedNotify = pListener->disconnectedNotify;
        }
        if (pListener->receivedNotify != NULL)
        {
            pChannel->listener.receivedNotify = pListener->receivedNotify;
        }
        if (pListener->eventOccurredNotify != NULL)
        {
            pChannel->listener.eventOccurredNotify = pListener->eventOccurredNotify;
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}


/**
 * UDP通道任务线程函数
 *
 * @param   arg                 in  - 线程参数(UDP通道对象)
 */
static void* ChannelTask(void* arg)
{
    EpsUdpChannelT* pChannel = (EpsUdpChannelT*)arg;

    while (! pChannel->canStop)
    {
        if (pChannel->status == EPS_UDPCHANNEL_STATUS_IDLE)
        {
            usleep(EPS_CHANNEL_IDLE_INTL * 1000);
            continue;
        }

        /* 打开UDP通道 */
        if (! IsChannelConnected(pChannel))
        {
            if (NOTOK(OpenUdpChannel(pChannel)))
            {
                pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                    ErrGetErrorCode(), ErrGetErrorDscr());

                ErrClearError();

#if defined(__WINDOWS__)
                Sleep(EPS_CHANNEL_RECONNECT_INTL);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
                usleep(EPS_CHANNEL_RECONNECT_INTL * 1000);
#endif

                continue;
            }
            else
            {
                pChannel->listener.connectedNotify(pChannel->listener.pListener);
            }
        }
        
        /* 优先处理异步事件 */
        if (NOTOK(HandleEvent(pChannel)))
        {
            ErrClearError();
         
            continue;
        }

        /* 处理网络接收数据 */
        if (NOTOK(ReceiveData(pChannel)))
        {
            ErrClearError();
         
            continue;
        }
    }
    
    CloseUdpChannel(pChannel);

    pChannel->status = EPS_UDPCHANNEL_STATUS_STOP;
    
    return 0;
}

/**
 * 打开UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT OpenUdpChannel(EpsUdpChannelT* pChannel)
{
    SOCKET fd = INVALID_SOCKET;

    TRY
    {
        int result = SOCKET_ERROR;
        
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == INVALID_SOCKET)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }

        BOOL reuseaddr = TRUE;
        result = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
        if (result == SOCKET_ERROR)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }

        struct sockaddr_in mcAddr;
        memset(&mcAddr, 0x00, sizeof(mcAddr));
            
        mcAddr.sin_family      = AF_INET;
        mcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        mcAddr.sin_port        = htons(pChannel->mcPort);

        result = bind(fd, (struct sockaddr*)&mcAddr, sizeof(mcAddr));
        if (result == SOCKET_ERROR)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }   

        int rcvBufferSize = EPS_SOCKET_RECVBUFFER_LEN;
        int len = sizeof(rcvBufferSize);
        result = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&rcvBufferSize, len);
        if (result == SOCKET_ERROR)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }   
 
        struct ip_mreq mreq;
        memset(&mreq, 0x00, sizeof(mreq));
        mreq.imr_multiaddr.s_addr = inet_addr(pChannel->mcAddr);
        mreq.imr_interface.s_addr = inet_addr(pChannel->localAddr);
        result = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
        if (result == SOCKET_ERROR)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }

#if defined(__WINDOWS__)
        unsigned long flags = 1;
        result = ioctlsocket(fd, FIONBIO, &flags);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        int flags = fcntl(fd, F_GETFL, 0);
        result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
        if (result == SOCKET_ERROR)
        {
            int lstErrno = SYS_ERRNO;
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        }

        pChannel->socket = fd;

        ClearEventQueue(pChannel);
    }
    CATCH
    {
    	if (fd != INVALID_SOCKET)
    	{
#if defined(__WINDOWS__)
    		closesocket(fd);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
            close(fd);
#endif

    		fd = INVALID_SOCKET;
    	}
    }
    FINALLY
    {
        RETURN_RESCODE;      
    }
}


/**
 * 关闭UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT CloseUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
       if (pChannel->socket != INVALID_SOCKET)
        {
        	shutdown(pChannel->socket, SHUT_RDWR);

#if defined(__WINDOWS__)
        	closesocket(pChannel->socket);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
            close(pChannel->socket);
#endif
   
        	pChannel->socket = INVALID_SOCKET;
        }

        ClearEventQueue(pChannel);
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}


/**
 * 处理通道事件
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT HandleEvent(EpsUdpChannelT* pChannel)
{
    TRY
    {
        EpsUdpChannelEventT* pEvent = NULL;
        THROW_ERROR(PopUniQueue(&pChannel->eventQueue, (void**)&pEvent));
      
        if (pEvent != NULL)
        {
            pChannel->listener.eventOccurredNotify(pChannel->listener.pListener, pEvent);
            free(pEvent);
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 接收通道数据
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT ReceiveData(EpsUdpChannelT* pChannel)
{
    TRY
    {
        struct sockaddr_in srcAddr;
        socklen_t addrlen = sizeof(srcAddr);

        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(pChannel->socket, &fdset);

        struct timeval timeout;
        timeout.tv_sec = EPS_SOCKET_RECV_TIMEOUT / 1000;
        timeout.tv_usec = (EPS_SOCKET_RECV_TIMEOUT % 1000) * 1000;

        int result = select(pChannel->socket+1, &fdset, 0, 0, &timeout);
        if (result == SOCKET_ERROR)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }
        
        if (FD_ISSET(pChannel->socket, &fdset))
        {
            int len = recvfrom(pChannel->socket, pChannel->recvBuffer, EPS_SOCKET_RECVBUFFER_LEN, 
                0, (struct sockaddr*)&srcAddr, &addrlen);
            if (len > 0)
            {
                pChannel->listener.receivedNotify(pChannel->listener.pListener, 
                        NO_ERR, pChannel->recvBuffer, (uint32)len);
            }
            else if(len == 0)
            {
                THROW_ERROR(ERCD_EPS_SOCKET_ERROR, "Connection closed by remote");
            }
            else
            {
                int lstErrno = NET_ERRNO;
                THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
            }
        }
        else
        {
            pChannel->listener.receivedNotify(pChannel->listener.pListener, 
                    ERCD_EPS_SOCKET_TIMEOUT, pChannel->recvBuffer, (uint32)0);
        }
    }
    CATCH
    {
        CloseUdpChannel(pChannel);
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 清除事件队列
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT ClearEventQueue(EpsUdpChannelT* pChannel)
{
    TRY
    {
        EpsUdpChannelEventT* pEvent = NULL;
        while (TRUE)
        {
            THROW_ERROR(PopUniQueue(&pChannel->eventQueue, (void**)&pEvent));

            if (pEvent != NULL)
            {
                free(pEvent);
            }
            else
            {
                break;
            }
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 判断通道是否初始化
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  已经初始化返回TRUE，否则返回FALSE
 */
static BOOL IsChannelInited(EpsUdpChannelT* pChannel)
{
    return (IsUniQueueInited(&pChannel->eventQueue));
}

/**
 * 判断通道是否启动
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  已经启动返回TRUE，否则返回FALSE
 */
static BOOL IsChannelStarted(EpsUdpChannelT* pChannel)
{
    return (pChannel->tid != 0);
}

/**
 * 判断通道是否连接成功
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  已经连接成功返回TRUE，否则返回FALSE
 */
static BOOL IsChannelConnected(EpsUdpChannelT * pChannel)
{
    return (pChannel->socket != INVALID_SOCKET);
}

/*
 * 通道监听回调占位函数
 */
static void OnChannelConnected(void* pListener)
{
}
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason)
{
}
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{
}
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent)
{
}
