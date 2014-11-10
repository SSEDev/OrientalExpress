/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpDriver.c
 *
 * UDP行情驱动器实现文件
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
04-AUG-2014 ZHENGWU         新增全市场状态消息
================================================================================
</pre>
*/

/**
 * 包含头文件
 */

#include "common.h"
#include "epsTypes.h"
#include "epsData.h"
#include "errlib.h"
#include "stepCodec.h"

#include "udpDriver.h"


/**
 * 类型定义
 */

/*
 * UDP事件类型枚举
 */
typedef enum EpsUdpEventTypeTag
{
    EPS_UDP_EVENTTYPE_LOGIN         = 1,    /* 登陆事件 */
    EPS_UDP_EVENTTYPE_LOGOUT        = 2,    /* 登出事件 */
    EPS_UDP_EVENTTYPE_SUBSCRIBED    = 3,    /* 订阅事件 */
} EpsUdpEventTypeT;


/**
 * 内部函数申明
 */

static void OnChannelConnected(void* pListener);
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason);
static void OnChannelReceived(void* pListener, ResCodeT rc, const char* data, uint32 dataLen);
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent);

static void OnEpsConnected(uint32 hid);
static void OnEpsDisconnected(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsLoginRsp(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason);
static void OnEpsLogoutRsp(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsMktDataSubRsp(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason);
static void OnEpsMktDataArrived(uint32 hid, const EpsMktDataT* pMktData);
static void OnEpsMktStatusChanged(uint32 hid, const EpsMktStatusT* pMktStatus);
static void OnEpsEventOccurred(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText);

static ResCodeT HandleReceiveTimeout(EpsUdpDriverT* pDriver);
static ResCodeT ParseAddress(const char* address, char* mcAddr, uint16* mcPort, char* localAddr);


/**
 * 函数实现
 */

/**
 *  初始化UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT InitUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        THROW_ERROR(InitUdpChannel(&pDriver->channel));
        THROW_ERROR(InitMktDatabase(&pDriver->database));
        
        EpsUdpChannelListenerT listener =
        {
            pDriver,
            OnChannelConnected,
            OnChannelDisconnected,
            OnChannelReceived,
            OnChannelEventOccurred
        };
        THROW_ERROR(RegisterUdpChannelListener(&pDriver->channel, &listener));

        EpsClientSpiT spi =
        {
            OnEpsConnected,
            OnEpsDisconnected,
            OnEpsLoginRsp,
            OnEpsLogoutRsp,
            OnEpsMktDataSubRsp,
            OnEpsMktDataArrived,
            OnEpsMktStatusChanged,
            OnEpsEventOccurred
        };
        pDriver->spi = spi;

        InitRecMutex(&pDriver->lock);
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
 * 反初始化UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UninitUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);
            
        UninitUdpChannel(&pDriver->channel);
        UninitMktDatabase(&pDriver->database);

        UnlockRecMutex(&pDriver->lock);
 
        UninitRecMutex(&pDriver->lock);
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
 *  注册UDP驱动器回调函数集
 *
 * @param   pDriver             in  - UDP驱动器
 * @param   pSpi                in  - 用户回调接口
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT RegisterUdpDriverSpi(EpsUdpDriverT* pDriver, const EpsClientSpiT* pSpi)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        if (pSpi->connectedNotify != NULL)
        {
            pDriver->spi.connectedNotify = pSpi->connectedNotify;
        }
        if (pSpi->disconnectedNotify != NULL)
        {
            pDriver->spi.disconnectedNotify = pSpi->disconnectedNotify;
        }
        if (pSpi->loginRspNotify != NULL)
        {
            pDriver->spi.loginRspNotify = pSpi->loginRspNotify;
        }
        if (pSpi->logoutRspNotify != NULL)
        {
            pDriver->spi.logoutRspNotify = pSpi->logoutRspNotify;
        }
        if (pSpi->mktDataSubRspNotify != NULL)
        {
            pDriver->spi.mktDataSubRspNotify = pSpi->mktDataSubRspNotify;
        }
        if (pSpi->mktDataArrivedNotify != NULL)
        {
            pDriver->spi.mktDataArrivedNotify = pSpi->mktDataArrivedNotify;
        }
        if (pSpi->mktStatusChangedNotify != NULL)
        {
            pDriver->spi.mktStatusChangedNotify = pSpi->mktStatusChangedNotify;
        }
        if (pSpi->eventOccurredNotify != NULL)
        {
            pDriver->spi.eventOccurredNotify = pSpi->eventOccurredNotify;
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * 建立UDP服务器连接
 *
 * @param   pDriver             in  - UDP驱动器
 * @param   address             in  - 连接地址
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT ConnectUdpDriver(EpsUdpDriverT* pDriver, const char* address)
{
    char        localAddr[EPS_IP_MAX_LEN+1];
    char        mcAddr[EPS_IP_MAX_LEN+1];
    uint16      mcPort;

    TRY
    {
        LockRecMutex(&pDriver->lock);
        
        memcpy(localAddr, pDriver->channel.localAddr, sizeof(localAddr));
        memcpy(mcAddr, pDriver->channel.mcAddr, sizeof(mcAddr));
        mcPort = pDriver->channel.mcPort;
   
        THROW_ERROR(ParseAddress(address, pDriver->channel.mcAddr, 
            &pDriver->channel.mcPort, pDriver->channel.localAddr));
        THROW_ERROR(StartupUdpChannel(&pDriver->channel));
    }
    CATCH
    {
        if(GET_RESCODE() == ERCD_EPS_DUPLICATE_CONNECT)
        {
            memcpy(pDriver->channel.localAddr, localAddr, sizeof(localAddr));
            memcpy(pDriver->channel.mcAddr, mcAddr, sizeof(mcAddr));
            pDriver->channel.mcPort = mcPort;
        }
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * 断开UDP服务器连接
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT DisconnectUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        ResCodeT rc = NO_ERR;
        
        LockRecMutex(&pDriver->lock);
        
        rc = ShutdownUdpChannel(&pDriver->channel);

        UnlockRecMutex(&pDriver->lock);

        THROW_ERROR(rc);
        THROW_ERROR(JoinUdpChannel(&pDriver->channel));
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
 * 登陆UDP驱动器 
 * 
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT LoginUdpDriver(EpsUdpDriverT* pDriver, const char* username, 
    const char* password, uint16 heartbeatIntl)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        EpsUdpChannelEventT event = 
        {
            EPS_UDP_EVENTTYPE_LOGIN, 0
        };

        snprintf(pDriver->username, sizeof(pDriver->username), username);
        snprintf(pDriver->password, sizeof(pDriver->password), username);
        pDriver->heartbeatIntl = heartbeatIntl;
        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, event));
    }
    CATCH
    {
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/*
 * 登出UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT LogoutUdpDriver(EpsUdpDriverT* pDriver, const char* reason)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        EpsUdpChannelEventT event = 
        {
            EPS_UDP_EVENTTYPE_LOGOUT, 0
        };

        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, event));
    }
    CATCH
    {
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * 订阅UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT SubscribeUdpDriver(EpsUdpDriverT* pDriver, EpsMktTypeT mktType)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        THROW_ERROR(SubscribeMktData(&pDriver->database, mktType));

        EpsUdpChannelEventT event = 
        {
            EPS_UDP_EVENTTYPE_SUBSCRIBED, (uint32)mktType
        };
        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, event));
    }
    CATCH
    {
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * UDP通道连接成功通知
 *
 * @param   pListener             in  - UDP驱动器
 */
static void OnChannelConnected(void* pListener)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    LockRecMutex(&pDriver->lock);
    pDriver->spi.connectedNotify(pDriver->hid);
    UnlockRecMutex(&pDriver->lock);
}

/**
 * UDP通道连接断开通知
 *
 * @param   pListener           in  - UDP驱动器
 * @param   result              in  - 断开错误码
 * @param   reason              in  - 断开原因描述
 */
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    LockRecMutex(&pDriver->lock);
    UnsubscribeAllMktData(&pDriver->database);
    pDriver->spi.disconnectedNotify(pDriver->hid, result, reason);
    UnlockRecMutex(&pDriver->lock);
}

