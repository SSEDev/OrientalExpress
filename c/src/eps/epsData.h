/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsData.h
 *
 * 东方快车接口API数据定义头文件
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
04-AUG-2014 ZHENGWU         新增全市场状态消息
================================================================================
</pre>
*/

#ifndef EPS_DATA_H
#define EPS_DATA_H

/**
 * 包含头文件
 */
 
#include "epsTypes.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * 宏定义
 */

/*
 * 字段长度定义
 */
#define EPS_TIME_LEN                    8
#define EPS_DATE_LEN                    8
#define EPS_UPDATETYPE_LEN              3
#define EPS_MKTDATA_MAX_LEN             4096
#define EPS_USERNAME_MAX_LEN            10
#define EPS_PASSWORD_MAX_LEN            10
#define EPS_MKTSTATUS_LEN               8


/**
 * 类型定义
 */

/*
 * 连接模式枚举
 */
typedef enum EpsConnModeTag
{
    EPS_CONNMODE_UDP        = 1,        /* UDP连接模式 */
    EPS_CONNMODE_TCP        = 2,        /* TCP连接模式 */
} EpsConnModeT;

/*
 * 市场类型枚举
 */
typedef enum EpsMktTypeTag
{
    EPS_MKTTYPE_ALL         = 0,        /* 所有市场 */
    EPS_MKTTYPE_STK         = 1,        /* 股票市场(含指数) */
    EPS_MKTTYPE_DEV         = 2,        /* 衍生品市场 */
    EPS_MKTTYPE_NUM         = 2,        /* 支持市场数量 */
} EpsMktTypeT;

/*
 * 交易模式枚举
 */
typedef enum EpsTrdSesModeTag
{
    EPS_TRDSES_MODE_TESTING     = 1,    /* 系统测试模式 */
    EPS_TRDSES_MODE_SIMULATED   = 2,    /* 模拟交易模式 */
    EPS_TRDSES_MODE_PRODUCTION  = 3,    /* 生产模式 */
} EpsTrdSesModeT;

/*
 * 事件类型枚举
 */
typedef enum EpsEventTypeTag
{
    EPS_EVENTTYPE_INFORMATION   = 1,    /* 提示信息类型 */
    EPS_EVENTTYPE_WARNING       = 2,    /* 警告信息类型 */
    EPS_EVENTTYPE_ERROR         = 3,    /* 错误信息类型 */
    EPS_EVENTTYPE_FATAL         = 4,    /* 严重错误信息类型 */
} EpsEventTypeT;

/*
 * 行情数据结构
 */
typedef struct EpsMktDataTag
{
    char    mktTime[EPS_TIME_LEN+1];    /* HHMMSSss格式 */
    EpsMktTypeT mktType;                /* 市场类型 */
    EpsTrdSesModeT tradSesMode;         /* 交易模式 */                
    uint32  applID;                     /* 发布源ID */
    uint64  applSeqNum;                 /* 发布数据序号 */
    char    tradeDate[EPS_DATE_LEN+1];  /* YYYYMMDD格式 */
    char    mdUpdateType[EPS_UPDATETYPE_LEN+1];/* 行情更新模式 */
    uint32  mdCount;                    /* 行情条目数量 */
    uint32  mdDataLen;                  /* 行情数据长度 */
    char    mdData[EPS_MKTDATA_MAX_LEN];/* 行情数据 */
} EpsMktDataT;

/*
 * 市场状态消息
 */
typedef struct EpsMktStatusTag
{
    EpsMktTypeT mktType;                /* 市场类型 */
    EpsTrdSesModeT tradSesMode;         /* 交易模式 */    
    char    mktStatus[EPS_MKTSTATUS_LEN+1];/* 市场状态 */
    uint32  totNoRelatedSym;            /* 市场产品总数 */
} EpsMktStatusT;


/*
 * 用户回调接口函数类型
 */
typedef void (*EpsConnectedCallback)(uint32 hid);
typedef void (*EpsDisconnectedCallback)(uint32 hid, int32 result, const char* reason);
typedef void (*EpsLoginRspCallback)(uint32 hid, uint16 heartbeatIntl, int32 result, const char* reason);
typedef void (*EpsLogoutRspCallback)(uint32 hid, int32 result, const char* reason);
typedef void (*EpsMktDataSubRspCallback)(uint32 hid, EpsMktTypeT mktType, int32 result, const char* reason);
typedef void (*EpsMktDataArrivedCallback)(uint32 hid, const EpsMktDataT* pMktData);
typedef void (*EpsMktStatusChangedCallback)(uint32 hid, const EpsMktStatusT* pMktStatus);
typedef void (*EpsEventOccurredCallback)(uint32 hid, EpsEventTypeT eventType, int32 eventCode, const char* eventText);

/*
 * 用户回调接口
 */
typedef struct EpsClientSpiTag
{
    EpsConnectedCallback        connectedNotify;     /* 连接成功通知 */
    EpsDisconnectedCallback     disconnectedNotify;  /* 连接断开通知 */
    EpsLoginRspCallback         loginRspNotify;      /* 登陆应答通知 */
    EpsLogoutRspCallback        logoutRspNotify;     /* 登出应答通知 */
    EpsMktDataSubRspCallback    mktDataSubRspNotify; /* 行情订阅应答通知 */
    EpsMktDataArrivedCallback   mktDataArrivedNotify;/* 行情数据到达通知 */
    EpsMktStatusChangedCallback mktStatusChangedNotify;/* 市场状态变化通知 */
    EpsEventOccurredCallback    eventOccurredNotify;  /* 事件发生通知 */
} EpsClientSpiT;

#ifdef __cplusplus
}
#endif

#endif /* EPS_DATA_H */
