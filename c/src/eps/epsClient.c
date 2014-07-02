/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsClient.c
 *
 * Express接口API实现文件
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
17-FEB-2014 ZHENGWU         创建
================================================================================
</pre>
*/

/**
 * 包含头文件
 */

#include "common.h"
#include "epsTypes.h"
#include "errlib.h"
#include "errtable.h"
#include "atomic.h"
#include "recMutex.h"
#include "udpDriver.h"
#include "tcpDriver.h"

#include "epsClient.h"


/**
 * 宏定义
 */

#define EPS_HANDLE_MAX_COUNT            32  /* 句柄最大个数 */


/**
 * 类型定义
 */

/*
 * 句柄类型
 */
typedef struct EpsHandleTag
{
    uint32          hid;        /* 句柄ID */
    EpsConnModeT    connMode;   /* 连接模式 */
    union EpsDriverTag
    {
        EpsUdpDriverT udpDriver;
        EpsTcpDriverT tcpDriver;
    } driver;                   /* 驱动器 */
} EpsHandleT;


/**
 * 全局定义
 */
 
static volatile int    g_isLibInited = FALSE;   /* 库初始化标记 */
static EpsHandleT      g_handlePool[EPS_HANDLE_MAX_COUNT];/* 句柄池 */
static EpsRecMutexT    g_libLock;               /* 库同步对象 */


/**
 * 内部函数申明
 */

static BOOL IsLibInited();


static ResCodeT InitHandlePool();
static ResCodeT UninitHandlePool();
static ResCodeT GetNewHandle(EpsHandleT** ppHandle);
static ResCodeT FindHandle(uint32 hid, EpsHandleT** ppHandle);

static void DisconnectHandle(EpsHandleT* pHandle);
static void DestroyHandle(EpsHandleT* pHandle);


/**
 * 接口函数定义
 */

/**
 * 初始化Express库
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsInitLib()
{
    TRY
    {
        if (EpsAtomicIntCompareAndExchange(&g_isLibInited, FALSE, TRUE))
        {
            EpsLoadErrorTable();
            
            InitHandlePool();
            InitRecMutex(&g_libLock);
        }
        else
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "library");
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
 * 反初始化Express库
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsUninitLib()
{
    TRY
    {
        if (EpsAtomicIntCompareAndExchange(&g_isLibInited, TRUE, FALSE))
        {
            LockRecMutex(&g_libLock);
            UninitHandlePool();
            UnlockRecMutex(&g_libLock);

            UninitRecMutex(&g_libLock);
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
 * 创建指定连接模式的操作句柄
 *
 * @param   pHid            out - 创建的句柄ID
 * @param   mode            in  - 连接模式
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsCreateHandle(uint32* pHid, EpsConnModeT mode)
{
    EpsHandleT* pHandle = NULL;
    
    TRY
    {
        if (pHid == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pHid");
        }

        if (mode != EPS_CONNMODE_UDP && mode != EPS_CONNMODE_TCP)
        {
            THROW_ERROR(ERCD_EPS_INVALID_CONNMODE);
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = GetNewHandle(&pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

        pHandle->connMode = mode;
        if (mode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            pDriver->hid = pHandle->hid;
            THROW_ERROR(InitUdpDriver(pDriver));
        }
        else /* mode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            pDriver->hid = pHandle->hid;
            THROW_ERROR(InitTcpDriver(pDriver));
        }

        *pHid = pHandle->hid;
    }
    CATCH
    {
        if (pHandle != NULL)
        {
            DestroyHandle(pHandle);
        }
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 销毁句柄
 *
 * @param   hid             in  - 待销毁的句柄ID
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsDestroyHandle(uint32 hid)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        if (OK(rc))
        {
            DisconnectHandle(pHandle);
            DestroyHandle(pHandle);
        }
        UnlockRecMutex(&g_libLock);

        THROW_ERROR(rc);
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
 * 注册用户回调接口
 *
 * @param   hid             in  - 待执行注册操作的句柄ID
 * @param   pSpi            in  - 待执行注册的用户回调接口
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsRegisterSpi(uint32 hid, const EpsClientSpiT* pSpi)
{
    TRY
    {
        if (pSpi == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pSpi");
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);
       
        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(RegisterUdpDriverSpi(pDriver, pSpi));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(RegisterTcpDriverSpi(pDriver, pSpi));
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
 * 连接服务器
 *
 * @param   hid             in  - 待执行连接操作的句柄ID
 * @param   address         in  - 主机地址字符串，例如
 *                                TCP: 196.123.1.1:8000
 *                                UDP: 230.11.1.1:3333;196.123.71.1
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsConnect(uint32 hid, const char* address)
{
    TRY
    {
        if (address == NULL || address[0] == 0x00)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "address");
        }
        
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);
        
        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(ConnectUdpDriver(pDriver, address));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(ConnectTcpDriver(pDriver, address));
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
 * 断开连接服务器
 *
 * @param   hid             in  - 待执行断开连接操作的句柄ID
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsDisconnect(uint32 hid)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(DisconnectUdpDriver(pDriver));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(DisconnectTcpDriver(pDriver));
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
 * 登陆服务器
 *
 * @param   hid             in  - 待执行登陆操作的句柄ID
 * @param   username        in  - 登陆用户名
 * @param   password        in  - 登录密码
 * @param   hearbeatIntl    in  - 心跳间隔
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsLogin(uint32 hid, const char* username, const char* password, uint16 heartbeatIntl)
{
    TRY
    {
        if (username == NULL || username[0] == 0x00)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "username");
        }

        if (password == NULL || password[0] == 0x00)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "password");
        }

        if (heartbeatIntl == 0)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, heartbeatIntl);
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(LoginUdpDriver(pDriver, username, password, heartbeatIntl));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(LoginTcpDriver(pDriver, username, password, heartbeatIntl));
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
 * 登出服务器
 *
 * @param   hid             in  - 待执行登出操作的句柄ID
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsLogout(uint32 hid, const char* reason)
{
    TRY
    {
        if (reason == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "reason");
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(LogoutUdpDriver(pDriver, reason));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(LogoutTcpDriver(pDriver, reason));
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
 * 订阅指定市场类型的行情数据
 *
 * @param   hid             in  - 待执行订阅操作的句柄ID
 * @param   mktType         in  - 待订阅的市场类型
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
int32 EpsSubscribeMarketData(uint32 hid, EpsMktTypeT mktType)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(SubscribeUdpDriver(pDriver, mktType));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(SubscribeTcpDriver(pDriver, mktType));
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
 * 获取最后一条错误信息
 *
 * @return  返回错误信息地址
 */
