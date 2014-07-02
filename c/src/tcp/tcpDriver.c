/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    tcpDriver.c
 *
 * TCP行情驱动器实现文件
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

/**
 * 包含头文件
 */
#include "common.h"
#include "epsTypes.h"
#include "errlib.h"
#include "stepCodec.h"

#include "tcpDriver.h"

/**
 * 内部函数申明
 */

static void OnChannelConnected(void* pListener);
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason);
static void OnChannelReceived(void* pListener, ResCodeT rc, const char* data, uint32 dataLen);
static void OnChannelSended(void* pListener, ResCodeT rc, const char* data, uint32 dataLen);

static void OnEpsConnected(uint32 hid);
static void OnEpsDisconnected(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsLoginRsp(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason);
static void OnEpsLogoutRsp(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsMktDataSubRsp(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason);
static void OnEpsMktDataArrived(uint32 hid, const EpsMktDataT* pMktData);
static void OnEpsEventOccurred(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText);

static ResCodeT HandleLoginRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);
static ResCodeT HandleLogoutRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);
static ResCodeT HandleMDSubscribeRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);
static ResCodeT HandleMarketData(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);
static ResCodeT HandleReceiveTimeout(EpsTcpDriverT* pDriver);

static ResCodeT BuildLoginRequest(uint64 msgSeqNum, const char* username, const char* password, 
            uint16 heartbeatIntl, char* data, int32* pDataLen);
static ResCodeT BuildLogoutRequest(uint64 msgSeqNum, const char* reason, char* data, int32* pDataLen);
static ResCodeT BuildSubscribeRequest(uint64 msgSeqNum, EpsMktTypeT mktType, char* data, int32* pDataLen);
static ResCodeT BuildHeartbeatRequest(uint64 msgSeqNum, char* data, int32* pDataLen);
    
static ResCodeT ParseAddress(const char* address, char* srvAddr, uint16* srvPort);
static ResCodeT GetSendingTime(char* szSendingTime);


/**
 * 函数实现
 */

