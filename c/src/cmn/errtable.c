/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    errtable.c
 *
 * 错误码表实现文件
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Jin Chengxun
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
17-FEB-2014 CXJIN           创建
================================================================================
</pre>
*/

/**
 * 包含头文件
 */

#include "common.h"
#include "epsTypes.h"
#include "errlib.h"
#include "errcode.h"

#include "errtable.h"

/**
 * 全局定义
 */
 
/* 
 * 错误码表 
 */
const ErrorInfoT  g_errorTable[] =
{
    {ERCD_EPS_OPERSYSTEM_ERROR, "operation system error, %s"},
    {ERCD_EPS_SOCKET_ERROR, "socket error, %s"},
    {ERCD_EPS_SOCKET_TIMEOUT, "socket timeout"},
    {ERCD_EPS_INVALID_PARM, "invalid param (%s)"},
    {ERCD_EPS_DUPLICATE_INITED, "%s already initialized"},
    {ERCD_EPS_UNINITED, "%s uninitialized"},
    {ERCD_EPS_INVALID_CONNMODE, "invalid connection mode"},
    {ERCD_EPS_INVALID_HID, "invalid hid"},
    {ERCD_EPS_DUPLICATE_REGISTERED, "%s already registered"},
    {ERCD_EPS_INVALID_MKTTYPE, "invalid market type"},
    {ERCD_EPS_UNEXPECTED_MSGTYPE, "unexpected message type(%s)"},
    {ERCD_EPS_INVALID_ADDRESS, "invalid address"},
    {ERCD_EPS_DUPLICATE_CONNECT, "connect already"},
    {ERCD_EPS_MKTTYPE_UNSUBSCRIBED, "market not subscribed"},
    {ERCD_EPS_MKTDATA_BACKFLOW, "market data backflow"},
    {ERCD_EPS_DATASOURCE_CHANGED, "data source changed"},
    {ERCD_EPS_MKTTYPE_DUPSUBSCRIBED, "market duplicate subscribed"},
    {ERCD_EPS_INVALID_OPERATION, "invalid operation, %s"},
    {ERCD_EPS_LOGIN_FAILED, "login failed"},
    {ERCD_EPS_SUBMARKETDATA_FAILED, "subscribe market data failed"},
    {ERCD_EPS_CHECK_KEEPALIVE_TIMEOUT, "check keepalive timeout"},
    {ERCD_EPS_HID_COUNT_BEYOND_LIMIT, "handle count beycound limit(%d)"},
    {ERCD_EPS_MKTSTATUS_UNCHANGED, "market status unchanged"},
    
    {ERCD_STEP_INVALID_FLDVALUE, "Invalid field value(%d=%.*s), %s"},
    {ERCD_STEP_BUFFER_OVERFLOW, "Step message buffer overflow"},
    {ERCD_STEP_INVALID_FLDFORMAT, "Invalid field format"},
    {ERCD_STEP_INVALID_TAG, "Invalid field tag(%*.s))"},
    {ERCD_STEP_INVALID_MSGTYPE, "Invalid message type(%s)"},
    {ERCD_STEP_FLD_NOTFOUND, "Field(tag=%s) not found"},
    {ERCD_STEP_STREAM_NOT_ENOUGH, "Message stream not enough"},
    {ERCD_STEP_INVALID_MSGFORMAT, "Invalid message format, %s"},
    {ERCD_STEP_CHECKSUM_FAILED, "Checksum validate failed, %3s(got) != %3s(expected)"},
    {ERCD_STEP_UNEXPECTED_TAG, "Unexpected field tag(%d))"},
};

/**
 * 加载错误码表
 */
ResCodeT EpsLoadErrorTable()
{
    TRY
    {
        ErrLoadErrorTable(g_errorTable, sizeof(g_errorTable)/sizeof(g_errorTable[0]));
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}