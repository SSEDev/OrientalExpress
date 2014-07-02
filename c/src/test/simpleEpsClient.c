/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    simpleEpsClient.c
 *
 * Express接口API简单测试程序(仅适用于UDP)
 *
 * @version $Id
 * @since   2014/03/05
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
05-MAR-2014 ZHENGWU         创建
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


/**
 * 函数实现
 */

static void Usage()
{
    printf("Usage: epsSimple <mcAddr:mcPort;localAddr>\n\n" \
           "example:\n" \
           "epsSimple \"230.11.1.1:3300;196.123.71.3\"\n");
}

static void OnEpsConnectedTest(uint32 hid)
{
    printf("==> OnConnected(), hid: %d\n", hid);
}

static void OnEpsDisconnectedTest(uint32 hid, ResCodeT result, const char* reason)
{
    printf("==> OnDisconnected(), hid: %d, reason: %s\n", hid, reason);
}

static void OnEpsMktDataArrivedTest(uint32 hid, const EpsMktDataT* pMktData)
{
#if defined (__LINUX__) || defined (__HPUX__)
    printf("==> OnMktDataArrived(), hid: %d, applID: %d, applSeqNum: %lld\n", hid, pMktData->applID, pMktData->applSeqNum);
#endif

#if defined (__WINDOWS__)
    printf("==> OnMktDataArrived(), hid: %d, applID: %d, applSeqNum: %I64d\n", hid, pMktData->applID, pMktData->applSeqNum);
#endif
}

static void OnEpsEventOccurredTest(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText)
{
    printf("==> OnEventOccurred(), hid: %d, eventType: %d, eventCode: %u, eventText: %s\n", 
        hid, eventType, eventCode, eventText);
}



int main(int argc, char *argv[])
{
    TRY
    {
        ResCodeT rc = NO_ERR;

        setvbuf(stdout, NULL, _IONBF, 0); /* 设置标准输出为非行缓冲模式 */

        if (argc < 2)
        {
            Usage();
            exit(0);
        }

#if defined(__WINDOWS__)
        WSADATA wsda;
        WSAStartup( MAKEWORD(2, 2), &wsda);
#endif

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
        rc = EpsCreateHandle(&hid, EPS_CONNMODE_UDP);
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
            NULL,
            NULL,
            NULL,
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
            printf("OK. hid :%d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", EpsGetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsSubscribeMarketData() ... ");
        rc = EpsSubscribeMarketData(hid, EPS_MKTTYPE_ALL);
        if (OK(rc))
        {
            printf("OK. hid :%d\n", hid);
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
            printf("OK. hid :%d\n", hid);
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
            printf("OK. hid :%d\n", hid);
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
        printf(">>> epsClientTest stop.\n");
        return (OK(GET_RESCODE()) ? 0 : (0 - GET_RESCODE()));
    }
}