const char* EpsGetLastError()
{
    return ErrGetErrorDscr();
}

/**
 * 判断库是否被初始化
 *
 * @return 已经初始化返回TRUE，否则返回FALSE
 */
static BOOL IsLibInited()
{
    return (g_isLibInited == TRUE);
}

/**
 * 初始化句柄池
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT InitHandlePool()
{
    TRY
    {
        memset(g_handlePool, 0x00, sizeof(g_handlePool));
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
 * 反初始化句柄池
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT UninitHandlePool()
{
    TRY
    {
        uint32 i = 0;
        for (i = 0; i < EPS_HANDLE_MAX_COUNT; i++)
        {
            if (g_handlePool[i].hid != 0)
            {
                DisconnectHandle(&g_handlePool[i]);
                DestroyHandle(&g_handlePool[i]);
            }
        }

        memset(g_handlePool, 0x00, sizeof(g_handlePool));
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
 * 获取新句柄
 *
 * @param   ppHandle             out  - 获取的新句柄
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT GetNewHandle(EpsHandleT** ppHandle)
{
    TRY
    {
        uint32 i = 0;
        for (i = 0; i < EPS_HANDLE_MAX_COUNT; i++)
        {
            if (g_handlePool[i].hid == 0)
            {
                g_handlePool[i].hid = i + 1;
                *ppHandle = &g_handlePool[i];
                break;
            }
        }

        if (i >= EPS_HANDLE_MAX_COUNT)
        {
            THROW_ERROR(ERCD_EPS_HID_COUNT_BEYOND_LIMIT, EPS_HANDLE_MAX_COUNT);
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
 * 查询句柄
 *
 * @param   hid                  in   - 待查询的句柄ID
 * @param   ppHandle             out  - 查询到的句柄
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT FindHandle(uint32 hid, EpsHandleT** ppHandle)
{
    TRY
    {
        if (hid == 0 || hid > EPS_HANDLE_MAX_COUNT)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (g_handlePool[hid - 1].hid == 0)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        *ppHandle = &g_handlePool[hid - 1];
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
 * 断开句柄
 *
 * @param   pHandle             in  - 待执行断开操作的句柄ID
 */
static void DisconnectHandle(EpsHandleT* pHandle)
{
    if (pHandle->connMode == EPS_CONNMODE_UDP)
    {
        EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
        DisconnectUdpDriver(pDriver);
    }
    else /* connMode == EPS_CONNMODE_TCP */
    {
        EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
        DisconnectTcpDriver(pDriver);
    }
}

/**
 * 销毁句柄
 *
 * @param   pHandle             in  - 待执行销毁操作的句柄ID
 */
static void DestroyHandle(EpsHandleT* pHandle)
{
    if (pHandle->connMode == EPS_CONNMODE_UDP)
    {
        EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
        UninitUdpDriver(pDriver);
    }
    else /* connMode == EPS_CONNMODE_TCP */
    {
        EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
        UninitTcpDriver(pDriver);
    }

    memset(pHandle, 0x00, sizeof(EpsHandleT));
}
