/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    complexEpsClient.c
 *
 * Express接口API复杂测试程序(适用于TCP和UDP)
 *
 * @version $Id
 * @since   2014/03/18
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
18-MAR-2014 ZHENGWU         创建
================================================================================
</pre>
*/

/**
 * 包含头文件
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "errlib.h"
#include "epsClient.h"


/*
 * 全局定义
 */

static char g_address[EPS_IP_MAX_LEN*2+1];
static char g_username[EPS_USERNAME_MAX_LEN+1];
static char g_password[EPS_PASSWORD_MAX_LEN+1];

/**
 * 函数实现
 */

static void Usage()
{
    printf("Usage: epsComplex <address> <username> <password>\n\n" \
           "example:\n" \
           "epsComplex \"230.11.1.1:3300;196.123.71.3\" username password\n" \
           "epsComplex \"196.123.71.3:3473 username password\"\n");
}

static void OnEpsConnectedTest(uint32 hid)
{
    printf("==> OnConnected(), hid: %d\n", hid);

    printf("==> call EpsLogin() ... ");
    ResCodeT rc = EpsLogin(hid, g_username, g_password, 10);
    if (OK(rc))
    {
        printf("OK. hid :%d\n", hid);
    }
    else
    {
        printf("failed, Error: %s!!!\n", EpsGetLastError());
    }
}

static void OnEpsDisconnectedTest(uint32 hid, ResCodeT result, const char* reason)
{
    printf("==> OnDisconnected(), hid: %d, reason: %s\n", hid, reason);
}

static void OnEpsLoginRspTest(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason)
{
    printf("==> OnEpsLoginRsp %s, hid: %d, reason: %s\n", OK(result) ? "OK" : "failed", hid, reason);

    if (OK(result))
    {
        printf("==> call EpsSubscribeMarketData() ... ");
  
        ResCodeT rc = EpsSubscribeMarketData(hid, EPS_MKTTYPE_ALL);
        if (OK(rc))
        {
            printf("OK. hid :%d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
        }
    }
}

static void OnEpsLogoutRspTest(uint32 hid, ResCodeT result, const char* reason)
{
    printf("==> OnEpsLogoutRsp %s, hid: %d, reason: %s\n", OK(result) ? "OK" : "failed", hid, reason);
}

static void OnEpsMktDataSubRspTest(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason)
{
    printf("==> OnEpsMktDataSubRsp %s, hid: %d, mktType: %d, reason: %s\n", 
        OK(result) ? "OK" : "failed", hid, mktType, reason);
}

static void OnEpsMktDataArrivedTest(uint32 hid, const EpsMktDataT* pMktData)
{
#if defined (__LINUX__) || defined (__HPUX__)
    printf("==> OnMktDataArrived(), hid: %d, applID: %d, applSeqNum: %lld, mdCount: %u\n", hid, pMktData->applID, pMktData->applSeqNum, pMktData->mdCount);
#endif

#if defined (__WINDOWS__)
    printf("==> OnMktDataArrived(), hid: %d, applID: %d, applSeqNum: %I64d, mdCount: %u\n", hid, pMktData->applID, pMktData->applSeqNum, pMktData->mdCount);
#endif
}

static void OnEpsEventOccurredTest(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText)
{
    printf("==> OnEventOccurred(), hid: %d, eventType: %d, eventCode: %x, eventText: %s\n", 
        hid, eventType, eventCode, eventText);
}



int main(int argc, char *argv[])
{
    TRY
    {
        ResCodeT rc = NO_ERR;

        setvbuf(stdout, NULL, _IONBF, 0); /* 设置标准输出为非行缓冲模式 */

        if (argc < 4)
        {
            Usage();
            exit(0);
        }

#if defined(__WIN32__)
        WSADATA wsda;
        WSAStartup( MAKEWORD(2, 2), &wsda);
#endif

        snprintf(g_address, sizeof(g_address), argv[1]);
        snprintf(g_username, sizeof(g_username), argv[2]);
        snprintf(g_password, sizeof(g_password), argv[3]);
        
        printf("\n>>> epsClientTest starting ... \n");

        printf("==> call EpsInitLib() ... ");
        rc = EpsInitLib();
        if (OK(rc))
        {
            printf("OK.\n");
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsCreateHandle() ... ");

        uint32 hid;

        char* p1 = strstr(g_address, ";");
        rc = EpsCreateHandle(&hid, (p1==NULL) ? EPS_CONNMODE_TCP : EPS_CONNMODE_UDP);
        if (OK(rc))
        {
            printf("OK. hid: %d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsRegisterSpi() ... ");
        EpsClientSpiT spi = 
        {
            OnEpsConnectedTest,
            OnEpsDisconnectedTest,
            OnEpsLoginRspTest,
            OnEpsLogoutRspTest,
            OnEpsMktDataSubRspTest,
            OnEpsMktDataArrivedTest,
            OnEpsEventOccurredTest,
        };
 
        rc = EpsRegisterSpi(hid, &spi);
        if (OK(rc))
        {
            printf("OK. hid: %d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsConnect() ... ");
        rc = EpsConnect(hid, argv[1]);
        if (OK(rc))
        {
            printf("OK. hid: %d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }

        while (1)
        {
            int c = getchar();
            if (c == 'q' || c == 'Q')
            {
                printf(">>> receive exit signal ...\n\n");
                break;
            }
        }

        printf("==> call EpsDisconnect() ... ");
        rc = EpsDisconnect(hid);
        if (OK(rc))
        {
            printf("OK. hid: %d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsDestroyHandle() ... ");
        rc = EpsDestroyHandle(hid);
        if (OK(rc))
        {
            printf("OK. hid: %d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsUninitLib() ... ");
        rc = EpsUninitLib();
        if (OK(rc))
        {
            printf("OK.\n");
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }
    }
    CATCH
    {
        
    }
    FINALLY
    {
#if defined(__WINDOWS__)
        WSACleanup();
#endif
        printf(">>> epsClientTest stop.\n");
        return (OK(GET_RESCODE()) ? 0 : (0 - GET_RESCODE()));
    }
}