/**
 *  初始化TCP驱动器
 *
 * @param   pDriver             in  - TCP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT InitTcpDriver(EpsTcpDriverT* pDriver)
{
    TRY
    {
        THROW_ERROR(InitTcpChannel(&pDriver->channel));
        THROW_ERROR(InitMktDatabase(&pDriver->database));
    
        EpsTcpChannelListenerT listener =
        {
            pDriver,
            OnChannelConnected,
            OnChannelDisconnected,
            OnChannelReceived,
            OnChannelSended
        };
        THROW_ERROR(RegisterTcpChannelListener(&pDriver->channel, &listener));

        EpsClientSpiT spi =
        {
            OnEpsConnected,
            OnEpsDisconnected,
            OnEpsLoginRsp,
            OnEpsLogoutRsp,
            OnEpsMktDataSubRsp,
            OnEpsMktDataArrived,
            OnEpsEventOccurred
        };
        pDriver->spi = spi;
       
        pDriver->status = EPS_TCP_STATUS_DISCONNECTED;
        pDriver->msgSeqNum = 1;
        pDriver->recvBufferLen = 0;

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
 * 反初始化TCP驱动器
 *
 * @param   pDriver             in  - TCP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UninitTcpDriver(EpsTcpDriverT* pDriver)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        UninitTcpChannel(&pDriver->channel);
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
 * 注册TCP驱动器回调函数集
 *
 * @param   pDriver             in  - TCP驱动器
 * @param   pSpi                in  - 用户回调接口
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT RegisterTcpDriverSpi(EpsTcpDriverT* pDriver, const EpsClientSpiT* pSpi)
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
 * 建立TCP服务器连接
 *
 * @param   pDriver             in  - TCP驱动器
 * @param   address             in  - 连接地址
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT ConnectTcpDriver(EpsTcpDriverT* pDriver, const char* address)
{
    char        srvAddr[EPS_IP_MAX_LEN+1];
    uint16      srvPort;

    TRY
    {
        LockRecMutex(&pDriver->lock);
        memcpy(srvAddr, pDriver->channel.srvAddr, sizeof(srvAddr));
        srvPort = pDriver->channel.srvPort;

        THROW_ERROR(ParseAddress(address, pDriver->channel.srvAddr, &pDriver->channel.srvPort));
        THROW_ERROR(StartupTcpChannel(&pDriver->channel));
    }
    CATCH
    {
        if(GET_RESCODE() == ERCD_EPS_DUPLICATE_CONNECT)
        {
            memcpy(pDriver->channel.srvAddr, srvAddr, sizeof(srvAddr));
            pDriver->channel.srvPort = srvPort;
        }
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * 断开TCP服务器连接
 *
 * @param   pDriver             in  - TCP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT DisconnectTcpDriver(EpsTcpDriverT* pDriver)
{
    TRY
    {
        ResCodeT rc = NO_ERR;
        
        LockRecMutex(&pDriver->lock);
        
        rc = ShutdownTcpChannel(&pDriver->channel);

        UnlockRecMutex(&pDriver->lock);

        THROW_ERROR(rc);
        THROW_ERROR(JoinTcpChannel(&pDriver->channel));
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
 * 登陆TCP驱动器 
 * 
 * @param   pDriver             in  - TCP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT LoginTcpDriver(EpsTcpDriverT* pDriver, const char* username, 
    const char* password, uint16 heartbeatIntl)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        EpsTcpStatusT status = pDriver->status;
        if (status != EPS_TCP_STATUS_CONNECTED)
        {
            char errorText[128];
            snprintf(errorText, sizeof(errorText), 
                "login operation disallowed in current status(%d)", status); 
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION, errorText);
        }

        snprintf(pDriver->username, sizeof(pDriver->username), username);
        snprintf(pDriver->password, sizeof(pDriver->password), username);
        pDriver->heartbeatIntl = heartbeatIntl;
        char data[STEP_MSG_MAX_LEN];
        int32 dataLen = (int32)sizeof(data);
        THROW_ERROR(BuildLoginRequest(pDriver->msgSeqNum++, 
            username, password, heartbeatIntl, data, &dataLen));

        pDriver->status = EPS_TCP_STATUS_LOGGING;
        THROW_ERROR(SendTcpChannel(&pDriver->channel, data, dataLen));
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
 * 登出TCP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT LogoutTcpDriver(EpsTcpDriverT* pDriver, const char* reason)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        EpsTcpStatusT status = pDriver->status;
        if (status != EPS_TCP_STATUS_LOGINED && status != EPS_TCP_STATUS_PUBLISHING)
        {
            char errorText[128];
            snprintf(errorText, sizeof(errorText), 
                "logout operation disallowed in current status(%d)", status); 
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION, errorText);   
        }

        char data[STEP_MSG_MAX_LEN];
        int32 dataLen = (int32)sizeof(data);
        THROW_ERROR(BuildLogoutRequest(pDriver->msgSeqNum++, reason, data, &dataLen));
        
        pDriver->status = EPS_TCP_STATUS_LOGOUTING;
        THROW_ERROR(SendTcpChannel(&pDriver->channel, data, dataLen));
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
 * 订阅TCP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT SubscribeTcpDriver(EpsTcpDriverT* pDriver, EpsMktTypeT mktType)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);
        EpsTcpStatusT status = pDriver->status;

        if (status != EPS_TCP_STATUS_LOGINED && status != EPS_TCP_STATUS_PUBLISHING)
        {
            char errorText[128];
            snprintf(errorText, sizeof(errorText), 
                "subscribe operation disallowed in current status(%d)", status); 
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION, errorText);
        }

        THROW_ERROR(SubscribeMktData(&pDriver->database, mktType));

        char data[STEP_MSG_MAX_LEN];
        int32 dataLen = (int32)sizeof(data);
        THROW_ERROR(BuildSubscribeRequest(pDriver->msgSeqNum++, mktType, data, &dataLen));

        THROW_ERROR(SendTcpChannel(&pDriver->channel, data, dataLen));
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
 * TCP通道连接成功通知
 *
 * @param   pListener             in  - TCP驱动器
 */
static void OnChannelConnected(void* pListener)
{
    EpsTcpDriverT* pDriver = (EpsTcpDriverT*)pListener;

    LockRecMutex(&pDriver->lock);

    pDriver->status = EPS_TCP_STATUS_CONNECTED;

    pDriver->spi.connectedNotify(pDriver->hid);

    UnlockRecMutex(&pDriver->lock);
}

