/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    mktDatabase.h
 *
 * 行情数据库定义头文件
 *
 * @version $Id
 * @since   2014/03/04
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
04-MAR-2014 ZHENGWU         创建
================================================================================
</pre>
*/

#ifndef EPS_MKTDATABASE_H
#define EPS_MKTDATABASE_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * 包含头文件
 */

#include "common.h"
#include "epsTypes.h"
#include "errlib.h"
#include "epsData.h"
#include "stepMessage.h"


/**
 * 类型定义
 */
 
/* 
 * 行情数据库定义结构体 
 */
typedef struct EpsMktDatabaseTag
{
    BOOL            isSubscribed[EPS_MKTTYPE_NUM + 1];  
    uint32          applID;
    uint64          applSeqNum[EPS_MKTTYPE_NUM + 1];
} EpsMktDatabaseT;


/**
 * 接口函数定义
 */

/*
 * 初始化行情数据库
 */
ResCodeT InitMktDatabase(EpsMktDatabaseT* pDatabase);

/*
 * 反初始化行情数据库
 */
ResCodeT UninitMktDatabase(EpsMktDatabaseT* pDatabase);

/*
 * 订阅行情数据
 */
ResCodeT SubscribeMktData(EpsMktDatabaseT* pDatabase, EpsMktTypeT mktType);

/**
 * 取消所有行情数据订阅
 */
ResCodeT UnsubscribeAllMktData(EpsMktDatabaseT* pDatabase);

/*
 * 判断是否接受该条行情数据
 */
ResCodeT AcceptMktData(EpsMktDatabaseT* pDatabase, const StepMessageT* pMsg);

/*
 * 转换行情数据格式
 */
ResCodeT ConvertMktData(const StepMessageT* pMsg, EpsMktDataT* pData);


#ifdef __cplusplus
}
#endif

#endif /* EPS_MKTDATABASE_H */