/**
 * UDP通道数据接收通知
 *
 * @param   pListener           in  - UDP驱动器
 * @param   result              in  - 接收错误码
 * @param   data                in  - 接收数据
 * @param   dataLen             in  - 接收数据长度
 */
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    TRY
    {
        LockRecMutex(&pDriver->lock);

        if (OK(result))
        {
            ResCodeT rc = NO_ERR;
            
            StepMessageT msg;
            int32 decodeSize = 0;
            THROW_ERROR(DecodeStepMessage(data, dataLen, &msg, &decodeSize));

            if (msg.msgType == STEP_MSGTYPE_MD_SNAPSHOT)
            {
                rc = AcceptMktData(&pDriver->database, &msg);
                if (NOTOK(rc))
                {
                    if (rc == ERCD_EPS_DATASOURCE_CHANGED)
                    {
                        pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
                    }
                    else if (rc == ERCD_EPS_MKTTYPE_UNSUBSCRIBED)
                    {
                        THROW_RESCODE(NO_ERR);
                    }
                    else if (rc == ERCD_EPS_MKTDATA_BACKFLOW)
                    {
                        THROW_RESCODE(NO_ERR);
                    }
                    else
                    {
                        THROW_ERROR(rc);
                    }
                }
            
                EpsMktDataT mktData;
                THROW_ERROR(ConvertMktData(&msg, &mktData));

                pDriver->spi.mktDataArrivedNotify(pDriver->hid, &mktData);

                pDriver->recvIdleTimes = 0;
            }
            else if (msg.msgType == STEP_MSGTYPE_TRADING_STATUS)
            {
                rc = AcceptMktStatus(&pDriver->database, &msg);
                if (NOTOK(rc))
                {
                    if (rc == ERCD_EPS_MKTSTATUS_UNCHANGED || rc == ERCD_EPS_MKTTYPE_UNSUBSCRIBED)
                    {
                        THROW_RESCODE(NO_ERR);
                    }
                    else 
                    {
                        THROW_ERROR(rc);
                    }
                }
            
                EpsMktStatusT mktStatus;
                THROW_ERROR(ConvertMktStatus(&msg, &mktStatus));

                pDriver->spi.mktStatusChangedNotify(pDriver->hid, &mktStatus);

                pDriver->recvIdleTimes = 0;
            }
            else
            {
            }
        }
        else
        {
            if (result == ERCD_EPS_SOCKET_TIMEOUT)
            {
                THROW_ERROR(HandleReceiveTimeout(pDriver));
            }
            else 
            {
                THROW_ERROR(result);
            }
        }
    }
    CATCH
    {
        pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_ERROR, ErrGetErrorCode(), ErrGetErrorDscr());

        CloseUdpChannel(&pDriver->channel);
        OnChannelDisconnected(pListener, ErrGetErrorCode(), ErrGetErrorDscr());

        ErrClearError();
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);
    }
}