/**
 * TCP通道连接断开通知
 *
 * @param   pListener           in  - TCP驱动器
 * @param   result              in  - 断开错误码
 * @param   reason              in  - 断开原因描述
 */
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason)
{
    EpsTcpDriverT* pDriver = (EpsTcpDriverT*)pListener;

    LockRecMutex(&pDriver->lock);
    
    pDriver->status = EPS_TCP_STATUS_DISCONNECTED;
    pDriver->msgSeqNum = 1;
    pDriver->recvBufferLen = 0;

    UnsubscribeAllMktData(&pDriver->database);
    
    pDriver->spi.disconnectedNotify(pDriver->hid, result, reason);

    UnlockRecMutex(&pDriver->lock);
}

/**
 * TCP通道数据接收通知
 *
 * @param   pListener           in  - TCP驱动器
 * @param   result              in  - 接收结果码
 * @param   data                in  - 接收数据
 * @param   dataLen             in  - 接收数据长度
 */
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{

    EpsTcpDriverT* pDriver = (EpsTcpDriverT*)pListener;

    TRY
    {
        LockRecMutex(&pDriver->lock);

        if (OK(result))
        {
            memcpy(pDriver->recvBuffer + pDriver->recvBufferLen, data, dataLen);
            pDriver->recvBufferLen += dataLen;

            ResCodeT rc = NO_ERR;
            StepMessageT msg;
            int32 decodeSize = 0;
            uint32 pickupLen = 0;
            while (TRUE)
            {
                rc = DecodeStepMessage(pDriver->recvBuffer + pickupLen, 
                    pDriver->recvBufferLen - pickupLen, &msg, &decodeSize);
                if (NOTOK(rc))
                {
                    if (rc == ERCD_STEP_STREAM_NOT_ENOUGH)
                    {
                        ErrClearError();
                        break;
                    }

                    THROW_ERROR(rc);
                }

                pickupLen += decodeSize;

                switch (msg.msgType)
                {
                    case STEP_MSGTYPE_LOGON:
                        THROW_ERROR(HandleLoginRsp(pDriver, &msg));
                        break;
                    case STEP_MSGTYPE_LOGOUT:
                        THROW_ERROR(HandleLogoutRsp(pDriver, &msg));
                        break;
                    case STEP_MSGTYPE_MD_REQUEST:
                        THROW_ERROR(HandleMDSubscribeRsp(pDriver, &msg));
                        break;
                    case STEP_MSGTYPE_MD_SNAPSHOT:
                        THROW_ERROR(HandleMarketData(pDriver, &msg));
                        break;
                    case STEP_MSGTYPE_HEARTBEAT:
                    case STEP_MSGTYPE_TRADING_STATUS:
                        break;
                    default:
                        THROW_ERROR(ERCD_EPS_UNEXPECTED_MSGTYPE);
                        break;
                }
                pDriver->recvIdleTimes = 0;
                pDriver->commIdleTimes = 0;
            }

            if (pDriver->recvBufferLen > pickupLen && pickupLen > 0)
    		{
    			memmove(pDriver->recvBuffer, pDriver->recvBuffer+pickupLen, pDriver->recvBufferLen-pickupLen);
    		}

            pDriver->recvBufferLen -= pickupLen;
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
        CloseTcpChannel(&pDriver->channel);
        OnChannelDisconnected(pListener, ErrGetErrorCode(), ErrGetErrorDscr());
        ErrClearError();
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);
        SET_RESCODE(GET_RESCODE());
    }
}

/**
 * TCP通道数据发送通知
 *
 * @param   pListener           in  - TCP驱动器
 * @param   result              in  - 发送结果码
 * @param   data                in  - 发送数据
 * @param   dataLen             in  - 发送数据长度
 */
static void OnChannelSended(void* pListener, ResCodeT rc, const char* data, uint32 dataLen)
{
}

