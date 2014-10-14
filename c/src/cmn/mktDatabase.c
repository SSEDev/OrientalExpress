/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    mktDatabase.c
 *
 * 行情数据库实现文件
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

/**
 * 包含头文件
 */

#include "common.h"

#include "mktDatabase.h"


/**
 * 接口函数实现
 */

/**
 * 初始化行情数据库
 *
 * @param   pDatabase               in  - 行情数据库
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT InitMktDatabase(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
        memset(pDatabase, 0x00, sizeof(EpsMktDatabaseT));
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
 * 反初始化行情数据库
 *
 * @param   pDatabase               in  - 行情数据库
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UninitMktDatabase(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
        memset(pDatabase, 0x00, sizeof(EpsMktDatabaseT));
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
 * 订阅行情数据
 *
 * @param   pDatabase               in  - 行情数据库
 * @param   mktType                 in  - 待订阅市场类型
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT SubscribeMktData(EpsMktDatabaseT* pDatabase, EpsMktTypeT mktType)
{
    TRY
    {
        if (mktType > EPS_MKTTYPE_NUM)
        {
            THROW_ERROR(ERCD_EPS_INVALID_MKTTYPE);
        }

        if (mktType == EPS_MKTTYPE_ALL)
        {
            BOOL hasSubscribeAll = TRUE;
            
            int32 mktType1;
            for (mktType1 = EPS_MKTTYPE_ALL + 1; mktType1 <= EPS_MKTTYPE_NUM; mktType1++)
            {
                if (! pDatabase->isSubscribed[mktType1])
                {
                    pDatabase->isSubscribed[mktType1] = TRUE;
                    hasSubscribeAll = FALSE;
                }
            }

            if (hasSubscribeAll)
            {
                THROW_ERROR(ERCD_EPS_MKTTYPE_DUPSUBSCRIBED);
            }
        }
        else
        {
            if (pDatabase->isSubscribed[mktType])
            {
                THROW_ERROR(ERCD_EPS_MKTTYPE_DUPSUBSCRIBED);
            }
            else
            {
                pDatabase->isSubscribed[mktType] = TRUE;
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
 * 取消所有行情数据订阅
 *
 * @param   pDatabase               in  - 行情数据库
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UnsubscribeAllMktData(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
        memset(pDatabase->isSubscribed, 0x00, sizeof(pDatabase->isSubscribed));
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
 * 判断是否接受该条行情数据
 *
 * @param   pDatabase           in  - 行情数据库
 * @param   pMsg                in  - STEP行情消息
 *
 * @return  接受返回TRUE，否则返回FALSE
 */