/**
 * UDP通道数据事件通知
 *
 * @param   pListener               in  - UDP驱动器
 * @param   pEvent                  in  - 事件对象
 */
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;

    LockRecMutex(&pDriver->lock);

    switch (pEvent->eventType)
    {
        case EPS_UDP_EVENTTYPE_LOGIN:
        {
            pDriver->spi.loginRspNotify(pDriver->hid, 
                    pDriver->heartbeatIntl, NO_ERR, "login succeed");
            break;
        }
        case EPS_UDP_EVENTTYPE_LOGOUT:
        {
            UnsubscribeAllMktData(&pDriver->database);
            
            pDriver->spi.logoutRspNotify(pDriver->hid, 
                    NO_ERR, "logout succeed");
            break;
        }
        case EPS_UDP_EVENTTYPE_SUBSCRIBED:
        {
            pDriver->spi.mktDataSubRspNotify(pDriver->hid, 
                    (EpsMktTypeT)(pEvent->eventParam), NO_ERR, "subscribe succeed");
            break;
        }
        default:
            break;
    }

    UnlockRecMutex(&pDriver->lock);
}

/**
 * UDP通道数据接收超时通知
 *
 * @param   pDriver             in  - UDP驱动器
 */
static ResCodeT HandleReceiveTimeout(EpsUdpDriverT* pDriver)
{
    TRY
    {
        pDriver->recvIdleTimes++;
        
        if ((pDriver->recvIdleTimes * EPS_SOCKET_RECV_TIMEOUT) >= EPS_DRIVER_KEEPALIVE_TIME)
        {
            ErrSetError(ERCD_EPS_CHECK_KEEPALIVE_TIMEOUT);
                    
            pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, 
                        GET_RESCODE(), ErrGetErrorDscr());

            pDriver->recvIdleTimes = 0;
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
 * 解析地址字符串
 *
 * @param   address                 in  - 地址字符串
 * @param   mcAddr                  out - 组播地址
 * @param   mcPort                  out - 组播端口
 * @param   localAddr               out - 本地地址
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT ParseAddress(const char* address, char* mcAddr, uint16* mcPort, char* localAddr)
{
    TRY
    {
        /* 字符串格式为230.11.1.1:3333;196.123.71.1 */
        const char* p1, *p2;

        int len = strlen(address);
        
        p1 = strstr(address, ":");
        if (p1 == NULL || p1 == address)
        {
            THROW_ERROR(ERCD_EPS_INVALID_ADDRESS);
        }

        p2 = strstr(p1+1, ";");
        if (p2 == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_ADDRESS);
        }

        memcpy(mcAddr, address, (p1-address));
        memcpy(localAddr, p2+1, (address+len-p2-1));
        *mcPort = atoi(p1 + 1);
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
 * Express SPI占位函数
 */
static void OnEpsConnected(uint32 hid)
{
}
static void OnEpsDisconnected(uint32 hid, ResCodeT result, const char* reason)
{
}
static void OnEpsLoginRsp(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason)
{
}
static void OnEpsLogoutRsp(uint32 hid, ResCodeT result, const char* reason)
{
}
static void OnEpsMktDataSubRsp(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason)
{
}
static void OnEpsMktDataArrived(uint32 hid, const EpsMktDataT* pMktData)
{
}
static void OnEpsMktStatusChanged(uint32 hid, const EpsMktStatusT* pMktStatus)
{
}
static void OnEpsEventOccurred(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText)
{
}