/**
 * 处理登陆应答
 *
 * @param   pDriver             in  - TCP驱动器
 * @param   pMsg                in  - 登陆应答消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT HandleLoginRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        pDriver->status = EPS_TCP_STATUS_LOGINED;

        LogonRecordT* pRecord = (LogonRecordT*)pMsg->body;
        pDriver->heartbeatIntl = pRecord->heartBtInt;
        
        pDriver->spi.loginRspNotify(pDriver->hid, pRecord->heartBtInt,
            NO_ERR, "login succeed");
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
 * 处理登出应答
 *
 * @param   pDriver             in  - TCP驱动器
 * @param   pMsg                in  - 登出应答消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT HandleLogoutRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        pDriver->status = EPS_TCP_STATUS_LOGOUT;

        UnsubscribeAllMktData(&pDriver->database);
        
        LogoutRecordT* pRecord = (LogoutRecordT*)pMsg->body;

        EpsTcpStatusT status = pDriver->status;
        switch (status)
        {
            case EPS_TCP_STATUS_LOGGING:
                pDriver->spi.loginRspNotify(pDriver->hid, pDriver->heartbeatIntl,
                    ERCD_EPS_LOGIN_FAILED, pRecord->text);
                break;
            case EPS_TCP_STATUS_LOGOUTING:
                pDriver->spi.logoutRspNotify(pDriver->hid, NO_ERR, pRecord->text);
                break;
            case EPS_TCP_STATUS_PUBLISHING:
                pDriver->spi.mktDataSubRspNotify(pDriver->hid, EPS_MKTTYPE_ALL,
                    ERCD_EPS_SUBMARKETDATA_FAILED, pRecord->text);
                break;
            default:
                pDriver->spi.logoutRspNotify(pDriver->hid, NO_ERR, pRecord->text);
 
                break;
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
 * 处理行情订阅应答
 *
 * @param   pDriver             in  - TCP驱动器
 * @param   pMsg                in  - 行情订阅应答消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT HandleMDSubscribeRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        MDRequestRecordT* pRecord = (MDRequestRecordT*)pMsg->body;
     
        EpsMktTypeT mktType = (EpsMktTypeT)atoi(pRecord->securityType);

        pDriver->status = EPS_TCP_STATUS_PUBLISHING;

        pDriver->spi.mktDataSubRspNotify(pDriver->hid, mktType, NO_ERR, "subscribe succeed");
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
 * 处理行情数据
 *
 * @param   pDriver             in  - TCP驱动器
 * @param   pMsg                in  - 行情数据消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT HandleMarketData(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        ResCodeT rc = AcceptMktData(&pDriver->database, pMsg);
        if (NOTOK(rc))
        {
            if (rc == ERCD_EPS_DATASOURCE_CHANGED)
            {
                pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
                ErrClearError();
            }
            else if (rc == ERCD_EPS_MKTTYPE_UNSUBSCRIBED)
            {
                ErrClearError();
                THROW_RESCODE(NO_ERR);
            }
            else
            {
                THROW_RESCODE(rc);
            }
        }

        EpsMktDataT mktData;
        THROW_ERROR(ConvertMktData(pMsg, &mktData));

        pDriver->spi.mktDataArrivedNotify(pDriver->hid, &mktData);
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
 * 处理数据接收超时
 *
 * @param   pDriver             in  - TCP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT HandleReceiveTimeout(EpsTcpDriverT* pDriver)
{
    TRY
    {
        if (pDriver->status != EPS_TCP_STATUS_LOGINED && 
            pDriver->status != EPS_TCP_STATUS_PUBLISHING)
        {
            THROW_RESCODE(NO_ERR);
        }
        
        pDriver->recvIdleTimes++;
        pDriver->commIdleTimes++;
        
        if ((pDriver->commIdleTimes * EPS_SOCKET_RECV_TIMEOUT) >= (pDriver->heartbeatIntl * 1000))
        {
            char data[STEP_MSG_MAX_LEN];
            int32 dataLen = (int32)sizeof(data);
            THROW_ERROR(BuildHeartbeatRequest(pDriver->msgSeqNum++, data, &dataLen));

            THROW_ERROR(SendTcpChannel(&pDriver->channel, data, dataLen));

            pDriver->commIdleTimes = 0;
        }

        if ((pDriver->recvIdleTimes * EPS_SOCKET_RECV_TIMEOUT) >= EPS_DRIVER_KEEPALIVE_TIME)
        {
            ErrSetError(ERCD_EPS_CHECK_KEEPALIVE_TIMEOUT);
                    
            pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, 
                GET_RESCODE(), ErrGetErrorDscr());

            pDriver->recvIdleTimes = 0;

            ErrClearError();
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
 * 创建登陆请求消息
 *
 * @param   msgSeqNum           in  - 消息序号
 * @param   username            in  - 登陆用户名
 * @param   password            in  - 登陆密码
 * @param   heartbeatIntl       in  - 心跳间隔
 * @param   data                out - 创建的消息缓冲区
 * @param   pDataLen            in  - 消息缓冲区长度
 *                              out - 创建的消息长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT BuildLoginRequest(uint64 msgSeqNum, const char* username, const char* password, 
        uint16 heartbeatIntl, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_LOGON;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);

        LogonRecordT* pRecord = (LogonRecordT*)msg.body;
        pRecord->encryptMethod = STEP_ENCRYPT_METHOD_NONE;
        pRecord->heartBtInt = heartbeatIntl;
        snprintf(pRecord->username, sizeof(pRecord->username), username);
        snprintf(pRecord->password, sizeof(pRecord->password), password);

        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
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
 * 创建登出请求消息
 *
 * @param   msgSeqNum           in  - 消息序号
 * @param   reason              in  - 登出原因
 * @param   data                out - 创建的登陆消息
 * @param   pDataLen            in  - 消息缓冲区长度
 *                              out - 创建的消息长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT BuildLogoutRequest(uint64 msgSeqNum, const char* reason, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_LOGOUT;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);
            
        LogoutRecordT* pRecord = (LogoutRecordT*)msg.body;
        snprintf(pRecord->text, sizeof(pRecord->text), reason);
    
        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
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
 * 创建订阅请求消息
 *
 * @param   msgSeqNum           in  - 消息序号
 * @param   mktType             in  - 订阅市场类型
 * @param   data                out - 创建的登陆消息
 * @param   pDataLen            in  - 消息缓冲区长度
 *                              out - 创建的消息长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT BuildSubscribeRequest(uint64 msgSeqNum, EpsMktTypeT mktType, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_MD_REQUEST;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);
            
        MDRequestRecordT* pRecord = (MDRequestRecordT*)msg.body;
        snprintf(pRecord->securityType, sizeof(pRecord->securityType), "%02d", mktType);
     
        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
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
 * 创建心跳请求消息
 *
 * @param   msgSeqNum           in  - 消息序号
 * @param   data                out - 创建的登陆消息
 * @param   pDataLen            in  - 消息缓冲区长度
 *                              out - 创建的消息长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT BuildHeartbeatRequest(uint64 msgSeqNum, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_HEARTBEAT;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);
            
        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
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
 * 获取行情发送时间
 *
 * @param   szSendingTime   in  - 发送时间缓冲区
 *
 * @return 返回格式为HHMMSSss的当前时间
 */