ResCodeT AcceptMktData(EpsMktDatabaseT* pDatabase, const StepMessageT* pMsg)
{
    TRY
    {
        MDSnapshotFullRefreshRecordT* pRecord = (MDSnapshotFullRefreshRecordT*)pMsg->body;

        EpsMktTypeT mktType = (EpsMktTypeT)(atoi(pRecord->securityType));
        if (mktType == EPS_MKTTYPE_ALL || mktType > EPS_MKTTYPE_NUM)
        {
            THROW_ERROR(ERCD_EPS_INVALID_MKTTYPE);
        }

        if (pDatabase->isSubscribed[mktType])
        {
            if (pRecord->applID == pDatabase->applID)
            {
                if (pRecord->applSeqNum > pDatabase->applSeqNum[mktType])
                {
                    pDatabase->applSeqNum[mktType] = pRecord->applSeqNum;
                    THROW_RESCODE(NO_ERR);
                }
                else
                {
                    THROW_RESCODE(ERCD_EPS_MKTDATA_BACKFLOW);
                }
            }
            else
            {
                uint32 applID = pDatabase->applID;
                
                pDatabase->applID = pRecord->applID;
                pDatabase->applSeqNum[mktType] = pRecord->applSeqNum;

                if (applID != 0)
                {
                    THROW_ERROR(ERCD_EPS_DATASOURCE_CHANGED);
                }
            }
        }
        else
        {
            THROW_RESCODE(ERCD_EPS_MKTTYPE_UNSUBSCRIBED);
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
 * 判断是否接受该条市场状态
 *
 * @param   pDatabase           in  - 行情数据库
 * @param   pMsg                in  - STEP市场状态消息
 *
 * @return  接受返回TRUE，否则返回FALSE
 */
ResCodeT AcceptMktStatus(EpsMktDatabaseT* pDatabase, const StepMessageT* pMsg)
{
    TRY
    {
        TradingStatusRecordT* pRecord = (TradingStatusRecordT*)pMsg->body;

        EpsMktTypeT mktType = (EpsMktTypeT)(atoi(pRecord->securityType));
        if (mktType == EPS_MKTTYPE_ALL || mktType > EPS_MKTTYPE_NUM)
        {
            THROW_ERROR(ERCD_EPS_INVALID_MKTTYPE);
        }

        if (! pDatabase->isSubscribed[mktType])
        {
            THROW_RESCODE(ERCD_EPS_MKTTYPE_UNSUBSCRIBED);
        }

        if (memcmp(pDatabase->mktStatus[mktType], pRecord->tradingSessionID, EPS_MKTSTATUS_LEN) == 0)
        {
            THROW_RESCODE(ERCD_EPS_MKTSTATUS_UNCHANGED);
        }
        else
        {
            memcpy(pDatabase->mktStatus[mktType], pRecord->tradingSessionID, EPS_MKTSTATUS_LEN);
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
 * 将行情消息从STEP格式转换成行情数据结构
 *
 * @param   pMsg                in  - STEP格式行情
 * @param   pData               out - 行情数据格式
 *
 * @return  转换成功返回TRUE，否则返回FALSE
 */
ResCodeT ConvertMktData(const StepMessageT* pMsg, EpsMktDataT* pData)
{
    TRY
    {
        MDSnapshotFullRefreshRecordT* pRecord = (MDSnapshotFullRefreshRecordT*)pMsg->body;

        memset(pData, 0x00, sizeof(EpsMktDataT));

        memcpy(pData->mktTime, pMsg->sendingTime, EPS_TIME_LEN);
        pData->mktType = (EpsMktTypeT)(atoi(pRecord->securityType));
        pData->tradSesMode = (EpsTrdSesModeT)pRecord->tradSesMode;
        pData->applID = pRecord->applID;
        pData->applSeqNum = pRecord->applSeqNum;
        memcpy(pData->tradeDate, pRecord->tradeDate, EPS_DATE_LEN);
        memcpy(pData->mdUpdateType, pRecord->mdUpdateType, EPS_UPDATETYPE_LEN);
        pData->mdCount = pRecord->mdCount;
        pData->mdDataLen = pRecord->mdDataLen;
        memcpy(pData->mdData, pRecord->mdData, EPS_MKTDATA_MAX_LEN);
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
 * 将行情消息从STEP格式转换成市场状态结构
 *
 * @param   pMsg                in  - STEP格式行情
 * @param   pStatus             out - 市场状态格式
 *
 * @return  转换成功返回TRUE，否则返回FALSE
 */
ResCodeT ConvertMktStatus(const StepMessageT* pMsg, EpsMktStatusT* pStatus)
{
    TRY
    {
        TradingStatusRecordT* pRecord = (TradingStatusRecordT*)pMsg->body;

        memset(pStatus, 0x00, sizeof(EpsMktStatusT));

        pStatus->mktType = (EpsMktTypeT)(atoi(pRecord->securityType));
        pStatus->tradSesMode = (EpsTrdSesModeT)pRecord->tradSesMode;
        memcpy(pStatus->mktStatus, pRecord->tradingSessionID, EPS_MKTSTATUS_LEN);
        pStatus->totNoRelatedSym = pRecord->totNoRelatedSym;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