static ResCodeT GetSendingTime(char* szSendingTime)
{
    TRY
    {
#if defined(__WINDOWS__)
        time_t tt = time(NULL);
    	struct tm* pNowTime = localtime(&tt);
        sprintf(szSendingTime, "%02d%02d%02d%02ld",
             pNowTime->tm_hour, pNowTime->tm_min, pNowTime->tm_sec, (long)0);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        struct tm nowTime;
    	struct timeval tv_time;
        
        gettimeofday(&tv_time, NULL);
        localtime_r((time_t *)&(tv_time.tv_sec), &nowTime);

        sprintf(szSendingTime, "%02d%02d%02d%02ld",
            nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec, (long)0);
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
 * 解析地址字符串
 *
 * @param   address                 in  - 地址字符串
 * @param   srvAddr                 out - 组播地址
 * @param   srvPort                 out - 组播端口
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT ParseAddress(const char* address, char* srvAddr, uint16* srvPort)
{
    TRY
    {
        /* 字符串格式为196.123.71.3:3333 */
        const char* p = strstr(address, ":");
        if (p == NULL || p == address)
        {
            THROW_ERROR(ERCD_EPS_INVALID_ADDRESS);
        }

        memcpy(srvAddr, address, (p-address));
        *srvPort = atoi(p + 1);
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
static void OnEpsEventOccurred(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText)
{
}

